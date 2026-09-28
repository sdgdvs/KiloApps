<#
.SYNOPSIS
    Extends or resumes the KiloApps Fleet Windows Scheduled Task for additional time.
.PARAMETER Hours
    Number of additional hours to authorize (default: 24.0 = 1 day).
#>
param(
    [double]$Hours = 24.0,
    [string]$TaskName = "KiloApps-Fleet-Orchestrator",
    [switch]$RunNow
)

$RegisterScript = Join-Path $PSScriptRoot "register_task.ps1"
Write-Host "Extending KiloApps Fleet Orchestrator session by $Hours hours..." -ForegroundColor Cyan

& $RegisterScript -Hours $Hours -TaskName $TaskName -RunNow:$RunNow
