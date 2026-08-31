@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O2 /FeKFortress.exe main.c user32.lib gdi32.lib winmm.lib shell32.lib advapi32.lib
if exist KFortress.exe (
    copy /Y KFortress.exe ..\KiloOS\public\exe\KFortress.exe
)
