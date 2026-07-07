import os

# Fix KCalc _fltused
main_c_path = os.path.join('KCalc', 'main.c')
with open(main_c_path, 'r') as f:
    content = f.read()
if 'int _fltused' not in content:
    with open(main_c_path, 'w') as f:
        f.write('int _fltused = 1;\n' + content)

apps = [
    'KCalc',
    'KClock',
    'KMines',
    'KPad',
    'KPaint',
    'KTask',
]

build_bat_template = """@echo off
set VCVARS="C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars32.bat"
call %VCVARS%

cl /O1 /Os /GS- /Gy /c main.c

rc /fo app.res app.rc
link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /NODEFAULTLIB /MERGE:.rdata=.text /MERGE:.pdata=.text /ALIGN:64 /FILEALIGN:64 main.obj app.res kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib /OUT:{app}.exe
"""

for app in apps:
    with open(os.path.join(app, 'build.bat'), 'w') as f:
        f.write(build_bat_template.format(app=app))
