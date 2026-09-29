<#
.SYNOPSIS
    Extends, resumes, or sets continuous mode for the KiloApps Fleet Windows Scheduled Task.
.PARAMETER Hours
    Number of additional hours to authorize (default: 0.0 = continuous mode).
.PARAMETER Continuous
    Explicit switch to activate continuous mode with no auto-stop timer.
#>
param(
    [double]$Hours = 0.0,
    [switch]$Continuous,
    [string]$TaskName = "KiloApps-Fleet-Orchestrator",
    [switch]$RunNow
)

$RegisterScript = Join-Path $PSScriptRoot "register_task.ps1"
if ($Continuous -or $Hours -le 0.0) {
    Write-Host "Setting KiloApps Fleet Orchestrator to Continuous Mode (no auto-stop timer)..." -ForegroundColor Cyan
    & $RegisterScript -Hours 0 -Continuous -TaskName $TaskName -RunNow:$RunNow
} else {
    Write-Host "Extending KiloApps Fleet Orchestrator session by $Hours hours..." -ForegroundColor Cyan
    & $RegisterScript -Hours $Hours -TaskName $TaskName -RunNow:$RunNow
}
