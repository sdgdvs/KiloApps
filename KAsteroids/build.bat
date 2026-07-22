@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O1 /Os main.c user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib /link /OUT:KAsteroids.exe
