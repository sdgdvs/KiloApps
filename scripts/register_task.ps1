# Register-KiloAppsOrchestrator.ps1
# Creates or updates the Windows Scheduled Task for KiloApps Fleet Orchestrator
param(
    [int]$OffsetMinutes = 0,
    [int]$IntervalMinutes = 15
)

$TaskName = "KiloApps-Fleet-Orchestrator"
$ScriptPath = Join-Path $PSScriptRoot "run_orchestrator.bat"
$StartTime = (Get-Date).AddMinutes($OffsetMinutes).ToString("HH:mm")

Write-Host "Configuring Windows Scheduled Task: $TaskName (Start Time: $StartTime, Interval: ${IntervalMinutes}m)..." -ForegroundColor Cyan

# Create base task using schtasks.exe
if ($IntervalMinutes -ge 60 -and ($IntervalMinutes % 60 -eq 0)) {
    $Hours = [int]($IntervalMinutes / 60)
    $CreateOutput = & schtasks.exe /create /tn $TaskName /tr "`"$ScriptPath`"" /sc HOURLY /mo $Hours /st $StartTime /f 2>&1
} else {
    $CreateOutput = & schtasks.exe /create /tn $TaskName /tr "`"$ScriptPath`"" /sc MINUTE /mo $IntervalMinutes /st $StartTime /f 2>&1
}
Write-Host "schtasks: $CreateOutput"

# Refine settings via PowerShell CIM (allow battery, catch-up missed runs, 30m timeout)
try {
    $Task = Get-ScheduledTask -TaskName $TaskName
    $Task.Settings.DisallowStartIfOnBatteries = $false
    $Task.Settings.StopIfGoingOnBatteries = $false
    $Task.Settings.StartWhenAvailable = $true
    $Task.Settings.ExecutionTimeLimit = "PT30M"
    $Task | Set-ScheduledTask | Out-Null
    Write-Host "Optimized settings applied: battery allowed, start when available enabled, 30m timeout." -ForegroundColor Green
} catch {
    Write-Warning "Could not modify extended settings: $($_.Exception.Message)"
}

Get-ScheduledTask -TaskName $TaskName | Format-Table TaskName, State
