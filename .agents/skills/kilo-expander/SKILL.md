---
name: kilo-expander
description: >-
  Executes deep functional feature expansions for productivity, system, dev, and media applications.
  Use this skill to deepen utility logic, add export/import formats, advanced data manipulation,
  diagnostic depth, verify file sizes (<999 KB) and builds, log tersely to next_work.md,
  advance the queue handoff, and commit/push.
---

# KiloApps Feature Expander Skill

This skill deepens functional utility and capabilities on exactly ONE application per turn.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the expander target (`current_targets.kilo_expander`).

## Expansion Directives by Category
1. **🛠️ System & Dev Tools** (*KTerm, KSys, KTask, KNet, KPing, KHex, KBase, KConverter, KCalc, KScript, KZip, KFont*):
   - Focus: Diagnostic depth, granular controls, syntax parsing, advanced regex filtering, hex inspection, performance analysis.
2. **📝 Productivity & Data** (*KPad, KNote, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass*):
   - Focus: Data interoperability, multi-tab sessions, tagging, search indexing, schema flexibility, export formats (CSV, JSON, Markdown).
3. **🎨 Media & Creative** (*KPaint, KImage, KAudio, KSynth, KMedia, KChart, KGraph, KMandel, KType*):
   - Focus: Format support, DSP/audio synthesis (ADSR envelopes, waveforms), image processing filters, canvas layers.
4. **🎮 Games (Meta & Engine Utility Only)**:
   - Focus: Replay viewers, custom key rebinding, save state management, PGN/FEN/board state import/export.
   - **DO NOT** add gameplay content, bosses, or campaigns (reserved for `kilo-graphics`).
5. **Maturity & Skip Protocol**:
   - If an app has undergone 6+ passes and is functionally complete without active requests: log `⏭️ Skip — app is feature-complete and mature.` Rotate to queue bottom and finish cleanly.

## Verification
1. Verify web app build: `cd KiloOS && npm run build`.
2. Verify size constraint: `< 999 KB`.

## Queue Handoff & Terse Logging (CRITICAL)
1. **Edit [next_work.md](../../next_work.md)**:
   - Advance `current_targets.kilo_expander` to next app in queue.
   - Set `current_agent` to next scheduled agent in `agent_rotation`.
   - Set `status: ready`.
   - Update `last_run.agent: kilo-expander`, `last_run.app: <TargetApp>`, and `last_run.timestamp`.
   - Append terse run log (strict limit: ≤ 8 lines of concise bullet points).
2. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "feat(expand): feature expansion for <TargetApp>"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
