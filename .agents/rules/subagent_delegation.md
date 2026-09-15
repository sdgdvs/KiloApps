# Subagent Delegation & Model Selection Rules

## Core Mandate: No Opus Subagents

When using `invoke_subagent` to delegate work to subagents:
1. **Never use `Model: "inherit"`** or leave `Model` unspecified.
2. **Always explicitly set `Model: "flash"`**.
3. **Fallback to `Model: "sonnet"`** only if `flash` encounters a capacity error (e.g. HTTP 503), rate limit, or model unavailability.

Claude Opus (Director) and other high-tier models must NEVER spawn subagents using their own model. Spawning Opus subagents for routine tasks, summarizations, file inspections, audits, and test runs exhausts the token budget and rate limits rapidly.

## Prepackaged Subagents (`self` and `research`)

Antigravity provides prepackaged subagents:
- `self`: Normally inherits the parent configuration and model by default. **YOU MUST OVERRIDE THIS DEFAULT** by explicitly passing `Model: "flash"`.
- `research`: Read-only codebase explorer. **YOU MUST OVERRIDE THIS DEFAULT** by explicitly passing `Model: "flash"` (or `Model: "sonnet"` if flash fails).

## Concrete Tool Invocation Format

Primary invocation:
```json
{
  "Subagents": [
    {
      "TypeName": "self",
      "Role": "Subagent Worker",
      "Prompt": "<actionable task description>",
      "Model": "flash"
    }
  ]
}
```

Fallback invocation (if flash returns an error):
```json
{
  "Subagents": [
    {
      "TypeName": "self",
      "Role": "Subagent Worker",
      "Prompt": "<actionable task description>",
      "Model": "sonnet"
    }
  ]
}
```

## Task Scoping Rules
- Code audits, bug fixes, UI audits, test runs, linting, and report summaries belong strictly to `flash` or `sonnet`.
- High-tier models (Claude Opus) are strictly reserved for parent orchestration, strategic review, and top-level decision synthesis.
