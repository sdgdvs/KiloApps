#!/usr/bin/env python3
# /// script
# dependencies = [
#     "websockets>=12.0",
#     "pillow>=10.0",
# ]
# ///
"""
scripts/visual_probe.py
Headless Visual & Gameplay Probe for KiloApps web apps / kweb nodes.

Gives agents *visual* feedback in one tool call + at most one `view_file`:
  1. Renders the app at its REAL KiloOS window size (parsed from KiloOS/src/App.jsx).
  2. Fresh isolated browser profile per probe (first-run tutorials appear like a new user).
  3. Boot frames t0/t0+0.5s  -> animation liveness, blank-screen detection.
  4. Dismisses startup overlay + clicks a Start/Play button with TRUSTED CDP input.
  5. Trusted playtest: canvas click, Enter/Space, held arrows, WASD (games) or safe
     button clicks (DOM apps) -> % pixels changed + changed-region bbox.
  6. Captures uncaught exceptions, console.error, missing local assets (file:// 404s),
     network requests fired on boot (multiplayer-autostart smell), window.open calls.
  7. Layout: page overflow, controls cut off at window size, stretched/blurry canvases,
     empty (uniform) canvases, tiny text.
  8. --compare-head: also probes the git HEAD version -> before/after rows + HEAD delta.
  9. Writes docs/gallery/review/<app>/sheet.png (single contact sheet) + report.json.

Usage:
  uv run scripts/visual_probe.py --app kmine
  uv run scripts/visual_probe.py --app kmine --compare-head
  uv run scripts/visual_probe.py --file KiloOS/public/web/portal.html
  uv run scripts/visual_probe.py --all [--limit N]      # fleet sweep -> review/_fleet.json
  Add --strict to turn warnings into failures.

Exit: 0 = no FAIL findings, 1 = FAIL findings (JS exceptions, missing assets, blank viewport).
"""

from __future__ import annotations

import argparse
import asyncio
import io
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path
from typing import Any, Optional

ROOT = Path(__file__).resolve().parent.parent
APPS_DIR = ROOT / "KiloOS" / "public" / "apps"
WEB_DIR = ROOT / "KiloOS" / "public" / "web"
APP_JSX = ROOT / "KiloOS" / "src" / "App.jsx"
REVIEW_DIR = ROOT / "docs" / "gallery" / "review"
HEAD_PREFIX = ".__probe_head_"
TITLEBAR_PX = 37  # .xp-titlebar 36px + 1px border (KiloOS/src/index.css)
DEFAULT_VIEWPORT = (1024, 720)

BROWSER_PATHS = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]

# key -> (code, windowsVirtualKeyCode, text)
KEYS = {
    "Enter": ("Enter", 13, "\r"),
    " ": ("Space", 32, " "),
    "Escape": ("Escape", 27, None),
    "ArrowLeft": ("ArrowLeft", 37, None),
    "ArrowUp": ("ArrowUp", 38, None),
    "ArrowRight": ("ArrowRight", 39, None),
    "ArrowDown": ("ArrowDown", 40, None),
    "w": ("KeyW", 87, "w"),
    "a": ("KeyA", 65, "a"),
    "s": ("KeyS", 83, "s"),
    "d": ("KeyD", 68, "d"),
}

# Injected before any app script: deterministic Math.random (cleaner HEAD-vs-WT diffs)
# and window.open capture (no stray popups).
PRELUDE_JS = r"""
(() => {
  let s = 0x9E3779B9;
  Math.random = function () {
    s |= 0; s = (s + 0x6D2B79F5) | 0;
    let t = Math.imul(s ^ (s >>> 15), 1 | s);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
  window.__probeOpened = [];
  window.open = function (u) { window.__probeOpened.push(String(u)); return null; };
})();
"""

