@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%
cl /O1 /Os /GS- /Gy /c test_float.c
..\crinkler\crinkler30a\Win32\Crinkler.exe test_float.obj /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /TINYIMPORT /ORDERTRIES:100 /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22000.0\um\x86" kernel32.lib /OUT:test_float.exe
