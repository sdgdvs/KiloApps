@echo off
if not exist build mkdir build
gcc main.c -o build\kstellar.exe -mwindows -lgdi32
