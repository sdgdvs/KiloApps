# Register-KiloAppsOrchestrator.ps1
# Creates or updates the Windows Scheduled Task for KiloApps Fleet Orchestrator

$TaskName = "KiloApps-Fleet-Orchestrator"
$ScriptPath = "d:\KiloApps\scripts\run_orchestrator.bat"
$WorkingDir = "d:\KiloApps"

Write-Host "Registering Windows Scheduled Task: $TaskName..." -ForegroundColor Cyan

# Define Action
$Action = New-ScheduledTaskAction -Execute $ScriptPath -WorkingDirectory $WorkingDir

# Define Trigger: Run daily, repeating every 2 hours indefinitely
$Trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(2) -RepetitionInterval (New-TimeSpan -Hours 2)

# Define Settings
$Settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -MultipleInstances IgnoreNew `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 30)

# Register or update task
try {
    # Unregister existing task if present
    Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue

    Register-ScheduledTask `
        -TaskName $TaskName `
        -Action $Action `
        -Trigger $Trigger `
        -Settings $Settings `
        -Description "Autonomous KiloApps fleet orchestrator invoking Gemini Skills via agy CLI" | Out-Null

    Write-Host "Task successfully registered!" -ForegroundColor Green
    Get-ScheduledTask -TaskName $TaskName | Format-Table TaskName, State
} catch {
    Write-Warning "PowerShell cmdlet failed ($($_.Exception.Message)). Falling back to schtasks.exe..."
    $SchCmd = "schtasks /create /tn `"$TaskName`" /tr `"`"$ScriptPath`"`" /sc HOURLY /mo 2 /f /it"
    Invoke-Expression $SchCmd
}
