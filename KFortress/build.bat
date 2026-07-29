@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
cl /O2 /c /MT main.c
crinkler /SUBSYSTEM:WINDOWS main.obj kernel32.lib user32.lib gdi32.lib winmm.lib msvcrt.lib /ENTRY:WinMain /OUT:KFortress.exe
