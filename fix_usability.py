import re

# 1. Update App.jsx
app_jsx_path = r"C:\KiloApps\KiloApps\KiloOS\src\App.jsx"
with open(app_jsx_path, "r", encoding="utf-8") as f:
    app_jsx = f.read()

app_jsx = re.sub(
    r"(id:\s*'kfortress'.*?w:\s*)1024(,\s*h:\s*)700", 
    r"\g<1>1024\g<2>740", 
    app_jsx
)

with open(app_jsx_path, "w", encoding="utf-8") as f:
    f.write(app_jsx)

# 2. Update kfortress.html
html_path = r"C:\KiloApps\KiloApps\KiloOS\public\apps\kfortress.html"
with open(html_path, "r", encoding="utf-8") as f:
    html = f.read()

html = html.replace(
    "if (e.key.toLowerCase() === 'h') {",
    "if (e.key.toLowerCase() === 'h' || e.key === 'F1') {\n        if(e.key === 'F1') e.preventDefault();"
)

with open(html_path, "w", encoding="utf-8") as f:
    f.write(html)

# 3. Update main.c
main_c_path = r"C:\KiloApps\KiloApps\KFortress\main.c"
with open(main_c_path, "r", encoding="utf-8") as f:
    main_c = f.read()

main_c = main_c.replace(
    "WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX",
    "(WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN"
)

main_c = main_c.replace(
    "if (wParam == 'h' || wParam == 'H') {",
    "if (wParam == 'h' || wParam == 'H' || wParam == VK_F1) {"
)

main_c = main_c.replace(
    "void Render(HDC hdc, HWND hwnd) {",
    "void Render(HDC hdc, HWND hwnd) {\n    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);"
)

main_c = re.sub(
    r"CreateFontA\(-(\d+),",
    r"CreateFontA(-MulDiv(\1, dpi, 72),",
    main_c
)

with open(main_c_path, "w", encoding="utf-8") as f:
    f.write(main_c)
