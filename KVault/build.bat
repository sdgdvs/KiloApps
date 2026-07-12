@echo off
echo Building KVault...
rc.exe /nologo app.rc
cl.exe /nologo /O2 /W3 main.c app.res user32.lib gdi32.lib advapi32.lib comdlg32.lib /link /SUBSYSTEM:WINDOWS /OUT:KVault.exe
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)
echo Build successful!
