#!/usr/bin/env python3
"""
Alias wrapper for scripts/byte_budget_gate.py so agents invoking
`python scripts/check_sizes.py` (per AGENTS.md) succeed seamlessly.
"""
import subprocess
import sys
from pathlib import Path

if __name__ == "__main__":
    script = Path(__file__).resolve().parent / "byte_budget_gate.py"
    res = subprocess.run([sys.executable, str(script)] + sys.argv[1:])
    sys.exit(res.returncode)
