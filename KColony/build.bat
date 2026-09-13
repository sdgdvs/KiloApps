@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul
cl /nologo /O1 /Os /Gy /c main.c
link /nologo /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib winmm.lib /OUT:KColony.exe
