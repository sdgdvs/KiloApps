@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O1 /Os /c main.c
if errorlevel 1 exit /b 1
link /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib winmm.lib /OUT:KSubmarine.exe
if errorlevel 1 exit /b 1
del main.obj
if not exist "..\KiloOS\public\exe" mkdir "..\KiloOS\public\exe"
copy /Y KSubmarine.exe "..\KiloOS\public\exe\KSubmarine.exe"
