@echo off
gcc main.c -o KFreecell.exe -mwindows -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo Build failed.
    pause
    exit /b %errorlevel%
)
echo Build succeeded.
