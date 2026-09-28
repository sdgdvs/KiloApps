<#
.SYNOPSIS
    Reports the status of the KiloApps Fleet Windows Scheduled Task and 24-hour session.
#>
param(
    [string]$TaskName = "KiloApps-Fleet-Orchestrator"
)

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$SessionFile = Join-Path $RepoRoot ".agents\scheduler_session.json"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " KiloApps Fleet Task & Session Status" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

# 1. Check Scheduled Task
try {
    $Task = Get-ScheduledTask -TaskName $TaskName -ErrorAction Stop
    $TaskInfo = Get-ScheduledTaskInfo -TaskName $TaskName -ErrorAction Stop

    $StateColor = "Red"
    if ($Task.State -eq "Ready") { $StateColor = "Green" }
    elseif ($Task.State -eq "Running") { $StateColor = "Yellow" }

    Write-Host "Scheduled Task:     $TaskName"
    Write-Host "Task State:         $($Task.State)" -ForegroundColor $StateColor
    Write-Host "Next Scheduled Run: $($TaskInfo.NextRunTime)"
    Write-Host "Last Run Time:      $($TaskInfo.LastRunTime)"
    Write-Host "Last Result Code:   $($TaskInfo.LastTaskResult)"
} catch {
    Write-Host "Scheduled Task:     $TaskName is NOT registered or inaccessible: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "------------------------------------------------------------"

# 2. Check Session Timer State
if (Test-Path $SessionFile) {
    try {
        $Session = Get-Content $SessionFile -Raw | ConvertFrom-Json
        $NowUtc = (Get-Date).ToUniversalTime()
        $EndUtc = ([DateTime]$Session.session_end).ToUniversalTime()
        $StartUtc = ([DateTime]$Session.session_start).ToUniversalTime()

        $Remaining = $EndUtc - $NowUtc
        $TotalDuration = $EndUtc - $StartUtc

        $StatusColor = "Yellow"
        if ($Session.status -eq "active" -and $Remaining.TotalSeconds -gt 0) { $StatusColor = "Green" }

        Write-Host "Session Status:     $($Session.status.ToUpper())" -ForegroundColor $StatusColor
        Write-Host "Session Start:      $($StartUtc.ToLocalTime().ToString('yyyy-MM-dd HH:mm:ss'))"
        Write-Host "Session Expiration: $($EndUtc.ToLocalTime().ToString('yyyy-MM-dd HH:mm:ss'))"

        if ($Remaining.TotalSeconds -gt 0) {
            Write-Host "Time Remaining:     $([Math]::Floor($Remaining.TotalHours)) hours, $($Remaining.Minutes) minutes" -ForegroundColor Green
        } else {
            Write-Host "Time Remaining:     EXPIRED (24-hour window completed)" -ForegroundColor Red
            Write-Host "Note:               Task halted until user runs .\scripts\extend_time.ps1" -ForegroundColor Yellow
        }

        Write-Host "Turns Executed:     $($Session.turns_executed) of estimated $($Session.max_turns_estimate)"
        if ($Session.last_turn_timestamp) {
            Write-Host "Last Turn Completed: $($Session.last_turn_timestamp)"
        }
    } catch {
        Write-Warning "Could not parse session state file: $($_.Exception.Message)"
    }
} else {
    Write-Host "Session State:      No active session file found (.agents\scheduler_session.json)." -ForegroundColor Yellow
}

Write-Host "------------------------------------------------------------"

# 3. Recent Fleet Activity
Write-Host "Recent Fleet Git Commits:" -ForegroundColor Cyan
git -C $RepoRoot log -n 5 --format="  %ad | %an | %s" --date=format:"%Y-%m-%d %H:%M:%S"

Write-Host "============================================================" -ForegroundColor Cyan
