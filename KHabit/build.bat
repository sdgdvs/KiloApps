@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
where cl >nul 2>nul || call %VCVARS% >nul
if exist app.rc ( rc /nologo /fo app.res app.rc )
if exist app.res (
    cl /nologo /O1 /Os main.c app.res /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comdlg32.lib /OUT:KHabit.exe
) else (
    cl /nologo /O1 /Os main.c /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib comdlg32.lib /OUT:KHabit.exe
)
if exist KHabit.exe (
    if not exist ..\KiloOS\public\exe mkdir ..\KiloOS\public\exe
    copy /Y KHabit.exe ..\KiloOS\public\exe\KHabit.exe >nul
)
if exist *.obj del *.obj
if exist *.res del *.res
