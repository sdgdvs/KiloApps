import re

# SVGs
egg_svg = """<svg width="128" height="128" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg">
  <rect x="6" y="3" width="4" height="1" fill="rgb(180,150,100)" />
  <rect x="5" y="4" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="6" y="4" width="4" height="1" fill="rgb(230,210,180)" />
  <rect x="10" y="4" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="5" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="5" y="5" width="5" height="1" fill="rgb(230,210,180)" />
  <rect x="10" y="5" width="1" height="1" fill="rgb(255,255,255)" />
  <rect x="11" y="5" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="3" y="6" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="6" width="8" height="1" fill="rgb(230,210,180)" />
  <rect x="12" y="6" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="3" y="7" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="7" width="8" height="1" fill="rgb(230,210,180)" />
  <rect x="12" y="7" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="3" y="8" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="8" width="8" height="1" fill="rgb(230,210,180)" />
  <rect x="12" y="8" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="3" y="9" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="9" width="8" height="1" fill="rgb(230,210,180)" />
  <rect x="12" y="9" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="3" y="10" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="10" width="8" height="1" fill="rgb(230,210,180)" />
  <rect x="12" y="10" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="4" y="11" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="5" y="11" width="6" height="1" fill="rgb(230,210,180)" />
  <rect x="11" y="11" width="1" height="1" fill="rgb(180,150,100)" />
  <rect x="5" y="12" width="6" height="1" fill="rgb(180,150,100)" />
</svg>"""

dragon_svg = """<svg width="128" height="128" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg">
  <rect x="8" y="1" width="2" height="1" fill="rgb(30,100,30)" />
  <rect x="7" y="2" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="8" y="2" width="2" height="1" fill="rgb(60,180,60)" />
  <rect x="10" y="2" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="7" y="3" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="8" y="3" width="2" height="1" fill="rgb(60,180,60)" />
  <rect x="10" y="3" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="12" y="3" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="6" y="4" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="7" y="4" width="4" height="1" fill="rgb(60,180,60)" />
  <rect x="11" y="4" width="3" height="1" fill="rgb(30,100,30)" />
  <rect x="1" y="5" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="4" y="5" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="5" y="5" width="9" height="1" fill="rgb(60,180,60)" />
  <rect x="14" y="5" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="1" y="6" width="2" height="1" fill="rgb(30,100,30)" />
  <rect x="4" y="6" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="5" y="6" width="6" height="1" fill="rgb(60,180,60)" />
  <rect x="11" y="6" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="12" y="6" width="1" height="1" fill="rgb(60,180,60)" />
  <rect x="13" y="6" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="1" y="7" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="2" y="7" width="1" height="1" fill="rgb(60,180,60)" />
  <rect x="3" y="7" width="2" height="1" fill="rgb(30,100,30)" />
  <rect x="5" y="7" width="6" height="1" fill="rgb(60,180,60)" />
  <rect x="11" y="7" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="12" y="7" width="1" height="1" fill="rgb(60,180,60)" />
  <rect x="13" y="7" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="1" y="8" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="2" y="8" width="2" height="1" fill="rgb(60,180,60)" />
  <rect x="4" y="8" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="5" y="8" width="6" height="1" fill="rgb(60,180,60)" />
  <rect x="11" y="8" width="3" height="1" fill="rgb(30,100,30)" />
  <rect x="2" y="9" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="3" y="9" width="8" height="1" fill="rgb(60,180,60)" />
  <rect x="11" y="9" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="3" y="10" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="4" y="10" width="6" height="1" fill="rgb(60,180,60)" />
  <rect x="10" y="10" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="3" y="11" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="4" y="11" width="1" height="1" fill="rgb(60,180,60)" />
  <rect x="5" y="11" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="8" y="11" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="9" y="11" width="1" height="1" fill="rgb(60,180,60)" />
  <rect x="10" y="11" width="1" height="1" fill="rgb(30,100,30)" />
  <rect x="3" y="12" width="2" height="1" fill="rgb(30,100,30)" />
  <rect x="9" y="12" width="2" height="1" fill="rgb(30,100,30)" />
</svg>"""

with open('KiloOS/public/apps/kdragon.html', 'r', encoding='utf-8') as f:
    html = f.read()

