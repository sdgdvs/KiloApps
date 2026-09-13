@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul

cl /nologo /O1 /Os /Gy /GS- /c main.c
if errorlevel 1 exit /b

if exist "C:\KiloApps\KiloApps\crinkler\crinkler30a\Win32\crinkler.exe" (
    "C:\KiloApps\KiloApps\crinkler\crinkler30a\Win32\crinkler.exe" /ENTRY:WinMain /SUBSYSTEM:WINDOWS /OUT:KVoid.exe main.obj kernel32.lib user32.lib gdi32.lib
) else (
    link /nologo /SUBSYSTEM:WINDOWS /ENTRY:WinMain main.obj kernel32.lib user32.lib gdi32.lib /OUT:KVoid.exe
)
del main.obj
