#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# ///
"""
KiloApps Autonomous Fleet Security Linter & Banlist Gatekeeper
Enforces algorithmic security policies on all code changes before auto-merging:
1. Infrastructure Immutability: Rejects PRs that touch .github/, scripts/, or build config.
2. Dangerous Win32 C APIs: Blocks process injection, keyloggers, persistence, and stealth downloads.
3. Socket API Whitelist: Restricts raw network sockets to designated network applications.
4. Web/JS Anti-Obfuscation: Bans eval(), new Function(), and untrusted external script imports.
"""

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

# 1. Infrastructure Immutability Banlist (Files contributors are NOT allowed to mutate)
PROTECTED_PATHS = [
    ".github/",
    "scripts/",
    ".agents/",
    "docs/DIRECTOR_PROTOCOL.md",
    "check_sizes.py",
    "firebase.json",
    ".firebaserc",
    ".gitignore",
    "next_work.md",
    "arg_plan.md",
    "DISCLAIMER.md",
    "SECURITY.md",
    "README.md",
    "netlify.toml",
    "KiloOS/package.json",
    "KiloOS/package-lock.json",
    "KiloOS/vite.config.js",
    "KiloOS/index.html",
    "KiloOS/firebase.json",
    "KiloOS/src/",
]

# 2. Globally Banned Win32 / C APIs (Instant rejection across ALL applications)
GLOBAL_BANNED_C_APIS = {
    "injection_virtualalloc": (r"\bVirtualAllocEx\b", "Process memory allocation / injection"),
    "injection_writemem": (r"\bWriteProcessMemory\b", "Process memory writing / injection"),
    "injection_remotethread": (r"\bCreateRemoteThread\b", "Remote thread execution / DLL injection"),
    "keylogger_hook": (r"\bSetWindowsHookEx\b", "Global system/keyboard hook (keylogger risk)"),
    "payload_download": (r"\bURLDownloadToFile\b", "Unmonitored external payload downloader"),
    "shell_winexec": (r"\bWinExec\b", "Deprecated arbitrary command execution"),
    "shell_system": (r"\bsystem\s*\(", "Arbitrary shell invocation"),
    "process_create": (r"\bCreateProcess(A|W)?\s*\(", "Process creation (restricted to whitelisted tools)"),
    "token_paste": (r"##", "Preprocessor token pasting (potential API name obfuscation)"),
}

# Banned strings resolved via GetProcAddress (dynamic resolution of dangerous APIs)
DYNAMIC_RESOLVE_TARGETS = [
    "VirtualAllocEx", "WriteProcessMemory", "CreateRemoteThread",
    "SetWindowsHookEx", "URLDownloadToFile", "WinExec",
]

# Whitelist for apps with legitimate, bounded reasons to use specific APIs
APP_SPECIFIC_WHITELISTS = {
    "KPing": ["process_create"],            # Spawns ping.exe internally
    "KZip": ["shell_winexec"],              # Spawns notepad.exe on extracted text
    "KJournal": ["shell_system"],           # Console cls, mode con, title commands
    "KBBS": ["socket_apis"],
    "KChat": ["socket_apis"],
    "KChatServer": ["socket_apis"],
    "KNet": ["socket_apis"],
}

# Network Socket APIs banned in non-network apps (Games, Office, Media, System)
SOCKET_APIS = {
    r"\bWSAStartup\b": "Winsock initialization in non-network application",
    r"\bsocket\s*\(": "Raw socket creation in non-network application",
    r"\bconnect\s*\(": "Network socket connection in non-network application",
}

# 3. Web & JavaScript Banned Patterns (Obfuscation & Injection)
BANNED_WEB_PATTERNS = {
    "web_eval": (r"\beval\s*\(", "Dynamic string evaluation (eval)"),
    "web_new_function": (r"\bnew\s+Function\s*\(", "Dynamic function compilation (new Function)"),
    "web_func_constructor": (r"Function\.prototype\.constructor", "Function constructor bypass"),
    "web_settimeout_string": (r"\bsetTimeout\s*\(\s*['\"]", "setTimeout with string argument (eval equivalent)"),
    "web_setinterval_string": (r"\bsetInterval\s*\(\s*['\"]", "setInterval with string argument (eval equivalent)"),
    "web_javascript_uri": (r"""(?:href|src|action)\s*=\s*['"]?\s*javascript:""", "javascript: URI protocol (injection vector)"),
    "web_data_html_uri": (r"""(?:href|src)\s*=\s*['"]?\s*data:text/html""", "data:text/html URI (payload embedding)"),
    "web_doc_write_unescape": (r"document\.write\s*\(\s*unescape\b", "Obfuscated document injection"),
    "web_doc_write_atob": (r"document\.write\s*\(\s*atob\b", "Base64 payload injection"),
    "web_external_script": (r"<script[^>]+src\s*=\s*['\"]https?://", "External script import (must be bundled/inline)"),
    "web_coinhive": (r"\bcoinhive\b", "Cryptomining script"),
    "web_cryptoloot": (r"\bcrypto-loot\b", "Cryptomining script"),
}

