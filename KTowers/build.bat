@echo off
where cl >nul 2>nul || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /O2 main.c user32.lib gdi32.lib winmm.lib /Fe:KTowers.exe
