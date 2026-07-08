# KiloApps Administrative Assistant script
# This script runs hourly to monitor agent cron jobs, resolve uncommitted file conflicts, log progress, and send daily email reports.

$repoPath = "c:\KiloApps\KiloApps"
$adminFolder = Join-Path $repoPath ".agents"
$logPath = Join-Path $adminFolder "admin_log.md"
$reportsFolder = Join-Path $adminFolder "reports"
$lastEmailFile = Join-Path $adminFolder "last_email_sent.txt"
$envFile = "C:\Users\laura\.env"

# Restore path for Git commands
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# Ensure folders exist
if (-not (Test-Path $adminFolder)) { New-Item -ItemType Directory -Path $adminFolder -Force | Out-Null }
if (-not (Test-Path $reportsFolder)) { New-Item -ItemType Directory -Path $reportsFolder -Force | Out-Null }
if (-not (Test-Path $logPath)) { New-Item -ItemType File -Path $logPath -Force | Out-Null }

# Load .env file if it exists
if (Test-Path $envFile) {
    Get-Content $envFile | ForEach-Object {
        if ($_ -match "^\s*([^#=]+)=(.*)$") {
            $name = $Matches[1].Trim()
            $value = $Matches[2].Trim()
            # Remove optional surrounding quotes
            if ($value -match '^"(.*)"$') { $value = $Matches[1] }
            elseif ($value -match "^'(.*)'$") { $value = $Matches[1] }
            [Environment]::SetEnvironmentVariable($name, $value, "Process")
        }
    }
}

# --- 1. Gather Repository & Agent Info ---
$currentTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$currentDate = Get-Date -Format "yyyy-MM-dd"

# Check Git Status
$uncommittedChanges = git status --porcelain
if ([string]::IsNullOrEmpty($uncommittedChanges)) { $hasUncommitted = "No" } else { $hasUncommitted = "Yes" }


# Get commits in the last 24 hours
$recentCommits = git log --since="24 hours ago" --oneline

# Read Plan Files
$expansionPlan = "Unknown (File Missing)"
$fixPlan = "Unknown (File Missing)"
$newAppPlan = "Unknown (File Missing)"
$argPlan = "Unknown (File Missing)"

if (Test-Path (Join-Path $repoPath "app_expansion_plan.md")) {
    $expansionPlan = (Get-Content (Join-Path $repoPath "app_expansion_plan.md") -Head 10) -join "`n"
}
if (Test-Path (Join-Path $repoPath "app_fix_plan.md")) {
    $fixPlan = (Get-Content (Join-Path $repoPath "app_fix_plan.md") -Head 10) -join "`n"
}
if (Test-Path (Join-Path $repoPath "new_app_plan.md")) {
    $newAppPlan = (Get-Content (Join-Path $repoPath "new_app_plan.md") -Head 10) -join "`n"
}
# ARG plan is located under its conversation brain directory or inside repo?
$argPlanPath = "C:\Users\laura\.gemini\antigravity\brain\a7a0517d-8c0b-4d16-a32f-36e3cbc48382\arg_master_plan.md"
if (Test-Path $argPlanPath) {
    $argPlan = (Get-Content $argPlanPath -Head 10) -join "`n"
}

# --- 2. Resolve Uncommitted Changes ---
# If changes are found, we commit them on behalf of the agent to keep the work flowing
$resolvedConflict = "None"
if (-not [string]::IsNullOrEmpty($uncommittedChanges)) {
    git add -A
    $commitMsg = "Administrative commit: Auto-saving uncommitted work-in-progress to prevent merge blocks"
    git commit -m $commitMsg
    git push
    $resolvedConflict = "Committed changes automatically: $uncommittedChanges"
}

# --- 3. Log execution ---
$logEntry = @"

## Check Run: $currentTime
- **Uncommitted Changes Detected**: $hasUncommitted
- **Conflict Resolution Action**: $resolvedConflict
- **Recent Git Activity (24h)**:
$recentCommits

