<#
.SYNOPSIS
    Registers and activates the Windows Scheduled Task for the KiloApps Fleet Orchestrator.
.DESCRIPTION
    Multi-PC Fleet Topology:
      - PC A (sdgdvs, Gemini Pro): 1 turn/hr at :12 past each hour (Window :10 - :18)
      - PC B (anonymous2, Gemini Pro): 1 turn/hr at :30 past each hour (Window :28 - :36)
      - PC C (This PC / anonymous1, Gemini Ultra): 4 turns/hr at :02, :20, :38, :48 past each hour
        (or 3 turns/hr at :02, :22, :42 past each hour).
    Supports continuous mode (default: indefinite repetition, no auto-stop timer)
    or optional timed sessions (e.g. -Hours 24).
.PARAMETER Hours
    Duration of the active contribution session in hours. Default: 0.0 (Continuous / Indefinite).
.PARAMETER Continuous
    Explicit switch to run in continuous mode with no auto-stop timer.
.PARAMETER NodeId
    Node identifier: "pc_c" (this PC), "pc_a", "pc_b", or "auto".
.PARAMETER Cadence
    For PC C: 4 (default, 4 turns/hr) or 3 (3 turns/hr).
.PARAMETER Minutes
    Custom explicit array of minute integers past each hour.
.PARAMETER TaskName
    Name of the scheduled task. Default: "KiloApps-Fleet-Orchestrator".
.PARAMETER RunNow
    Optionally trigger an immediate turn right now (if inside safe window).
