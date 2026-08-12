import os

# 1. Update main.c
path_main = r'c:\KiloApps\KiloApps\KScript\main.c'
with open(path_main, 'r', encoding='utf-8') as f:
    content = f.read()

if 'int dpi = 96;' not in content:
    content = content.replace('int nodeCount = 0;', 'int nodeCount = 0;\nint dpi = 96;\n#define S(x) MulDiv(x, dpi, 96)')

if 'dpi = GetDeviceCaps' not in content:
    main_entry_old = '''void MainEntry() {\n    SetProcessDPIAware();'''
    main_entry_new = '''void MainEntry() {\n    SetProcessDPIAware();\n    HDC hdc = GetDC(NULL);\n    dpi = GetDeviceCaps(hdc, LOGPIXELSX);\n    ReleaseDC(NULL, hdc);'''
    content = content.replace(main_entry_old, main_entry_new)

    window_create_old = '''    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript", WS_OVERLAPPEDWINDOW,\n        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);'''
    window_create_new = '''    DWORD style = WS_OVERLAPPEDWINDOW;\n    RECT rect = {0, 0, S(W), S(H)};\n    AdjustWindowRect(&rect, style, FALSE);\n    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript - Press F1 for Help", style,\n        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);'''
    content = content.replace(window_create_old, window_create_new)

if 'S(-15)' not in content:
    content = content.replace('CreateFontA(-15', 'CreateFontA(S(-15)')
    content = content.replace(', 10, 10, 100, 24', ', S(10), S(10), S(100), S(24)')
    content = content.replace(', 120, 10, 100, 24', ', S(120), S(10), S(100), S(24)')
    content = content.replace(', 230, 10, 80, 24', ', S(230), S(10), S(80), S(24)')
    content = content.replace(', 320, 10, 80, 24', ', S(320), S(10), S(80), S(24)')
    content = content.replace(', 410, 10, 80, 24', ', S(410), S(10), S(80), S(24)')
    content = content.replace(', 500, 10, 80, 24', ', S(500), S(10), S(80), S(24)')
    content = content.replace(', 590, 10, 80, 24', ', S(590), S(10), S(80), S(24)')
    content = content.replace(', 680, 10, 80, 24', ', S(680), S(10), S(80), S(24)')
    content = content.replace(', 770, 10, 80, 24', ', S(770), S(10), S(80), S(24)')
    content = content.replace(', 860, 10, 70, 24', ', S(860), S(10), S(70), S(24)')
    
    content = content.replace('10, 44, 250, H - 95', 'S(10), S(44), S(250), S(H) - S(95)')
    content = content.replace('270, 44, 250, H - 95', 'S(270), S(44), S(250), S(H) - S(95)')
    content = content.replace('530, 44, 240, H - 95', 'S(530), S(44), S(240), S(H) - S(95)')

if 'S(40)' not in content:
    content = content.replace('int panelW = (nw - 40) / 3;', 'int panelW = (nw - S(40)) / 3;')
    content = content.replace('10, 44, panelW, nh - 55', 'S(10), S(44), panelW, nh - S(55)')
    content = content.replace('20 + panelW, 44, panelW, nh - 55', 'S(20) + panelW, S(44), panelW, nh - S(55)')
    content = content.replace('30 + panelW * 2, 44, panelW, nh - 55', 'S(30) + panelW * 2, S(44), panelW, nh - S(55)')

with open(path_main, 'w', encoding='utf-8') as f:
    f.write(content)


# 2. Update App.jsx
path_app = r'c:\KiloApps\KiloApps\KiloOS\src\App.jsx'
with open(path_app, 'r', encoding='utf-8') as f:
    app_content = f.read()
app_content = app_content.replace("{ id: 'kscript', title: 'KScript', url: '/apps/kscript.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kscript.ico', w: 650, h: 500", 
                                  "{ id: 'kscript', title: 'KScript', url: '/apps/kscript.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kscript.ico', w: 800, h: 600")
with open(path_app, 'w', encoding='utf-8') as f:
    f.write(app_content)


# 3. Update kscript.html
path_html = r'c:\KiloApps\KiloApps\KiloOS\public\apps\kscript.html'
with open(path_html, 'r', encoding='utf-8') as f:
    html_content = f.read()

# Add text rendering improvements
if 'text-rendering: optimizeLegibility;' not in html_content:
    html_content = html_content.replace('font-size: 14px;', 'font-size: 14px;\n        text-rendering: optimizeLegibility;\n        -webkit-font-smoothing: antialiased;')
    
    # Increase font size slightly to 15px for crispness
    html_content = html_content.replace('font-size: 14px;', 'font-size: 15px;')
    html_content = html_content.replace('<title>KScript</title>', '<title>KScript - Press F1 for Help</title>')
    
with open(path_html, 'w', encoding='utf-8') as f:
    f.write(html_content)

print('Patches applied successfully. Remember to build KiloOS (cd KiloOS && npm run build) and KScript (cd KScript && build.bat)!')
