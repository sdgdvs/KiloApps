#!/usr/bin/env python3
"""
Prunes old Firebase Hosting versions for site 'kiloapps' to prevent HTTP 429
RESOURCE_EXHAUSTED storage quota failures on the Firebase Spark plan.

Supports two authentication modes:
1. GitHub Actions CI: reads `FIREBASE_SERVICE_ACCOUNT` env var (JSON string) and
   exchanges an RS256 JWT assertion (signed via `openssl`) for a Google OAuth2 token.
2. Local Workstation: reads `~/.config/configstore/firebase-tools.json` access token.
"""

import argparse
import base64
import concurrent.futures
import json
import os
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

SITE_ID = "kiloapps"
API_BASE = f"https://firebasehosting.googleapis.com/v1beta1/sites/{SITE_ID}"


def _b64url(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("ascii")


def get_token_from_service_account(sa_json_str: str) -> str:
    sa = json.loads(sa_json_str)
    now = int(time.time())
    header = _b64url(json.dumps({"alg": "RS256", "typ": "JWT"}, separators=(",", ":")).encode("utf-8"))
    claims = _b64url(
        json.dumps(
            {
                "iss": sa["client_email"],
                "scope": "https://www.googleapis.com/auth/firebase https://www.googleapis.com/auth/cloud-platform",
                "aud": sa.get("token_uri", "https://oauth2.googleapis.com/token"),
                "iat": now,
                "exp": now + 3600,
            },
            separators=(",", ":"),
        ).encode("utf-8")
    )
    signing_input = f"{header}.{claims}".encode("ascii")

    with tempfile.NamedTemporaryFile("w", suffix=".pem", delete=False, encoding="utf-8") as kf:
        kf.write(sa["private_key"])
        key_path = kf.name

    try:
        res = subprocess.run(
            ["openssl", "dgst", "-sha256", "-sign", key_path],
            input=signing_input,
            capture_output=True,
            check=True,
        )
        sig = _b64url(res.stdout)
    finally:
        try:
            os.remove(key_path)
        except OSError:
            pass

    jwt_assertion = f"{header}.{claims}.{sig}"
    body = urllib.parse.urlencode(
        {
            "grant_type": "urn:ietf:params:oauth:grant-type:jwt-bearer",
            "assertion": jwt_assertion,
        }
    ).encode("utf-8")
    req = urllib.request.Request(
        sa.get("token_uri", "https://oauth2.googleapis.com/token"),
        data=body,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=15) as resp:
        return json.loads(resp.read().decode("utf-8"))["access_token"]


def get_access_token() -> str:
    sa_env = os.environ.get("FIREBASE_SERVICE_ACCOUNT", "").strip()
    if sa_env:
        return get_token_from_service_account(sa_env)

    cfg_path = Path.home() / ".config" / "configstore" / "firebase-tools.json"
    if cfg_path.exists():
        cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
        token = cfg.get("tokens", {}).get("access_token", "")
        if token:
            return token

    raise RuntimeError("No Firebase credentials found in FIREBASE_SERVICE_ACCOUNT or ~/.config/configstore/firebase-tools.json")


def get_live_version_name(headers: dict) -> str:
    req = urllib.request.Request(f"{API_BASE}/channels/live", headers=headers)
    with urllib.request.urlopen(req, timeout=15) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        return data.get("release", {}).get("version", {}).get("name", "")


def list_all_versions(headers: dict, max_pages: int = 50) -> list[dict]:
    versions = []
    page_token = ""
    for _ in range(max_pages):
        url = f"{API_BASE}/versions?pageSize=100"
        if page_token:
            url += f"&pageToken={urllib.parse.quote(page_token)}"
        req = urllib.request.Request(url, headers=headers)
        with urllib.request.urlopen(req, timeout=20) as resp:
            data = json.loads(resp.read().decode("utf-8"))
        batch = data.get("versions", [])
        versions.extend(batch)
        page_token = data.get("nextPageToken", "")
        if not page_token or not batch:
            break
    return versions


def delete_version(ver_name: str, headers: dict) -> tuple[str, bool, str]:
    url = f"https://firebasehosting.googleapis.com/v1beta1/{ver_name}"
    for attempt in range(3):
        req = urllib.request.Request(url, headers=headers, method="DELETE")
        try:
            with urllib.request.urlopen(req, timeout=20) as _:
                return ver_name, True, ""
        except urllib.error.HTTPError as e:
            if e.code == 429 and attempt < 2:
                time.sleep(1.5 * (attempt + 1))
                continue
            return ver_name, False, f"HTTP {e.code}"
        except Exception as e:
            return ver_name, False, str(e)
    return ver_name, False, "retries_exhausted"


def main():
    parser = argparse.ArgumentParser(description="Prune old Firebase Hosting versions.")
    parser.add_argument("--keep", type=int, default=3, help="Number of recent finalized versions to keep (default: 3)")
    parser.add_argument("--max-pages", type=int, default=50, help="Max 100-item pages to scan (default: 50)")
    parser.add_argument("--max-delete", type=int, default=500, help="Max versions to delete in one run")
    parser.add_argument("--workers", type=int, default=8, help="Concurrent delete workers")
    parser.add_argument("--dry-run", action="store_true", help="List versions without deleting")
    args = parser.parse_args()

    try:
        token = get_access_token()
    except Exception as e:
        print(f"[firebase-prune] Warning: Could not obtain access token: {e}")
        sys.exit(0)

    headers = {"Authorization": f"Bearer {token}"}
    try:
        live_ver = get_live_version_name(headers)
        print(f"[firebase-prune] Active live version: {live_ver}")
        all_versions = list_all_versions(headers, max_pages=args.max_pages)
    except Exception as e:
        print(f"[firebase-prune] Warning: Failed to list Hosting versions: {e}")
        sys.exit(0)

    total_bytes = sum(int(v.get("versionBytes", 0)) for v in all_versions if v.get("status") != "DELETED")
    active_versions = [v for v in all_versions if v.get("status") not in ("DELETED",)]
    print(
        f"[firebase-prune] Scanned {len(all_versions)} version records "
        f"({len(active_versions)} non-deleted, ~{total_bytes / (1024 * 1024):.1f} MB)."
    )

    # Sort newest first by createTime
    active_versions.sort(key=lambda v: v.get("createTime", ""), reverse=True)

    keep_names = {live_ver} if live_ver else set()
    finalized_kept = 0
    for v in active_versions:
        vname = v.get("name", "")
        if v.get("status") == "FINALIZED":
            if vname in keep_names or finalized_kept < args.keep:
                keep_names.add(vname)
                finalized_kept += 1

    to_delete = [
        v for v in active_versions
        if v.get("name") not in keep_names and v.get("status") in ("FINALIZED", "ABANDONED", "CREATED", "EXPIRED")
    ][: args.max_delete]

    reclaim_bytes = sum(int(v.get("versionBytes", 0)) for v in to_delete)
    print(
        f"[firebase-prune] Keeping {len(keep_names)} recent/live version(s); "
        f"selecting {len(to_delete)} older version(s) for deletion (~{reclaim_bytes / (1024 * 1024):.1f} MB)."
    )

    if args.dry_run or not to_delete:
        return

    deleted = 0
    failed = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = [pool.submit(delete_version, v["name"], headers) for v in to_delete]
        for fut in concurrent.futures.as_completed(futures):
            _, ok, _ = fut.result()
            if ok:
                deleted += 1
            else:
                failed += 1

    print(f"[firebase-prune] Completed: deleted {deleted} version(s), failed {failed} (~{reclaim_bytes / (1024 * 1024):.1f} MB targeted).")


if __name__ == "__main__":
    main()
