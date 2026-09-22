#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# ///
"""
KiloApps Fleet Continuous Runner
Repeats orchestrate.py every ~15 minutes, ensuring that each agent completes
its full turn before the next iteration begins.

Cadence logic:
- If a turn completes in < 15 minutes, it sleeps the remainder of the 15-minute window.
- If a turn completes in >= 15 minutes, it pauses for a brief 30-second breather,
  then immediately triggers the next agent in rotation.
"""

import argparse
import datetime
import os
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORCHESTRATE_SCRIPT = REPO_ROOT / "scripts" / "orchestrate.py"
LOG_DIR = REPO_ROOT / "logs"
LOG_FILE = LOG_DIR / "continuous_runner.log"


def log(msg: str):
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat()
    line = f"[{timestamp}] [RUNNER] {msg}"
    print(line, flush=True)
    try:
        LOG_DIR.mkdir(parents=True, exist_ok=True)
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception:
        pass


def run_continuous(interval_minutes: float = 15.0, duration_hours: float = 4.0, max_turns: int = 0):
    interval_sec = interval_minutes * 60.0
    start_time = time.time()
    end_time = start_time + (duration_hours * 3600.0) if duration_hours > 0 else float("inf")

    log("=" * 60)
    log(f"Continuous Runner initiated.")
    log(f"Target interval: {interval_minutes:.1f} minutes | Duration limit: {duration_hours:.1f} hours")
    if duration_hours > 0:
        finish_est = datetime.datetime.now() + datetime.timedelta(hours=duration_hours)
        log(f"Active window expires at approx: {finish_est.strftime('%Y-%m-%d %H:%M:%S')}")
    log("=" * 60)

    turn_count = 0

    while True:
        now = time.time()
        if now >= end_time:
            log(f"Specified run duration ({duration_hours:.1f}h) elapsed. Shutting down gracefully.")
            break

        turn_count += 1
        if max_turns > 0 and turn_count > max_turns:
            log(f"Specified max turns ({max_turns}) completed. Stopping.")
            break

        turn_start = time.time()
        log(f"--- Launching Fleet Turn #{turn_count} ---")

        # Execute orchestrate.py
        try:
            res = subprocess.run(
                [sys.executable, str(ORCHESTRATE_SCRIPT)],
                cwd=str(REPO_ROOT),
                text=True,
            )
            exit_code = res.returncode
        except Exception as e:
            log(f"Turn #{turn_count} execution error: {e}")
            exit_code = -1

        turn_duration = time.time() - turn_start
        log(f"Turn #{turn_count} finished in {turn_duration:.1f}s (code: {exit_code}).")

        # Check remaining time in current window
        time_left_in_cadence = interval_sec - turn_duration
        if time_left_in_cadence > 0:
            sleep_sec = max(30.0, time_left_in_cadence)
            log(f"Cadence pause: sleeping {sleep_sec:.0f}s (until {interval_minutes:.0f}m mark)...")
        else:
            sleep_sec = 30.0
            log(f"Turn took {turn_duration / 60.0:.1f}m (>= {interval_minutes:.0f}m). Pausing 30s breather before next turn...")

        # If sleeping past end_time, clamp
        remaining_duration = end_time - time.time()
        if remaining_duration <= 0:
            log("Session duration reached. Ending runner.")
            break
        if sleep_sec > remaining_duration:
            sleep_sec = remaining_duration

        try:
            time.sleep(sleep_sec)
        except KeyboardInterrupt:
            log("Interrupted by user (SIGINT). Exiting.")
            break


def main():
    parser = argparse.ArgumentParser(description="KiloApps Continuous Runner Loop")
    parser.add_argument("--interval", type=float, default=15.0, help="Cadence interval in minutes (default: 15)")
    parser.add_argument("--hours", type=float, default=4.0, help="Total duration to run in hours (default: 4, 0 for infinite)")
    parser.add_argument("--max-turns", type=int, default=0, help="Maximum turns to execute (default: 0 = unlimited)")
    args = parser.parse_args()

    run_continuous(interval_minutes=args.interval, duration_hours=args.hours, max_turns=args.max_turns)


if __name__ == "__main__":
    main()
