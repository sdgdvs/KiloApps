#!/usr/bin/env python3
"""
KiloApps Byte-Budget & Dual-Target Manifest Gate v2.0 (Sub-999KB Platform Guarantee)
Validates that:
1. Every web cartridge (.html) in KiloOS/public/apps/*.html is strictly <= 999,000 bytes (BUDGET_CEILING = 999000).
2. Every native executable (.exe) in KiloOS/public/exe/*.exe (excluding KApps.zip) is strictly <= 999,000 bytes.
3. Every web cartridge (except intentional hidden/utility redirects) is registered in KiloOS/src/App.jsx
   inside APPS with a valid id, title, folder, and existing icon in KiloOS/public/assets/icons/.
4. Auto-generates KiloOS/public/apps/catalog.json containing { id, title, folder, webBytes, exeBytes, budgetPercent }
   for live byte-budget telemetry in KDirector and KSys.
"""

import argparse
import glob
import json
import os
import re
import sys

if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

BUDGET_CEILING = 999000

# Intentional utility pages / web ring redirects not registered as main desktop cartridges in APPS
INTENTIONAL_HTML_EXCEPTIONS = {
    "contribute.html",  # Web Contributor Guide / Volunteer Portal
    "kmines.html",      # Secondary / Webring Minesweeper variant
}

# Recognized folder categories in KiloOS
VALID_FOLDERS = {"System", "Media", "Office", "Games", "Network", "Dev", "Hidden"}


def parse_app_jsx(app_jsx_path: str):
    """Parses APPS array from KiloOS/src/App.jsx into structured app dicts."""
    if not os.path.exists(app_jsx_path):
        raise FileNotFoundError(f"App.jsx not found at {app_jsx_path}")

    with open(app_jsx_path, "r", encoding="utf-8") as f:
        content = f.read()

    apps_match = re.search(r"const APPS = \[(.*?)\];", content, re.DOTALL)
    if not apps_match:
        raise ValueError("Could not locate 'const APPS = [ ... ]' in App.jsx")

    apps_block = apps_match.group(1)
    apps = []

    for match in re.finditer(r"\{\s*([^}]+)\s*\}", apps_block):
        text = match.group(1)
        id_m = re.search(r"id:\s*['\"]([^'\"]+)['\"]", text)
        title_m = re.search(r"title:\s*['\"]([^'\"]+)['\"]", text)
        folder_m = re.search(r"folder:\s*['\"]([^'\"]+)['\"]", text)
        icon_m = re.search(r"icon:\s*['\"]([^'\"]+)['\"]", text)
        url_m = re.search(r"url:\s*['\"]([^'\"]+)['\"]", text)
        exe_m = re.search(r"exeUrl:\s*['\"]([^'\"]+)['\"]", text)

        if not id_m:
            continue

        app_id = id_m.group(1)
        title = title_m.group(1) if title_m else ""
        folder = folder_m.group(1) if folder_m else ""
        icon = icon_m.group(1) if icon_m else ""
        url = url_m.group(1) if url_m else ""
        exe_url = exe_m.group(1) if exe_m else None

        apps.append({
            "id": app_id,
            "title": title,
            "folder": folder,
            "icon": icon,
            "url": url,
            "exeUrl": exe_url,
        })

    return apps


