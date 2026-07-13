@echo off
gcc main.c -o KHabit.exe -mwindows -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)
echo Build succeeded! KHabit.exe created.