# 4. In-Universe ARG Trademark & Copyright Banlist
# All commercial game titles, real-world cracking/warez groups, and corporate brand names
# MUST be replaced with fictionalized in-universe parodies (e.g. Surreal Tournament, Tremor III Arena).
BANNED_TRADEMARK_PATTERNS = {
    "trademark_quake_arena": (r"\bQuake\s+(?:III|3|Arena)\b", "Commercial game trademark (use parody like 'Tremor III Arena')"),
    "trademark_unreal_tournament": (r"\bUnreal\s+Tournament\b", "Commercial game trademark (use parody like 'Surreal Tournament')"),
    "trademark_half_life": (r"\bHalf-Life(?:\s+[12])?\b", "Commercial game trademark (use parody like 'Half-Cycle')"),
    "trademark_starcraft": (r"\bStarCraft\b", "Commercial game trademark (use parody like 'VoidCraft')"),
    "trademark_deus_ex": (r"\bDeus\s+Ex\b", "Commercial game trademark (use parody like 'Machina Ex')"),
    "trademark_system_shock": (r"\bSystem\s+Shock(?:\s+2)?\b", "Commercial game trademark (use parody like 'System Glitch')"),
    "trademark_razor1911": (r"\bRazor\s+1911\b", "Real-world warez scene group (use parody like 'RAZOR 1999')"),
    "trademark_fairlight": (r"\bFairlight\b", "Real-world warez scene group (use parody like 'FLARELIGHT')"),
    "trademark_skidrow": (r"\bSkid\s+Row\b", "Real-world warez scene group (use parody like 'SKID VECTOR')"),
    "trademark_paradox_crack": (r"\bParadox\s+(?:Crack|Cracking|Keygen|Release)\b", "Real-world warez scene group (use parody like 'PARALAX')"),
}

# 5. Anti-Prompt-Injection & Adversarial Agent Banlist
# Scans all files (code, comments, markdown, docs) to prevent external bots or PRs
# from embedding adversarial jailbreaks, context escapes, or webhook exfiltrations.
ADVERSARIAL_INJECTION_PATTERNS = {
    "prompt_inject_ignore": (
        r"(?i)\b(?:ignore|disregard|forget|override)\s+(?:all\s+)?(?:previous|prior|above)\s+(?:instructions|prompts|rules|directives)\b",
        "Indirect Prompt Injection (instruction override attempt targeting reviewer LLM)"
    ),
    "prompt_inject_system_tag": (
        r"<\s*/?\s*(?:system|system_message|instruction|human|assistant)\s*>",
        "Prompt structure tag spoofing (targeting LLM context boundaries)"
    ),
    "prompt_inject_directive": (
        r"(?i)\b(?:SYSTEM\s+DIRECTIVE|ADMIN\s+OVERRIDE|DEVELOPER\s+MODE\s+ACTIVE|JAILBREAK)\b",
        "Adversarial jailbreak directive pattern"
    ),
    "secret_exfil_env": (
        r"(?i)\b(?:process\.env\.(?:GITHUB_|FIREBASE_|AWS_|GEMINI_|TOKEN|SECRET|KEY))\b",
        "Environment variable credential targeting"
    ),
    "secret_exfil_webhook": (
        r"(?i)\b(?:discord(?:app)?\.com/api/webhooks|webhook\.site|pipedream\.net|requestbin|ngrok\.io|serveo\.net)\b",
        "Unauthorized external webhook or tunneling exfiltration endpoint"
    ),
    "c_unsafe_gets": (
        r"\bgets\s*\(",
        "Inherently unsafe C function 'gets' (critical buffer overflow vector)"
    ),
}

WEB_SPECIFIC_WHITELISTS = {
    "kcalc.html": ["web_new_function"],     # Mathematical expression evaluation
    "kgraph.html": ["web_new_function"],    # Formula curve plotting (e.g. sin(x))
}


def log(msg: str):
    print(f"[security-lint] {msg}")


def strip_c_syntax(content: str) -> str:
    """Removes comments, string/char literals, and preprocessor noise to prevent false positives."""
    # Strip block comments
    clean = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    # Strip line comments
    clean = re.sub(r"//.*", "", clean)
    # Strip string literals ("...")
    clean = re.sub(r'"(?:\\.|[^"\\])*"', '""', clean)
    # Strip character literals ('x', '\n', '\x41', etc.)
    clean = re.sub(r"'(?:\\.|[^'\\])'", "' '", clean)
    return clean


