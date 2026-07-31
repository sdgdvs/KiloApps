@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O1 /Os /Gy /c main.c
link /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib winmm.lib /OUT:KColony.exe
