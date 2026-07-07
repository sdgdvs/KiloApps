@echo off
set APPS=KBBS KCalc KChat KChatServer KClock KMines KPad KPaint KTask KSnake
for %%A in (%APPS%) do (
    echo Building %%A
    cd %%A
    cmd /c build.bat
    copy /Y %%A.exe ..\KiloOS\public\exe\
    cd ..
)
