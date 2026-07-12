@echo off
rc.exe app.rc
cl.exe main.c app.res user32.lib gdi32.lib /FeKBudget.exe /nologo
