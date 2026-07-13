@echo off
setlocal

set "APP_NAME=KHabit"
set "SRC=main.c"

rc.exe /nologo app.rc
if %errorlevel% neq 0 (
    echo Resource compilation failed.
    exit /b %errorlevel%
)

cl.exe /nologo /O2 /W3 /DNDEBUG /MD %SRC% app.res user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib /link /SUBSYSTEM:WINDOWS /OUT:%APP_NAME%.exe
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)

echo Build successful: %APP_NAME%.exe