DOM_PROBE_JS = r"""
(() => {
  const vw = innerWidth, vh = innerHeight;
  const vis = el => {
    if (!el) return false;
    if (el.checkVisibility && !el.checkVisibility({ checkOpacity: true, checkVisibilityCSS: true })) return false;
    const s = getComputedStyle(el);
    if (s.display === 'none' || s.visibility === 'hidden' || parseFloat(s.opacity) < 0.05) return false;
    const r = el.getBoundingClientRect();
    return r.width > 0 && r.height > 0;
  };
  const inter = r => Math.max(0, Math.min(r.right, vw) - Math.max(r.left, 0)) *
                     Math.max(0, Math.min(r.bottom, vh) - Math.max(r.top, 0));
  const canvases = [...document.querySelectorAll('canvas')].filter(vis).map(c => {
    const r = c.getBoundingClientRect();
    return { id: c.id || null, x: Math.round(r.left), y: Math.round(r.top),
             w: Math.round(r.width), h: Math.round(r.height), bw: c.width, bh: c.height,
             pixelated: /pixelated|crisp-edges|optimizespeed/i.test(getComputedStyle(c).imageRendering),
             area: inter(r) / (vw * vh) };
  }).sort((a, b) => b.area - a.area);

  const de = document.documentElement, b = document.body || de;
  const sw = Math.max(de.scrollWidth, b.scrollWidth), sh = Math.max(de.scrollHeight, b.scrollHeight);
  const ovx = getComputedStyle(b).overflowX + getComputedStyle(de).overflowX;
  const ovy = getComputedStyle(b).overflowY + getComputedStyle(de).overflowY;
  const scrollX = !/hidden|clip/.test(ovx), scrollY = !/hidden|clip/.test(ovy);
  const scrollX_ = window.pageXOffset || de.scrollLeft || b.scrollLeft || 0, scrollY_ = window.pageYOffset || de.scrollTop || b.scrollTop || 0;

  const sel = '[class*="modal"],[id*="modal"],[class*="overlay"],[id*="overlay"],[class*="dialog"],[role="dialog"],' +
              '[class*="splash"],[id*="splash"],[class*="tutorial"],[id*="tutorial"],[class*="welcome"],[id*="welcome"],' +
              '[class*="intro"],[id*="intro"],[class*="title-screen"],[id*="title-screen"],[class*="backdrop"]';
  let overlay = null;
  for (const el of document.querySelectorAll(sel)) {
    if (!vis(el)) continue;
    const s = getComputedStyle(el);
    if (s.pointerEvents === 'none') continue;
    if (canvases.length && el.querySelector('canvas')) continue;
    const r = el.getBoundingClientRect();
    const cov = inter(r) / (vw * vh);
    if (cov <= 0.35) continue;
    const cx = Math.max(4, Math.min(vw - 4, r.left + r.width / 2));
    const cy = Math.max(4, Math.min(vh - 4, r.top + r.height / 2));
    const hit = document.elementFromPoint(cx, cy);
    if (!hit || !(el === hit || el.contains(hit))) continue;
    overlay = { id: el.id || null, cls: String(el.className || '').slice(0, 50), cov: Math.round(cov * 100) };
    break;
  }

  const canAncScroll = (el, axis) => {
    for (let p = el.parentElement; p && p !== de; p = p.parentElement) {
      const ps = getComputedStyle(p);
      if (axis === 'x' && /auto|scroll/.test(ps.overflowX) && p.scrollWidth > p.clientWidth + 2) return true;
      if (axis === 'y' && /auto|scroll/.test(ps.overflowY) && p.scrollHeight > p.clientHeight + 2) return true;
    }
    return axis === 'x' ? (scrollX && sw > vw + 2) : (scrollY && sh > vh + 2);
  };

  // Interactive controls partially clipped by the window edge when no ancestor can scroll there.
  const cut = [];
  for (const el of document.querySelectorAll('button, input, select, textarea, [role="button"], [onclick]')) {
    if (!vis(el)) continue;
    const r = el.getBoundingClientRect();
    if (inter(r) <= 0) continue; // completely offscreen drawer/panel
    if (el.closest('[class*="modal"],[class*="overlay"],[role="dialog"]') && overlay) continue;
    const pos = getComputedStyle(el).position;
    const sx = pos === 'fixed' ? false : canAncScroll(el, 'x');
    const sy = pos === 'fixed' ? false : canAncScroll(el, 'y');
    const clipR = r.right - vw, clipB = r.bottom - vh, clipL = -(r.left + scrollX_), clipT = -(r.top + scrollY_);
    const sides = (clipR > 8 && clipR > r.width * 0.25 && !sx ? 'R' : '') +
                  (clipB > 8 && clipB > r.height * 0.25 && !sy ? 'B' : '') +
                  (clipL > 8 && clipL > r.width * 0.25 ? 'L' : '') +
                  (clipT > 8 && clipT > r.height * 0.25 ? 'T' : '');
    if (sides) {
      const lbl = ((el.innerText || el.value || el.id || el.tagName) + '').replace(/\s+/g, ' ').trim().slice(0, 22);
      cut.push(`${lbl}[${sides}@${Math.round(r.left)},${Math.round(r.top)}]`);
    }
  }

  let tiny = 0;
  for (const el of document.querySelectorAll('body *')) {
    if (el.children.length || !(el.textContent || '').trim() || !vis(el)) continue;
    if (parseFloat(getComputedStyle(el).fontSize) < 9) tiny++;
  }
  return { vw, vh, canvases, overflowX: scrollX ? Math.max(0, sw - vw) : 0, overflowY: scrollY ? Math.max(0, sh - vh) : 0,
           overlay, cutOff: cut.slice(0, 8), cutOffCount: cut.length, tinyText: tiny,
           buttons: document.querySelectorAll('button, [role="button"]').length,
           opened: (window.__probeOpened || []).slice(0, 3), title: document.title, scrollPos: Math.round(scrollX_ + scrollY_) };
})()
"""

# Returns center of the best clickable (topmost, in-viewport) button matching `re`.
FIND_BTN_JS = r"""
((pattern, scopeOverlay, avoid) => {
  const re = new RegExp(pattern, 'i');
  const bad = avoid ? new RegExp(avoid, 'i') : null;
  const vw = innerWidth, vh = innerHeight;
  let scope = document;
  if (scopeOverlay) {
    const ov = [...document.querySelectorAll('[class*="modal"],[id*="modal"],[class*="overlay"],[id*="overlay"],[role="dialog"],[class*="splash"],[id*="splash"],[class*="tutorial"],[id*="tutorial"],[class*="welcome"],[id*="welcome"],[class*="intro"],[id*="intro"],[class*="backdrop"]')]
      .filter(e => {
        if (e.checkVisibility && !e.checkVisibility({ checkOpacity: true, checkVisibilityCSS: true })) return false;
        const r = e.getBoundingClientRect(); const s = getComputedStyle(e);
        if (s.display === 'none' || s.visibility === 'hidden' || parseFloat(s.opacity) < 0.05 || s.pointerEvents === 'none') return false;
        if (r.width * r.height <= vw * vh * 0.2) return false;
        const cx = Math.max(4, Math.min(vw - 4, r.left + r.width / 2)), cy = Math.max(4, Math.min(vh - 4, r.top + r.height / 2));
        const hit = document.elementFromPoint(cx, cy);
        return hit && (e === hit || e.contains(hit));
      });
    if (ov.length) scope = ov[0];
  }
  const cands = [...scope.querySelectorAll('button, [role="button"], .btn, a.button, input[type="button"], input[type="submit"]')];
  const out = [];
  for (const el of cands) {
    if (el.disabled) continue;
    const r = el.getBoundingClientRect();
    if (r.width < 4 || r.height < 4) continue;
    const cx = r.left + r.width / 2, cy = r.top + r.height / 2;
    if (cx < 0 || cy < 0 || cx > vw || cy > vh) continue;
    const top = document.elementFromPoint(cx, cy);
    if (!top || !(top === el || el.contains(top))) continue;
    const label = ((el.innerText || el.value || el.getAttribute('aria-label') || el.title || '') + '').trim().replace(/\s+/g, ' ');
    if (bad && bad.test(label + ' ' + (el.id || ''))) continue;
    if (re.test(label) || re.test(el.id || '')) {
      const pri = /primary|start|play|new|begin|commence/i.test((el.className || '') + ' ' + (el.id || '')) ? 0 : 1;
      out.push({ x: Math.round(cx), y: Math.round(cy), text: label.slice(0, 30), pri });
    }
  }
  out.sort((a, b) => a.pri - b.pri);
  return out;
})
"""