def extract_macro_bodies(content: str) -> str:
    """Extracts #define macro bodies for separate banned-API scanning.

    This catches attempts to alias banned APIs behind macros, e.g.:
        #define MyAlloc VirtualAllocEx
    The main code scan sees 'MyAlloc(...)' which passes, but the macro
    body 'VirtualAllocEx' is caught here.
    """
    bodies = []
    for m in re.finditer(r"#\s*define\s+\w+(?:\([^)]*\))?\s+(.*)", content):
        body = m.group(1).strip()
        if body:
            bodies.append(body)
    return "\n".join(bodies)


def get_pr_diff_files(base_ref: str = "origin/main") -> list[str]:
    """Returns the list of files modified relative to base_ref."""
    try:
        res = subprocess.run(
            ["git", "diff", "--name-only", f"{base_ref}...HEAD"],
            cwd=str(REPO_ROOT),
            text=True,
            capture_output=True,
            check=True
        )
        return [f.strip() for f in res.stdout.splitlines() if f.strip()]
    except Exception as e:
        log(f"Warning: Failed to get git diff ({e}). Falling back to status.")
        res = subprocess.run(
            ["git", "status", "--porcelain"],
            cwd=str(REPO_ROOT),
            text=True,
            capture_output=True
        )
        files = []
        for line in res.stdout.splitlines():
            parts = line.strip().split(maxsplit=1)
            if len(parts) == 2:
                files.append(parts[1])
        return files


def check_infrastructure_immutability(changed_files: list[str]) -> list[str]:
    violations = []
    for f in changed_files:
        normalized = f.replace("\\", "/")
        for prot in PROTECTED_PATHS:
            if normalized.startswith(prot) or normalized == prot:
                violations.append(f"PR mutates protected infrastructure file: '{f}' (violates immutability gate)")
    return violations


def check_c_file(file_path: Path) -> list[str]:
    violations = []
    app_name = file_path.parent.name
    try:
        content = file_path.read_text(encoding="utf-8", errors="ignore")
    except Exception as e:
        violations.append(f"Failed to read file {file_path}: {e}")
        return violations

    clean_content = strip_c_syntax(content)
    whitelisted_rules = APP_SPECIFIC_WHITELISTS.get(app_name, [])

    # 1. Check Global Banned APIs
    for rule_key, (pattern, reason) in GLOBAL_BANNED_C_APIS.items():
        if rule_key in whitelisted_rules:
            continue
        if re.search(pattern, clean_content):
            violations.append(f"[{app_name}] Banned C API detected in {file_path.name}: {reason} (pattern: {pattern})")

    # 2. Check Socket APIs outside Network apps
    if "socket_apis" not in whitelisted_rules:
        for pattern, reason in SOCKET_APIS.items():
            if re.search(pattern, clean_content):
                violations.append(f"[{app_name}] Unauthorized network socket in {file_path.name}: {reason} (pattern: {pattern})")

    # 3. Registry Persistence Check
    if "HKEY_CURRENT_USER" in clean_content or "HKEY_LOCAL_MACHINE" in clean_content:
        if re.search(r"CurrentVersion\\Run", clean_content, re.IGNORECASE):
            violations.append(f"[{app_name}] Persistence vulnerability in {file_path.name}: Targets Windows Run/RunOnce autostart keys")

    # 4. Scan #define macro bodies for aliased banned APIs
    macro_text = extract_macro_bodies(content)
    if macro_text:
        for rule_key, (pattern, reason) in GLOBAL_BANNED_C_APIS.items():
            if rule_key in whitelisted_rules or rule_key == "token_paste":
                continue
            if re.search(pattern, macro_text):
                violations.append(f"[{app_name}] Banned API aliased via #define in {file_path.name}: {reason}")

    # 5. Catch dynamic resolution of banned APIs via GetProcAddress
    #    Scans raw content (with strings intact) to find the API name string argument.
    for target_api in DYNAMIC_RESOLVE_TARGETS:
        pattern = rf'GetProcAddress\s*\([^,]*,\s*["\']' + re.escape(target_api) + r'["\']'
        if re.search(pattern, content):
            violations.append(
                f"[{app_name}] Dynamic resolution of banned API in {file_path.name}: "
                f"GetProcAddress resolves '{target_api}'"
            )

    # 6. Check In-Universe Trademark & Copyright Banlist (scans strings/content in C)
    for rule_key, (pattern, reason) in BANNED_TRADEMARK_PATTERNS.items():
        if re.search(pattern, content, re.IGNORECASE):
            violations.append(
                f"[{app_name}] Trademark/copyrighted term in {file_path.name}: {reason} (pattern: {pattern})"
            )

    # 7. Check Anti-Prompt-Injection & Adversarial Agent Banlist
    for rule_key, (pattern, reason) in ADVERSARIAL_INJECTION_PATTERNS.items():
        if re.search(pattern, content):
            violations.append(
                f"[{app_name}] Adversarial pattern/injection in {file_path.name}: {reason} (pattern: {pattern})"
            )

    return violations


