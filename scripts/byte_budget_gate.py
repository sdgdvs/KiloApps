#!/usr/bin/env python3
"""
KiloApps Byte-Budget Enforcement Gate (Sub-999KB Platform Guarantee)
Validates that all web cartridges and native assets strictly adhere to <= 999,000 bytes.
Outputs a markdown summary table for PR comments and CI logs.
"""
import os
import glob
import sys

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

BUDGET_CEILING = 999000

def run_gate():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    apps_dir = os.path.join(root, "KiloOS", "public", "apps")
    
    if not os.path.exists(apps_dir):
        print(f"Error: Cartridges directory not found at {apps_dir}")
        sys.exit(1)
        
    html_files = sorted(glob.glob(os.path.join(apps_dir, "*.html")))
    records = []
    failed = []
    
    for f in html_files:
        name = os.path.basename(f)
        slug = os.path.splitext(name)[0]
        size = os.path.getsize(f)
        pct = (size / BUDGET_CEILING) * 100
        status = "PASS" if size <= BUDGET_CEILING else "FAIL"
        
        record = {
            "slug": slug,
            "filename": name,
            "bytes": size,
            "pct": round(pct, 1),
            "status": status
        }
        records.append(record)
        if status == "FAIL":
            failed.append(record)
            
    print("## 🕹️ KiloApps Byte-Budget Gate Report")
    print(f"**Budget Ceiling**: 999,000 bytes per cartridge | **Total Cartridges Audited**: {len(records)}\n")
    print("| Cartridge | Size (Bytes) | % of 999KB | Status |")
    print("| :--- | :--- | :--- | :--- |")
    
    records.sort(key=lambda x: x["bytes"], reverse=True)
    for r in records[:15]:
        emoji = "✅" if r["status"] == "PASS" else "❌"
        print(f"| `{r['slug']}` | {r['bytes']:,} B | {r['pct']}% | {emoji} {r['status']} |")
        
    if len(records) > 15:
        print(f"| *(+ {len(records) - 15} additional compliant cartridges)* | ... | ... | ✅ PASS |")
        
    print(f"\n**Audit Result**: {'❌ BLOCKED' if failed else '✅ ALL CARTRIDGES WITHIN 999KB BUDGET'}")
    
    if failed:
        print("\n### 🚨 Budget Violations:")
        for f in failed:
            print(f"- `{f['slug']}`: {f['bytes']:,} bytes (exceeded by {f['bytes'] - BUDGET_CEILING:,} bytes)")
        sys.exit(1)

if __name__ == "__main__":
    run_gate()
