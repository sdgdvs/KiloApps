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

        $NodeId = $Session.node_id
        if (-not $NodeId) {
            $GitUser = ""
            try { $GitUser = (git -C $RepoRoot config user.name) } catch {}
            if ($GitUser -match "sdgdvs") { $NodeId = "pc_b" }
            elseif ($GitUser -match "anonymous2") { $NodeId = "pc_c" }
            else { $NodeId = "pc_a" }
        }
        $NodeFile = Join-Path $RepoRoot ".agents\fleet_nodes\node_$($NodeId.Replace('pc_', '')).json"
        if (Test-Path $NodeFile) {
            $NodeData = Get-Content $NodeFile -Raw -Encoding UTF8 | ConvertFrom-Json
            $NodeData.status = "stopped"
            $NodeData.last_heartbeat = (Get-Date).ToUniversalTime().ToString("o")
            $NodeData | ConvertTo-Json -Depth 4 | Set-Content $NodeFile -Encoding UTF8
            Write-Host "[✓] Node status set to 'stopped' in .agents\fleet_nodes\node_$($NodeId.Replace('pc_', '')).json." -ForegroundColor Green
        }
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
