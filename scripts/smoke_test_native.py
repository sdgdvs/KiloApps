"""
scripts/smoke_test_native.py
Automated smoke test and size validation suite for all KiloApps native Win32 apps.
Validates:
1. App source structure (main.c, build.bat).
2. Binary presence (<AppName>.exe).
3. Strict 1999 retro size limit: no binary may exceed 999 KB.
4. Valid PE executable headers (MZ / PE signature).
"""

import os
import sys
from pathlib import Path

WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
MAX_EXE_SIZE_BYTES = 999 * 1024

EXCLUDED_DIRS = {
    ".agents",
    ".git",
    "archive",
    "docs",
    "KiloOS",
    "KiloOS_Server",
    "nasm",
    "node_modules",
    "scripts",
    "test_icon",
    "TinyRetroPad",
}


def check_pe_header(file_path: Path) -> bool:
    try:
        with open(file_path, "rb") as f:
            header = f.read(2)
            if header != b"MZ":
                return False
            f.seek(0x3C)
            pe_offset_bytes = f.read(4)
            if len(pe_offset_bytes) < 4:
                return False
            pe_offset = int.from_bytes(pe_offset_bytes, byteorder="little")
            f.seek(pe_offset)
            pe_sig = f.read(4)
            return pe_sig == b"PE\x00\x00"
    except Exception:
        return False


def run_native_smoke_tests():
    app_dirs = [
        d for d in WORKSPACE_ROOT.iterdir()
        if d.is_dir() and d.name not in EXCLUDED_DIRS and not d.name.startswith(".")
    ]
    app_dirs.sort(key=lambda d: d.name.lower())

    total = 0
    passed = 0
    missing_exes = []
    size_violations = []
    header_errors = []

    print(f"Scanning {len(app_dirs)} potential native app directories...")

    for app_dir in app_dirs:
        main_c = app_dir / "main.c"
        build_bat = app_dir / "build.bat"

        if not main_c.exists() and not build_bat.exists():
            continue

        total += 1
        app_name = app_dir.name
        exe_file = app_dir / f"{app_name}.exe"

        # Check case variations if needed
        if not exe_file.exists():
            candidates = list(app_dir.glob("*.exe"))
            if candidates:
                exe_file = candidates[0]
            else:
                missing_exes.append(app_name)
                sys.stdout.write("M")
                continue

        size = exe_file.stat().st_size
        if size > MAX_EXE_SIZE_BYTES:
            size_violations.append((app_name, size))
            sys.stdout.write("V")
            continue

        if not check_pe_header(exe_file):
            header_errors.append(app_name)
            sys.stdout.write("H")
            continue

        passed += 1
        sys.stdout.write(".")

    print(f"\n\n--- Native Smoke Test Summary ---")
    print(f"Total Native Apps Discovered: {total}")
    print(f"Passed Checks:                {passed}")
    print(f"Missing Executables:          {len(missing_exes)}")
    print(f"Size Limit (>999KB) Breaches: {len(size_violations)}")
    print(f"Corrupt / Invalid PE Headers: {len(header_errors)}")

    if missing_exes:
        print("\nMissing Executables (need compilation):")
        for m in missing_exes:
            print(f"  - {m}")

    if size_violations:
        print("\nSize Violations (> 999 KB):")
        for name, sz in size_violations:
            print(f"  - {name}: {sz:,} bytes")

    if header_errors:
        print("\nInvalid PE Headers:")
        for h in header_errors:
            print(f"  - {h}")

    if size_violations or header_errors:
        print("\nFAILURE: Size violations or corrupt binaries found.")
        sys.exit(1)
    else:
        print("\nSUCCESS: All compiled native binaries satisfy size and integrity constraints.")


if __name__ == "__main__":
    run_native_smoke_tests()
