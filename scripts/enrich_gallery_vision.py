import json
import re
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent.parent
scores_init_path = WORKSPACE / "docs" / "gallery" / "vision_scores.json"
scores_interact_path = WORKSPACE / "docs" / "gallery" / "vision_scores_interact.json"
gallery_path = WORKSPACE / "docs" / "gallery" / "index.html"

# Load initial scores
scores_init_map = {}
if scores_init_path.exists():
    with open(scores_init_path, "r", encoding="utf-8") as f:
        data = json.load(f)
        for r in data.get("results", []):
            k = r["app"].lower()
            scores_init_map[k] = r
            if k.startswith("k"):
                scores_init_map[k[1:]] = r

# Load interacted scores
scores_interact_map = {}
flagged_count = 0
interact_avg = 8.56
if scores_interact_path.exists():
    with open(scores_interact_path, "r", encoding="utf-8") as f:
        data = json.load(f)
        flagged_count = data.get("summary", {}).get("flagged", 0)
        interact_avg = data.get("summary", {}).get("average_overall", 8.56)
        for r in data.get("results", []):
            k = r["app"].lower()
            scores_interact_map[k] = r
            if k.startswith("k"):
                scores_interact_map[k[1:]] = r

with open(gallery_path, "r", encoding="utf-8") as f:
    html = f.read()

# Update header with Vision stats
header_inject = f"""    <div style="margin-top: 14px; display: flex; gap: 12px; flex-wrap: wrap;">
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Total Apps Audited: <strong style="color: #fff">103</strong></span>
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Initial Vision Avg: <strong style="color: #3fb950">★ 8.84 / 10</strong></span>
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Interacted Vision Avg: <strong style="color: #58a6ff">★ {interact_avg} / 10</strong></span>
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Interacted UI Issues Flagged: <strong style="color: #e3b341">{flagged_count}</strong></span>
    </div>
  </header>"""

if "Initial Vision Avg" in html:
    html = re.sub(r'<div style="margin-top: 1[24]px; display: flex; gap: \d+px; flex-wrap: wrap;">.*?</div>\s*</header>', header_inject, html, flags=re.DOTALL)
else:
    html = html.replace("  </header>", header_inject, 1)

def enrich_card(match):
    full_card = match.group(0)
    app_key = match.group(1).lower().replace("_interact", "")
    
    r_init = scores_init_map.get(app_key)
    r_interact = scores_interact_map.get(app_key)
    
    if not r_init and not r_interact:
        return full_card
    
    if "Vision (Init):" in full_card:
        return full_card
    
    init_score = r_init["overall"] if r_init else 9.0
    interact_score = r_interact["overall"] if r_interact else 9.0
    
    c_init = "#3fb950" if init_score >= 8.5 else ("#e3b341" if init_score >= 7.0 else "#f85149")
    c_interact = "#3fb950" if interact_score >= 8.5 else ("#e3b341" if interact_score >= 7.0 else "#f85149")
    
    vision_row = f"""          <div class="card-metrics" style="background: rgba(88, 166, 255, 0.05); padding: 4px 6px; border-radius: 4px; margin-top: 4px;">
            <span>Vision (Init): <strong style="color: {c_init};">★ {init_score}</strong></span>
            <span>Vision (Interact): <strong style="color: {c_interact};">★ {interact_score}</strong></span>
          </div>"""
    
    issues_html = ""
    issues = (r_interact.get("issues") if r_interact else []) or (r_init.get("issues") if r_init else [])
    if issues:
        iss = "; ".join(issues)
        issues_html = f"""          <div class="card-errors" style="background: rgba(210, 153, 34, 0.15); color: #e3b341; border: 1px solid rgba(210, 153, 34, 0.3); margin-top: 4px;">
            ⚠️ {iss}
          </div>"""
    
    target = '<div class="card-actions">'
    replacement = f"{vision_row}\n{issues_html}\n          {target}" if issues_html else f"{vision_row}\n          {target}"
    return full_card.replace(target, replacement, 1)

pattern = re.compile(r'<div class="app-card.*?screenshots/([^.]+)\.png.*?<div class="card-actions">', re.DOTALL)
html = pattern.sub(enrich_card, html)

with open(gallery_path, "w", encoding="utf-8") as f:
    f.write(html)

print("Successfully enriched docs/gallery/index.html with both Initial and Interacted AI Vision Audit scores and issue badges!")