START_RE = (r"^[^a-z0-9]*(start|play|begin|commence|initialize|launch|continue|resume|enter|dive|deploy|proceed|"
            r"acknowledge|accept|got it|get started|ok|okay|close|dismiss|skip|let'?s go|new game|start game|play now|"
            r"insert coin)\b|^(×|x|✕|✖)$")
GAME_START_RE = r"^[^a-z0-9]*(start|play|new game|start game|play now|begin|commence|initialize|deal|launch|insert coin|campaign|arcade|dive|deploy)\b"
UNSAFE_RE = (r"reset|clear|delete|quit|exit|close|reload|restart|remove|erase|logout|disconnect|connect|online|"
             r"multiplayer|matchmak|host|join|save|load|export|import|download|upload|print|format|wipe|shutdown|"
             r"help|manual|guide|tutorial|codex|rules|about|settings|config|options|^\?$")


# ----------------------------------------------------------------------------- utils

def find_browser() -> Optional[str]:
    for p in BROWSER_PATHS:
        if os.path.exists(p):
            return p
    return shutil.which("chrome") or shutil.which("msedge")


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def start_static_server() -> tuple[Any, int]:
    """Starts a quiet background HTTP server rooted at KiloOS/public/."""
    import functools
    import http.server
    import threading

    class QuietHandler(http.server.SimpleHTTPRequestHandler):
        def log_message(self, format: str, *args: Any) -> None:
            pass

    pub_dir = ROOT / "KiloOS" / "public"
    handler = functools.partial(QuietHandler, directory=str(pub_dir))
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
    port = srv.server_address[1]
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv, port


def target_url(html: Path, http_port: Optional[int]) -> str:
    pub_dir = (ROOT / "KiloOS" / "public").resolve()
    try:
        rel = html.resolve().relative_to(pub_dir).as_posix()
        if http_port:
            return f"http://127.0.0.1:{http_port}/{rel}"
    except ValueError:
        pass
    return html.as_uri()


def load_viewports() -> dict[str, tuple[int, int]]:
    """Maps html basename -> (w, h) from the KiloOS APPS registry (window size minus titlebar)."""
    out: dict[str, tuple[int, int]] = {}
    try:
        for line in APP_JSX.read_text(encoding="utf-8", errors="ignore").splitlines():
            m = re.search(r"url:\s*'([^']+\.html)'.*?\bw:\s*(\d+),\s*h:\s*(\d+)", line)
            if m:
                w, h = int(m.group(2)), int(m.group(3))
                out[Path(m.group(1)).name.lower()] = (max(480, w), max(360, h - TITLEBAR_PX))
    except OSError:
        pass
    return out


def resolve_target(app: Optional[str], file: Optional[str]) -> Path:
    if file:
        p = Path(file)
        return p if p.is_absolute() else (ROOT / p)
    t = (app or "").strip()
    if t.startswith("kweb://"):
        try:
            sys.path.insert(0, str(Path(__file__).parent))
            from verify_turn import WEB_MAP  # type: ignore
            if t in WEB_MAP:
                return WEB_DIR / WEB_MAP[t]
        except Exception:
            pass
        slug = t.replace("kweb://", "").strip("/").replace("-", "_").replace(".", "_")
        return WEB_DIR / f"{slug}.html"
    name = t.lower().removesuffix(".html")
    for cand in (APPS_DIR / f"{name}.html", APPS_DIR / f"k{name}.html", WEB_DIR / f"{name}.html",
                 WEB_DIR / f"{name.removeprefix('k')}.html"):
        if cand.exists():
            return cand
    return APPS_DIR / f"{name if name.startswith('k') else 'k' + name}.html"


def cleanup_head_copies(max_age_s: float = 300) -> None:
    """Removes orphaned HEAD copies (crashed runs). Age-gated so concurrent probes don't clobber each other."""
    now = time.time()
    for d in (APPS_DIR, WEB_DIR):
        if d.exists():
            for f in d.glob(f"{HEAD_PREFIX}*"):
                try:
                    if now - f.stat().st_mtime > max_age_s:
                        f.unlink()
                except OSError:
                    pass


def git_head_copy(path: Path) -> Optional[Path]:
    """Writes the git HEAD version next to the original (so relative assets resolve). None if unchanged/new."""
    rel = path.relative_to(ROOT).as_posix()
    diff = subprocess.run(["git", "diff", "--quiet", "HEAD", "--", rel], cwd=ROOT)
    if diff.returncode == 0:
        return None
    show = subprocess.run(["git", "show", f"HEAD:{rel}"], cwd=ROOT, capture_output=True)
    if show.returncode != 0 or not show.stdout:
        return None
    dest = path.parent / f"{HEAD_PREFIX}{path.name}"
    dest.write_bytes(show.stdout)
    return dest


# ----------------------------------------------------------------------------- image metrics