#>
param(
    [double]$Hours = 0.0,
    [switch]$Continuous,
    [string]$NodeId = "auto",
    [int]$Cadence = 0,
    [int[]]$Minutes = @(),
    [string]$TaskName = "KiloApps-Fleet-Orchestrator",
    [switch]$RunNow
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$BatPath = Join-Path $RepoRoot "scripts\run_orchestrator.bat"
$SessionFile = Join-Path $RepoRoot ".agents\scheduler_session.json"

if (-not (Test-Path $BatPath)) {
    Write-Error "Could not find batch runner at: $BatPath"
    exit 1
}

# Auto-detect PC Node identity
$GitUser = ""
try { $GitUser = (git -C $RepoRoot config user.name) } catch {}
$HostName = $env:COMPUTERNAME

if ($NodeId -eq "auto") {
    if ($GitUser -match "anonymous1" -or $HostName -match "12900K") {
        $NodeId = "pc_c"
    } elseif ($GitUser -match "sdgdvs") {
        $NodeId = "pc_a"
    } elseif ($GitUser -match "anonymous2") {
        $NodeId = "pc_b"
    } else {
        $NodeId = "pc_c"
    }
}

if ($Minutes.Count -eq 0) {
    if ($NodeId -eq "pc_a") {
        $Minutes = @(12)
        $NodeDesc = "PC A (sdgdvs • Gemini Pro - 1 turn/hr at :12)"
    } elseif ($NodeId -eq "pc_b") {
        $Minutes = @(30)
        $NodeDesc = "PC B (anonymous2 • Gemini Pro - 1 turn/hr at :30)"
    } else {
        if ($Cadence -eq 3) {
            $Minutes = @(2, 22, 42)
            $NodeDesc = "PC C (This PC / anonymous1 • Gemini Ultra - 3 turns/hr at :02, :22, :42)"
        } else {
            $Minutes = @(2, 20, 38, 48)
            $NodeDesc = "PC C (This PC / anonymous1 • Gemini Ultra - 4 turns/hr at :02, :20, :38, :48)"
        }
    }
} else {
    $minList = ($Minutes | ForEach-Object { ":$($_.ToString('D2'))" }) -join ", "
    $NodeDesc = "$NodeId (Custom Minutes: $minList)"
}

$IsContinuous = ($Continuous -or $Hours -le 0.0)
$Now = Get-Date
$SessionEnd = if ($IsContinuous) { $null } else { $Now.AddHours($Hours) }
$minDisplay = ($Minutes | ForEach-Object { ":$($_.ToString('D2'))" }) -join ", "

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " KiloApps Multi-PC Fleet Scheduler Configuration" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Target Machine:      $NodeDesc [Node ID: $NodeId]"
Write-Host "Session Start:       $($Now.ToString('yyyy-MM-dd HH:mm:ss'))"
if ($IsContinuous) {
    Write-Host "Session Mode:        Continuous (No auto-stop timer; unlimited session)" -ForegroundColor Green
    Write-Host "Session Expiration:  None (repeats every hour indefinitely)" -ForegroundColor Green
    Write-Host "Dispatch Cadence:    Every hour at $minDisplay ($($Minutes.Count) turns/hr)"
} else {
    Write-Host "Session Mode:        Timed Session (${Hours}h window)" -ForegroundColor Yellow
    Write-Host "Session Expiration:  $($SessionEnd.ToString('yyyy-MM-dd HH:mm:ss'))"
    Write-Host "Dispatch Cadence:    Every hour at $minDisplay ($($Minutes.Count) turns/hr = $([int]($Hours * $Minutes.Count)) turns/session)"
}
Write-Host "Fleet Harmony:       Dedicated collision-free slot across PC A, PC B, and PC C"
Write-Host "------------------------------------------------------------"

# Preserve previous turn statistics if session file exists
$PreviousTurns = 0
$PrevLastTurn = $null
if (Test-Path $SessionFile) {
    try {
        $Existing = Get-Content $SessionFile -Raw -Encoding UTF8 | ConvertFrom-Json
        if ($Existing.turns_executed) { $PreviousTurns = [int]$Existing.turns_executed }
        if ($Existing.last_turn_timestamp) { $PrevLastTurn = $Existing.last_turn_timestamp }
    } catch {}
}

# 1. Initialize session tracking file
$SessionData = [ordered]@{
    node_id               = $NodeId
    session_start         = $Now.ToUniversalTime().ToString("o")
    session_end           = if ($IsContinuous) { $null } else { $SessionEnd.ToUniversalTime().ToString("o") }
    session_limit_enabled = (-not $IsContinuous)
    timer_enabled         = (-not $IsContinuous)
    duration_hours        = if ($IsContinuous) { 0.0 } else { $Hours }
    max_turns_estimate    = if ($IsContinuous) { 0 } else { [int]($Hours * $Minutes.Count) }
    turns_executed        = $PreviousTurns
    minutes               = $Minutes
    task_name             = $TaskName
    status                = "active"
    last_turn_timestamp   = $PrevLastTurn
}

# Update fleet node broadcast file
$NodeFile = Join-Path $RepoRoot ".agents\fleet_nodes\node_$($NodeId.Replace('pc_', '')).json"
if (Test-Path $NodeFile) {
    try {
        $NodeData = Get-Content $NodeFile -Raw -Encoding UTF8 | ConvertFrom-Json
        $NodeData.status = "active"
        $NodeData.session_start = $Now.ToUniversalTime().ToString("o")
        $NodeData.session_end = if ($IsContinuous) { $null } else { $SessionEnd.ToUniversalTime().ToString("o") }
        $NodeData.session_limit_enabled = (-not $IsContinuous)
        $NodeData.timer_enabled = (-not $IsContinuous)
        $NodeData.schedule_minutes = $Minutes
        $NodeData.last_heartbeat = $Now.ToUniversalTime().ToString("o")
        $NodeData.remote_command = "none"
        $NodeData | ConvertTo-Json -Depth 4 | Set-Content $NodeFile -Encoding UTF8
    } catch {}
}

$SessionDir = Split-Path $SessionFile
if (-not (Test-Path $SessionDir)) {
    New-Item -ItemType Directory -Path $SessionDir -Force | Out-Null
}
$SessionData | ConvertTo-Json -Depth 4 | Set-Content $SessionFile -Encoding UTF8
Write-Host "[OK] Session state initialized: .agents\scheduler_session.json" -ForegroundColor Green

# 2. Create scheduled task triggers
$Triggers = @()
foreach ($m in $Minutes) {
    $s = (Get-Date -Hour $Now.Hour -Minute $m -Second 0)
    if ($s -le $Now) { $s = $s.AddHours(1) }
    if ($IsContinuous) {
        # Continuous repetition with NO EndBoundary and NO RepetitionDuration limit
        $trig = New-ScheduledTaskTrigger -Once -At $s -RepetitionInterval (New-TimeSpan -Hours 1)
    } else {
        $TimeSpanHours = [TimeSpan]::FromHours($Hours)
        $trig = New-ScheduledTaskTrigger -Once -At $s -RepetitionInterval (New-TimeSpan -Hours 1) -RepetitionDuration $TimeSpanHours
        $trig.EndBoundary = $SessionEnd.ToString("s")
    }
    $Triggers += $trig
}

# 3. Action and Settings
$Action = New-ScheduledTaskAction -Execute $BatPath -WorkingDirectory $RepoRoot
$Settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 25) `
    -MultipleInstances IgnoreNew

# 4. Register Task
Register-ScheduledTask -TaskName $TaskName -Action $Action -Trigger $Triggers -Settings $Settings -Force | Out-Null
Enable-ScheduledTask -TaskName $TaskName | Out-Null

$Task = Get-ScheduledTask -TaskName $TaskName
$TaskInfo = Get-ScheduledTaskInfo -TaskName $TaskName

Write-Host "[OK] Windows Scheduled Task '$TaskName' registered successfully!" -ForegroundColor Green
Write-Host "    State:          $($Task.State)"
Write-Host "    Next Run:       $($TaskInfo.NextRunTime)"
Write-Host "    Triggers Active: $($Triggers.Count) (at $minDisplay past each hour)"
if ($IsContinuous) {
    Write-Host "    Auto-Stop At:   Disabled (Continuous indefinite execution)" -ForegroundColor Green
} else {
    Write-Host "    Auto-Stop At:   $($SessionEnd.ToString('yyyy-MM-dd HH:mm:ss'))"
}
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Fleet Commands:"
Write-Host "  Check Status:    .\scripts\task_status.ps1" -ForegroundColor Yellow
Write-Host "  Stop Task:       .\scripts\stop_task.ps1" -ForegroundColor Yellow
Write-Host "  Extend/Resume:   .\scripts\extend_time.ps1 -Hours 24" -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan

if ($RunNow) {
    Write-Host "Launching initial turn now via Task Scheduler..." -ForegroundColor Cyan
    Start-ScheduledTask -TaskName $TaskName
}