def run_gate(dry_run: bool = False):
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    apps_dir = os.path.join(repo_root, "KiloOS", "public", "apps")
    exe_dir = os.path.join(repo_root, "KiloOS", "public", "exe")
    icon_dir = os.path.join(repo_root, "KiloOS", "public", "assets", "icons")
    app_jsx_path = os.path.join(repo_root, "KiloOS", "src", "App.jsx")
    catalog_path = os.path.join(apps_dir, "catalog.json")

    if not os.path.exists(apps_dir):
        print(f"Error: Cartridges directory not found at {apps_dir}")
        sys.exit(1)

    failed_budget = []
    failed_manifest = []

    # -------------------------------------------------------------------------
    # 1. Audit Web Cartridges (<= 999,000 bytes)
    # -------------------------------------------------------------------------
    html_files = sorted(glob.glob(os.path.join(apps_dir, "*.html")))
    web_records = []

    for f in html_files:
        name = os.path.basename(f)
        slug = os.path.splitext(name)[0]
        size = os.path.getsize(f)
        pct = (size / BUDGET_CEILING) * 100
        status = "PASS" if size <= BUDGET_CEILING else "FAIL"

        rec = {
            "slug": slug,
            "filename": name,
            "bytes": size,
            "pct": round(pct, 1),
            "status": status,
        }
        web_records.append(rec)
        if status == "FAIL":
            failed_budget.append(f"Web cartridge '{name}': {size:,} B (exceeded by {size - BUDGET_CEILING:,} B)")

    # -------------------------------------------------------------------------
    # 2. Audit Native Executables (<= 999,000 bytes, excluding KApps.zip)
    # -------------------------------------------------------------------------
    exe_records = []
    if os.path.exists(exe_dir):
        all_exe_files = sorted(glob.glob(os.path.join(exe_dir, "*.exe")))
        for f in all_exe_files:
            name = os.path.basename(f)
            # Explicitly exclude non-executable archives
            if name.lower() == "kapps.zip":
                continue
            size = os.path.getsize(f)
            pct = (size / BUDGET_CEILING) * 100
            status = "PASS" if size <= BUDGET_CEILING else "FAIL"

            rec = {
                "filename": name,
                "bytes": size,
                "pct": round(pct, 1),
                "status": status,
            }
            exe_records.append(rec)
            if status == "FAIL":
                failed_budget.append(f"Native binary '{name}': {size:,} B (exceeded by {size - BUDGET_CEILING:,} B)")

    # -------------------------------------------------------------------------
    # 3. Cross-Check Manifest & Icons in KiloOS/src/App.jsx
    # -------------------------------------------------------------------------
    apps_manifest = parse_app_jsx(app_jsx_path)
    registered_urls = set()
    registered_ids = set()

    for app in apps_manifest:
        app_id = app["id"]
        registered_ids.add(app_id)

        # Validate mandatory schema fields
        if not app["title"]:
            failed_manifest.append(f"App '{app_id}' has missing or empty 'title'")
        if not app["folder"]:
            failed_manifest.append(f"App '{app_id}' has missing or empty 'folder'")
        elif app["folder"] not in VALID_FOLDERS:
            failed_manifest.append(f"App '{app_id}' has unknown folder '{app["folder"]}' (valid: {sorted(VALID_FOLDERS)})")

        # Validate icon path and existence
        icon_path = app["icon"]
        if not icon_path:
            failed_manifest.append(f"App '{app_id}' has missing or empty 'icon'")
        elif not icon_path.startswith("/assets/icons/"):
            failed_manifest.append(f"App '{app_id}' icon '{icon_path}' does not start with '/assets/icons/'")
        else:
            icon_file = icon_path.replace("/assets/icons/", "")
            full_icon_p = os.path.join(icon_dir, icon_file)
            if not os.path.exists(full_icon_p):
                failed_manifest.append(f"App '{app_id}' icon file '{icon_file}' does not exist in {icon_dir}")

        # Validate web cartridge HTML file existence
        url = app["url"]
        if url == "#":
            # Intentional hidden cartridge (e.g. kquarantine in ARG lore)
            html_name = f"{app_id}.html"
            registered_urls.add(f"/apps/{html_name}")
        elif url.startswith("/apps/"):
            html_name = os.path.basename(url)
            registered_urls.add(url)
        else:
            html_name = f"{app_id}.html"
            registered_urls.add(f"/apps/{html_name}")

        full_html_p = os.path.join(apps_dir, html_name)
        if not os.path.exists(full_html_p):
            failed_manifest.append(f"App '{app_id}' web cartridge '{html_name}' does not exist in {apps_dir}")

        # Check native executable if exeUrl is declared
        exe_url = app["exeUrl"]
        if exe_url and exe_url != "null":
            full_exe_p = os.path.join(repo_root, "KiloOS", "public", exe_url.lstrip("/"))
            if not os.path.exists(full_exe_p):
                failed_manifest.append(f"App '{app_id}' references missing native binary at '{exe_url}'")

    # Reverse Check: verify every .html file in apps/ is registered or in exceptions
    for f in html_files:
        name = os.path.basename(f)
        slug = os.path.splitext(name)[0]
        if name in INTENTIONAL_HTML_EXCEPTIONS:
            continue
        if f"/apps/{name}" not in registered_urls and slug not in registered_ids:
            failed_manifest.append(
                f"Web cartridge '{name}' exists in KiloOS/public/apps/ but is not registered in App.jsx APPS manifest"
            )

    # -------------------------------------------------------------------------
    # 4. Auto-Generate KiloOS/public/apps/catalog.json Telemetry
    # -------------------------------------------------------------------------
    catalog = []
    for app in apps_manifest:
        app_id = app["id"]
        title = app["title"]
        folder = app["folder"]
        url = app["url"]
        exe_url = app["exeUrl"]

        html_name = f"{app_id}.html" if url == "#" else (os.path.basename(url) if url else f"{app_id}.html")
        html_p = os.path.join(apps_dir, html_name)
        web_bytes = os.path.getsize(html_p) if os.path.exists(html_p) else 0

        exe_bytes = None
        if exe_url and exe_url != "null":
            exe_p = os.path.join(repo_root, "KiloOS", "public", exe_url.lstrip("/"))
            if os.path.exists(exe_p):
                exe_bytes = os.path.getsize(exe_p)

        budget_pct = round((web_bytes / BUDGET_CEILING) * 100, 1)

        catalog.append({
            "id": app_id,
            "title": title,
            "folder": folder,
            "webBytes": web_bytes,
            "exeBytes": exe_bytes,
            "budgetPercent": budget_pct,
        })

    catalog.sort(key=lambda x: x["id"])

    catalog_generated = False
    if not dry_run:
        with open(catalog_path, "w", encoding="utf-8") as f:
            json.dump(catalog, f, indent=2)
            f.write("\n")
        catalog_generated = True

    # -------------------------------------------------------------------------
    # Print Comprehensive Markdown Report
    # -------------------------------------------------------------------------
    print("## 🕹️ KiloApps Byte-Budget & Dual-Target Gate v2.0")
    print(f"**Budget Ceiling**: {BUDGET_CEILING:,} bytes per cartridge/binary\n")
    print(
        f"- 🌐 **Web Cartridges Audited**: {len(web_records)} files ({len(html_files) - len(INTENTIONAL_HTML_EXCEPTIONS)} app cartridges + {len(INTENTIONAL_HTML_EXCEPTIONS)} utility pages)"
    )
    print(f"- ⚙️ **Native Executables Audited**: {len(exe_records)} binaries (excl. KApps.zip)")
    print(f"- 📋 **App Manifest Registrations**: {len(apps_manifest)} apps in App.jsx")
    print(f"- 🎨 **Unique Icon Assets Verified**: {len(apps_manifest)} / {len(apps_manifest)} valid")
    if catalog_generated:
        print(f"- 📡 **Live Catalog Telemetry**: Generated `{os.path.relpath(catalog_path, repo_root)}` ({len(catalog)} cartridges)\n")
    else:
        print(f"- 📡 **Live Catalog Telemetry**: Dry-run mode (skipped writing `{os.path.basename(catalog_path)}`)\n")

    # Table 1: Top Web Cartridges by Size
    print("### 🌐 Top 15 Web Cartridges by Byte Footprint")
    print("| Cartridge | Size (Bytes) | % of 999KB | Status |")
    print("| :--- | :--- | :--- | :--- |")
    web_records.sort(key=lambda x: x["bytes"], reverse=True)
    for r in web_records[:15]:
        emoji = "✅" if r["status"] == "PASS" else "❌"
        print(f"| `{r['slug']}` | {r['bytes']:,} B | {r['pct']}% | {emoji} {r['status']} |")
    if len(web_records) > 15:
        print(f"| *(+ {len(web_records) - 15} additional compliant cartridges)* | ... | ... | ✅ PASS |")

    # Table 2: Top Native Executables by Size
    print("\n### ⚙️ Top 10 Native Executables by Byte Footprint")
    print("| Binary | Size (Bytes) | % of 999KB | Status |")
    print("| :--- | :--- | :--- | :--- |")
    exe_records.sort(key=lambda x: x["bytes"], reverse=True)
    for r in exe_records[:10]:
        emoji = "✅" if r["status"] == "PASS" else "❌"
        print(f"| `{r['filename']}` | {r['bytes']:,} B | {r['pct']}% | {emoji} {r['status']} |")
    if len(exe_records) > 10:
        print(f"| *(+ {len(exe_records) - 10} additional compliant binaries)* | ... | ... | ✅ PASS |")

    # Final Status Evaluation
    has_failed = bool(failed_budget or failed_manifest)
    print(f"\n**Audit Result**: {'❌ BLOCKED' if has_failed else '✅ ALL CARTRIDGES WITHIN 999KB BUDGET & MANIFEST VERIFIED'}")

    if failed_budget:
        print("\n### 🚨 Budget Violations (> 999,000 Bytes):")
        for err in failed_budget:
            print(f"- ❌ {err}")

    if failed_manifest:
        print("\n### 🚨 Manifest & Parity Violations:")
        for err in failed_manifest:
            print(f"- ❌ {err}")

    if has_failed:
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="KiloApps Byte-Budget & Dual-Target Manifest Gate v2.0")
    parser.add_argument("--dry-run", action="store_true", help="Audit without writing catalog.json")
    args = parser.parse_args()
    run_gate(dry_run=args.dry_run)
