#!/usr/bin/env python3
"""
KiloApps Autonomous Fleet Dashboard
Lightweight graphical control panel for managing KiloApps creation turns.
Features:
- "Start Kiloapps creation turns" / "Stop Kiloapps creation turns"
- Real-time status display (Scheduled task state, 24h countdown, turn counter, next run time)
- Collision window guard monitor (Safe Window :00-:32 vs Remote Fleet :32-:58)
- Resource freeing: Instant task kill on stop
- Windows Autostart management (Startup folder shortcut)
- Auto-starts creation turns on login by default
"""

import datetime
import json
import os
import subprocess
import sys
import threading
import time
from pathlib import Path
import tkinter as tk
from tkinter import messagebox, ttk

# Repository paths
REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG_FILE = REPO_ROOT / ".agents" / "dashboard_config.json"
SESSION_FILE = REPO_ROOT / ".agents" / "scheduler_session.json"
FLEET_NODES_DIR = REPO_ROOT / ".agents" / "fleet_nodes"
LOCK_FILE = REPO_ROOT / ".agents" / ".orchestrator.lock"
NEXT_WORK_FILE = REPO_ROOT / "next_work.md"
ICON_PATH = REPO_ROOT / "KiloOS" / "public" / "assets" / "icons" / "ksys.ico"

TASK_NAME = "KiloApps-Fleet-Orchestrator"
CREATE_NO_WINDOW = 0x08000000 if sys.platform == "win32" else 0

# Multi-PC Node Profiles: Each computer has distinct, collision-free dispatch minutes
NODE_PROFILES = {
    "pc_a": {
        "node_id": "pc_a",
        "name": "PC A (anonymous1)",
        "account": "anonymous1",
        "hostname_hint": "12900K",
        "minutes": [2, 17],
        "safe_window": ":00 - :20",
        "is_safe": lambda m: 0 <= m <= 20,
        "collision_desc": "PC B (:30-:50) & PC C (:20-:30, :50-:60)",
    },
    "pc_b": {
        "node_id": "pc_b",
        "name": "PC B (sdgdvs)",
        "account": "sdgdvs",
        "hostname_hint": "PC-B",
        "minutes": [32, 47],
        "safe_window": ":30 - :50",
        "is_safe": lambda m: 30 <= m <= 50,
        "collision_desc": "PC A (:00-:20) & PC C (:20-:30, :50-:60)",
    },
    "pc_c": {
        "node_id": "pc_c",
        "name": "PC C (anonymous2)",
        "account": "anonymous2",
        "hostname_hint": "PC-C",
        "minutes": [22, 52],
        "safe_window": ":20 - :30 & :50 - :60",
        "is_safe": lambda m: (20 <= m <= 30) or (50 <= m <= 60),
        "collision_desc": "PC A (:00-:20) & PC B (:30-:50)",
    },
}


def get_current_node_id() -> str:
    """Detects which physical PC / account is running this dashboard."""
    if SESSION_FILE.exists():
        try:
            s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8-sig"))
            if s_data.get("node_id") in NODE_PROFILES:
                return s_data["node_id"]
        except Exception:
            pass

    if CONFIG_FILE.exists():
        try:
            cfg = json.loads(CONFIG_FILE.read_text(encoding="utf-8-sig"))
            if cfg.get("node_id") in NODE_PROFILES:
                return cfg["node_id"]
        except Exception:
            pass

    try:
        res = subprocess.run(["git", "config", "user.name"], cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=2)
        if res.returncode == 0:
            uname = res.stdout.strip().lower()
            if "sdgdvs" in uname:
                return "pc_b"
            if "anonymous2" in uname:
                return "pc_c"
            if "anonymous1" in uname or "kiloapps" in uname:
                return "pc_a"
    except Exception:
        pass

    import socket
    h = socket.gethostname().upper()
    if "12900K" in h:
        return "pc_a"

    return "pc_a"


def load_fleet_nodes() -> dict:
    """Reads status files from all computers in the fleet."""
    nodes = {}
    if FLEET_NODES_DIR.exists():
        for f in FLEET_NODES_DIR.glob("node_*.json"):
            try:
                data = json.loads(f.read_text(encoding="utf-8-sig"))
                nid = data.get("node_id", f.stem.replace("node_", "pc_"))
                nodes[nid] = data
            except Exception:
                pass
    return nodes


def get_startup_folder() -> Path:
    """Returns the Windows User Startup directory."""
    appdata = os.environ.get("APPDATA")
    if appdata:
        return Path(appdata) / "Microsoft" / "Windows" / "Start Menu" / "Programs" / "Startup"
    return Path.home() / "AppData" / "Roaming" / "Microsoft" / "Windows" / "Start Menu" / "Programs" / "Startup"


def get_startup_shortcut_path() -> Path:
    return get_startup_folder() / "KiloApps Dashboard.lnk"


def get_desktop_shortcut_path() -> Path:
    userprofile = os.environ.get("USERPROFILE")
    if userprofile:
        return Path(userprofile) / "Desktop" / "KiloApps Dashboard.lnk"
    return Path.home() / "Desktop" / "KiloApps Dashboard.lnk"


