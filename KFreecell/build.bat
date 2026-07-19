@echo off
cl.exe /O1 /Os /GS- /Gy main.c kernel32.lib user32.lib gdi32.lib /link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /OUT:KFreecell.exe
