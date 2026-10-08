#!/usr/bin/env python3
"""
A/B Protocol Comparison Script
Compares git commit quality between two authors (protocol ON vs legacy)
since the 'protocol-ab-test-start' tag.

Outputs:
  1. Console summary
  2. Retro 1999-style HTML report at KiloOS/public/web/ab-report.html
     (also copied to dist/web/ for immediate serving)

Designed to run daily via Windows Task Scheduler.
"""

import subprocess
import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

# --- Configuration ---
REPO_DIR = r"c:\KiloApps\KiloApps"
BASELINE_TAG = "protocol-ab-test-start"

# Author identities — use email substrings unique to each machine
# (git --author does substring matching, so "sdgdvs" alone matches both)
AUTHOR_PROTOCOL = "protonmail"         # This machine — anonymous2 <sdgdvs@protonmail.com>
AUTHOR_LEGACY   = "users.noreply"      # Other machine — sdgdvs <sdgdvs@users.noreply.github.com>

OUTPUT_DIR_PUBLIC = os.path.join(REPO_DIR, "KiloOS", "public", "web")
OUTPUT_DIR_DIST   = os.path.join(REPO_DIR, "KiloOS", "dist", "web")
OUTPUT_FILENAME   = "ab-report.html"

# --- Git helpers ---

