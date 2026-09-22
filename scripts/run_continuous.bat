@echo off
setlocal
cd /d "%~dp0.."

:: Ensure paths for agy, uv, python
set "PATH=%LOCALAPPDATA%\agy\bin;%USERPROFILE%\.local\bin;%LOCALAPPDATA%\Microsoft\WinGet\Packages\astral-sh.uv_Microsoft.Winget.Source_8wekyb3d8bbwe;%PATH%"

:: Run continuous runner with 15-minute cadence and 4-hour window
python scripts/continuous_runner.py --interval 15 --hours 4 %*

pause
