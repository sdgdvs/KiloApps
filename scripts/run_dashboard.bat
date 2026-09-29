@echo off
where pythonw >nul 2>nul
if %errorlevel% equ 0 (
    start "" pythonw "%~dp0dashboard.py"
) else (
    start "" "C:\Python313\pythonw.exe" "%~dp0dashboard.py"
)
