@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul
cl /nologo /O1 /Os /c main.c
if errorlevel 1 exit /b 1
link /nologo /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib winmm.lib /OUT:KCosmic.exe
if errorlevel 1 exit /b 1
del main.obj
if not exist "..\KiloOS\public\exe" mkdir "..\KiloOS\public\exe"
copy /Y KCosmic.exe "..\KiloOS\public\exe\KCosmic.exe"
