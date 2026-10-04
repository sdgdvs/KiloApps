#!/usr/bin/env python3
# /// script
# dependencies = [
#     "websockets>=12.0",
# ]
# ///
"""
scripts/test_app_startup.py
Automated human-perspective gameplay startup & UX audit for KiloApps web applications.

Checks performed from a real user's perspective:
1. Static CSS Balance: Verifies all <style> blocks have balanced curly braces ({ vs }).
2. Console Errors: Captures any unhandled JavaScript exceptions thrown on startup.
3. Startup Modal & Overlay Obtrusiveness:
   - Detects whether large splash screens / modals / manuals cover the viewport on load.
   - If an overlay is present, finds its close/dismiss/play button and simulates a click.
   - CRITICAL: Verifies whether the overlay ACTUALLY disappears after dismissal.
4. Gameplay Hit-Testing (Occlusion Check):
   - Locates primary canvas / game board / interactive workspace.
   - Evaluates document.elementFromPoint() at the center of the gameplay area.
   - Verifies mouse clicks reach the game surface rather than being intercepted by rogue/invisible overlays.
5. Canvas & UI Liveness:
   - Ensures game canvas / primary containers have non-zero dimensions and are properly rendered.
"""

import argparse
import asyncio
import json
import os
import re
import socket
import subprocess
import sys
import time
import urllib.request
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
APPS_DIR = WORKSPACE_ROOT / "KiloOS" / "public" / "apps"
MAX_FILE_SIZE_BYTES = 999 * 1024

BROWSER_PATHS = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]


def find_browser() -> Optional[str]:
    for p in BROWSER_PATHS:
        if os.path.exists(p):
            return p
    return None


def find_free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def check_css_brace_balance(html_content: str) -> Tuple[bool, int, Optional[int]]:
    """Checks for unclosed or mismatched curly braces in <style> blocks."""
    style_matches = re.findall(r"<style[^>]*>(.*?)</style>", html_content, re.DOTALL | re.IGNORECASE)
    if not style_matches:
        return True, 0, None

    open_braces = 0
    first_unbalanced_line = None

    for match in style_matches:
        lines = match.split("\n")
        for i, line in enumerate(lines):
            # Strip comments and strings roughly
            cleaned = re.sub(r"/\*.*?\*/", "", line)
            cleaned = re.sub(r'"[^"]*"', '""', cleaned)
            cleaned = re.sub(r"'[^']*'", "''", cleaned)

            for char in cleaned:
                if char == "{":
                    open_braces += 1
                elif char == "}":
                    open_braces -= 1

            if open_braces > 0 and first_unbalanced_line is None:
                first_unbalanced_line = i + 1

    return (open_braces == 0), open_braces, first_unbalanced_line