def frame_stats(img) -> dict[str, Any]:
    from PIL import ImageStat
    small = img.resize((128, 96)).point(lambda v: v & 0xF0)
    colors = small.getcolors(128 * 96) or []
    top = max(colors, key=lambda c: c[0]) if colors else (0, (0, 0, 0))
    st = ImageStat.Stat(img.convert("L").resize((256, 192)))
    dom = top[0] / (128 * 96)
    std = st.stddev[0]
    return {"colors": len(colors), "dominant": round(dom, 3), "dominant_hex": "#%02x%02x%02x" % top[1][:3],
            "luma": round(st.mean[0], 1), "contrast": round(std, 1), "blank": dom >= 0.985 or std < 2.5}


def diff_stats(a, b) -> tuple[dict[str, Any], Any]:
    from PIL import ImageChops
    if a.size != b.size:
        b = b.resize(a.size)
    d = ImageChops.difference(a, b).convert("L")
    mask = d.point(lambda p: 255 if p > 28 else 0)
    changed = mask.histogram()[255] / (a.size[0] * a.size[1])
    return {"changed": round(changed * 100, 2), "bbox": mask.getbbox()}, mask


def heatmap(base, mask, bbox):
    from PIL import Image, ImageDraw, ImageFilter, ImageOps
    g = ImageOps.grayscale(base).convert("RGB")
    g = Image.blend(g, Image.new("RGB", g.size, (0, 0, 0)), 0.55)
    red = Image.new("RGB", g.size, (255, 40, 40))
    out = Image.composite(red, g, mask.filter(ImageFilter.MaxFilter(5)))
    if bbox:
        ImageDraw.Draw(out).rectangle(bbox, outline=(255, 220, 0), width=3)
    return out


def region_stats(img, c: dict) -> Optional[dict]:
    x0, y0 = max(0, c["x"]), max(0, c["y"])
    x1, y1 = min(img.size[0], c["x"] + c["w"]), min(img.size[1], c["y"] + c["h"])
    if x1 - x0 < 8 or y1 - y0 < 8:
        return None
    return frame_stats(img.crop((x0, y0, x1, y1)))


def make_sheet(title: str, rows: list[list[tuple[str, Any]]], verdict: str, dest: Path) -> None:
    from PIL import Image, ImageDraw, ImageFont
    pw, label_h, head_h, pad = 360, 18, 26, 6
    try:
        font = ImageFont.load_default(size=13)
    except TypeError:
        font = ImageFont.load_default()
    scaled = [[(lbl, im.resize((pw, max(1, round(im.size[1] * pw / im.size[0]))))) for lbl, im in row] for row in rows]
    cols = max(len(r) for r in scaled)
    row_hs = [max(im.size[1] for _, im in r) + label_h for r in scaled]
    W = cols * pw + (cols + 1) * pad
    H = head_h + sum(row_hs) + (len(rows) + 1) * pad
    sheet = Image.new("RGB", (W, H), (18, 20, 26))
    dr = ImageDraw.Draw(sheet)
    color = (90, 220, 120) if verdict.startswith("PASS") else ((240, 190, 60) if verdict.startswith("WARN") else (250, 80, 80))
    dr.text((pad, 6), f"{title}   [{verdict}]", fill=color, font=font)
    y = head_h + pad
    for r, rh in zip(scaled, row_hs):
        x = pad
        for lbl, im in r:
            dr.text((x, y + 2), lbl, fill=(200, 205, 215), font=font)
            sheet.paste(im, (x, y + label_h))
            x += pw + pad
        y += rh + pad
    dest.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(dest, optimize=True)


# ----------------------------------------------------------------------------- CDP

class CDP:
    def __init__(self, ws):
        self.ws = ws
        self.mid = 0
        self.pending: dict[int, asyncio.Future] = {}
        self.handlers: list = []

    async def send(self, method: str, params: Optional[dict] = None, session: Optional[str] = None, timeout: float = 15.0):
        self.mid += 1
        i = self.mid
        fut = asyncio.get_running_loop().create_future()
        self.pending[i] = fut
        msg: dict[str, Any] = {"id": i, "method": method, "params": params or {}}
        if session:
            msg["sessionId"] = session
        await self.ws.send(json.dumps(msg))
        return await asyncio.wait_for(fut, timeout)

    async def listen(self):
        try:
            async for raw in self.ws:
                m = json.loads(raw)
                if "id" in m and m["id"] in self.pending:
                    fut = self.pending.pop(m["id"])
                    if not fut.done():
                        if "error" in m:
                            fut.set_exception(RuntimeError(m["error"].get("message", "CDP error")))
                        else:
                            fut.set_result(m.get("result", {}))
                elif "method" in m:
                    for h in list(self.handlers):
                        h(m)
        except Exception:
            pass


