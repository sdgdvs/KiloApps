@echo off
set "VC_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
if exist "%VC_DIR%\vcvarsall.bat" (
    call "%VC_DIR%\vcvarsall.bat" x64
) else (
    echo Visual Studio build environment not found.
    exit /b 1
)

cl.exe /O2 /W3 /Fe:KSudoku.exe main.c user32.lib
del main.obj
