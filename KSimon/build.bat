@echo off
set "VC_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
if not exist "%VC_DIR%\vcvars32.bat" (
    set "VC_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build"
)
if exist "%VC_DIR%\vcvars32.bat" (
    call "%VC_DIR%\vcvars32.bat"
) else (
    echo Visual Studio build environment not found.
    exit /b 1
)
if exist app.rc ( rc /fo app.res app.rc )
if exist app.res (
    cl.exe /O1 /Os /GS- /Gy /Fe:KSimon.exe main.c app.res kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib
) else (
    cl.exe /O1 /Os /GS- /Gy /Fe:KSimon.exe main.c kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib
)
del main.obj app.res