class Page:
    """One isolated browser context + tab with trusted-input helpers."""

    def __init__(self, cdp: CDP):
        self.cdp = cdp
        self.sid: Optional[str] = None
        self.ctx = self.tid = None
        self.loaded = asyncio.Event()
        self.errors: list[str] = []
        self.console_errors: list[str] = []
        self.missing: list[str] = []
        self.boot_net: list[str] = []
        self.navigations: list[str] = []
        self.req_urls: dict[str, str] = {}
        self.booting = True

    def _on(self, m: dict):
        if m.get("sessionId") != self.sid:
            return
        meth, p = m["method"], m.get("params", {})
        if meth == "Page.loadEventFired":
            self.loaded.set()
        elif meth == "Runtime.exceptionThrown":
            d = p.get("exceptionDetails", {})
            txt = (d.get("exception", {}) or {}).get("description") or d.get("text") or "exception"
            line = d.get("lineNumber")
            self.errors.append(f"{txt.splitlines()[0][:160]}" + (f" (line {line + 1})" if line is not None else ""))
        elif meth == "Runtime.consoleAPICalled" and p.get("type") == "error":
            args = p.get("args", [])
            txt = " ".join(str(a.get("value", a.get("description", ""))) for a in args)[:160]
            self.console_errors.append(txt)
        elif meth == "Network.requestWillBeSent":
            url = p.get("request", {}).get("url", "")
            self.req_urls[p.get("requestId", "")] = url
            if (self.booting and p.get("type") in ("XHR", "Fetch", "EventSource", "WebSocket")
                    and re.match(r"^(https?|wss?):", url) and "127.0.0.1" not in url and "localhost" not in url):
                self.boot_net.append(url[:120])
        elif meth == "Network.webSocketCreated":
            if self.booting:
                self.boot_net.append(p.get("url", "")[:120])
        elif meth == "Network.responseReceived":
            url = p.get("response", {}).get("url", "")
            status = p.get("response", {}).get("status", 200)
            if url.startswith("http://127.0.0.1:") and status >= 400 and not url.endswith("/favicon.ico"):
                path_part = "/" + url.split("/", 3)[-1]
                self.missing.append(f"{path_part} (HTTP {status})"[:120])
        elif meth == "Network.loadingFailed":
            url = self.req_urls.get(p.get("requestId", ""), "")
            if (url.startswith("file:") or url.startswith("http://127.0.0.1:")) and not p.get("canceled") and not url.endswith("/favicon.ico"):
                self.missing.append(url.split("/public/")[-1][:120])
        elif meth == "Page.javascriptDialogOpening":
            asyncio.ensure_future(self.send("Page.handleJavaScriptDialog", {"accept": True}))
        elif meth == "Page.frameNavigated":
            fr = p.get("frame", {})
            if not fr.get("parentId"):
                self.navigations.append(fr.get("url", ""))

    async def send(self, method, params=None, timeout=15.0):
        return await self.cdp.send(method, params, self.sid, timeout)

    async def open(self, w: int, h: int):
        self.ctx = (await self.cdp.send("Target.createBrowserContext", {"disposeOnDetach": True}))["browserContextId"]
        self.tid = (await self.cdp.send("Target.createTarget", {"url": "about:blank", "browserContextId": self.ctx}))["targetId"]
        self.sid = (await self.cdp.send("Target.attachToTarget", {"targetId": self.tid, "flatten": True}))["sessionId"]
        self.cdp.handlers.append(self._on)
        for m in ("Page.enable", "Runtime.enable", "Network.enable"):
            await self.send(m)
        await self.send("Emulation.setDeviceMetricsOverride", {"width": w, "height": h, "deviceScaleFactor": 1, "mobile": False})
        await self.send("Emulation.setFocusEmulationEnabled", {"enabled": True})
        await self.send("Page.addScriptToEvaluateOnNewDocument", {"source": PRELUDE_JS})

    async def close(self):
        if self._on in self.cdp.handlers:
            self.cdp.handlers.remove(self._on)
        for meth, prm in (("Target.closeTarget", {"targetId": self.tid}), ("Target.disposeBrowserContext", {"browserContextId": self.ctx})):
            try:
                await self.cdp.send(meth, prm, timeout=5)
            except Exception:
                pass

    async def goto(self, url: str, timeout: float = 5.0):
        self.loaded.clear()
        self.navigations.clear()
        await self.send("Page.navigate", {"url": url})
        try:
            await asyncio.wait_for(self.loaded.wait(), timeout)
        except asyncio.TimeoutError:
            pass

    async def eval(self, expr: str, await_promise: bool = False) -> Any:
        r = await self.send("Runtime.evaluate", {"expression": expr, "returnByValue": True, "awaitPromise": await_promise})
        return (r.get("result") or {}).get("value")

    async def shot(self):
        from PIL import Image
        r = await self.send("Page.captureScreenshot", {"format": "png"})
        import base64
        return Image.open(io.BytesIO(base64.b64decode(r["data"]))).convert("RGB")

    async def click(self, x: int, y: int):
        base = {"x": x, "y": y, "button": "left", "clickCount": 1}
        await self.send("Input.dispatchMouseEvent", {"type": "mouseMoved", "x": x, "y": y})
        await self.send("Input.dispatchMouseEvent", {"type": "mousePressed", **base})
        await asyncio.sleep(0.04)
        await self.send("Input.dispatchMouseEvent", {"type": "mouseReleased", **base})

    async def key(self, key: str, hold: float = 0.06):
        code, vk, text = KEYS[key]
        down: dict[str, Any] = {"type": "keyDown" if text else "rawKeyDown", "key": key, "code": code,
                                "windowsVirtualKeyCode": vk, "nativeVirtualKeyCode": vk}
        if text:
            down.update(text=text, unmodifiedText=text)
        await self.send("Input.dispatchKeyEvent", down)
        await asyncio.sleep(hold)
        await self.send("Input.dispatchKeyEvent", {"type": "keyUp", "key": key, "code": code,
                                                   "windowsVirtualKeyCode": vk, "nativeVirtualKeyCode": vk})


# ----------------------------------------------------------------------------- probe

