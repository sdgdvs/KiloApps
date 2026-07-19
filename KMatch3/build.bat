@echo off
gcc main.c -o KMatch3.exe -mwindows
if %ERRORLEVEL% equ 0 (
    echo Build successful!
) else (
    echo Build failed!
)
