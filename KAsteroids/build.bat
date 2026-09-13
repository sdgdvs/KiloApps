@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul
cl /nologo /O1 /Os main.c user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib /link /OUT:KAsteroids.exe