def ensure_desktop_shortcut():
    """Ensures a desktop shortcut exists pointing to pythonw.exe dashboard."""
    shortcut_path = get_desktop_shortcut_path()
    if not shortcut_path.exists():
        try:
            ps_cmd = (
                f'$WshShell = New-Object -ComObject WScript.Shell; '
                f'$Shortcut = $WshShell.CreateShortcut("{shortcut_path}"); '
                f'$Shortcut.TargetPath = "{sys.executable.replace("python.exe", "pythonw.exe")}"; '
                f'$Shortcut.Arguments = \'"{Path(__file__).resolve()}\"\'; '
                f'$Shortcut.WorkingDirectory = "{REPO_ROOT}"; '
                f'$Shortcut.Description = "KiloApps Autonomous Fleet Dashboard"; '
                f'$Shortcut.IconLocation = "{ICON_PATH}"; '
                f'$Shortcut.Save()'
            )
            subprocess.run(
                ["powershell.exe", "-NoProfile", "-Command", ps_cmd],
                capture_output=True,
                text=True,
                stdin=subprocess.DEVNULL,
                creationflags=CREATE_NO_WINDOW,
            )
        except Exception:
            pass


def load_config() -> dict:
    defaults = {
        "autostart_with_windows": True,
        "autostart_turns_on_launch": False,  # Defensive default: only start on this PC if explicitly configured!
        "always_on_top": False,
        "node_id": "auto",
    }
    if CONFIG_FILE.exists():
        try:
            data = json.loads(CONFIG_FILE.read_text(encoding="utf-8-sig"))
            defaults.update(data)
        except Exception:
            pass
    return defaults


def save_config(config: dict):
    try:
        CONFIG_FILE.parent.mkdir(parents=True, exist_ok=True)
        CONFIG_FILE.write_text(json.dumps(config, indent=2), encoding="utf-8")
    except Exception:
        pass


def set_windows_autostart(enable: bool):
    """Creates or removes the Startup folder shortcut."""
    shortcut_path = get_startup_shortcut_path()
    if enable:
        try:
            ps_cmd = (
                f'$WshShell = New-Object -ComObject WScript.Shell; '
                f'$Shortcut = $WshShell.CreateShortcut("{shortcut_path}"); '
                f'$Shortcut.TargetPath = "{sys.executable.replace("python.exe", "pythonw.exe")}"; '
                f'$Shortcut.Arguments = \'"{Path(__file__).resolve()}\"\'; '
                f'$Shortcut.WorkingDirectory = "{REPO_ROOT}"; '
                f'$Shortcut.Description = "KiloApps Autonomous Fleet Dashboard"; '
                f'$Shortcut.IconLocation = "{ICON_PATH}"; '
                f'$Shortcut.Save()'
            )
            subprocess.run(
                ["powershell.exe", "-NoProfile", "-Command", ps_cmd],
                capture_output=True,
                text=True,
                stdin=subprocess.DEVNULL,
                creationflags=CREATE_NO_WINDOW,
            )
        except Exception as e:
            print(f"Failed to create startup shortcut: {e}", file=sys.stderr)
    else:
        try:
            if shortcut_path.exists():
                shortcut_path.unlink()
        except Exception as e:
            print(f"Failed to remove startup shortcut: {e}", file=sys.stderr)


def query_task_state_from_system() -> str:
    """Queries schtasks.exe safely with stdin redirected and creation flags to prevent error 0x800700E8 pipe closed."""
    try:
        res = subprocess.run(
            ["schtasks.exe", "/query", "/tn", TASK_NAME, "/fo", "csv", "/nh"],
            capture_output=True,
            text=True,
            stdin=subprocess.DEVNULL,
            creationflags=CREATE_NO_WINDOW,
            timeout=5,
        )
        if res.returncode == 0:
            lines = res.stdout.strip().splitlines()
            for line in lines:
                parts = [p.strip().strip('"') for p in line.split(",")]
                if len(parts) >= 3 and parts[2]:
                    return parts[2]
        elif res.returncode == 1:
            if "cannot find the file" in res.stderr.lower():
                return "NOT REGISTERED"
    except Exception:
        pass

    # Seamless fallback to session file state (prevents errors if RPC pipe is closed)
    if SESSION_FILE.exists():
        try:
            s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8"))
            if s_data.get("status") == "stopped":
                return "Disabled"
            return "Ready"
        except Exception:
            pass
    return "Ready"


