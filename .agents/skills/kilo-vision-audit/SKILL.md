---
name: kilo-vision-audit
description: >-
  AI-powered visual quality audit using the agent's native vision capabilities.
  Reads app screenshots from docs/gallery/screenshots/, scores them on layout/contrast/completeness/consistency/functionality,
  and outputs structured results to docs/gallery/vision_scores.json.
  No API keys or external dependencies required — uses the agent's built-in image understanding via view_file.
---

# KiloApps Vision Quality Audit Skill

This skill performs AI-powered visual quality scoring of KiloApps web application screenshots using the agent's own vision capabilities (via `view_file` on PNG files). It replaces the standalone `scripts/vision_audit.py` which required a `GEMINI_API_KEY`.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Confirm screenshots exist in `docs/gallery/screenshots/`. If not, run the headless test suite first:
   ```
   node scripts/test_web_apps.js
   ```
3. Open [next_work.md](../../next_work.md) to identify context (optional — this skill can be run standalone).

## Audit Procedure

### 1. Discover Screenshots
- List all `*.png` files in `docs/gallery/screenshots/`.
- **Exclude** files ending in `_interact.png` (post-interaction screenshots are supplementary).
- Sort alphabetically for deterministic ordering.

### 2. Batch Processing
- Process screenshots in batches of **10 per turn** to stay within token budget.
- Track progress in `docs/gallery/vision_audit_progress.json`:
  ```json
  { "last_batch_end": "kfarm", "completed": ["k2048", "kabyss", ...] }
  ```
- On subsequent turns, resume from where the previous batch ended.
- If all screenshots have been processed, regenerate the full report.

### 3. Visual Evaluation (Per Screenshot)
For each screenshot, use `view_file` to examine the image, then evaluate on these 5 dimensions (1-10 scale):

- **LAYOUT (1-10):** Is content properly arranged? No overlapping elements? Readable text? Proper spacing?
- **CONTRAST (1-10):** Can you read all text? Are interactive elements visually distinct from background?
- **COMPLETENESS (1-10):** Does the UI look finished? Are there blank areas that should have content? Are labels present?
- **CONSISTENCY (1-10):** Does the visual style match a coherent retro/90s theme?
- **FUNCTIONALITY (1-10):** Based on visible UI elements, does this look like a working, interactive application?

Calculate **overall** as the average of all 5 scores, rounded to 1 decimal place.

List any specific **issues** as short strings (e.g., "text overlaps sidebar", "blank area in bottom-right", "buttons have no visible borders").

### 4. Output Format
Write results to `docs/gallery/vision_scores.json` with this exact schema (backward-compatible with the legacy `vision_audit.py` output):

```json
{
  "timestamp": "2026-09-21T10:00:00Z",
  "model": "agent-native-vision",
  "threshold": 5.0,
  "summary": {
    "total": 95,
    "flagged": 3,
    "average_overall": 7.2
  },
  "results": [
    {
      "app": "ksnake",
      "layout": 8,
      "contrast": 7,
      "completeness": 9,
      "consistency": 8,
      "functionality": 9,
      "overall": 8.2,
      "issues": []
    }
  ]
}
```

### 5. Baseline Comparison (Optional)
If a previous `vision_scores.json` exists, load it before overwriting and compute deltas:
- For each app, compare `overall` score to previous run.
- Log improvements (↑) and regressions (↓) in the console output.
- Apps that dropped ≥1.0 points should be flagged for review.

### 6. Flagging & Threshold
- Default threshold: **5.0** (configurable via task instructions).
- Any app scoring below the threshold is flagged.
- Print a summary table to console showing all scores and flags.

### 7. Queue Handoff & Logging
- Edit [next_work.md](../../next_work.md):
  - In the Execution Log section, append a terse entry (≤8 lines):
    ```
    ### Agent Run Log — <timestamp> (Vision Audit)
    - **Status:** 🟢 Completed (Vision Audit Batch N)
    - Scored N apps. Fleet avg: X.X/10. Flagged: Y below 5.0.
    - Regressions: [list apps that dropped ≥1.0 if any].
    ```

### 8. Commit and Push
- `git add docs/gallery/vision_scores.json docs/gallery/vision_audit_progress.json next_work.md`
- `git commit -m "audit(vision): AI visual quality scores for batch N"`
- `git push` (if push fails, `git pull --rebase` then push again).
- STOP after one batch. Resume on next turn.

## Important Notes
- **No API key required.** This skill uses the agent's built-in vision capabilities.
- **No Python dependencies.** No `uv`, no `google-genai`, no `pip install`.
- **Model selection:** When the orchestrator invokes this skill, use `Model: "flash"` per fleet policy.
- The standalone `scripts/vision_audit.py` is preserved as an optional human-only tool for users who prefer to run vision audits outside the agent system.