def run_git(args: list[str]) -> str:
    """Run a git command in the repo and return stdout."""
    result = subprocess.run(
        ["git"] + args,
        cwd=REPO_DIR,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode != 0:
        print(f"git error: {result.stderr.strip()}", file=sys.stderr)
    return result.stdout.strip()


def get_commits_since_tag(author: str) -> list[dict]:
    """Get commits by author since the baseline tag."""
    log_format = "%H||%s||%ai||%an"
    raw = run_git([
        "log",
        f"{BASELINE_TAG}..HEAD",
        f"--author={author}",
        f"--format={log_format}",
    ])
    if not raw:
        return []

    commits = []
    for line in raw.splitlines():
        parts = line.split("||", 3)
        if len(parts) < 4:
            continue
        sha, subject, date_str, name = parts
        commits.append({
            "sha": sha,
            "subject": subject,
            "date": date_str.strip(),
            "author": name.strip(),
        })
    return commits


def get_diffstat(sha: str) -> dict:
    """Get insertions, deletions, and files changed for a commit."""
    raw = run_git(["diff", "--shortstat", f"{sha}~1", sha])
    # Example: " 3 files changed, 45 insertions(+), 12 deletions(-)"
    stat = {"files": 0, "insertions": 0, "deletions": 0}
    if not raw:
        return stat
    for token in raw.split(","):
        token = token.strip()
        if "file" in token:
            stat["files"] = int(token.split()[0])
        elif "insertion" in token:
            stat["insertions"] = int(token.split()[0])
        elif "deletion" in token:
            stat["deletions"] = int(token.split()[0])
    return stat


def get_files_changed(sha: str) -> list[str]:
    """Get list of files changed in a commit."""
    raw = run_git(["diff-tree", "--no-commit-id", "-r", "--name-only", sha])
    return [f for f in raw.splitlines() if f.strip()] if raw else []


def compute_churn(all_files: list[list[str]]) -> dict:
    """Count how many times each file appears across commits."""
    counts: dict[str, int] = {}
    for file_list in all_files:
        for f in file_list:
            counts[f] = counts.get(f, 0) + 1
    multi_touch = {f: c for f, c in counts.items() if c > 1}
    return multi_touch


# --- Analysis ---

def analyze_author(author: str) -> dict:
    """Full analysis for one author."""
    commits = get_commits_since_tag(author)
    if not commits:
        return {
            "author": author,
            "commit_count": 0,
            "commits": [],
            "avg_files": 0,
            "avg_insertions": 0,
            "avg_deletions": 0,
            "total_insertions": 0,
            "total_deletions": 0,
            "churn_files": {},
            "churn_score": 0,
            "commit_types": {},
        }

    all_stats = []
    all_files = []
    commit_types: dict[str, int] = {}

    for c in commits:
        stat = get_diffstat(c["sha"])
        files = get_files_changed(c["sha"])
        c["stat"] = stat
        c["files"] = files
        all_stats.append(stat)
        all_files.append(files)

        # Parse conventional commit type from subject
        subj = c["subject"]
        ctype = subj.split("(")[0].split(":")[0].strip() if "(" in subj or ":" in subj else "other"
        commit_types[ctype] = commit_types.get(ctype, 0) + 1

    n = len(commits)
    churn = compute_churn(all_files)

    return {
        "author": author,
        "commit_count": n,
        "commits": commits,
        "avg_files": round(sum(s["files"] for s in all_stats) / n, 1),
        "avg_insertions": round(sum(s["insertions"] for s in all_stats) / n, 1),
        "avg_deletions": round(sum(s["deletions"] for s in all_stats) / n, 1),
        "total_insertions": sum(s["insertions"] for s in all_stats),
        "total_deletions": sum(s["deletions"] for s in all_stats),
        "churn_files": churn,
        "churn_score": sum(churn.values()) - len(churn),  # excess touches
        "commit_types": commit_types,
    }


# --- HTML Report ---

def generate_html(protocol_data: dict, legacy_data: dict) -> str:
    """Generate a glorious 1999 Geocities-style comparison report."""
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    def metric_row(label, val_p, val_l, lower_is_better=False):
        """Generate a table row with winner highlighting."""
        if val_p == val_l or val_p == 0 or val_l == 0:
            color_p, color_l = "#00ffcc", "#00ffcc"
        elif lower_is_better:
            color_p = "#00ff00" if val_p < val_l else "#ff4444"
            color_l = "#00ff00" if val_l < val_p else "#ff4444"
        else:
            color_p = "#00ff00" if val_p > val_l else "#ff4444"
            color_l = "#00ff00" if val_l > val_p else "#ff4444"
        return f"""<tr>
  <td style="text-align:left; padding:6px; border:1px solid #00ffff;">{label}</td>
  <td style="text-align:center; padding:6px; border:1px solid #00ffff; color:{color_p}; font-weight:bold;">{val_p}</td>
  <td style="text-align:center; padding:6px; border:1px solid #00ffff; color:{color_l}; font-weight:bold;">{val_l}</td>
</tr>"""

    def commit_log_html(data: dict) -> str:
        if not data["commits"]:
            return "<p><i>No commits yet.</i></p>"
        rows = ""
        for c in data["commits"][:15]:  # cap at 15 most recent
            s = c.get("stat", {})
            rows += f"""<tr>
  <td style="padding:4px; border:1px solid #333; font-size:11px; color:#aaa;">{c['date'][:10]}</td>
  <td style="padding:4px; border:1px solid #333; font-size:11px;">{c['subject'][:80]}</td>
  <td style="padding:4px; border:1px solid #333; font-size:11px; color:#00ff00;">+{s.get('insertions',0)}</td>
  <td style="padding:4px; border:1px solid #333; font-size:11px; color:#ff4444;">-{s.get('deletions',0)}</td>
</tr>"""
        return f"""<table style="width:100%; border-collapse:collapse; margin:8px 0;">
<tr style="background:#1a0033;">
  <th style="padding:4px; border:1px solid #00ffff; font-size:11px;">Date</th>
  <th style="padding:4px; border:1px solid #00ffff; font-size:11px;">Subject</th>
  <th style="padding:4px; border:1px solid #00ffff; font-size:11px;">+</th>
  <th style="padding:4px; border:1px solid #00ffff; font-size:11px;">-</th>
</tr>
{rows}
</table>"""

    def churn_html(data: dict) -> str:
        churn = data["churn_files"]
        if not churn:
            return "<p style='color:#00ff00;'>&#10003; Zero churn detected. Clean.</p>"
        sorted_churn = sorted(churn.items(), key=lambda x: -x[1])[:10]
        rows = ""
        for f, count in sorted_churn:
            rows += f"<tr><td style='padding:3px; border:1px solid #333; font-size:11px;'>{f}</td><td style='padding:3px; border:1px solid #333; font-size:11px; color:#ffcc00;'>{count}x</td></tr>"
        return f"""<table style="border-collapse:collapse; margin:8px 0;">
<tr><th style="padding:3px; border:1px solid #00ffff; font-size:11px;">File</th><th style="padding:3px; border:1px solid #00ffff; font-size:11px;">Touches</th></tr>
{rows}
</table>"""

    # Determine overall winner
    p_score, l_score = 0, 0
    if protocol_data["commit_count"] > legacy_data["commit_count"]: p_score += 1
    else: l_score += 1
    if protocol_data["churn_score"] < legacy_data["churn_score"]: p_score += 1
    else: l_score += 1
    if protocol_data["total_insertions"] > legacy_data["total_insertions"]: p_score += 1
    else: l_score += 1

    if protocol_data["commit_count"] == 0 and legacy_data["commit_count"] == 0:
        verdict = "&#9203; NO DATA YET — Check back after both machines have run their scheduled tasks."
        verdict_color = "#ffcc00"
    elif p_score > l_score:
        verdict = "&#9989; PROTOCOL wins on automated metrics."
        verdict_color = "#00ff00"
    elif l_score > p_score:
        verdict = "&#9888; LEGACY leads on automated metrics. Protocol may be adding friction."
        verdict_color = "#ff4444"
    else:
        verdict = "&#9878; TIE — no clear winner on automated metrics."
        verdict_color = "#ffcc00"

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="300">
<title>~*~ A/B Protocol Showdown ~*~</title>
<style>
  body {{
    background-color: #000000;
    color: #00ffcc;
    font-family: 'Comic Sans MS', 'Chalkboard SE', 'Courier New', sans-serif;
    margin: 0; padding: 16px;
    background-image: radial-gradient(white, rgba(255,255,255,.2) 2px, transparent 40px),
                      radial-gradient(white, rgba(255,255,255,.15) 1px, transparent 30px);
    background-size: 550px 550px, 350px 350px;
    background-position: 0 0, 40px 60px;
  }}
  .page-card {{
    max-width: 900px; margin: 0 auto;
    border: 3px dashed #ff00ff; padding: 16px;
    background: rgba(10, 5, 25, 0.92);
    box-shadow: 0 0 25px #ff00ff;
  }}
  h1 {{
    font-size: 24px; color: #ff00ff; text-align: center;
    text-shadow: 0 0 8px #ff00ff, 0 0 16px #00ffff;
    margin: 0 0 4px 0; letter-spacing: 2px;
  }}
  h2 {{
    font-size: 18px; color: #00ffff;
    text-shadow: 0 0 6px #00ffff;
    border-bottom: 1px solid #00ffff; padding-bottom: 4px;
    margin-top: 20px;
  }}
  .banner {{
    background: repeating-linear-gradient(45deg, #ffcc00, #ffcc00 15px, #000 15px, #000 30px);
    color: #000; font-weight: bold; font-family: Arial, sans-serif;
    padding: 6px; text-align: center; border: 2px solid #fff;
    margin: 10px 0; font-size: 13px; text-shadow: 1px 1px 0 #fff;
  }}
  .verdict {{
    text-align: center; font-size: 20px; padding: 12px;
    border: 2px solid {verdict_color}; margin: 16px 0;
    color: {verdict_color};
    text-shadow: 0 0 10px {verdict_color};
  }}
  table {{ border-collapse: collapse; }}
  .metrics-table {{ width: 100%; margin: 10px 0; }}
  .metrics-table th {{
    padding: 8px; border: 1px solid #00ffff;
    background: #1a0033; color: #ff00ff; font-size: 13px;
  }}
  .section-box {{
    border: 2px inset #00ffff; padding: 12px;
    background: rgba(0,0,30,0.5); margin: 10px 0;
  }}
  .visitor-counter {{
    text-align: center; margin-top: 16px; font-size: 11px; color: #888;
  }}
  marquee {{ color: #ffcc00; font-size: 13px; }}
</style>
</head>
<body>
<div class="page-card">

<h1>~*~ A/B PROTOCOL SHOWDOWN ~*~</h1>
<p style="text-align:center; font-size:12px; color:#aaa;">
  Anti-Vibe-Coding Protocol vs. Legacy Context &bull; Since: {BASELINE_TAG}
</p>

<div class="banner">&#9888; UNDER CONSTRUCTION &#9888; Auto-updated daily &#9888;</div>

<marquee scrollamount="3">
  *** LIVE A/B TEST *** Protocol Machine vs Legacy Machine *** Last updated: {now} ***
</marquee>

<div class="verdict">{verdict}</div>

<h2>&#9889; Head-to-Head Metrics</h2>
<table class="metrics-table">
<tr style="background:#1a0033;">
  <th style="text-align:left;">Metric</th>
  <th>&#9889; Protocol</th>
  <th>&#128190; Legacy</th>
</tr>
{metric_row("Commits", protocol_data['commit_count'], legacy_data['commit_count'])}
{metric_row("Avg files/commit", protocol_data['avg_files'], legacy_data['avg_files'])}
{metric_row("Avg insertions/commit", protocol_data['avg_insertions'], legacy_data['avg_insertions'])}
{metric_row("Avg deletions/commit", protocol_data['avg_deletions'], legacy_data['avg_deletions'])}
{metric_row("Total lines added", protocol_data['total_insertions'], legacy_data['total_insertions'])}
{metric_row("Total lines removed", protocol_data['total_deletions'], legacy_data['total_deletions'])}
{metric_row("Churn score (lower=better)", protocol_data['churn_score'], legacy_data['churn_score'], lower_is_better=True)}
</table>

<h2>&#9889; Protocol Machine — Commit Log</h2>
<div class="section-box">
{commit_log_html(protocol_data)}
</div>

<h2>&#128190; Legacy Machine — Commit Log</h2>
<div class="section-box">
{commit_log_html(legacy_data)}
</div>

<h2>&#128260; Churn Analysis (files edited multiple times)</h2>
<div class="section-box">
  <h3 style="color:#ff00ff; font-size:14px; margin:0 0 6px 0;">Protocol Machine</h3>
  {churn_html(protocol_data)}
  <h3 style="color:#ff00ff; font-size:14px; margin:12px 0 6px 0;">Legacy Machine</h3>
  {churn_html(legacy_data)}
</div>

<div class="visitor-counter">
  <img src="data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7" width="1" height="1" alt="">
  You are visitor #<span id="vc">{hash(now) % 9000 + 1000}</span> &bull; Best viewed in Netscape Navigator 4.0
</div>

<p style="text-align:center; font-size:10px; color:#555; margin-top:12px;">
  Auto-generated by ab_compare.py &bull; {now}
</p>

</div>
</body>
</html>"""


# --- Main ---

def main():
    print(f"[ab_compare] Running at {datetime.now()}")
    print(f"[ab_compare] Repo: {REPO_DIR}")
    print(f"[ab_compare] Baseline tag: {BASELINE_TAG}")
    print()

    # Verify tag exists
    tag_check = run_git(["rev-parse", BASELINE_TAG])
    if not tag_check:
        print(f"ERROR: Tag '{BASELINE_TAG}' not found. Run: git tag {BASELINE_TAG} HEAD", file=sys.stderr)
        sys.exit(1)

    # Analyze both authors
    print(f"[ab_compare] Analyzing '{AUTHOR_PROTOCOL}' (protocol)...")
    protocol = analyze_author(AUTHOR_PROTOCOL)

    print(f"[ab_compare] Analyzing '{AUTHOR_LEGACY}' (legacy)...")
    legacy = analyze_author(AUTHOR_LEGACY)

    # Print console summary
    print()
    print("=" * 60)
    print("  A/B PROTOCOL COMPARISON REPORT")
    print("=" * 60)
    for label, data in [("PROTOCOL", protocol), ("LEGACY", legacy)]:
        print(f"\n  [{label}] {data['author']}")
        print(f"    Commits:          {data['commit_count']}")
        print(f"    Avg files/commit: {data['avg_files']}")
        print(f"    Avg +/-:          +{data['avg_insertions']} / -{data['avg_deletions']}")
        print(f"    Total +/-:        +{data['total_insertions']} / -{data['total_deletions']}")
        print(f"    Churn score:      {data['churn_score']}")
        if data['commit_types']:
            types_str = ", ".join(f"{k}:{v}" for k, v in sorted(data['commit_types'].items(), key=lambda x: -x[1]))
            print(f"    Commit types:     {types_str}")
    print()

    # Generate HTML report
    html = generate_html(protocol, legacy)

    for out_dir in [OUTPUT_DIR_PUBLIC, OUTPUT_DIR_DIST]:
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, OUTPUT_FILENAME)
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(html)
        print(f"[ab_compare] Wrote: {out_path}")

    print("[ab_compare] Done.")


if __name__ == "__main__":
    main()