class FleetDashboard(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("KiloApps Multi-PC Fleet Dashboard")
        self.geometry("540x820")
        self.minsize(480, 700)
        self.configure(bg="#1e1e2e")

        if ICON_PATH.exists():
            try:
                self.iconbitmap(str(ICON_PATH))
            except Exception:
                pass

        self.config_data = load_config()
        self.is_busy = False

        # Identify local node
        self.current_node_id = get_current_node_id()
        self.current_node_profile = NODE_PROFILES.get(self.current_node_id, NODE_PROFILES["pc_a"])
        self.current_node_name = self.current_node_profile["name"]

        # Telemetry query caching
        self.cached_task_state = "UNKNOWN"
        self.last_task_query_time = 0.0

        self._build_ui()
        self._apply_always_on_top()

        # Handle window closing
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        # Ensure shortcuts exist
        ensure_desktop_shortcut()

        # Initial query for task state
        self.cached_task_state = query_task_state_from_system()
        self.last_task_query_time = time.time()

        # Initial autostart turn trigger if enabled
        if self.config_data.get("autostart_turns_on_launch", False):
            self.after(500, self._auto_start_on_launch)

        # Start periodic telemetry polling (1-second ticker, 0-subprocess overhead)
        self.after(300, self.refresh_status_loop)

    def _build_ui(self):
        # Color Palette
        self.c_bg = "#1e1e2e"
        self.c_card = "#282a36"
        self.c_card_border = "#44475a"
        self.c_fg = "#f8f8f2"
        self.c_sub = "#6272a4"
        self.c_green = "#50fa7b"
        self.c_green_btn = "#2e7d32"
        self.c_green_hover = "#388e3c"
        self.c_red = "#ff5555"
        self.c_red_btn = "#c62828"
        self.c_red_hover = "#d32f2f"
        self.c_accent = "#8be9fd"
        self.c_orange = "#ffb86c"

        # Main Container
        main_frame = tk.Frame(self, bg=self.c_bg, padx=16, pady=12)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # 1. Header Frame
        hdr_frame = tk.Frame(main_frame, bg=self.c_bg)
        hdr_frame.pack(fill=tk.X, pady=(0, 6))

        title_lbl = tk.Label(
            hdr_frame,
            text="KILOAPPS FLEET DASHBOARD",
            font=("Segoe UI", 15, "bold"),
            fg=self.c_fg,
            bg=self.c_bg,
        )
        title_lbl.pack(anchor="w")

        min_desc = f":{self.current_node_profile['minutes'][0]:02d} & :{self.current_node_profile['minutes'][1]:02d}"
        subtitle_lbl = tk.Label(
            hdr_frame,
            text=f"Multi-PC Dispatcher • Local: {self.current_node_name} (Slots {min_desc})",
            font=("Segoe UI", 9),
            fg=self.c_sub,
            bg=self.c_bg,
        )
        subtitle_lbl.pack(anchor="w")

        # 2. Main Local Control Buttons Frame
        btn_frame = tk.Frame(main_frame, bg=self.c_bg)
        btn_frame.pack(fill=tk.X, pady=(4, 6))

        # Start Button (Local Machine)
        self.btn_start = tk.Button(
            btn_frame,
            text=f"▶  Start Kiloapps creation turns (This PC)",
            font=("Segoe UI", 11, "bold"),
            bg=self.c_green_btn,
            fg="#ffffff",
            activebackground=self.c_green_hover,
            activeforeground="#ffffff",
            relief=tk.FLAT,
            padx=10,
            pady=8,
            cursor="hand2",
            command=self.start_creation_turns,
        )
        self.btn_start.pack(fill=tk.X, pady=(0, 4))

        # Stop Button (Local Machine)
        self.btn_stop = tk.Button(
            btn_frame,
            text=f"⏹  Stop Kiloapps creation turns (This PC)",
            font=("Segoe UI", 11, "bold"),
            bg=self.c_red_btn,
            fg="#ffffff",
            activebackground=self.c_red_hover,
            activeforeground="#ffffff",
            relief=tk.FLAT,
            padx=10,
            pady=8,
            cursor="hand2",
            command=self.stop_creation_turns,
        )
        self.btn_stop.pack(fill=tk.X, pady=(0, 6))

        # 3. Multi-PC Fleet Network Card
        fleet_card = tk.Frame(main_frame, bg=self.c_card, highlightbackground=self.c_card_border, highlightthickness=1, padx=10, pady=8)
        fleet_card.pack(fill=tk.X, pady=(0, 8))

        flt_hdr = tk.Frame(fleet_card, bg=self.c_card)
        flt_hdr.pack(fill=tk.X, pady=(0, 4))
        tk.Label(flt_hdr, text="FLEET COMPUTERS (MULTI-PC NETWORK)", font=("Segoe UI", 8, "bold"), fg=self.c_accent, bg=self.c_card).pack(side=tk.LEFT)
        self.lbl_fleet_badge = tk.Label(flt_hdr, text=f"★ THIS PC: {self.current_node_id.upper()}", font=("Segoe UI", 8, "bold"), fg=self.c_green, bg="#313244", padx=6, pady=1)
        self.lbl_fleet_badge.pack(side=tk.RIGHT)

        self.fleet_rows = {}
        for nid in ("pc_a", "pc_b", "pc_c"):
            prof = NODE_PROFILES[nid]
            row = tk.Frame(fleet_card, bg=self.c_card)
            row.pack(fill=tk.X, pady=1)
            lbl_name = tk.Label(row, text="💻 " + prof["name"], font=("Segoe UI", 8, "bold"), fg=self.c_fg, bg=self.c_card, width=20, anchor="w")
            lbl_name.pack(side=tk.LEFT)
            lbl_stat = tk.Label(row, text="CHECKING...", font=("Segoe UI", 8), fg=self.c_sub, bg=self.c_card, width=20, anchor="w")
            lbl_stat.pack(side=tk.LEFT)
            lbl_time = tk.Label(row, text="--", font=("Segoe UI", 8), fg=self.c_sub, bg=self.c_card, anchor="e")
            lbl_time.pack(side=tk.RIGHT, fill=tk.X, expand=True)
            self.fleet_rows[nid] = {"name": lbl_name, "status": lbl_stat, "time": lbl_time}

        flt_btn_row = tk.Frame(fleet_card, bg=self.c_card)
        flt_btn_row.pack(fill=tk.X, pady=(4, 0))

        self.btn_remote_stop = tk.Button(
            flt_btn_row,
            text="⏹ Request Stop on Other PCs",
            font=("Segoe UI", 8),
            bg="#383a59",
            fg="#ff5555",
            relief=tk.FLAT,
            padx=6,
            pady=2,
            command=self.stop_all_remote_pcs,
        )
        self.btn_remote_stop.pack(side=tk.LEFT)

        self.btn_git_sync = tk.Button(
            flt_btn_row,
            text="🔄 Git Sync Fleet",
            font=("Segoe UI", 8),
            bg="#383a59",
            fg=self.c_fg,
            relief=tk.FLAT,
            padx=6,
            pady=2,
            command=self.sync_fleet_via_git,
        )
        self.btn_git_sync.pack(side=tk.RIGHT)

        # 3. Telemetry Card Frame
        card = tk.Frame(main_frame, bg=self.c_card, highlightbackground=self.c_card_border, highlightthickness=1, padx=12, pady=10)
        card.pack(fill=tk.X, pady=(0, 10))

        # Status Badge Row
        badge_row = tk.Frame(card, bg=self.c_card)
        badge_row.pack(fill=tk.X, pady=(0, 6))

        tk.Label(badge_row, text="ENGINE STATUS:", font=("Segoe UI", 9, "bold"), fg=self.c_sub, bg=self.c_card).pack(side=tk.LEFT)
        self.lbl_status = tk.Label(badge_row, text="CHECKING...", font=("Segoe UI", 10, "bold"), fg=self.c_orange, bg=self.c_card)
        self.lbl_status.pack(side=tk.RIGHT)

        # Separator line
        tk.Frame(card, bg=self.c_card_border, height=1).pack(fill=tk.X, pady=4)

        # Metric grid
        grid_frame = tk.Frame(card, bg=self.c_card)
        grid_frame.pack(fill=tk.X)
        grid_frame.columnconfigure(1, weight=1)

        def add_metric_row(parent, row, label_text):
            lbl_key = tk.Label(parent, text=label_text, font=("Segoe UI", 9), fg="#bd93f9", bg=self.c_card)
            lbl_key.grid(row=row, column=0, sticky="w", pady=2)
            lbl_val = tk.Label(parent, text="--", font=("Segoe UI", 9, "bold"), fg=self.c_fg, bg=self.c_card)
            lbl_val.grid(row=row, column=1, sticky="e", pady=2)
            return lbl_val

        self.lbl_next_run = add_metric_row(grid_frame, 0, "Next Scheduled Run:")
        self.lbl_window = add_metric_row(grid_frame, 1, "Fleet Window Alignment:")
        self.lbl_session = add_metric_row(grid_frame, 2, "24h Session Remaining:")
        self.lbl_turns = add_metric_row(grid_frame, 3, "Turns Executed (Today):")
        self.lbl_target = add_metric_row(grid_frame, 4, "Active Fleet Target:")

        # 4. Activity Log Box
        log_header = tk.Frame(main_frame, bg=self.c_bg)
        log_header.pack(fill=tk.X, pady=(4, 2))
        tk.Label(log_header, text="ACTIVITY & TELEMETRY", font=("Segoe UI", 8, "bold"), fg=self.c_sub, bg=self.c_bg).pack(side=tk.LEFT)

        self.txt_log = tk.Text(
            main_frame,
            height=6,
            bg="#11111b",
            fg="#a6adc8",
            insertbackground="#ffffff",
            font=("Consolas", 8),
            relief=tk.FLAT,
            padx=8,
            pady=6,
            wrap=tk.WORD,
        )
        self.txt_log.pack(fill=tk.BOTH, expand=True, pady=(0, 6))

        # 5. Preferences & Settings Frame
        pref_frame = tk.Frame(main_frame, bg=self.c_bg)
        pref_frame.pack(fill=tk.X, pady=(2, 6))

        self.var_autostart_win = tk.BooleanVar(value=self.config_data.get("autostart_with_windows", True))
        self.chk_autostart_win = tk.Checkbutton(
            pref_frame,
            text="Autostart with Windows",
            variable=self.var_autostart_win,
            command=self._on_toggle_autostart_win,
            font=("Segoe UI", 8),
            fg=self.c_fg,
            bg=self.c_bg,
            selectcolor="#313244",
            activebackground=self.c_bg,
            activeforeground=self.c_fg,
        )
        self.chk_autostart_win.pack(side=tk.LEFT, padx=(0, 10))

        self.var_autostart_turns = tk.BooleanVar(value=self.config_data.get("autostart_turns_on_launch", True))
        self.chk_autostart_turns = tk.Checkbutton(
            pref_frame,
            text="Auto-start turns on login",
            variable=self.var_autostart_turns,
            command=self._on_toggle_autostart_turns,
            font=("Segoe UI", 8),
            fg=self.c_fg,
            bg=self.c_bg,
            selectcolor="#313244",
            activebackground=self.c_bg,
            activeforeground=self.c_fg,
        )
        self.chk_autostart_turns.pack(side=tk.LEFT, padx=(0, 10))

        self.var_ontop = tk.BooleanVar(value=self.config_data.get("always_on_top", False))
        self.chk_ontop = tk.Checkbutton(
            pref_frame,
            text="Always on top",
            variable=self.var_ontop,
            command=self._on_toggle_ontop,
            font=("Segoe UI", 8),
            fg=self.c_fg,
            bg=self.c_bg,
            selectcolor="#313244",
            activebackground=self.c_bg,
            activeforeground=self.c_fg,
        )
        self.chk_ontop.pack(side=tk.RIGHT)

        # 6. Bottom Actions Bar
        bot_bar = tk.Frame(main_frame, bg=self.c_bg)
        bot_bar.pack(fill=tk.X)

        self.btn_refresh = tk.Button(
            bot_bar,
            text="↻ Refresh",
            font=("Segoe UI", 8),
            bg=self.c_card,
            fg=self.c_fg,
            relief=tk.FLAT,
            padx=8,
            pady=3,
            command=lambda: self.refresh_status(force_query=True),
        )
        self.btn_refresh.pack(side=tk.LEFT)

        self.btn_manual_turn = tk.Button(
            bot_bar,
            text="⚡ Run Single Turn Now",
            font=("Segoe UI", 8),
            bg=self.c_card,
            fg=self.c_accent,
            relief=tk.FLAT,
            padx=8,
            pady=3,
            command=self.run_manual_turn,
        )
        self.btn_manual_turn.pack(side=tk.RIGHT)

    def log(self, msg: str):
        timestamp = datetime.datetime.now().strftime("%H:%M:%S")
        line = f"[{timestamp}] {msg}\n"
        self.txt_log.insert(tk.END, line)
        self.txt_log.see(tk.END)

    def _auto_start_on_launch(self):
        self.log("Auto-start on login enabled. Ensuring creation turns are scheduled...")
        self.start_creation_turns(silent=True)

    def _apply_always_on_top(self):
        self.attributes("-topmost", self.var_ontop.get())

    def _on_toggle_ontop(self):
        self.config_data["always_on_top"] = self.var_ontop.get()
        save_config(self.config_data)
        self._apply_always_on_top()

    def _on_toggle_autostart_win(self):
        val = self.var_autostart_win.get()
        self.config_data["autostart_with_windows"] = val
        save_config(self.config_data)
        set_windows_autostart(val)
        if val:
            self.log("Autostart with Windows enabled (Startup shortcut created).")
        else:
            self.log("Autostart with Windows disabled (Startup shortcut removed).")

    def _on_toggle_autostart_turns(self):
        val = self.var_autostart_turns.get()
        self.config_data["autostart_turns_on_launch"] = val
        save_config(self.config_data)
        self.log(f"Auto-start turns on login set to: {val}")

    def on_close(self):
        self.destroy()

    # --- Actions ---

    def start_creation_turns(self, silent: bool = False):
        if self.is_busy:
            return
        self.is_busy = True
        self.btn_start.configure(state=tk.DISABLED)
        self.log(f"Activating KiloApps creation turns on {self.current_node_name} (24h session armed)...")

        def task():
            try:
                # 1. Run register_task.ps1 via PowerShell with -NodeId and -Hours 24
                ps_script = REPO_ROOT / "scripts" / "register_task.ps1"
                res = subprocess.run(
                    [
                        "powershell.exe",
                        "-NoProfile",
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        str(ps_script),
                        "-Hours",
                        "24",
                        "-NodeId",
                        self.current_node_id,
                    ],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                )
                if res.returncode == 0:
                    self.after(0, lambda: self.log("Windows Scheduled Task active & 24h timer set."))
                else:
                    err = res.stderr.strip() or res.stdout.strip()
                    self.after(0, lambda: self.log(f"Registration warning: {err}"))

                # 2. Ensure task is enabled
                subprocess.run(
                    ["schtasks.exe", "/change", "/tn", TASK_NAME, "/enable"],
                    capture_output=True,
                    text=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                )

                # 3. Update local node json broadcast file
                node_file = FLEET_NODES_DIR / f"node_{self.current_node_id.replace('pc_', '')}.json"
                if node_file.exists():
                    try:
                        n_data = json.loads(node_file.read_text(encoding="utf-8-sig"))
                        n_data["status"] = "active"
                        n_data["remote_command"] = "none"
                        n_data["last_heartbeat"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
                        node_file.write_text(json.dumps(n_data, indent=2), encoding="utf-8")
                    except Exception:
                        pass
            except Exception as e:
                self.after(0, lambda: self.log(f"Error starting turns: {e}"))
            finally:
                self.after(0, self._finish_start)

        threading.Thread(target=task, daemon=True).start()

    def _finish_start(self):
        self.is_busy = False
        self.btn_start.configure(state=tk.NORMAL)
        self.cached_task_state = "Ready"
        self.refresh_status(force_query=True)

    def stop_creation_turns(self):
        if self.is_busy:
            return
        self.is_busy = True
        self.btn_stop.configure(state=tk.DISABLED)
        self.log(f"Stopping KiloApps creation turns on {self.current_node_name} and freeing system resources...")

        def task():
            try:
                # 1. Run stop_task.ps1 via PowerShell
                ps_script = REPO_ROOT / "scripts" / "stop_task.ps1"
                subprocess.run(
                    [
                        "powershell.exe",
                        "-NoProfile",
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        str(ps_script),
                    ],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                )

                # 2. Terminate running worker processes (agy.exe, orchestrator.py)
                subprocess.run(
                    ["taskkill.exe", "/F", "/IM", "agy.exe", "/T"],
                    capture_output=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                )
                
                # Check lockfile for active PID
                if LOCK_FILE.exists():
                    try:
                        pid = LOCK_FILE.read_text(encoding="utf-8").strip()
                        if pid:
                            subprocess.run(
                                ["taskkill.exe", "/F", "/PID", pid, "/T"],
                                capture_output=True,
                                stdin=subprocess.DEVNULL,
                                creationflags=CREATE_NO_WINDOW,
                            )
                        LOCK_FILE.unlink()
                    except Exception:
                        pass

                # 3. Update session status to stopped
                if SESSION_FILE.exists():
                    try:
                        s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8"))
                        s_data["status"] = "stopped"
                        SESSION_FILE.write_text(json.dumps(s_data, indent=2), encoding="utf-8")
                    except Exception:
                        pass

                # 4. Update local node json file
                node_file = FLEET_NODES_DIR / f"node_{self.current_node_id.replace('pc_', '')}.json"
                if node_file.exists():
                    try:
                        n_data = json.loads(node_file.read_text(encoding="utf-8-sig"))
                        n_data["status"] = "stopped"
                        n_data["last_heartbeat"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
                        node_file.write_text(json.dumps(n_data, indent=2), encoding="utf-8")
                    except Exception:
                        pass

                self.after(0, lambda: self.log("⏹ Turns stopped. Scheduled task disabled. All resources freed."))
            except Exception as e:
                self.after(0, lambda: self.log(f"Error stopping turns: {e}"))
            finally:
                self.after(0, self._finish_stop)

        threading.Thread(target=task, daemon=True).start()

    def _finish_stop(self):
        self.is_busy = False
        self.btn_stop.configure(state=tk.NORMAL)
        self.cached_task_state = "Disabled"
        self.refresh_status(force_query=True)

    def stop_all_remote_pcs(self):
        other_nodes = [nid for nid in ("pc_a", "pc_b", "pc_c") if nid != self.current_node_id]
        other_names = ", ".join([NODE_PROFILES[nid]["name"] for nid in other_nodes])
        confirm = messagebox.askyesno(
            "Request Stop on Remote Fleet PCs",
            f"Send a voluntary stop command to remote nodes ({other_names}) via git?\n\n"
            f"When those computers run git pull, their orchestrator will honor the stop command, "
            f"disable their scheduled task, and free system resources.\n\n"
            f"Proceed?",
        )
        if not confirm:
            return

        self.log(f"Broadcasting stop command to remote nodes ({other_names})...")

        def task():
            try:
                for nid in other_nodes:
                    node_key = nid.replace("pc_", "")
                    node_file = FLEET_NODES_DIR / f"node_{node_key}.json"
                    if node_file.exists():
                        try:
                            data = json.loads(node_file.read_text(encoding="utf-8-sig"))
                            data["remote_command"] = "stop"
                            node_file.write_text(json.dumps(data, indent=2), encoding="utf-8")
                        except Exception:
                            pass

                # Stage, commit, and push via git
                subprocess.run(["git", "add", ".agents/fleet_nodes/"], cwd=str(REPO_ROOT), capture_output=True, creationflags=CREATE_NO_WINDOW)
                c_res = subprocess.run(
                    ["git", "commit", "-m", "chore(fleet): request remote stop across fleet nodes"],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                    creationflags=CREATE_NO_WINDOW,
                )
                if c_res.returncode == 0:
                    p_res = subprocess.run(["git", "push"], cwd=str(REPO_ROOT), capture_output=True, text=True, creationflags=CREATE_NO_WINDOW)
                    if p_res.returncode == 0:
                        self.after(0, lambda: self.log("✓ Remote stop request committed and pushed to git."))
                    else:
                        self.after(0, lambda: self.log(f"Git push warning: {p_res.stderr.strip()}"))
                else:
                    self.after(0, lambda: self.log("Fleet nodes already marked for stop or no changes."))
            except Exception as e:
                self.after(0, lambda: self.log(f"Error requesting remote stop: {e}"))
            finally:
                self.after(0, lambda: self.refresh_status(force_query=True))

        threading.Thread(target=task, daemon=True).start()

    def sync_fleet_via_git(self):
        self.log("Syncing fleet telemetry with remote repository (git pull --rebase)...")

        def task():
            try:
                res = subprocess.run(
                    ["git", "pull", "--rebase"],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                    timeout=20,
                )
                if res.returncode == 0:
                    self.after(0, lambda: self.log("✓ Fleet telemetry synced with remote repository."))
                else:
                    err = res.stderr.strip() or res.stdout.strip()
                    self.after(0, lambda: self.log(f"Git sync warning: {err}"))
            except Exception as e:
                self.after(0, lambda: self.log(f"Git sync error: {e}"))
            finally:
                self.after(0, lambda: self.refresh_status(force_query=True))

        threading.Thread(target=task, daemon=True).start()

    def run_manual_turn(self):
        now = datetime.datetime.now()
        is_safe = self.current_node_profile.get("is_safe", lambda m: True)
        if not is_safe(now.minute):
            msg = self.current_node_profile.get("collision_desc", "other fleet nodes")
            messagebox.showwarning(
                "Fleet Collision Guard",
                f"Current time ({now.strftime('%H:%M')}) is in the reserved window for {msg}.\n\n"
                f"Manual dispatch is blocked to prevent git merge conflicts across computers.\n"
                f"Your node's safe dispatch window is: {self.current_node_profile.get('safe_window', 'N/A')}.",
            )
            return

        self.log(f"Launching single turn manually on {self.current_node_name} via orchestrator.bat...")
        bat_script = REPO_ROOT / "scripts" / "run_orchestrator.bat"

        def task():
            try:
                res = subprocess.run(
                    [str(bat_script)],
                    cwd=str(REPO_ROOT),
                    capture_output=True,
                    text=True,
                    stdin=subprocess.DEVNULL,
                    creationflags=CREATE_NO_WINDOW,
                )
                self.after(0, lambda: self.log(f"Manual turn completed with exit code {res.returncode}."))
            except Exception as e:
                self.after(0, lambda: self.log(f"Manual turn error: {e}"))
            finally:
                self.after(0, lambda: self.refresh_status(force_query=True))

        threading.Thread(target=task, daemon=True).start()

    # --- Telemetry & Refresh ---

    def refresh_status(self, force_query: bool = False):
        # 1. Check Task State (only query schtasks if forced or 60s elapsed)
        now_ts = time.time()
        if force_query or (now_ts - self.last_task_query_time) >= 60.0:
            self.cached_task_state = query_task_state_from_system()
            self.last_task_query_time = now_ts

        task_state = self.cached_task_state

        # 2. Check Session State
        session_active = False
        remaining_str = "No active session"
        turns_count = 0
        if SESSION_FILE.exists():
            try:
                s_data = json.loads(SESSION_FILE.read_text(encoding="utf-8-sig"))
                turns_count = s_data.get("turns_executed", 0)
                status = s_data.get("status", "unknown")
                end_str = s_data.get("session_end", "")
                if end_str:
                    if end_str.endswith("Z"):
                        end_str = end_str[:-1] + "+00:00"
                    if "." in end_str and ("+" in end_str or "-" in end_str):
                        base_dt, tz_part = end_str.split("+") if "+" in end_str else end_str.rsplit("-", 1)
                        if "." in base_dt:
                            sec_base, frac = base_dt.split(".", 1)
                            end_str = f"{sec_base}.{frac[:6]}+{tz_part}"
                    end_dt = datetime.datetime.fromisoformat(end_str)
                    now_dt = datetime.datetime.now(datetime.timezone.utc)
                    if now_dt < end_dt and status == "active":
                        session_active = True
                        diff = end_dt - now_dt
                        h = int(diff.total_seconds() // 3600)
                        m = int((diff.total_seconds() % 3600) // 60)
                        remaining_str = f"{h}h {m}m remaining"
                    elif status == "stopped":
                        remaining_str = "STOPPED by user"
                    else:
                        remaining_str = "EXPIRED (24h elapsed)"
            except Exception:
                pass

        # Update Engine Status Badge
        if task_state == "Ready" and session_active:
            self.lbl_status.configure(text="🟢 ACTIVE & SCHEDULED", fg=self.c_green)
        elif task_state == "Ready" and not session_active:
            self.lbl_status.configure(text="🟡 READY (Session Expired/Stopped)", fg=self.c_orange)
        elif task_state == "Disabled":
            self.lbl_status.configure(text="🔴 STOPPED (Resources 100% Free)", fg=self.c_red)
        else:
            self.lbl_status.configure(text=f"⚪ {task_state}", fg=self.c_sub)

        # 3. Calculate Next Run Time
        now = datetime.datetime.now()
        mins = self.current_node_profile.get("minutes", [2, 17])
        candidates = []
        for m in mins:
            t = now.replace(minute=m, second=0, microsecond=0)
            if t <= now:
                t += datetime.timedelta(hours=1)
            candidates.append(t)
        next_time = min(candidates)
        diff_next = next_time - now
        next_m = int(diff_next.total_seconds() // 60)
        next_s = int(diff_next.total_seconds() % 60)

        if task_state == "Ready" and session_active:
            self.lbl_next_run.configure(
                text=f"{next_time.strftime('%H:%M:%S')} (in {next_m}m {next_s}s)",
                fg=self.c_fg,
            )
        else:
            self.lbl_next_run.configure(text="Suspended (Task Disabled)", fg=self.c_sub)

        # 4. Window Alignment
        cur_min = now.minute
        is_safe = self.current_node_profile.get("is_safe", lambda m: True)
        if is_safe(cur_min):
            self.lbl_window.configure(
                text=f"🟢 Safe Window (:{cur_min:02d} within {self.current_node_profile.get('safe_window', '')})",
                fg=self.c_green,
            )
        else:
            self.lbl_window.configure(
                text=f"🟡 Busy Window (:{cur_min:02d} — {self.current_node_profile.get('collision_desc', 'Reserved')})",
                fg=self.c_orange,
            )

        # 5. Session & Turns
        self.lbl_session.configure(text=remaining_str, fg=self.c_fg if session_active else self.c_sub)
        self.lbl_turns.configure(text=f"{turns_count} turns executed", fg=self.c_fg)

        # 6. Active Queue Target
        if NEXT_WORK_FILE.exists():
            try:
                content = NEXT_WORK_FILE.read_text(encoding="utf-8-sig")
                current_agent = ""
                current_target = ""
                in_targets = False
                target_key = ""
                for line in content.splitlines():
                    line_str = line.strip()
                    if line_str == "---":
                        if current_agent and current_target:
                            break
                        continue
                    if line_str.startswith("current_agent:"):
                        current_agent = line_str.split(":", 1)[1].strip()
                        target_key = current_agent.replace("-", "_") + ":"
                    elif line_str.startswith("current_targets:"):
                        in_targets = True
                    elif in_targets and target_key and line_str.startswith(target_key):
                        raw_tgt = line_str.split(":", 1)[1].strip()
                        current_target = raw_tgt.strip("\"'")
                if current_agent and current_target:
                    self.lbl_target.configure(text=f"{current_agent} ➔ {current_target}", fg=self.c_accent)
                elif current_agent:
                    self.lbl_target.configure(text=current_agent, fg=self.c_accent)
                else:
                    self.lbl_target.configure(text="--", fg=self.c_sub)
            except Exception:
                self.lbl_target.configure(text="--", fg=self.c_sub)

        # 7. Update Multi-PC Fleet Network Card Rows
        fleet_data = load_fleet_nodes()
        for nid, row_widgets in self.fleet_rows.items():
            nd = fleet_data.get(nid, {})
            status = nd.get("status", "unknown").lower()
            remote_cmd = nd.get("remote_command", "none")
            sched_mins = nd.get("schedule_minutes", NODE_PROFILES.get(nid, {}).get("minutes", []))
            sched_str = f":{sched_mins[0]:02d}, :{sched_mins[1]:02d}" if len(sched_mins) >= 2 else "--"

            if nid == self.current_node_id:
                if task_state == "Ready" and session_active:
                    stat_text = f"ACTIVE ({sched_str})"
                    stat_color = self.c_green
                elif task_state == "Disabled" or status == "stopped":
                    stat_text = "STOPPED (Local)"
                    stat_color = self.c_red
                else:
                    stat_text = f"{task_state.upper()} ({sched_str})"
                    stat_color = self.c_orange
            else:
                if remote_cmd == "stop":
                    stat_text = "STOP REQUESTED"
                    stat_color = self.c_orange
                elif status == "active":
                    stat_text = f"ACTIVE ({sched_str})"
                    stat_color = self.c_green
                elif status == "stopped":
                    stat_text = "STOPPED"
                    stat_color = self.c_sub
                else:
                    stat_text = f"{status.upper()} ({sched_str})"
                    stat_color = self.c_sub

            hb_str = nd.get("last_heartbeat") or nd.get("last_turn_timestamp")
            time_text = "--"
            if hb_str:
                try:
                    if hb_str.endswith("Z"):
                        hb_str = hb_str[:-1] + "+00:00"
                    if "." in hb_str and ("+" in hb_str or "-" in hb_str):
                        base_dt, tz_part = hb_str.split("+") if "+" in hb_str else hb_str.rsplit("-", 1)
                        if "." in base_dt:
                            sec_base, frac = base_dt.split(".", 1)
                            hb_str = f"{sec_base}.{frac[:6]}+{tz_part}"
                    hb_dt = datetime.datetime.fromisoformat(hb_str)
                    now_utc = datetime.datetime.now(datetime.timezone.utc)
                    delta_sec = max(0, (now_utc - hb_dt).total_seconds())
                    if delta_sec < 60:
                        time_text = f"{int(delta_sec)}s ago"
                    elif delta_sec < 3600:
                        time_text = f"{int(delta_sec // 60)}m ago"
                    else:
                        time_text = f"{int(delta_sec // 3600)}h ago"
                except Exception:
                    time_text = "--"

            row_widgets["status"].configure(text=stat_text, fg=stat_color)
            row_widgets["time"].configure(text=time_text)

    def refresh_status_loop(self):
        try:
            self.refresh_status(force_query=False)
        finally:
            self.after(1000, self.refresh_status_loop)


def main():
    # Enforce single-instance via Win32 Mutex
    try:
        import ctypes
        mutex = ctypes.windll.kernel32.CreateMutexW(None, False, "KiloApps_Fleet_Dashboard_Mutex")
        if ctypes.windll.kernel32.GetLastError() == 183:  # ERROR_ALREADY_EXISTS
            hwnd = ctypes.windll.user32.FindWindowW(None, "KiloApps Creation Dashboard")
            if hwnd:
                ctypes.windll.user32.ShowWindow(hwnd, 9)  # SW_RESTORE
                ctypes.windll.user32.SetForegroundWindow(hwnd)
            sys.exit(0)
    except Exception:
        pass

    log_dir = REPO_ROOT / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    with open(log_dir / "dashboard.log", "a", encoding="utf-8") as f:
        f.write(f"[{datetime.datetime.now().isoformat()}] Dashboard process started (PID {os.getpid()})\n")
    try:
        # Ensure Windows autostart shortcut exists if enabled in config
        cfg = load_config()
        if cfg.get("autostart_with_windows", True):
            set_windows_autostart(True)

        app = FleetDashboard()
        app.mainloop()
    except Exception as e:
        import traceback
        with open(log_dir / "dashboard.log", "a", encoding="utf-8") as f:
            f.write(f"[{datetime.datetime.now().isoformat()}] {traceback.format_exc()}\n")


if __name__ == "__main__":
    main()