async def run_probe(cdp: CDP, html: Path, viewport: tuple[int, int], http_port: Optional[int] = None) -> dict[str, Any]:
    """Runs one full boot -> dismiss -> playtest pass. Returns metrics + PIL frames."""
    pg = Page(cdp)
    res: dict[str, Any] = {"actions": [], "frames": {}}
    url = target_url(html, http_port)
    await pg.open(*viewport)
    try:
        await pg.goto(url)
        await asyncio.sleep(0.7)
        boot_a = await pg.shot()
        await asyncio.sleep(0.5)
        boot_b = await pg.shot()
        pg.booting = False
        dom = await pg.eval(DOM_PROBE_JS) or {}
        res["boot_errors"] = list(pg.errors)
        res["anim"], _ = diff_stats(boot_a, boot_b)

        # Dismiss startup overlay(s) with trusted input (handles 2-stage splash -> tutorial modals).
        if dom.get("overlay"):
            res["overlay"] = dom["overlay"]
            again = dom
            for _ in range(2):
                if not again.get("overlay"):
                    break
                btns = await pg.eval(f"{FIND_BTN_JS}({json.dumps(START_RE)}, true, null)") or []
                if btns:
                    await pg.click(btns[0]["x"], btns[0]["y"])
                    res["actions"].append(f'click "{btns[0]["text"]}"')
                else:
                    await pg.key("Escape")
                    res["actions"].append("Escape")
                await asyncio.sleep(0.45)
                again = await pg.eval(DOM_PROBE_JS) or {}
            if again.get("overlay"):
                any_btn = await pg.eval(f"{FIND_BTN_JS}('.', true, {json.dumps(UNSAFE_RE)})") or []
                if any_btn:
                    await pg.click(any_btn[0]["x"], any_btn[0]["y"])
                    res["actions"].append(f'click "{any_btn[0]["text"]}"')
                    await asyncio.sleep(0.45)
                    again = await pg.eval(DOM_PROBE_JS) or {}
            res["overlay_stuck"] = bool(again.get("overlay"))

        # Click an obvious Start/Play button like a human would.
        start = await pg.eval(f"{FIND_BTN_JS}({json.dumps(GAME_START_RE)}, false, {json.dumps(UNSAFE_RE)})") or []
        if start:
            await pg.click(start[0]["x"], start[0]["y"])
            res["actions"].append(f'click "{start[0]["text"]}"')
            await asyncio.sleep(0.5)

        dom = await pg.eval(DOM_PROBE_JS) or dom
        ready = await pg.shot()
        err_before_play = len(pg.errors)

        # Trusted playtest.
        canv = (dom.get("canvases") or [None])[0]
        is_game = bool(canv and canv["area"] >= 0.12)
        if is_game:
            cx = max(2, min(viewport[0] - 2, canv["x"] + canv["w"] // 2))
            cy = max(2, min(viewport[1] - 2, canv["y"] + canv["h"] // 2))
            await pg.click(cx, cy)
            for k, hold in (("Enter", 0.06), (" ", 0.06), ("ArrowRight", 0.45), ("ArrowUp", 0.3), ("ArrowLeft", 0.45),
                            ("ArrowDown", 0.3), ("d", 0.3), ("w", 0.2), ("a", 0.3), ("s", 0.2), (" ", 0.06)):
                await pg.key(k, hold)
            res["actions"].append("canvas click + Enter/Space + held arrows + WASD")
        else:
            safe = await pg.eval(f"{FIND_BTN_JS}('.', false, {json.dumps(UNSAFE_RE)})") or []
            for b in safe[:3]:
                await pg.click(b["x"], b["y"])
                res["actions"].append(f'click "{b["text"]}"')
                await asyncio.sleep(0.25)
            for k in ("ArrowDown", "ArrowRight", "Enter"):
                await pg.key(k)
            res["actions"].append("arrows + Enter")
        await asyncio.sleep(0.6)
        play = await pg.shot()
        dom_after = await pg.eval(DOM_PROBE_JS) or {}

        res["input"], mask = diff_stats(ready, play)
        res["is_game"] = is_game
        res["play_errors"] = pg.errors[err_before_play:]
        res["dom"] = dom
        res["opened"] = dom_after.get("opened", [])
        res["scroll_shift"] = abs(dom_after.get("scrollPos", 0) - dom.get("scrollPos", 0))
        base_url = url.split("#")[0].split("?")[0]
        res["navigated"] = [u for u in pg.navigations if u and not u.startswith(base_url)]
        res["console_errors"] = list(dict.fromkeys(pg.console_errors))[:5]
        res["missing"] = list(dict.fromkeys(pg.missing))[:8]
        res["boot_net"] = list(dict.fromkeys(pg.boot_net))[:5]
        res["stats"] = {"boot": frame_stats(boot_a), "ready": frame_stats(ready), "play": frame_stats(play)}
        res["canvas_stats"] = None
        if canv:
            res["canvas_stats"] = {"ready": region_stats(ready, canv), "play": region_stats(play, canv)}
        res["frames"] = {"boot": boot_a, "ready": ready, "play": play,
                         "input_heat": heatmap(play, mask, res["input"]["bbox"])}
    finally:
        await pg.close()
    return res


def assess(r: dict[str, Any]) -> tuple[list[str], list[str]]:
    fails, warns = [], []
    if r.get("boot_errors"):
        fails.append(f"JS exception on boot: {r['boot_errors'][0]}")
    if r.get("play_errors"):
        fails.append(f"JS exception during playtest ({len(r['play_errors'])}): {r['play_errors'][0]}")
    if r.get("missing"):
        fails.append(f"Missing local asset(s) ({len(r['missing'])}): {', '.join(r['missing'][:4])}")
    st = r.get("stats", {})
    if st.get("ready", {}).get("blank") and st.get("play", {}).get("blank"):
        fails.append(f"Viewport renders blank (uniform {st['ready']['dominant_hex']}) before and after input")
    if r.get("console_errors"):
        warns.append(f"console.error x{len(r['console_errors'])}: {r['console_errors'][0][:100]}")
    if r.get("overlay_stuck"):
        warns.append(f"Startup overlay {r['overlay'].get('id') or r['overlay'].get('cls')} still covers screen after dismiss attempt")
    if r.get("boot_net"):
        warns.append(f"Network request on boot before user action (autostart?): {r['boot_net'][0]}")
    if r.get("navigated"):
        warns.append(f"Page navigated away during playtest: {r['navigated'][0][:80]}")
    if r.get("is_game") and r.get("scroll_shift", 0) > 4:
        warns.append(f"Arrow/Space keys scrolled the page {r['scroll_shift']}px during gameplay (missing preventDefault)")
    if r.get("opened"):
        warns.append(f"window.open called during probe: {r['opened'][0][:80]}")
    dom = r.get("dom", {})
    if dom.get("overflowX", 0) > 8:
        warns.append(f"Horizontal scroll: content {dom['overflowX']}px wider than KiloOS window")
    if dom.get("cutOffCount"):
        warns.append(f"{dom['cutOffCount']} control(s) cut off at window edge: {', '.join(dom['cutOff'][:4])}")
    canv = (dom.get("canvases") or [None])[0]
    if canv and canv["bw"] and canv["bh"] and canv["w"] and canv["h"]:
        css_ar, bs_ar = canv["w"] / canv["h"], canv["bw"] / canv["bh"]
        if abs(css_ar - bs_ar) / bs_ar > 0.08:
            warns.append(f"Canvas stretched: backing {canv['bw']}x{canv['bh']} shown at {canv['w']}x{canv['h']} (aspect distortion)")
        elif canv["bw"] < canv["w"] * 0.7 and not canv["pixelated"]:
            warns.append(f"Canvas upscaled {canv['w'] / canv['bw']:.1f}x without image-rendering:pixelated (blurry)")
    cs = r.get("canvas_stats") or {}
    if canv and canv["area"] >= 0.12 and cs.get("ready") and cs.get("play") and cs["ready"]["dominant"] >= 0.99 and cs["play"]["dominant"] >= 0.99:
        warns.append(f"Main canvas appears empty (uniform {cs['ready']['dominant_hex']}) — nothing drawn")
    if r.get("is_game") and r["anim"]["changed"] < 0.1 and r["input"]["changed"] < 0.3:
        warns.append("Game canvas static at boot AND unresponsive to trusted keyboard/mouse input — check input handlers / render loop")
    if dom.get("tinyText", 0) > 10:
        warns.append(f"{dom['tinyText']} text nodes below 9px font (readability)")
    return fails, warns


def fmt_bbox(b) -> str:
    return f"({b[0]},{b[1]})-({b[2]},{b[3]})" if b else "none"


async def probe_app(cdp: CDP, html: Path, viewports: dict, compare_head: bool, strict: bool, quiet: bool, http_port: Optional[int] = None) -> dict[str, Any]:
    slug = html.stem
    vp = viewports.get(html.name.lower(), DEFAULT_VIEWPORT)
    out_dir = REVIEW_DIR / slug
    out_dir.mkdir(parents=True, exist_ok=True)
    t0 = time.time()

    r = await run_probe(cdp, html, vp, http_port)
    head = None
    head_delta = None
    if compare_head:
        hp = git_head_copy(html)
        if hp:
            try:
                head = await run_probe(cdp, hp, vp, http_port)
            finally:
                try:
                    hp.unlink()
                except OSError:
                    pass
            head_delta, hmask = diff_stats(head["frames"]["ready"], r["frames"]["ready"])
            head["frames"]["delta_heat"] = heatmap(r["frames"]["ready"], hmask, head_delta["bbox"])

    fails, warns = assess(r)
    if strict:
        fails, warns = fails + warns, []
    verdict = "FAIL" if fails else ("WARN" if warns else "PASS")

    for name, im in r["frames"].items():
        im.save(out_dir / f"{name}.png", optimize=True)
    rows = []
    if head:
        rows.append([("HEAD boot", head["frames"]["boot"]), ("HEAD ready", head["frames"]["ready"]),
                     ("HEAD after input", head["frames"]["play"]),
                     (f"HEAD->now ready diff {head_delta['changed']}%", head["frames"]["delta_heat"])])
    tag = "NOW " if head else ""
    rows.append([(f"{tag}boot (t+0.7s)", r["frames"]["boot"]), (f"{tag}ready (after dismiss/start)", r["frames"]["ready"]),
                 (f"{tag}after input", r["frames"]["play"]), (f"input response {r['input']['changed']}%", r["frames"]["input_heat"])])
    sheet = out_dir / "sheet.png"
    make_sheet(f"{slug}  {vp[0]}x{vp[1]}", rows, verdict, sheet)

    st = r["stats"]
    dom = r.get("dom", {})
    canv = (dom.get("canvases") or [None])[0]
    lines = [
        f"=== VISUAL PROBE: {slug} ({vp[0]}x{vp[1]} = KiloOS window) ===",
        f"[BOOT]   {len(r['boot_errors'])} JS exc | {len(r['missing'])} missing assets | {st['boot']['colors']} colors, "
        f"dominant {round(st['boot']['dominant'] * 100)}% {st['boot']['dominant_hex']}, luma {st['boot']['luma']} | "
        f"animating {r['anim']['changed']}% px/0.5s",
    ]
    if r.get("overlay"):
        lines.append(f"[START]  overlay '{r['overlay'].get('id') or r['overlay'].get('cls')}' ({r['overlay']['cov']}%) -> "
                     f"{'STUCK' if r.get('overlay_stuck') else 'dismissed'}")
    lines.append(f"[PLAY]   {'game' if r['is_game'] else 'ui'}: {'; '.join(r['actions'])[:150]} -> "
                 f"{r['input']['changed']}% px changed in {fmt_bbox(r['input']['bbox'])}")
    if canv:
        lines.append(f"[CANVAS] {('#' + canv['id']) if canv['id'] else 'canvas'} css {canv['w']}x{canv['h']} / backing "
                     f"{canv['bw']}x{canv['bh']}{' pixelated' if canv['pixelated'] else ''}, {round(canv['area'] * 100)}% of window")
    lines.append(f"[LAYOUT] overflow x{dom.get('overflowX', 0)}/y{dom.get('overflowY', 0)}px | "
                 f"{dom.get('cutOffCount', 0)} cut-off controls | {dom.get('tinyText', 0)} tiny-text nodes")
    if head_delta is not None:
        lines.append(f"[HEAD Δ] ready frame differs {head_delta['changed']}% vs git HEAD in {fmt_bbox(head_delta['bbox'])}"
                     + (" (no visible change!)" if head_delta["changed"] < 0.05 else ""))
    elif compare_head:
        lines.append("[HEAD Δ] file unchanged vs git HEAD (or new) — single-row sheet")
    lines += [f"❌ FAIL  {f}" for f in fails] + [f"⚠️ WARN  {w}" for w in warns]
    lines.append(f"👁 REVIEW SHEET: {sheet.as_posix()}  (view_file once: boot | ready | after-input | diff heatmap"
                 f"{'; top row = git HEAD' if head else ''})")
    summary = (f"{verdict} (anim {r['anim']['changed']}%, input {r['input']['changed']}%"
               + (f", HEAD Δ {head_delta['changed']}%" if head_delta is not None else "")
               + f", {len(fails)} fail, {len(warns)} warn, {time.time() - t0:.1f}s)")
    lines.append(f"VISUAL_PROBE_SUMMARY: {summary}")

    report = {k: v for k, v in r.items() if k != "frames"}
    report.update(app=slug, viewport=vp, verdict=verdict, fails=fails, warns=warns, summary=summary,
                  head_delta=head_delta, sheet=str(sheet), timestamp=time.strftime("%Y-%m-%dT%H:%M:%S"))
    (out_dir / "report.json").write_text(json.dumps(report, indent=1, default=str), encoding="utf-8")
    if not quiet:
        print("\n".join(lines))
    return report


async def main_async(args) -> int:
    import websockets

    browser = find_browser()
    if not browser:
        print("❌ No Chrome/Edge found.", file=sys.stderr)
        return 2
    cleanup_head_copies()
    viewports = load_viewports()

    if args.all:
        targets = sorted(APPS_DIR.glob("*.html")) + sorted(WEB_DIR.glob("*.html"))
        targets = [t for t in targets if not t.name.startswith(HEAD_PREFIX)]
        if args.limit:
            targets = targets[: args.limit]
    else:
        t = resolve_target(args.app, args.file)
        if not t.exists():
            print(f"❌ File not found: {t}", file=sys.stderr)
            return 2
        targets = [t]

    http_srv, http_port = start_static_server()
    port = free_port()
    profile = tempfile.mkdtemp(prefix="kilo_probe_")
    proc = subprocess.Popen([browser, "--headless=new", f"--remote-debugging-port={port}", f"--user-data-dir={profile}",
                             "--no-first-run", "--no-default-browser-check", "--mute-audio", "--disable-gpu",
                             "--hide-scrollbars", "--disable-background-timer-throttling",
                             "--disable-renderer-backgrounding", "--disable-backgrounding-occluded-windows",
                             "--allow-file-access-from-files", "about:blank"],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    rc = 0
    try:
        ws_url = None
        for _ in range(60):
            await asyncio.sleep(0.15)
            try:
                with urllib.request.urlopen(f"http://127.0.0.1:{port}/json/version", timeout=1) as resp:
                    ws_url = json.loads(resp.read().decode()).get("webSocketDebuggerUrl")
                if ws_url:
                    break
            except Exception:
                pass
        if not ws_url:
            print("❌ Could not connect to browser CDP.", file=sys.stderr)
            return 2
        async with websockets.connect(ws_url, max_size=64_000_000) as ws:
            cdp = CDP(ws)
            listener = asyncio.create_task(cdp.listen())
            fleet = []
            for t in targets:
                try:
                    rep = await probe_app(cdp, t, viewports, args.compare_head, args.strict, quiet=args.all, http_port=http_port)
                except Exception as e:  # keep sweeping
                    rep = {"app": t.stem, "verdict": "FAIL", "fails": [f"probe crashed: {e}"], "warns": [], "summary": f"FAIL (probe crashed: {e})"}
                if rep["verdict"] == "FAIL":
                    rc = 1
                if args.all:
                    print(f"{rep['verdict']:4} {rep['app']:<18} {rep['summary']}")
                    for f in rep["fails"][:2]:
                        print(f"       ❌ {f[:150]}")
                    for w in rep["warns"][:3]:
                        print(f"       ⚠️ {w[:150]}")
                fleet.append({k: rep.get(k) for k in ("app", "verdict", "summary", "fails", "warns")})
            listener.cancel()
            if args.all:
                (REVIEW_DIR / "_fleet.json").write_text(json.dumps(fleet, indent=1), encoding="utf-8")
                counts = {v: sum(1 for f in fleet if f["verdict"] == v) for v in ("PASS", "WARN", "FAIL")}
                print(f"\nFLEET: {counts}  -> docs/gallery/review/_fleet.json")
    finally:
        cleanup_head_copies()
        http_srv.shutdown()
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except Exception:
            proc.kill()
        shutil.rmtree(profile, ignore_errors=True)
    return rc


def main() -> int:
    for s in (sys.stdout, sys.stderr):
        if hasattr(s, "reconfigure"):
            s.reconfigure(encoding="utf-8", errors="replace")
    ap = argparse.ArgumentParser(description="KiloApps headless visual & gameplay probe")
    ap.add_argument("--app", help="App id (kmine), kweb:// URL, or web node name")
    ap.add_argument("--file", help="Explicit HTML path (relative to repo root or absolute)")
    ap.add_argument("--all", action="store_true", help="Sweep every app + web node")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--compare-head", action="store_true", help="Also probe git HEAD version for before/after review")
    ap.add_argument("--strict", action="store_true", help="Treat warnings as failures")
    args = ap.parse_args()
    if not (args.app or args.file or args.all):
        ap.error("one of --app / --file / --all is required")
    return asyncio.run(main_async(args))


if __name__ == "__main__":
    sys.exit(main())