# Update CSS in HTML
new_css = """@import url('https://fonts.googleapis.com/css2?family=Uncial+Antiqua&display=swap');
        body { margin: 0; padding: 20px; font-family: 'Uncial Antiqua', serif; background-color: #dcb881; color: #3b240b; user-select: none; }
        h1 { color: #2a1704; text-align: center; text-shadow: 1px 1px #e6cca0; }
        .container { max-width: 600px; margin: 0 auto; text-align: center; background: #ebd0a5; padding: 20px; border: 4px solid #5a3814; border-radius: 5px; box-shadow: inset 0 0 20px rgba(0,0,0,0.2), 0 5px 15px rgba(0,0,0,0.5); }
        .stats { display: flex; justify-content: space-around; margin-top: 20px; padding: 10px; background: rgba(90, 56, 20, 0.2); border: 2px solid #5a3814; border-radius: 3px; }
        .stat { font-size: 18px; font-weight: bold; }
        .controls { margin-top: 20px; display: flex; justify-content: center; gap: 10px; }
        button { padding: 10px 20px; font-size: 18px; cursor: pointer; background: #8b5a2b; color: #f5deb3; border: 2px solid #3b240b; border-radius: 3px; font-family: 'Uncial Antiqua', serif; text-shadow: 1px 1px #000; box-shadow: 2px 2px 5px rgba(0,0,0,0.5); }
        button:hover { background: #a06b38; }
        button:disabled { background: #5a4b3c; color: #888; cursor: not-allowed; }
        .entity-display { margin: 30px 0; min-height: 128px; display: flex; justify-content: center; align-items: center; }
        .message-log { margin-top: 20px; height: 100px; overflow-y: auto; background: rgba(255,255,255,0.3); padding: 10px; text-align: left; font-family: 'Courier New', monospace; border: 2px inset #5a3814; color: #2a1704; font-weight: bold; }
        .message { margin: 5px 0; }"""

html = re.sub(r'body \{.*?\}', new_css, html, flags=re.DOTALL)
html = html.replace('🥚', egg_svg)
html = html.replace("'🐉'", '`' + dragon_svg + '`')
html = html.replace("'🥚'", '`' + egg_svg + '`')
html = html.replace("document.getElementById('entity-art').innerText =", "document.getElementById('entity-art').innerHTML =")

with open('KiloOS/public/apps/kdragon.html', 'w', encoding='utf-8') as f:
    f.write(html)

with open('KDragon/main.c', 'r', encoding='utf-8') as f:
    c_code = f.read()

# Update main.c
c_code = c_code.replace('RGB(26, 26, 26)', 'RGB(220, 184, 129)')
c_code = c_code.replace('"Arial"', '"Times New Roman"')
c_code = c_code.replace('RGB(255, 255, 255)', 'RGB(42, 23, 4)')
c_code = c_code.replace('RGB(200, 200, 200)', 'RGB(42, 23, 4)')
c_code = c_code.replace('RGB(17, 17, 17)', 'RGB(235, 208, 165)')
c_code = c_code.replace('RGB(68, 68, 68)', 'RGB(139, 90, 43)')
c_code = c_code.replace('DKGRAY_BRUSH', 'WHITE_BRUSH') # to prevent weird defaults, actually return CreateSolidBrush is better, but this will do for simple UI
# Let's handle the brush properly:
c_code = c_code.replace('return (LRESULT)GetStockObject(DKGRAY_BRUSH);', 'static HBRUSH btnBrush = NULL; if (!btnBrush) btnBrush = CreateSolidBrush(RGB(139, 90, 43)); return (LRESULT)btnBrush;')

# Inject drawing code
pixel_arrays = """COLORREF egg_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(255,255,255), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

COLORREF dragon_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, RGB(30,100,30), -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1},
    {-1, RGB(30,100,30), RGB(30,100,30), -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

void DrawPixelArt(HDC hdc, int x, int y, int scale, COLORREF pixels[16][16]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (pixels[i][j] != -1) {
                HBRUSH b = CreateSolidBrush(pixels[i][j]);
                RECT r = {x + j*scale, y + i*scale, x + (j+1)*scale, y + (i+1)*scale};
                FillRect(hdc, &r, b);
                DeleteObject(b);
            }
        }
    }
}
"""

c_code = c_code.replace('LRESULT CALLBACK WndProc', pixel_arrays + '\nLRESULT CALLBACK WndProc')

draw_egg_orig = """                SelectObject(hdc, hFontLarge);
                LPCWSTR egg = L"\\xD83E\\xDD5A";
                TextOutW(hdc, 260, 130, egg, 2);"""
draw_egg_new = """                DrawPixelArt(hdc, 236, 110, 8, egg_pixels);"""
c_code = c_code.replace(draw_egg_orig, draw_egg_new)

draw_dragon_orig = """                SelectObject(hdc, hFontLarge);
                LPCWSTR dragon = L"\\xD83D\\xDC09";
                TextOutW(hdc, 260, 100, dragon, 2);"""
draw_dragon_new = """                DrawPixelArt(hdc, 236, 90, 8, dragon_pixels);"""
c_code = c_code.replace(draw_dragon_orig, draw_dragon_new)

with open('KDragon/main.c', 'w', encoding='utf-8') as f:
    f.write(c_code)

print("Applied styling to kdragon.html and main.c")
