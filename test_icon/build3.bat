@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS%

python gen_ico.py
rc /fo test.res test.rc
cl /O1 /Os /GS- /Gy /c test.c

link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /NODEFAULTLIB /MERGE:.rdata=.text /MERGE:.pdata=.text /ALIGN:16 /FILEALIGN:16 test.obj test.res kernel32.lib /OUT:test.exe
