#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# ///
"""
KiloApps Anti-Platform-Lock-in & Git Mirror Sync Utility

Protects the project against platform-level outages, fraudulent DMCA strikes,
or account flags on GitHub by providing:
1. One-click offline full-repository backup bundles (.bundle).
2. Dual-push multi-remote configuration (GitHub + GitLab / Codeberg / Self-Hosted Git).
3. Automated push sync across all configured remotes.
"""

import argparse
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

REPO_ROOT = Path(__file__).resolve().parent.parent


def run_git(args: list[str], check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["git"] + args,
        cwd=str(REPO_ROOT),
        text=True,
        capture_output=True,
        check=check
    )


def create_backup_bundle(output_file: str | None = None) -> int:
    """Creates a standalone git bundle containing all branches and tags."""
    if not output_file:
        output_file = str(REPO_ROOT / "kiloapps_full_backup.bundle")
    
    print(f"[mirror-sync] Generating full repository offline bundle -> {output_file} ...")
    try:
        res = run_git(["bundle", "create", output_file, "--all"])
        if res.returncode == 0:
            size_mb = Path(output_file).stat().st_size / (1024 * 1024)
            print(f"[mirror-sync] ✅ Backup bundle created successfully! ({size_mb:.2f} MB)")
            print(f"[mirror-sync] To clone/restore from this bundle: git clone {output_file} KiloApps-Restored")
            return 0
        else:
            print(f"[mirror-sync] ❌ Error creating bundle: {res.stderr}")
            return 1
    except Exception as e:
        print(f"[mirror-sync] ❌ Failed: {e}")
        return 1


def show_remotes() -> None:
    """Displays current git remotes and push URLs."""
    res = run_git(["remote", "-v"], check=False)
    print("=== Configured Git Remotes ===")
    print(res.stdout if res.stdout else "(No remotes configured)")


def add_dual_push_remote(url: str, remote_name: str = "origin") -> int:
    """Configures git to push to an additional backup remote simultaneously whenever 'git push' runs."""
    print(f"[mirror-sync] Adding backup push target '{url}' to remote '{remote_name}'...")
    
    # First, ensure existing push URL is registered
    res = run_git(["remote", "get-url", "--push", remote_name], check=False)
    existing_url = res.stdout.strip()
    
    if existing_url and existing_url != url:
        # Re-add existing url as push url so it's not replaced
        run_git(["remote", "set-url", "--add", "--push", remote_name, existing_url], check=False)
    
    # Add secondary backup url
    res2 = run_git(["remote", "set-url", "--add", "--push", remote_name, url], check=False)
    if res2.returncode == 0:
        print(f"[mirror-sync] ✅ Dual-push configured! Every 'git push {remote_name}' will now update both remotes simultaneously.")
        show_remotes()
        return 0
    else:
        print(f"[mirror-sync] ❌ Failed to add push remote: {res2.stderr}")
        return 1


def sync_all_remotes() -> int:
    """Pushes all branches and tags to origin."""
    print("[mirror-sync] Pushing all branches and tags across all configured push remotes...")
    try:
        res = run_git(["push", "--all", "origin"])
        print(res.stdout)
        res_tags = run_git(["push", "--tags", "origin"], check=False)
        print(res_tags.stdout)
        print("[mirror-sync] ✅ All remotes synchronized.")
        return 0
    except subprocess.CalledProcessError as e:
        print(f"[mirror-sync] ❌ Push sync failed: {e.stderr}")
        return 1


def main():
    parser = argparse.ArgumentParser(description="KiloApps Anti-Lock-in Git Mirror Sync")
    parser.add_argument("--bundle", action="store_true", help="Generate an offline git bundle archive of the entire repo")
    parser.add_argument("--bundle-output", type=str, default=None, help="Output path for bundle archive")
    parser.add_argument("--dual-push", type=str, metavar="BACKUP_URL", help="Add a secondary Git remote URL for simultaneous pushing (e.g. GitLab/Codeberg)")
    parser.add_argument("--sync", action="store_true", help="Push all branches and tags to all configured remotes")
    parser.add_argument("--status", action="store_true", help="Show current remotes and push URLs")

    args = parser.parse_args()

    if args.bundle:
        sys.exit(create_backup_bundle(args.bundle_output))
    elif args.dual_push:
        sys.exit(add_dual_push_remote(args.dual_push))
    elif args.sync:
        sys.exit(sync_all_remotes())
    elif args.status:
        show_remotes()
        sys.exit(0)
    else:
        parser.print_help()
        sys.exit(0)


if __name__ == "__main__":
    main()
