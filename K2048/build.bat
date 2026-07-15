@echo off
rc.exe app.rc
cl.exe /O1 /Os /GS- /Gy main.c app.res kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib /link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /OUT:K2048.exe
