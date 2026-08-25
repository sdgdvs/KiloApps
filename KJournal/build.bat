@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O2 /W3 main.c user32.lib gdi32.lib /Fe:KJournal.exe

