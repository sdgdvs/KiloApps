@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul
cl /nologo /O1 /Os /GS- /Gy /c main.c
if exist app.rc ( rc /nologo /fo app.res app.rc )
if exist app.res (
    link /nologo /ENTRY:MainEntry /SUBSYSTEM:WINDOWS main.obj app.res kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib /OUT:KDB.exe
) else (
    link /nologo /ENTRY:MainEntry /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib /OUT:KDB.exe
)
