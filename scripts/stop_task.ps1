<#
.SYNOPSIS
    Immediately stops the KiloApps Fleet Windows Scheduled Task and any active execution.
#>
param(
    [string]$TaskName = "KiloApps-Fleet-Orchestrator"
)

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$SessionFile = Join-Path $RepoRoot ".agents\scheduler_session.json"
$LockFile = Join-Path $RepoRoot ".agents\.orchestrator.lock"

Write-Host "Stopping KiloApps Fleet Orchestrator..." -ForegroundColor Yellow

# 1. Disable scheduled task
try {
    Disable-ScheduledTask -TaskName $TaskName -ErrorAction Stop | Out-Null
    Write-Host "[✓] Windows Scheduled Task '$TaskName' disabled." -ForegroundColor Green
} catch {
    Write-Warning "Could not disable scheduled task '$TaskName': $($_.Exception.Message)"
}

# 2. Update session file status
if (Test-Path $SessionFile) {
    try {
        $Session = Get-Content $SessionFile -Raw | ConvertFrom-Json
        $Session.status = "stopped"
        $Session | ConvertTo-Json -Depth 4 | Set-Content $SessionFile -Encoding UTF8
        Write-Host "[✓] Session status set to 'stopped' in .agents\scheduler_session.json." -ForegroundColor Green
    } catch {
        Write-Warning "Could not update session file: $($_.Exception.Message)"
    }
}

# 3. Release any lockfile
if (Test-Path $LockFile) {
    try {
        Remove-Item $LockFile -Force
        Write-Host "[✓] Orchestrator lockfile removed." -ForegroundColor Green
    } catch {
        Write-Warning "Could not remove lockfile: $($_.Exception.Message)"
    }
}

Write-Host "Fleet orchestrator is stopped. To resume or request more time, run .\scripts\extend_time.ps1" -ForegroundColor Cyan
