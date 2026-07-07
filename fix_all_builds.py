import os
import glob
import subprocess
import shutil

libs = 'kernel32.lib user32.lib gdi32.lib advapi32.lib comdlg32.lib shell32.lib winmm.lib ws2_32.lib comctl32.lib'
template = f'''@echo off
set VCVARS="C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars32.bat"
call %VCVARS%
cl /O1 /Os /GS- /Gy /c main.c
if exist app.rc ( rc /fo app.res app.rc )
if exist app.res (
    link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS main.obj app.res {libs} /OUT:{{app}}.exe
) else (
    link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS main.obj {libs} /OUT:{{app}}.exe
)
'''

for d in glob.glob('K*'):
    if os.path.isdir(d) and os.path.exists(os.path.join(d, 'main.c')):
        app = d
        bat_path = os.path.join(d, 'build.bat')
        with open(bat_path, 'w') as f:
            f.write(template.format(app=app))
        print(f'Rebuilding {app}...')
        # use shell=True since it's a .bat file
        subprocess.run('build.bat', cwd=d, shell=True)
        exe_path = os.path.join(d, f'{app}.exe')
        if os.path.exists(exe_path):
            shutil.copy(exe_path, os.path.join('KiloOS', 'public', 'exe', f'{app}.exe'))
            print(f'Copied {app}.exe to KiloOS/public/exe/')
