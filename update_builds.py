import os
import shutil

apps = [
    ('KCalc', 'kcalc.ico'),
    ('KClock', 'kclock.ico'),
    ('KMines', 'kmines.ico'),
    ('KPad', 'kpad.ico'),
    ('KPaint', 'kpaint.ico'),
    ('KTask', 'ktask.ico'),
]

for app_dir, icon_file in apps:
    # copy icon
    shutil.copy(os.path.join('icons', icon_file), os.path.join(app_dir, 'icon.ico'))
    
    # write app.rc
    with open(os.path.join(app_dir, 'app.rc'), 'w') as f:
        f.write('1 ICON "icon.ico"\n')
        
    # update build.bat
    build_bat_path = os.path.join(app_dir, 'build.bat')
    with open(build_bat_path, 'r') as f:
        lines = f.readlines()
        
    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('..\\crinkler'):
            # replace Crinkler with link.exe
            # We need to extract the /OUT: parameter to keep the same output name
            exe_name = app_dir + '.exe'
            new_lines.append(f'rc /fo app.res app.rc\n')
            new_lines.append(f'link /ENTRY:MainEntry /SUBSYSTEM:WINDOWS /NODEFAULTLIB /MERGE:.rdata=.text /MERGE:.pdata=.text /ALIGN:16 /FILEALIGN:16 main.obj app.res kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib /OUT:{exe_name}\n')
            
            # Since some apps might use specific libs, we just link all common ones to be safe
        elif line.startswith('echo Linking with Crinkler'):
            new_lines.append('echo Linking with MSVC...\n')
        else:
            new_lines.append(line)
        i += 1
        
    with open(build_bat_path, 'w') as f:
        f.writelines(new_lines)
