@echo off
setlocal
cd /d "%~dp0.."

:: Refresh Path in case running from minimal Task Scheduler environment
set "PATH=%LOCALAPPDATA%\agy\bin;%USERPROFILE%\.local\bin;%LOCALAPPDATA%\Microsoft\WinGet\Packages\astral-sh.uv_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Program Files\Git\cmd;C:\Program Files\nodejs;C:\Python313;%PATH%"

:: Run orchestrator via uv
uv run scripts/orchestrate.py %*

exit /b %ERRORLEVEL%
