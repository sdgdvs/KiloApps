<#
.SYNOPSIS
    Registers and activates the Windows Scheduled Task for the KiloApps Fleet Orchestrator.
.DESCRIPTION
    Harmonizes with remote fleet contributors (sdgdvs and anonymous2) by dispatching
    at the maximum practical non-conflicting rate (2 turns/hour at XX:02 and XX:17) for
    exactly 24 hours (1 day). Automatically halts and disables the task when the 24-hour
    duration expires until the user explicitly requests more time.
.PARAMETER Hours
    Duration of the active contribution session in hours. Default: 24.0 (1 day).
.PARAMETER Minute1
    First trigger minute past each hour. Default: 2.
.PARAMETER Minute2
    Second trigger minute past each hour. Default: 17.
.PARAMETER TaskName
    Name of the scheduled task. Default: "KiloApps-Fleet-Orchestrator".
.PARAMETER RunNow
    Optionally trigger an immediate turn right now (if inside the safe window).
#>
param(
    [double]$Hours = 24.0,
    [string]$NodeId = "auto",
    [int]$Minute1 = -1,
    [int]$Minute2 = -1,
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
    if ($GitUser -match "sdgdvs") {
        $NodeId = "pc_b"
    } elseif ($GitUser -match "anonymous2") {
        $NodeId = "pc_c"
    } else {
        $NodeId = "pc_a"
    }
}

if ($Minute1 -lt 0 -or $Minute2 -lt 0) {
    if ($NodeId -eq "pc_b") {
        $Minute1 = 32
        $Minute2 = 47
        $NodeDesc = "PC B (sdgdvs)"
    } elseif ($NodeId -eq "pc_c") {
        $Minute1 = 22
        $Minute2 = 52
        $NodeDesc = "PC C (anonymous2)"
    } else {
        $Minute1 = 2
        $Minute2 = 17
        $NodeDesc = "PC A (anonymous1 / 12900K)"
    }
} else {
    $NodeDesc = "$NodeId (Custom :$($Minute1.ToString('D2')) & :$($Minute2.ToString('D2')))"
}

$Now = Get-Date
$SessionEnd = $Now.AddHours($Hours)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " KiloApps Multi-PC Fleet Scheduler Configuration" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Target Machine:      $NodeDesc [Node ID: $NodeId]"
Write-Host "Session Start:       $($Now.ToString('yyyy-MM-dd HH:mm:ss'))"
Write-Host "Session Expiration:  $($SessionEnd.ToString('yyyy-MM-dd HH:mm:ss')) (${Hours}h window)"
Write-Host "Dispatch Cadence:    Every hour at :$($Minute1.ToString('D2')) and :$($Minute2.ToString('D2')) (2 turns/hr = $([int]($Hours * 2)) turns/session)"
Write-Host "Fleet Harmony:       Dedicated collision-free slot across PC A, PC B, and PC C"
Write-Host "------------------------------------------------------------"

# 1. Initialize session tracking file
$SessionData = @{
    node_id             = $NodeId
    session_start       = $Now.ToUniversalTime().ToString("o")
    session_end         = $SessionEnd.ToUniversalTime().ToString("o")
    duration_hours      = $Hours
    max_turns_estimate  = [int]($Hours * 2)
    turns_executed      = 0
    minute1             = $Minute1
    minute2             = $Minute2
    task_name           = $TaskName
    status              = "active"
    last_turn_timestamp = $null
}

# Update fleet node broadcast file
$NodeFile = Join-Path $RepoRoot ".agents\fleet_nodes\node_$($NodeId.Replace('pc_', '')).json"
if (Test-Path $NodeFile) {
    try {
        $NodeData = Get-Content $NodeFile -Raw -Encoding UTF8 | ConvertFrom-Json
        $NodeData.status = "active"
        $NodeData.session_start = $Now.ToUniversalTime().ToString("o")
        $NodeData.session_end = $SessionEnd.ToUniversalTime().ToString("o")
        $NodeData.schedule_minutes = @($Minute1, $Minute2)
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
Write-Host "[✓] Session state initialized: .agents\scheduler_session.json" -ForegroundColor Green

# 2. Calculate trigger start boundaries
$s1 = (Get-Date -Hour $Now.Hour -Minute $Minute1 -Second 0)
if ($s1 -le $Now) { $s1 = $s1.AddHours(1) }

$s2 = (Get-Date -Hour $Now.Hour -Minute $Minute2 -Second 0)
if ($s2 -le $Now) { $s2 = $s2.AddHours(1) }

# 3. Create scheduled task triggers
$TimeSpanHours = [TimeSpan]::FromHours($Hours)

$Trigger1 = New-ScheduledTaskTrigger -Once -At $s1 -RepetitionInterval (New-TimeSpan -Hours 1) -RepetitionDuration $TimeSpanHours
$Trigger1.EndBoundary = $SessionEnd.ToString("s")

$Trigger2 = New-ScheduledTaskTrigger -Once -At $s2 -RepetitionInterval (New-TimeSpan -Hours 1) -RepetitionDuration $TimeSpanHours
$Trigger2.EndBoundary = $SessionEnd.ToString("s")

# 4. Action and Settings
$Action = New-ScheduledTaskAction -Execute $BatPath -WorkingDirectory $RepoRoot
$Settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 25) `
    -MultipleInstances IgnoreNew

# 5. Register Task
Register-ScheduledTask -TaskName $TaskName -Action $Action -Trigger @($Trigger1, $Trigger2) -Settings $Settings -Force | Out-Null
Enable-ScheduledTask -TaskName $TaskName | Out-Null

$Task = Get-ScheduledTask -TaskName $TaskName
$TaskInfo = Get-ScheduledTaskInfo -TaskName $TaskName

Write-Host "[✓] Windows Scheduled Task '$TaskName' registered successfully!" -ForegroundColor Green
Write-Host "    State:          $($Task.State)"
Write-Host "    Next Run:       $($TaskInfo.NextRunTime)"
Write-Host "    Triggers Active: 2 (at :$($Minute1.ToString('D2')) and :$($Minute2.ToString('D2')) past each hour)"
Write-Host "    Auto-Stop At:   $($SessionEnd.ToString('yyyy-MM-dd HH:mm:ss'))"
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Fleet Commands:"
Write-Host "  Check Status:    .\scripts\task_status.ps1" -ForegroundColor Yellow
Write-Host "  Stop Task:       .\scripts\stop_task.ps1" -ForegroundColor Yellow
Write-Host "  Extend/Resume:   .\scripts\extend_time.ps1 -Hours 24" -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan

if ($RunNow) {
    if ($Now.Minute -ge 32 -and $Now.Minute -le 58) {
        Write-Warning "Current minute (:$($Now.Minute.ToString('D2'))) is in the remote fleet contributor window (XX:32-XX:58). Skipping immediate run to prevent git collisions. Next run will trigger at $($TaskInfo.NextRunTime)."
    } else {
        Write-Host "Launching initial turn now via Task Scheduler..." -ForegroundColor Cyan
        Start-ScheduledTask -TaskName $TaskName
    }
}
