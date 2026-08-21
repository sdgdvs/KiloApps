@echo off
gcc main.c -o KTrader.exe
if %ERRORLEVEL% equ 0 (
    echo Build successful. Run KTrader.exe
) else (
    echo Build failed.
)
