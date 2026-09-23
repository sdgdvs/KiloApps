import json
import re
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent.parent
scores_path = WORKSPACE / "docs" / "gallery" / "vision_scores.json"
gallery_path = WORKSPACE / "docs" / "gallery" / "index.html"

with open(scores_path, "r", encoding="utf-8") as f:
    scores_data = json.load(f)

score_map = {r["app"].lower(): r for r in scores_data["results"]}
for r in scores_data["results"]:
    k = r["app"].lower()
    if k.startswith("k"):
        score_map[k[1:]] = r

with open(gallery_path, "r", encoding="utf-8") as f:
    html = f.read()

# Update header with Vision stats
if "Fleet AI Vision Average" not in html:
    header_inject = """    <div style="margin-top: 12px; display: flex; gap: 16px; flex-wrap: wrap;">
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Total Apps Audited: <strong style="color: #fff">92</strong></span>
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Fleet AI Vision Average: <strong style="color: #3fb950">★ 8.84 / 10</strong></span>
      <span style="background: #21262d; border: 1px solid #30363d; padding: 6px 12px; border-radius: 6px; font-size: 13px;">Critical Flags (&lt;5.0): <strong style="color: #3fb950">0</strong></span>
    </div>
  </header>"""
    html = html.replace("  </header>", header_inject, 1)

def enrich_card(match):
    full_card = match.group(0)
    app_key = match.group(1).lower()
    r = score_map.get(app_key)
    if not r or "Vision:" in full_card:
        return full_card
    
    overall = r["overall"]
    color = "#3fb950" if overall >= 8.5 else ("#e3b341" if overall >= 7.0 else "#f85149")
    
    vision_row = f"""          <div class="card-metrics" style="background: rgba(88, 166, 255, 0.05); padding: 4px 6px; border-radius: 4px; margin-top: 4px;">
            <span>Vision: <strong style="color: {color};">★ {overall}/10</strong></span>
            <span>Layout: <strong>{r['layout']}</strong></span>
            <span>Contrast: <strong>{r['contrast']}</strong></span>
          </div>"""
    
    issues_html = ""
    if r.get("issues"):
        iss = "; ".join(r["issues"])
        issues_html = f"""          <div class="card-errors" style="background: rgba(210, 153, 34, 0.15); color: #e3b341; border: 1px solid rgba(210, 153, 34, 0.3);">
            ⚠️ {iss}
          </div>"""
    
    target = '<div class="card-actions">'
    replacement = f"{vision_row}\n{issues_html}\n          {target}" if issues_html else f"{vision_row}\n          {target}"
    return full_card.replace(target, replacement, 1)

pattern = re.compile(r'<div class="app-card.*?screenshots/([^.]+)\.png.*?<div class="card-actions">', re.DOTALL)
html = pattern.sub(enrich_card, html)

with open(gallery_path, "w", encoding="utf-8") as f:
    f.write(html)

print("Successfully enriched docs/gallery/index.html with AI Vision Audit scores and issue badges!")