async def run_cdp_audit(app_name: str, browser_path: str, port: int) -> Dict[str, Any]:
    import websockets

    # Normalise app file name
    clean_name = app_name.lower().replace(".html", "")
    if not clean_name.startswith("k") and (APPS_DIR / f"k{clean_name}.html").exists():
        clean_name = f"k{clean_name}"

    app_file = APPS_DIR / f"{clean_name}.html"
    if not app_file.exists():
        return {
            "app": clean_name,
            "success": False,
            "error": f"File not found: {app_file}"
        }

    raw_html = app_file.read_text(encoding="utf-8", errors="ignore")
    stat = app_file.stat()
    size_kb = round(stat.st_size / 1024, 1)

    result = {
        "app": clean_name,
        "file_size_kb": size_kb,
        "size_valid": stat.st_size <= MAX_FILE_SIZE_BYTES,
        "css_balanced": False,
        "css_open_braces": 0,
        "css_error_line": None,
        "console_errors": [],
        "startup_modal": None,
        "modal_dismiss_tested": False,
        "modal_dismiss_success": False,
        "modal_dismiss_error": None,
        "occlusion_check": "not_tested",
        "occlusion_blocker": None,
        "canvas_detected": False,
        "canvas_size": None,
        "human_perspective_summary": "",
        "passed": False,
        "failures": []
    }

    # 1. Static CSS Balance Check
    css_ok, open_braces, err_line = check_css_brace_balance(raw_html)
    result["css_balanced"] = css_ok
    result["css_open_braces"] = open_braces
    result["css_error_line"] = err_line
    if not css_ok:
        result["failures"].append(
            f"CSS syntax error: {open_braces} unclosed brace(s). First unclosed near line {err_line} in stylesheet!"
        )

    if not result["size_valid"]:
        result["failures"].append(f"File size {size_kb} KB exceeds 999 KB limit!")

    # 2. Launch headless browser
    browser_proc = subprocess.Popen([
        browser_path,
        "--headless=new",
        f"--remote-debugging-port={port}",
        "--disable-gpu",
        "--no-sandbox",
        "--disable-dev-shm-usage",
        "--mute-audio",
        "--window-size=1024,768",
        "about:blank"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    try:
        # Wait for CDP endpoint to be ready
        ws_url = None
        for _ in range(30):
            await asyncio.sleep(0.15)
            try:
                req = urllib.request.Request(f"http://127.0.0.1:{port}/json/version")
                with urllib.request.urlopen(req, timeout=1) as resp:
                    data = json.loads(resp.read().decode())
                    if "webSocketDebuggerUrl" in data:
                        # Get pages list
                        with urllib.request.urlopen(f"http://127.0.0.1:{port}/json/list", timeout=1) as l_resp:
                            pages = json.loads(l_resp.read().decode())
                            page = next((p for p in pages if p.get("type") == "page"), pages[0] if pages else None)
                            if page and "webSocketDebuggerUrl" in page:
                                ws_url = page["webSocketDebuggerUrl"]
                                break
            except Exception:
                pass

        if not ws_url:
            result["failures"].append("Failed to establish CDP connection to browser")
            return result

        async with websockets.connect(ws_url, max_size=10_000_000) as ws:
            msg_id = 0
            pending = {}
            console_errors = []
            page_loaded = asyncio.Event()

            async def send_cmd(method: str, params: Optional[dict] = None) -> Any:
                nonlocal msg_id
                msg_id += 1
                curr_id = msg_id
                fut = asyncio.get_running_loop().create_future()
                pending[curr_id] = fut
                payload = json.dumps({"id": curr_id, "method": method, "params": params or {}})
                await ws.send(payload)
                return await fut

            async def message_listener():
                try:
                    async for raw in ws:
                        msg = json.loads(raw)
                        mid = msg.get("id")
                        if mid and mid in pending:
                            pending[mid].set_result(msg.get("result"))
                            del pending[mid]

                        method = msg.get("method")
                        if method == "Page.loadEventFired":
                            page_loaded.set()
                        elif method == "Runtime.exceptionThrown":
                            details = msg.get("params", {}).get("exceptionDetails", {})
                            text = details.get("exception", {}).get("description") or details.get("text")
                            if text:
                                console_errors.append(text)
                        elif method == "Page.javascriptDialogOpening":
                            # Auto-accept alerts/confirms
                            asyncio.create_task(send_cmd("Page.handleJavaScriptDialog", {"accept": True}))
                except asyncio.CancelledError:
                    pass
                except Exception:
                    pass

            listener_task = asyncio.create_task(message_listener())

            await send_cmd("Page.enable")
            await send_cmd("Runtime.enable")

            # Navigate to local file URL
            file_url = app_file.as_uri()
            await send_cmd("Page.navigate", {"url": file_url})

            try:
                await asyncio.wait_for(page_loaded.wait(), timeout=3.0)
            except asyncio.TimeoutError:
                pass

            # Settle period for animations and startup scripts
            await asyncio.sleep(0.4)

            # Record console exceptions
            result["console_errors"] = list(console_errors)
            if console_errors:
                result["failures"].append(f"Runtime JavaScript error on load: {console_errors[0]}")

            # 3. Evaluate Startup UI & Modal Obtrusiveness from human perspective
            eval_probe_js = """
            (function() {
                const vw = window.innerWidth || 1024;
                const vh = window.innerHeight || 768;
                const totalArea = vw * vh;

                function isVisible(el) {
                    if (!el) return false;
                    const style = window.getComputedStyle(el);
                    if (style.display === 'none' || style.visibility === 'hidden' || style.opacity === '0') return false;
                    const rect = el.getBoundingClientRect();
                    return rect.width > 0 && rect.height > 0;
                }

                // Check for canvas elements and primary game surface
                const canvas = document.querySelector('canvas');
                let canvasInfo = null;
                if (canvas && isVisible(canvas)) {
                    const cr = canvas.getBoundingClientRect();
                    canvasInfo = { width: Math.round(cr.width), height: Math.round(cr.height), left: Math.round(cr.left), top: Math.round(cr.top) };
                }

                // Check for large overlay / modal elements
                const overlayCandidates = Array.from(document.querySelectorAll(
                    '[class*="modal"], [id*="modal"], [class*="overlay"], [id*="overlay"], [class*="dialog"], [class*="backdrop"], [class*="manual"], [id*="manual"]'
                )).filter(isVisible);

                let primaryOverlay = null;
                for (const el of overlayCandidates) {
                    const rect = el.getBoundingClientRect();
                    const area = rect.width * rect.height;
                    const coverage = (area / totalArea);
                    if (coverage > 0.35) { // Covers >35% of the viewport
                        // Find close button
                        const closeBtn = el.querySelector(
                            'button[class*="close"], .btn-close, button[title*="Close"], button[title*="close"], button'
                        );
                        primaryOverlay = {
                            id: el.id,
                            className: el.className,
                            coverage: Math.round(coverage * 100),
                            hasCloseBtn: Boolean(closeBtn),
                            closeBtnSelector: closeBtn ? (closeBtn.id ? '#' + closeBtn.id : (closeBtn.className ? '.' + closeBtn.className.split(' ')[0] : 'button')) : null,
                            closeBtnText: closeBtn ? closeBtn.innerText.trim() : null
                        };
                        break;
                    }
                }

                // Check center hit-test
                const centerX = Math.round(vw / 2);
                const centerY = Math.round(vh / 2);
                const hitEl = document.elementFromPoint(centerX, centerY);
                let hitInfo = null;
                if (hitEl) {
                    hitInfo = {
                        tag: hitEl.tagName.toLowerCase(),
                        id: hitEl.id || null,
                        className: hitEl.className || null,
                        isCanvas: hitEl.tagName.toLowerCase() === 'canvas',
                        isOverlay: Boolean(hitEl.closest('[class*="modal"], [id*="modal"], [class*="overlay"], [id*="overlay"]'))
                    };
                }

                return {
                    canvasInfo,
                    primaryOverlay,
                    hitInfo
                };
            })()
            """

            probe_res = await send_cmd("Runtime.evaluate", {"expression": eval_probe_js, "returnByValue": True})
            probe_data = probe_res.get("result", {}).get("value") or {}

            canvas_info = probe_data.get("canvasInfo")
            if canvas_info:
                result["canvas_detected"] = True
                result["canvas_size"] = f"{canvas_info['width']}x{canvas_info['height']}"

            primary_overlay = probe_data.get("primaryOverlay")
            hit_info = probe_data.get("hitInfo") or {}

            if primary_overlay:
                result["startup_modal"] = primary_overlay
                cov = primary_overlay["coverage"]
                # A full-screen or large startup modal was found!
                # Test dismissal to ensure it's not stuck
                result["modal_dismiss_tested"] = True

                dismiss_test_js = """
                (async function() {
                    const sel = %s;
                    let clicked = false;
                    const candidates = [
                        sel,
                        '.help-control-btn.help-btn-close',
                        '.btn-help-footer-close',
                        'button.close',
                        '.modal-close',
                        'button[class*="close"]',
                        '[class*="modal"] button',
                        '[class*="overlay"] button'
                    ];

                    for (const s of candidates) {
                        if (!s) continue;
                        const btn = document.querySelector(s);
                        if (btn) {
                            btn.click();
                            clicked = true;
                            break;
                        }
                    }

                    if (!clicked) {
                        // Try ESC key
                        document.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', code: 'Escape', bubbles: true }));
                    }

                    await new Promise(r => setTimeout(r, 200));

                    // Verify if overlay is now hidden
                    const overlays = Array.from(document.querySelectorAll(
                        '[class*="modal"], [id*="modal"], [class*="overlay"], [id*="overlay"]'
                    ));
                    const stillVisible = overlays.some(el => {
                        const style = window.getComputedStyle(el);
                        if (style.display === 'none' || style.visibility === 'hidden' || style.opacity === '0') return false;
                        const rect = el.getBoundingClientRect();
                        return (rect.width * rect.height) > (window.innerWidth * window.innerHeight * 0.35);
                    });

                    return { clicked, stillVisible };
                })()
                """ % json.dumps(primary_overlay.get("closeBtnSelector"))

                dismiss_res = await send_cmd("Runtime.evaluate", {
                    "expression": dismiss_test_js,
                    "returnByValue": True,
                    "awaitPromise": True
                })
                dismiss_data = dismiss_res.get("result", {}).get("value") or {}

                if dismiss_data.get("stillVisible"):
                    result["modal_dismiss_success"] = False
                    result["failures"].append(
                        f"Unclosable startup modal: Overlay '{primary_overlay.get('id') or primary_overlay.get('className')}' covers {cov}% of screen and remained visible after close button click / ESC!"
                    )
                else:
                    result["modal_dismiss_success"] = True

            # 4. Occlusion Check: Does the center of the screen hit the gameplay area or a blocking overlay?
            if hit_info:
                if hit_info.get("isOverlay"):
                    result["occlusion_check"] = "BLOCKED"
                    result["occlusion_blocker"] = f"<{hit_info['tag']} id='{hit_info['id']}' class='{hit_info['className']}'>"
                    result["failures"].append(
                        f"Gameplay surface occluded: Center viewport hit-test intercepted by modal overlay: {result['occlusion_blocker']}"
                    )
                else:
                    result["occlusion_check"] = "PASS"

            listener_task.cancel()

    finally:
        browser_proc.terminate()
        try:
            browser_proc.wait(timeout=1.0)
        except Exception:
            browser_proc.kill()

    result["passed"] = (len(result["failures"]) == 0)

    # Generate terse human summary
    notes = []
    if result["css_balanced"]:
        notes.append("CSS valid")
    else:
        notes.append("CSS broken")

    if result["console_errors"]:
        notes.append(f"{len(result['console_errors'])} JS err")
    else:
        notes.append("0 JS err")

    if result["startup_modal"]:
        if result["modal_dismiss_success"]:
            notes.append("Modal dismisses cleanly")
        else:
            notes.append("MODAL STUCK")
    else:
        notes.append("Clean direct startup")

    if result["canvas_detected"]:
        notes.append(f"Canvas {result['canvas_size']}")

    if result["occlusion_check"] == "PASS":
        notes.append("Hit-test unblocked")
    elif result["occlusion_check"] == "BLOCKED":
        notes.append("OCCLUDED")

    result["human_perspective_summary"] = ", ".join(notes)
    return result


def main():
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    if hasattr(sys.stderr, "reconfigure"):
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")

    parser = argparse.ArgumentParser(description="KiloApps Human-Perspective Startup & UX Test Suite")
    parser.add_argument("--app", help="Specific app name to test (e.g. kalchemy, kchess, kquest)")
    parser.add_argument("--all", action="store_true", help="Test all web apps in KiloOS/public/apps/")
    parser.add_argument("--limit", type=int, default=0, help="Limit number of apps when testing all")
    args = parser.parse_args()

    browser = find_browser()
    if not browser:
        print("❌ Error: No Chrome or Edge browser executable found on system.", file=sys.stderr)
        sys.exit(1)

    port = find_free_port()

    apps_to_test = []
    if args.app:
        apps_to_test.append(args.app)
    elif args.all:
        for f in sorted(os.listdir(APPS_DIR)):
            if f.endswith(".html"):
                apps_to_test.append(f.replace(".html", ""))
        if args.limit > 0:
            apps_to_test = apps_to_test[:args.limit]
    else:
        # Default test target
        apps_to_test.append("kalchemy")

    print(f"=== KiloApps Human-Perspective Gameplay Startup Test ===")
    print(f"Browser: {browser}")
    print(f"Testing {len(apps_to_test)} app(s)...\n")

    overall_pass = True
    for app in apps_to_test:
        report = asyncio.run(run_cdp_audit(app, browser, port))
        status_icon = "🟢 PASS" if report["passed"] else "🔴 FAIL"
        print(f"[{status_icon}] {report['app'].upper()} ({report['file_size_kb']} KB)")
        print(f"       Summary : {report['human_perspective_summary']}")

        if not report["passed"]:
            overall_pass = False
            for f in report["failures"]:
                print(f"       ⚠️  {f}")
        print()

    sys.exit(0 if overall_pass else 1)


if __name__ == "__main__":
    main()