- **Active Plan Summary**:
  - **App Expansion Plan**:
    $expansionPlan
  - **Bug Fix Plan**:
    $fixPlan
  - **New App Plan**:
    $newAppPlan
  - **ARG Plan**:
    $argPlan
"@

Add-Content -Path $logPath -Value $logEntry

# --- 4. Daily Report Compilation & Email ---
# Determine if we should send a daily report
$shouldSendEmail = $false
$lastEmailSent = ""

if (Test-Path $lastEmailFile) {
    $lastEmailSent = Get-Content $lastEmailFile
}

if ([string]::IsNullOrEmpty($lastEmailSent)) {
    $shouldSendEmail = $true
} else {
    try {
        $lastDate = [datetime]::ParseExact($lastEmailSent, "yyyy-MM-dd HH:mm:ss", $null)
        $elapsed = (Get-Date) - $lastDate
        if ($elapsed.TotalHours -ge 23.5) {
            $shouldSendEmail = $true
        }
    } catch {
        $shouldSendEmail = $true
    }
}

if ($shouldSendEmail) {
    # Compile the Report
    $reportTitle = "KiloApps Administrative Status Report - $currentDate"
    $reportContent = @"
# KiloApps Project Status Report ($currentDate)

This report summarizes developer agent activities, plan updates, and repository checks.

## Active Plan File States

### 1. App Expansion Plan
$expansionPlan

### 2. App Bug-Fix Plan
$fixPlan

### 3. New App Development Plan
$newAppPlan

### 4. ARG Lore & Narrative Plan
$argPlan

## Git Code Changes (Last 24 Hours)
$recentCommits

## Repository Health & Integrity Check
- **Local Uncommitted Changes at Run**: $hasUncommitted
- **Conflict Management Actions taken**: $resolvedConflict
- **Last Clean Check Run**: $currentTime

-- 
Sent automatically by the Administrative Assistant Agent.
"@
    
    # Save a copy to the reports folder
    $reportFileName = "daily_report_$currentDate.md"
    $reportFileFullPath = Join-Path $reportsFolder $reportFileName
    $reportContent | Out-File -FilePath $reportFileFullPath -Encoding utf8
    
    # Attempt to send email
    $smtpServer = [Environment]::GetEnvironmentVariable("SMTP_SERVER", "Process")
    $smtpPortVal = [Environment]::GetEnvironmentVariable("SMTP_PORT", "Process")
    $smtpUser = [Environment]::GetEnvironmentVariable("SMTP_USER", "Process")
    $smtpPass = [Environment]::GetEnvironmentVariable("SMTP_PASSWORD", "Process")
    $smtpTo = [Environment]::GetEnvironmentVariable("SMTP_TO", "Process")
    
    if (-not [string]::IsNullOrEmpty($smtpServer) -and -not [string]::IsNullOrEmpty($smtpUser) -and -not [string]::IsNullOrEmpty($smtpPass) -and -not [string]::IsNullOrEmpty($smtpTo)) {
        try {
            $smtpPort = [int]$smtpPortVal
            $securePass = ConvertTo-SecureString $smtpPass -AsPlainText -Force
            $creds = New-Object System.Management.Automation.PSCredential ($smtpUser, $securePass)
            
            Send-MailMessage -SmtpServer $smtpServer -Port $smtpPort -UseSsl -Credential $creds -From $smtpUser -To $smtpTo -Subject $reportTitle -Body $reportContent -BodyAsHtml:$false -Encoding UTF8
            
            # Update last email sent timestamp
            $currentTime | Out-File -FilePath $lastEmailFile -Encoding utf8
            Write-Output "Email report sent successfully to $smtpTo."
        } catch {
            Write-Error "Failed to send email: $_"
        }
    } else {
        Write-Output "SMTP configuration not complete in C:\Users\laura\.env. Report saved locally to $reportFileFullPath."
    }
}
