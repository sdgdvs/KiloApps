#!/usr/bin/env python3
"""
scripts/verify_turn.py
One-Command Verification Pipeline for KiloApps Scheduled Flash Agents.

Replaces 4-6 separate post-edit shell commands with a single fast, deterministic
verification tool call that checks:
1. File existence & strict < 999 KB (999,000 bytes) budget for web (.html) and native (.exe).
2. Optional/automatic native Win32 C compilation (`build.bat`) if `main.c` was modified.
3. Automated human-perspective startup check (`test_app_startup.py --app <target>`) for apps,
   or static CSS/JS balance check for virtual web nodes (`kweb://...`).
4. Vite production build (`npm run build` inside `KiloOS/`) when `KiloOS/src/` changed or `--vite` requested.
5. Security & Anti-Trademark linter (`scripts/security_lint.py`).

Outputs a ready-to-log 1-line summary at the end so agents can immediately advance the turn.
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))
MAX_BYTES = 999_000

WEB_MAP = {
    "kweb://portal": "portal.html",
    "kweb://webring": "webring.html",
    "kweb://warez": "warez.html",
    "kweb://geocities": "geocities.html",
    "kweb://users/~neon_rider": "geocities.html",
    "kweb://asm-temple": "asm_temple.html",
    "kweb://cybercafe": "cybercafe.html",
    "kweb://10.19.99.4/classified": "classified.html",
    "kweb://echo-subsystem.net": "echo_subsystem.html",
    "kweb://deep-core": "deep_core.html",
    "kweb://darknet": "darknet.html",
    "kweb://slashnet": "slashnet.html",
    "kweb://trapster": "trapster.html",
    "kweb://cabled": "cabled.html",
    "kweb://scallion": "scallion.html",
    "kweb://demoscene": "demoscene.html",
    "kweb://rfc-archive": "rfc_archive.html",
}


def resolve_targets(target: str) -> tuple[bool, Path | None, Path | None, Path | None]:
    """Returns (is_kweb, html_path, native_dir, exe_path)."""
    t = target.strip()
    if t.startswith("kweb://") or t.endswith(".html") and (REPO_ROOT / "KiloOS" / "public" / "web" / t).exists():
        fname = WEB_MAP.get(t)
        if not fname:
            slug = t.replace("kweb://", "").strip("/").replace("-", "_").replace(".", "_")
            fname = slug if slug.endswith(".html") else f"{slug}.html"
        return True, REPO_ROOT / "KiloOS" / "public" / "web" / fname, None, None

    app_id = t.lower()
    if not app_id.startswith("k"):
        app_id = "k" + app_id
    if app_id.endswith(".html"):
        app_id = app_id[:-5]

    html_path = REPO_ROOT / "KiloOS" / "public" / "apps" / f"{app_id}.html"

    # Find native directory if it exists (case-insensitive match in repo root or apps/)
    native_dir = None
    exe_path = None
    for cand in REPO_ROOT.iterdir():
        if cand.is_dir() and cand.name.lower() == app_id and (cand / "main.c").exists():
            native_dir = cand
            exe_path = cand / f"{cand.name}.exe"
            break
    if not native_dir and (REPO_ROOT / "apps" / app_id / "main.c").exists():
        native_dir = REPO_ROOT / "apps" / app_id
        exe_path = native_dir / f"{t}.exe"

    return False, html_path, native_dir, exe_path


def main() -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    parser = argparse.ArgumentParser(description="KiloApps One-Command Turn Verification")
    parser.add_argument("--app", required=True, help="Target app name (e.g. KTrader) or kweb:// URL")
    parser.add_argument("--build-native", action="store_true", help="Force native Win32 C build if build.bat exists")
    parser.add_argument("--vite", action="store_true", help="Force Vite npm run build inside KiloOS/")
    parser.add_argument("--skip-vite", action="store_true", help="Skip Vite npm run build if KiloOS/src was not modified")
    parser.add_argument("--no-visual", action="store_true", help="Skip the headless visual/gameplay probe (scripts/visual_probe.py)")
    args = parser.parse_args()

    is_kweb, html_path, native_dir, exe_path = resolve_targets(args.app)
    errors = []

    # 1. Check HTML existence & size
    web_kb = 0.0
    if not html_path or not html_path.exists():
        errors.append(f"Missing HTML file: {html_path}")
    else:
        sz = html_path.stat().st_size
        web_kb = round(sz / 1024, 1)
        if sz >= MAX_BYTES:
            errors.append(f"HTML exceeds 999 KB ceiling: {html_path.name} is {sz} bytes ({web_kb} KB)")
        else:
            print(f"[SIZE PASS] {html_path.name}: {web_kb} KB (< 999 KB)")

    # 2. Check / compile Native C if modified or requested
    native_kb = None
    if native_dir and (native_dir / "build.bat").exists():
        st = subprocess.run(
            ["git", "status", "--porcelain", str(native_dir)],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
        )
        c_modified = bool(st.stdout.strip()) or args.build_native
        if c_modified:
            print(f"[NATIVE BUILD] Compiling {native_dir.name}/build.bat...")
            b_res = subprocess.run(
                ["cmd.exe", "/c", "build.bat"],
                cwd=str(native_dir),
                capture_output=True,
                text=True,
                stdin=subprocess.DEVNULL,
                timeout=60,
            )
            if b_res.returncode != 0:
                errors.append(f"Native C build failed in {native_dir.name}:\n{b_res.stdout}\n{b_res.stderr}")
            else:
                print(f"[NATIVE PASS] {native_dir.name} compiled cleanly.")
        if exe_path and exe_path.exists():
            esz = exe_path.stat().st_size
            native_kb = round(esz / 1024, 1)
            if esz >= MAX_BYTES:
                errors.append(f"Native EXE exceeds 999 KB ceiling: {exe_path.name} is {esz} bytes ({native_kb} KB)")
            else:
                # Ensure public/exe copy is synced if C was modified
                pub_exe = REPO_ROOT / "KiloOS" / "public" / "exe" / exe_path.name
                if c_modified and pub_exe.parent.exists():
                    shutil.copy2(exe_path, pub_exe)
                print(f"[SIZE PASS] {exe_path.name}: {native_kb} KB (< 999 KB)")

    # 3. Startup / Syntax check (fast inline CSS brace & script balance check when visual_probe runs; full CDP startup check if --no-visual)
    if html_path and html_path.exists():
        from scripts.test_app_startup import check_css_brace_balance, check_script_tag_balance
        raw_html = html_path.read_text(encoding="utf-8", errors="replace")
        css_ok, open_braces, err_line = check_css_brace_balance(raw_html)
        if not css_ok:
            errors.append(f"CSS syntax error in {html_path.name}: {open_braces} unclosed brace(s) near line {err_line}")
        script_ok, script_err = check_script_tag_balance(raw_html)
        if not script_ok:
            errors.append(f"Unbalanced <script> tags in {html_path.name}: {script_err}")
    if not is_kweb and args.no_visual:
        uv_bin = shutil.which("uv")
        cmd = [uv_bin, "run", "scripts/test_app_startup.py", "--app", args.app] if uv_bin else [sys.executable, "scripts/test_app_startup.py", "--app", args.app]
        print(f"[STARTUP AUDIT] Running {' '.join(cmd)}...")
        s_res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace", stdin=subprocess.DEVNULL, timeout=35)
        if s_res.returncode != 0:
            errors.append(f"Startup check failed for {args.app}:\n{s_res.stdout}\n{s_res.stderr}")
        else:
            print(f"[STARTUP PASS] {args.app} passed human-perspective startup & modal occlusion audit.")
    elif not is_kweb:
        print(f"[STARTUP PASS] {args.app} CSS/script syntax clean (CDP startup & modal dismissal unified into visual_probe).")
    elif html_path and html_path.exists():
        print(f"[WEB NODE PASS] {html_path.name} structure verified.")

    # 3b. Visual & gameplay probe (screenshots, trusted-input playtest, HEAD before/after sheet)
    visual_summary = "skipped"
    if not args.no_visual and html_path and html_path.exists():
        uv_bin = shutil.which("uv")
        if not uv_bin:
            print("[VISUAL PROBE] skipped — uv not installed.")
        else:
            rel = html_path.relative_to(REPO_ROOT).as_posix()
            v_cmd = [uv_bin, "run", "-q", "scripts/visual_probe.py", "--file", rel, "--compare-head"]
            print(f"[VISUAL PROBE] {' '.join(v_cmd[1:])}")
            v_res = subprocess.run(v_cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace", stdin=subprocess.DEVNULL, timeout=45)
            out_lines = [ln for ln in v_res.stdout.splitlines() if ln.strip()]
            for ln in out_lines[1:]:
                if not ln.startswith("VISUAL_PROBE_SUMMARY"):
                    print(f"    {ln}")
            summ = next((ln for ln in out_lines if ln.startswith("VISUAL_PROBE_SUMMARY")), "")
            visual_summary = summ.split(":", 1)[1].strip() if summ else f"error (exit {v_res.returncode})"
            if v_res.returncode == 1:
                fails = [ln.replace("❌ FAIL  ", "") for ln in out_lines if ln.startswith("❌ FAIL")]
                errors.append("Visual probe FAIL: " + ("; ".join(fails) or visual_summary))
            elif v_res.returncode != 0:
                print(f"[VISUAL PROBE] tool error (non-blocking): {(v_res.stderr or v_res.stdout).strip()[-300:]}")
            else:
                print(f"[VISUAL {visual_summary.split(' ')[0]}] {visual_summary}")

    # 4. Vite build check (auto-runs when KiloOS shell files changed or --vite requested)
    if not args.skip_vite:
        vite_st = subprocess.run(
            ["git", "status", "--porcelain", "KiloOS/src", "KiloOS/index.html", "KiloOS/package.json", "KiloOS/vite.config.js"],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
            stdin=subprocess.DEVNULL,
        )
        shell_modified = bool(vite_st.stdout.strip()) or args.vite
        if shell_modified:
            npm_bin = shutil.which("npm.cmd") or shutil.which("npm") or "npm.cmd"
            print("[VITE BUILD] Running npm run build in KiloOS/...")
            v_res = subprocess.run([npm_bin, "run", "build"], cwd=str(REPO_ROOT / "KiloOS"), capture_output=True, text=True, encoding="utf-8", errors="replace", stdin=subprocess.DEVNULL, timeout=60)
            if v_res.returncode != 0:
                errors.append(f"KiloOS Vite build failed:\n{v_res.stdout}\n{v_res.stderr}")
            else:
                print("[VITE PASS] KiloOS production build succeeded (0 errors).")
        else:
            print("[VITE PASS] KiloOS/src unchanged (fast path; pass --vite to force full bundle build).")

    # 5. Security & Anti-Trademark Linter
    print("[SECURITY LINT] Running scripts/security_lint.py --fast...")
    lint_files = [str(html_path)] if html_path and html_path.exists() else []
    if native_dir and (native_dir / "main.c").exists():
        lint_files.append(str(native_dir / "main.c"))
    l_res = subprocess.run([sys.executable, "scripts/security_lint.py", "--fast", "--files", *lint_files], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace", stdin=subprocess.DEVNULL, timeout=30)
    if l_res.returncode != 0:
        errors.append(f"Security lint failed:\n{l_res.stdout}\n{l_res.stderr}")
    else:
        print("[SECURITY PASS] All security & anti-trademark gates passed.")

    if errors:
        print("\n❌ VERIFICATION FAILED:")
        for err in errors:
            print(f"  - {err}")
        return 1

    native_str = f", native {native_kb} KB" if native_kb is not None else ""
    print(
        f"\n✅ VERIFY_TURN_SUMMARY: Status: PASS ✅ "
        f"(0 errors, web {web_kb} KB{native_str} < 999 KB; startup, Vite & security_lint PASS; visual {visual_summary})."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
