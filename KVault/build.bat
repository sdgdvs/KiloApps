@echo off
echo Building KVault...
gcc main.c -o KVault.exe -mwindows
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)
echo Build successful!