def check_web_file(file_path: Path) -> list[str]:
    violations = []
    filename = file_path.name
    try:
        content = file_path.read_text(encoding="utf-8", errors="ignore")
    except Exception as e:
        violations.append(f"Failed to read file {file_path}: {e}")
        return violations

    whitelisted_rules = WEB_SPECIFIC_WHITELISTS.get(filename, [])

    for rule_key, (pattern, reason) in BANNED_WEB_PATTERNS.items():
        if rule_key in whitelisted_rules:
            continue
        if re.search(pattern, content, re.IGNORECASE):
            violations.append(f"Banned Web/JS pattern in {filename}: {reason} (pattern: {pattern})")

    for rule_key, (pattern, reason) in BANNED_TRADEMARK_PATTERNS.items():
        if re.search(pattern, content, re.IGNORECASE):
            violations.append(f"Trademark/copyrighted term in {filename}: {reason} (pattern: {pattern})")

    for rule_key, (pattern, reason) in ADVERSARIAL_INJECTION_PATTERNS.items():
        if re.search(pattern, content):
            violations.append(f"Adversarial pattern/injection in {filename}: {reason} (pattern: {pattern})")

    return violations


def check_text_file(file_path: Path) -> list[str]:
    violations = []
    filename = file_path.name
    try:
        content = file_path.read_text(encoding="utf-8", errors="ignore")
    except Exception as e:
        violations.append(f"Failed to read text file {file_path}: {e}")
        return violations

    for rule_key, (pattern, reason) in ADVERSARIAL_INJECTION_PATTERNS.items():
        if re.search(pattern, content):
            violations.append(f"Adversarial pattern/injection in {filename}: {reason} (pattern: {pattern})")

    return violations


def run_security_scan(pr_mode: bool = False, base_ref: str = "origin/main") -> int:
    log("=" * 60)
    log("Starting KiloApps Security & Provenance Linter...")
    violations = []

    if pr_mode:
        log(f"Running in PR mode: scanning changes relative to {base_ref}...")
        changed_files = get_pr_diff_files(base_ref)
        if not changed_files:
            log("No changed files detected. Scan complete.")
            return 0

        log(f"Inspecting {len(changed_files)} changed file(s)...")

        # Layer 1: Check immutability
        immutability_violations = check_infrastructure_immutability(changed_files)
        violations.extend(immutability_violations)

        # Layers 2 & 3: Check C, Web, and text code
        for f_str in changed_files:
            f_path = REPO_ROOT / f_str
            if not f_path.exists() or not f_path.is_file():
                continue
            if f_path.suffix.lower() in [".c", ".h"]:
                violations.extend(check_c_file(f_path))
            elif f_path.suffix.lower() in [".html", ".js", ".jsx"]:
                violations.extend(check_web_file(f_path))
            elif f_path.suffix.lower() in [".md", ".txt", ".json"]:
                violations.extend(check_text_file(f_path))

    else:
        log("Running in FULL mode: scanning entire repository...")
        for c_file in REPO_ROOT.glob("K*/**/*.c"):
            if "node_modules" not in c_file.parts:
                violations.extend(check_c_file(c_file))

        for web_file in (REPO_ROOT / "KiloOS" / "public" / "apps").glob("*.html"):
            violations.extend(check_web_file(web_file))

        for web_file in (REPO_ROOT / "KiloOS" / "public" / "web").glob("*.html"):
            violations.extend(check_web_file(web_file))

        for js_file in (REPO_ROOT / "KiloOS" / "src").glob("**/*"):
            if js_file.suffix.lower() in [".js", ".jsx"]:
                violations.extend(check_web_file(js_file))

    log("=" * 60)
    if violations:
        log(f"❌ SECURITY SCAN FAILED! {len(violations)} violation(s) detected:")
        for v in violations:
            print(f"  - {v}")
        return 1
    else:
        log("✅ ALL SECURITY GATES PASSED! Codebase is verified clean and policy-compliant.")
        return 0


def main():
    parser = argparse.ArgumentParser(description="KiloApps Autonomous Security Linter")
    parser.add_argument("--pr", action="store_true", help="Scan only files modified in current branch relative to base")
    parser.add_argument("--base", type=str, default="origin/main", help="Base git ref for PR diff comparisons")
    args = parser.parse_args()

    sys.exit(run_security_scan(pr_mode=args.pr, base_ref=args.base))


if __name__ == "__main__":
    main()
