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
    [int]$Minute1 = 2,
    [int]$Minute2 = 17,
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

$Now = Get-Date
$SessionEnd = $Now.AddHours($Hours)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " KiloApps 24-Hour Autonomous Fleet Scheduler Configuration" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "Session Start:       $($Now.ToString('yyyy-MM-dd HH:mm:ss'))"
Write-Host "Session Expiration:  $($SessionEnd.ToString('yyyy-MM-dd HH:mm:ss')) (${Hours}h window)"
Write-Host "Dispatch Cadence:    Every hour at :$($Minute1.ToString('D2')) and :$($Minute2.ToString('D2')) (2 turns/hr = $([int]($Hours * 2)) turns/session)"
Write-Host "Remote Fleet Window: XX:32 - XX:58 reserved for sdgdvs & anonymous2 (zero collisions)"
Write-Host "------------------------------------------------------------"

# 1. Initialize session tracking file
$SessionData = @{
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
