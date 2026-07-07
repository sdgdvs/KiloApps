@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%

cvtres /MACHINE:X86 /OUT:test_res.obj test.res

..\crinkler\crinkler30a\Win32\Crinkler.exe test.obj test_res.obj /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /TINYIMPORT /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22000.0\um\x86" kernel32.lib /OUT:test.exe
