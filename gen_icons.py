import struct
import os

def save_ico(filename, pixels):
    width = 32
    height = 32
    bpp = 24
    
    bmp_header = struct.pack('<IiiHHIIIIII', 40, width, height * 2, 1, bpp, 0, 0, 0, 0, 0, 0)
    
    row_size = ((width * bpp + 31) // 32) * 4
    img_data = b''
    # BMP is bottom-up
    for y in range(height - 1, -1, -1):
        row = b''
        for x in range(width):
            val = pixels[y][x]
            if len(val) == 4:
                r, g, b, a = val
            else:
                r, g, b = val
            row += struct.pack('<BBB', b, g, r) # BGR
        row += b'\x00' * (row_size - len(row))
        img_data += row
        
    mask_row_size = ((width + 31) // 32) * 4
    # Create mask: 1 = transparent, 0 = opaque
    mask_data = b''
    for y in range(height - 1, -1, -1):
        row_bits = 0
        for x in range(width):
            if pixels[y][x] == (0, 0, 0, 0): # transparent color marker
                row_bits |= (1 << (7 - (x % 8)))
            if (x % 8) == 7 or x == width - 1:
                mask_data += struct.pack('B', row_bits)
                row_bits = 0
        mask_data += b'\x00' * (mask_row_size - len(mask_data))
        
    full_data = bmp_header + img_data + mask_data
    
    ico_header = struct.pack('<HHH', 0, 1, 1)
    direntry = struct.pack('<BBBBHHII', width, height, 0, 0, 1, bpp, len(full_data), 22)
        
    with open(filename, 'wb') as f:
        f.write(ico_header + direntry + full_data)

def new_img():
    # transparent by default
    return [[(0,0,0,0) for _ in range(32)] for _ in range(32)]

def rect(img, x1, y1, x2, y2, color):
    for y in range(y1, y2+1):
        for x in range(x1, x2+1):
            if 0 <= y < 32 and 0 <= x < 32:
                img[y][x] = color

def circle(img, cx, cy, r, color):
    for y in range(32):
        for x in range(32):
            if (x - cx)**2 + (y - cy)**2 <= r**2:
                img[y][x] = color

def line(img, x0, y0, x1, y1, color):
    dx = abs(x1 - x0)
    dy = abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy
    while True:
        if 0 <= y0 < 32 and 0 <= x0 < 32:
            img[y0][x0] = color
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x0 += sx
        if e2 < dx:
            err += dx
            y0 += sy

def draw_art(art, color_map):
    img = new_img()
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                if 0 <= y < 32 and 0 <= x < 32:
                    img[y][x] = color_map[char]
    return img

def generate_explorer():
    img = new_img()
    rect(img, 2, 6, 12, 10, (200, 150, 50)) # back tab
    rect(img, 8, 4, 24, 20, (255, 255, 255)) # paper inside
    rect(img, 10, 6, 22, 8, (200, 200, 200)) # text line
    rect(img, 10, 10, 18, 12, (200, 200, 200)) # text line
    rect(img, 2, 12, 30, 28, (250, 200, 80)) # front folder
    rect(img, 4, 14, 28, 26, (240, 190, 70)) # inner highlight
    return img

def generate_pad():
    img = new_img()
    rect(img, 4, 2, 26, 30, (230, 230, 250)) # paper
    rect(img, 2, 2, 6, 30, (100, 100, 200)) # binding
    rect(img, 10, 2, 10, 30, (255, 100, 100)) # red margin
    for y in range(6, 28, 4):
        rect(img, 12, y, 24, y, (150, 150, 200)) # lines
    return img

def generate_calc():
    img = new_img()
    rect(img, 4, 2, 28, 30, (180, 180, 180)) # body
    rect(img, 6, 4, 26, 10, (220, 240, 220)) # screen
    for y in range(14, 28, 4):
        for x in range(6, 26, 5):
            color = (255, 150, 50) if x == 21 else (100, 100, 100)
            rect(img, x, y, x+3, y+2, color) # buttons
    return img

def generate_paint():
    img = new_img()
    # palette shape
    rect(img, 4, 10, 28, 28, (210, 180, 140))
    rect(img, 12, 4, 28, 14, (210, 180, 140))
    rect(img, 6, 12, 10, 16, (0, 0, 0, 0)) # thumb hole
    # colors
    rect(img, 14, 8, 18, 12, (255, 0, 0))
    rect(img, 22, 10, 26, 14, (0, 255, 0))
    rect(img, 10, 20, 14, 24, (0, 0, 255))
    rect(img, 18, 22, 22, 26, (255, 255, 0))
    rect(img, 24, 20, 28, 24, (255, 0, 255))
    return img

def generate_mines():
    img = new_img()
    # bomb body
    rect(img, 8, 12, 24, 28, (50, 50, 50))
    rect(img, 10, 10, 22, 30, (50, 50, 50))
    rect(img, 12, 14, 16, 18, (200, 200, 200)) # highlight
    # fuse
    rect(img, 14, 8, 18, 10, (100, 100, 100))
    rect(img, 16, 4, 22, 8, (150, 100, 50))
    rect(img, 22, 2, 26, 6, (255, 100, 0)) # spark
    return img

def generate_clock():
    img = new_img()
    circle(img, 16, 16, 13, (200, 180, 50)) # outer ring
    circle(img, 16, 16, 11, (255, 255, 255)) # face
    # tick marks
    rect(img, 15, 4, 17, 6, (0, 0, 0)) # 12
    rect(img, 15, 26, 17, 28, (0, 0, 0)) # 6
    rect(img, 4, 15, 6, 17, (0, 0, 0)) # 9
    rect(img, 26, 15, 28, 17, (0, 0, 0)) # 3
    rect(img, 15, 15, 17, 17, (0, 0, 0)) # center
    line(img, 16, 16, 16, 8, (0, 0, 0)) # hour hand
    line(img, 16, 16, 24, 16, (0, 0, 0)) # min hand
    line(img, 16, 16, 10, 22, (255, 0, 0)) # second hand
    return img

def generate_task():
    img = new_img()
    rect(img, 4, 2, 28, 30, (230, 230, 230)) # window
    rect(img, 4, 2, 28, 8, (100, 50, 150)) # purple title
    for y in range(12, 28, 5):
        rect(img, 6, y, 10, y+3, (50, 200, 50)) # green check
        rect(img, 12, y+1, 26, y+2, (150, 150, 150)) # line
    return img

def generate_chat():
    img = new_img()
    # Back bubble
    rect(img, 10, 4, 30, 20, (150, 220, 150)) 
    rect(img, 24, 20, 28, 24, (150, 220, 150))
    # Front bubble
    rect(img, 2, 12, 22, 28, (100, 150, 255))
    rect(img, 6, 28, 10, 32, (100, 150, 255))
    # Dots on front
    rect(img, 6, 19, 8, 21, (255, 255, 255))
    rect(img, 11, 19, 13, 21, (255, 255, 255))
    rect(img, 16, 19, 18, 21, (255, 255, 255))
    return img

def generate_server():
    img = new_img()
    rect(img, 6, 2, 26, 30, (50, 50, 50)) # rack
    for y in range(4, 30, 8):
        rect(img, 8, y, 24, y+4, (80, 80, 80)) # units
        rect(img, 10, y+1, 12, y+1, (0, 255, 0)) # lights
        rect(img, 14, y+1, 16, y+1, (0, 0, 255))
    return img

def generate_bbs():
    img = new_img()
    # Monitor frame
    rect(img, 2, 6, 30, 26, (80, 80, 80))   # outer shell
    rect(img, 4, 8, 28, 24, (0, 0, 0))       # screen (black)
    # Green terminal text lines
    rect(img, 6, 10, 20, 10, (0, 255, 0))    # line 1
    rect(img, 6, 13, 24, 13, (0, 200, 0))    # line 2
    rect(img, 6, 16, 18, 16, (0, 180, 0))    # line 3
    rect(img, 6, 19, 8, 19, (0, 255, 0))     # cursor block
    # Antenna
    rect(img, 14, 2, 16, 6, (180, 180, 180)) # mast
    rect(img, 10, 2, 12, 4, (180, 180, 180)) # left tip
    rect(img, 18, 2, 20, 4, (180, 180, 180)) # right tip
    # Base/stand
    rect(img, 12, 26, 20, 28, (100, 100, 100))
    rect(img, 10, 28, 22, 30, (120, 120, 120))
    return img

def generate_snake():
    img = new_img()
    rect(img, 2, 2, 30, 30, (30, 30, 30)) # background
    rect(img, 6, 14, 20, 16, (0, 255, 0)) # snake body
    rect(img, 20, 10, 22, 16, (0, 255, 0)) # snake head turn
    rect(img, 20, 10, 24, 12, (0, 255, 0)) # snake head
    rect(img, 22, 11, 22, 11, (0, 0, 0)) # eye
    rect(img, 8, 8, 10, 10, (255, 0, 0)) # food
    return img

def generate_settings():
    img = new_img()
    circle(img, 15, 15, 12, (150, 150, 150))
    circle(img, 15, 15, 6, (0, 0, 0, 0)) # transparent center
    # Teeth
    rect(img, 13, 1, 17, 3, (150, 150, 150))
    rect(img, 13, 27, 17, 29, (150, 150, 150))
    rect(img, 1, 13, 3, 17, (150, 150, 150))
    rect(img, 27, 13, 29, 17, (150, 150, 150))
    return img

def generate_taskmgr():
    img = new_img()
    rect(img, 2, 4, 30, 28, (50, 50, 50)) # monitor
    rect(img, 4, 6, 28, 26, (0, 0, 0)) # screen
    for x in range(6, 28, 4):
        rect(img, x, 6, x, 26, (0, 50, 0)) # grid lines vertical
    for y in range(8, 26, 4):
        rect(img, 4, y, 28, y, (0, 50, 0)) # grid lines horizontal
    line(img, 4, 20, 10, 16, (0, 255, 0)) # line graph
    line(img, 10, 16, 16, 18, (0, 255, 0))
    line(img, 16, 18, 22, 10, (0, 255, 0))
    line(img, 22, 10, 28, 14, (0, 255, 0))
    return img

def generate_db():
    img = new_img()
    for y_offset in [2, 10, 18]:
        rect(img, 6, y_offset+4, 26, y_offset+10, (100, 100, 200))
        rect(img, 8, y_offset+2, 24, y_offset+4, (150, 150, 250))
    return img

def generate_type():
    img = new_img()
    rect(img, 2, 10, 30, 26, (200, 200, 200))
    for y in [12, 16, 20]:
        for x in range(4, 28, 4):
            rect(img, x, y, x+2, y+2, (100, 100, 100))
    rect(img, 10, 24, 22, 24, (100, 100, 100))
    return img

def generate_zip():
    art = [
        "   YYYYYY   ",
        "   Y    Y   ",
        "   Y    Y   ",
        "   YYYYYY   ",
        "    YYYY    ",
        "     YY     ",
        "    Z  Z    ",
        "     ZZ     ",
        "    Z  Z    ",
        "     ZZ     ",
        "    Z  Z    ",
    ]
    color_map = {'Y': (200, 200, 50), 'Z': (150, 150, 150)}
    img = new_img()
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+4, y*2+4, x*2+5, y*2+5, color_map[char])
    return img

def generate_font():
    art = [
        "     AA     ",
        "    A  A    ",
        "   A    A   ",
        "  AAAAAAAA  ",
        " A        A ",
        "AAA      AAA",
    ]
    color_map = {'A': (0, 0, 0)}
    img = new_img()
    rect(img, 2, 2, 30, 30, (255, 255, 255))
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+4, y*2+10, x*2+5, y*2+11, color_map[char])
    return img

def generate_contacts():
    img = new_img()
    rect(img, 4, 2, 24, 30, (200, 150, 100))
    rect(img, 24, 6, 28, 10, (255, 200, 150))
    rect(img, 24, 14, 28, 18, (255, 150, 100))
    circle(img, 14, 12, 4, (50, 50, 50))
    rect(img, 8, 20, 20, 30, (50, 50, 50))
    return img

def generate_converter():
    img = new_img()
    rect(img, 4, 8, 20, 12, (50, 200, 50))
    rect(img, 20, 4, 24, 16, (50, 200, 50))
    rect(img, 4, 20, 20, 24, (50, 50, 200))
    rect(img, 8, 16, 12, 28, (50, 50, 200))
    return img

def generate_base():
    img = new_img()
    rect(img, 2, 2, 30, 30, (50, 50, 50))
    rect(img, 4, 4, 28, 20, (0, 0, 0))
    rect(img, 6, 6, 10, 8, (0, 255, 0))
    circle(img, 15, 25, 4, (150, 150, 150))
    return img

def generate_budget():
    img = new_img()
    rect(img, 6, 4, 26, 28, (50, 150, 50))
    rect(img, 15, 8, 17, 24, (0, 0, 0))
    rect(img, 10, 8, 22, 10, (0, 0, 0))
    rect(img, 10, 8, 12, 16, (0, 0, 0))
    rect(img, 10, 14, 22, 16, (0, 0, 0))
    rect(img, 20, 14, 22, 24, (0, 0, 0))
    rect(img, 10, 22, 22, 24, (0, 0, 0))
    return img

def generate_habit():
    img = new_img()
    rect(img, 4, 4, 28, 28, (200, 200, 255))
    rect(img, 4, 4, 28, 8, (100, 100, 255))
    for y in range(10, 26, 4):
        for x in range(6, 26, 4):
            rect(img, x, y, x+2, y+2, (255, 255, 255))
    rect(img, 6, 10, 8, 12, (50, 200, 50))
    rect(img, 10, 10, 12, 12, (50, 200, 50))
    rect(img, 6, 14, 8, 16, (50, 200, 50))
    return img

def generate_flash():
    img = new_img()
    rect(img, 4, 6, 28, 26, (255, 255, 255))
    rect(img, 4, 10, 28, 12, (255, 100, 100))
    for y in [16, 20, 24]:
        rect(img, 6, y, 26, y, (150, 150, 255))
    return img

def generate_journal():
    img = new_img()
    rect(img, 6, 2, 26, 30, (150, 50, 50))
    rect(img, 6, 2, 10, 30, (100, 30, 30))
    rect(img, 22, 10, 28, 14, (200, 200, 100))
    return img

def generate_read():
    img = new_img()
    rect(img, 2, 8, 30, 26, (200, 200, 200))
    rect(img, 14, 8, 18, 26, (150, 150, 150))
    for y in range(10, 24, 3):
        rect(img, 4, y, 12, y, (100, 100, 100))
    for y in range(10, 24, 3):
        rect(img, 20, y, 28, y, (100, 100, 100))
    return img

def generate_todo():
    img = new_img()
    rect(img, 6, 2, 26, 30, (255, 255, 255))
    for y in [8, 16, 24]:
        rect(img, 8, y, 12, y+4, (200, 200, 200))
        rect(img, 16, y+2, 24, y+2, (0, 0, 0))
    rect(img, 8, 18, 10, 20, (50, 200, 50))
    rect(img, 10, 14, 12, 16, (50, 200, 50))
    return img

def generate_media():
    img = new_img()
    rect(img, 2, 6, 30, 26, (50, 50, 50))
    rect(img, 4, 8, 28, 20, (0, 0, 0))
    for i in range(6):
        rect(img, 12+i, 10+i, 12+i, 18-i, (0, 200, 0))
    rect(img, 4, 22, 28, 24, (100, 100, 100))
    return img

def generate_color():
    img = new_img()
    circle(img, 12, 12, 8, (255, 0, 0))
    circle(img, 20, 12, 8, (0, 255, 0))
    circle(img, 16, 20, 8, (0, 0, 255))
    return img

os.makedirs('icons', exist_ok=True)
save_ico('icons/kexplorer.ico', generate_explorer())
save_ico('icons/kpad.ico', generate_pad())
save_ico('icons/kcalc.ico', generate_calc())
save_ico('icons/kpaint.ico', generate_paint())
save_ico('icons/kmines.ico', generate_mines())
save_ico('icons/kclock.ico', generate_clock())
save_ico('icons/ktask.ico', generate_task())
save_ico('icons/kchat.ico', generate_chat())
save_ico('icons/kchatserver.ico', generate_server())
save_ico('icons/kbbs.ico', generate_bbs())
save_ico('icons/ksnake.ico', generate_snake())

# Phase 2 Icons
save_ico('icons/ksettings.ico', generate_settings())
save_ico('icons/ktaskmgr.ico', generate_taskmgr())
save_ico('icons/kdb.ico', generate_db())
save_ico('icons/ktype.ico', generate_type())
save_ico('icons/kzip.ico', generate_zip())
save_ico('icons/kfont.ico', generate_font())
save_ico('icons/kcontacts.ico', generate_contacts())
save_ico('icons/kconverter.ico', generate_converter())
save_ico('icons/kbase.ico', generate_base())

def generate_budget():
    art = [
        "    $$    ",
        "   $$$$   ",
        "  $$  $$  ",
        "  $$      ",
        "   $$$$   ",
        "      $$  ",
        "  $$  $$  ",
        "   $$$$   ",
        "    $$    "
    ]
    color_map = {'$': (50, 200, 50)}
    img = new_img()
    rect(img, 4, 4, 28, 28, (240, 240, 240)) # ledger background
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+6, y*2+6, x*2+7, y*2+7, color_map[char])
    return img

def generate_habit():
    img = new_img()
    rect(img, 2, 2, 30, 30, (250, 250, 250))
    for y in range(6, 26, 6):
        rect(img, 4, y, 8, y+4, (200, 200, 200)) # boxes
        rect(img, 12, y+2, 28, y+2, (150, 150, 150)) # lines
    # Check one box
    rect(img, 5, 13, 7, 15, (0, 200, 0))
    return img

def generate_flash():
    img = new_img()
    rect(img, 4, 6, 28, 24, (255, 255, 200)) # card
    rect(img, 6, 10, 26, 10, (255, 100, 100)) # red line
    for y in range(14, 22, 4):
        rect(img, 6, y, 26, y, (150, 150, 255)) # blue lines
    # A letter or symbol
    art = [
        "  A  ",
        " A A ",
        "AAAAA",
        "A   A"
    ]
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char == 'A':
                if 12+y < 32 and 13+x < 32:
                    img[12+y][13+x] = (0, 0, 0)
    return img

def generate_journal():
    img = new_img()
    rect(img, 6, 2, 26, 30, (150, 50, 50)) # cover
    rect(img, 4, 2, 8, 30, (100, 30, 30)) # spine
    rect(img, 14, 12, 18, 18, (200, 200, 50)) # lock
    circle(img, 16, 15, 1, (0, 0, 0)) # keyhole
    return img

def generate_read():
    img = new_img()
    rect(img, 2, 6, 30, 26, (100, 100, 100)) # cover back
    rect(img, 4, 8, 14, 24, (250, 250, 250)) # left page
    rect(img, 18, 8, 28, 24, (250, 250, 250)) # right page
    rect(img, 14, 8, 18, 24, (200, 200, 200)) # spine
    # lines
    for y in range(10, 22, 3):
        rect(img, 6, y, 12, y, (150, 150, 150))
        rect(img, 20, y, 26, y, (150, 150, 150))
    return img

def generate_todo():
    img = new_img()
    rect(img, 6, 2, 26, 30, (255, 255, 255))
    rect(img, 8, 4, 24, 6, (50, 50, 200)) # title
    for y in range(10, 28, 6):
        rect(img, 8, y, 10, y+2, (100, 100, 100)) # box
        rect(img, 14, y+1, 24, y+1, (150, 150, 150)) # line
    # big red checkmark
    rect(img, 8, 11, 9, 12, (255, 0, 0))
    rect(img, 10, 13, 11, 14, (255, 0, 0))
    rect(img, 12, 9, 13, 12, (255, 0, 0))
    return img

def generate_media():
    img = new_img()
    rect(img, 4, 4, 28, 28, (50, 50, 50)) # film strip
    for y in range(6, 28, 6):
        rect(img, 6, y, 8, y+2, (255, 255, 255)) # holes
        rect(img, 24, y, 26, y+2, (255, 255, 255)) # holes
    # play button
    for x in range(12, 20):
        y1 = 16 - (x - 12) // 2
        y2 = 16 + (x - 12) // 2
        rect(img, x, y1, x, y2, (255, 200, 50))
    return img

def generate_color():
    img = new_img()
    circle(img, 16, 16, 12, (200, 200, 200)) # palette
    circle(img, 10, 22, 3, (0, 0, 0, 0)) # thumb hole
    circle(img, 12, 10, 2, (255, 0, 0))
    circle(img, 18, 8, 2, (0, 255, 0))
    circle(img, 24, 14, 2, (0, 0, 255))
    circle(img, 20, 22, 2, (255, 255, 0))
    return img

# Phase 3 Icons
save_ico('icons/kbudget.ico', generate_budget())
save_ico('icons/khabit.ico', generate_habit())
save_ico('icons/kflash.ico', generate_flash())
save_ico('icons/kjournal.ico', generate_journal())
save_ico('icons/kread.ico', generate_read())
save_ico('icons/ktodo.ico', generate_todo())
save_ico('icons/kmedia.ico', generate_media())
save_ico('icons/kcolor.ico', generate_color())

def generate_breakout():
    img = new_img()
    rect(img, 2, 2, 30, 30, (20, 20, 20)) # background
    # bricks
    for x in range(4, 28, 6):
        rect(img, x, 4, x+4, 6, (255, 100, 100))
        rect(img, x, 8, x+4, 10, (100, 255, 100))
        rect(img, x, 12, x+4, 14, (100, 100, 255))
    # paddle
    rect(img, 12, 26, 20, 28, (200, 200, 200))
    # ball
    rect(img, 15, 22, 17, 24, (255, 255, 0))
    return img

def generate_2048():
    img = new_img()
    rect(img, 2, 2, 30, 30, (187, 173, 160)) # background
    for y in range(4, 28, 6):
        for x in range(4, 28, 6):
            rect(img, x, y, x+4, y+4, (205, 193, 180)) # empty cells
    # filled cells
    rect(img, 4, 10, 8, 14, (237, 224, 200)) # 2
    rect(img, 10, 16, 14, 20, (242, 177, 121)) # 8
    rect(img, 16, 10, 20, 14, (245, 149, 99)) # 16
    rect(img, 22, 22, 26, 26, (237, 204, 97)) # 2048
    return img

def generate_solitaire():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 128, 0)) # felt background
    # card 1
    rect(img, 6, 6, 16, 20, (255, 255, 255))
    rect(img, 10, 10, 12, 12, (255, 0, 0)) # diamond
    # card 2
    rect(img, 12, 12, 22, 26, (255, 255, 255))
    rect(img, 16, 16, 18, 18, (0, 0, 0)) # spade
    return img

def generate_space():
    img = new_img()
    rect(img, 2, 2, 30, 30, (10, 10, 30)) # space
    # stars
    rect(img, 6, 6, 6, 6, (255, 255, 255))
    rect(img, 24, 10, 24, 10, (255, 255, 255))
    rect(img, 10, 20, 10, 20, (255, 255, 255))
    # ship
    rect(img, 14, 20, 18, 26, (200, 200, 200))
    rect(img, 16, 16, 16, 19, (255, 100, 100)) # nose
    # wings
    rect(img, 10, 24, 13, 26, (150, 150, 150))
    rect(img, 19, 24, 22, 26, (150, 150, 150))
    # flame
    rect(img, 15, 27, 17, 29, (255, 150, 0))
    return img

def generate_pac():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 0)) # background
    circle(img, 16, 16, 10, (255, 255, 0)) # body
    # mouth
    for y in range(16, 26):
        for x in range(16, 28):
            if (x - 16) >= (y - 16) and (x - 16) >= -(y - 16):
                if 0 <= y < 32 and 0 <= x < 32:
                    img[y][x] = (0, 0, 0)
    # eye
    circle(img, 16, 10, 2, (0, 0, 0))
    # pellet
    circle(img, 26, 16, 2, (255, 255, 255))
    return img

def generate_chess():
    img = new_img()
    rect(img, 2, 2, 30, 30, (100, 150, 200)) # background
    # pawn
    circle(img, 16, 10, 4, (255, 255, 255))
    rect(img, 14, 14, 18, 16, (255, 255, 255))
    rect(img, 12, 17, 20, 20, (255, 255, 255))
    rect(img, 14, 20, 18, 24, (255, 255, 255))
    rect(img, 10, 25, 22, 28, (255, 255, 255))
    return img

def generate_maze():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 0)) # background
    # walls
    for y in range(4, 28, 4):
        for x in range(4, 28, 4):
            rect(img, x, 4, x+2, 28, (0, 0, 255))
            rect(img, 4, y, 28, y+2, (0, 0, 255))
    # carve paths
    rect(img, 6, 4, 8, 8, (0, 0, 0))
    rect(img, 10, 8, 16, 10, (0, 0, 0))
    rect(img, 20, 12, 22, 20, (0, 0, 0))
    # player / goal
    rect(img, 6, 6, 8, 8, (255, 255, 0))
    rect(img, 22, 22, 24, 24, (255, 0, 0))
    return img

# Phase 4 Icons
save_ico('icons/kbreakout.ico', generate_breakout())
save_ico('icons/k2048.ico', generate_2048())
save_ico('icons/ksolitaire.ico', generate_solitaire())
save_ico('icons/kspace.ico', generate_space())
save_ico('icons/kpac.ico', generate_pac())
save_ico('icons/kchess.ico', generate_chess())
save_ico('icons/kmaze.ico', generate_maze())

def generate_radio():
    img = new_img()
    rect(img, 4, 8, 28, 26, (150, 100, 50)) # wood case
    rect(img, 6, 12, 16, 22, (50, 50, 50)) # speaker grill
    for x in range(6, 16, 2):
        rect(img, x, 12, x, 22, (0, 0, 0)) # lines
    circle(img, 22, 16, 4, (200, 200, 200)) # dial
    rect(img, 16, 2, 18, 8, (100, 100, 100)) # antenna
    return img

def generate_graph():
    img = new_img()
    rect(img, 2, 2, 30, 30, (255, 255, 255))
    # Edges
    line(img, 8, 8, 24, 12, (100, 100, 100))
    line(img, 24, 12, 16, 24, (100, 100, 100))
    line(img, 16, 24, 8, 8, (100, 100, 100))
    # Nodes
    circle(img, 8, 8, 3, (0, 0, 255))
    circle(img, 24, 12, 3, (0, 0, 255))
    circle(img, 16, 24, 3, (0, 0, 255))
    return img

def generate_vault():
    img = new_img()
    rect(img, 2, 2, 30, 30, (100, 100, 100)) # metal safe
    rect(img, 4, 4, 28, 28, (80, 80, 80)) # door
    circle(img, 16, 16, 6, (150, 150, 150)) # dial
    circle(img, 16, 16, 4, (50, 50, 50)) # inner dial
    rect(img, 15, 10, 17, 12, (255, 0, 0)) # notch
    # hinge
    rect(img, 4, 8, 6, 12, (50, 50, 50))
    rect(img, 4, 20, 6, 24, (50, 50, 50))
    return img

def generate_quarantine():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 0))
    # biohazard symbol (approximate with circles)
    circle(img, 16, 16, 10, (255, 255, 0))
    circle(img, 16, 16, 8, (0, 0, 0))
    circle(img, 10, 10, 4, (255, 255, 0))
    circle(img, 22, 10, 4, (255, 255, 0))
    circle(img, 16, 22, 4, (255, 255, 0))
    circle(img, 16, 16, 2, (255, 255, 0))
    return img

# Phase 5 Icons
save_ico('icons/kradio.ico', generate_radio())
save_ico('icons/kgraph.ico', generate_graph())
save_ico('icons/kvault.ico', generate_vault())
save_ico('icons/kquarantine.ico', generate_quarantine())

def generate_term():
    img = new_img()
    rect(img, 2, 4, 30, 28, (30, 30, 30)) # terminal background
    rect(img, 2, 4, 30, 8, (200, 200, 200)) # title bar
    rect(img, 26, 5, 28, 7, (255, 50, 50)) # close button
    rect(img, 4, 12, 10, 14, (0, 255, 0)) # prompt
    rect(img, 12, 12, 16, 14, (0, 255, 0)) # cursor
    return img

save_ico('icons/kterm.ico', generate_term())
def generate_rogue():
    img = new_img()
    rect(img, 4, 4, 28, 28, (50, 50, 50))
    # Draw @
    art = [
        " @@@ ",
        "@   @",
        "@ @ @",
        "@   @",
        " @@@@"
    ]
    color_map = {'@': (0, 255, 0)}
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+10, y*2+10, x*2+11, y*2+11, color_map[char])
    return img

def generate_tetris():
    img = new_img()
    rect(img, 4, 2, 28, 30, (0, 0, 0))
    rect(img, 6, 20, 10, 28, (255, 0, 0)) # I block
    rect(img, 12, 24, 20, 28, (0, 255, 0)) # L block
    rect(img, 16, 20, 20, 24, (0, 255, 0))
    rect(img, 22, 24, 26, 28, (255, 255, 0)) # O block
    rect(img, 22, 20, 26, 24, (255, 255, 0))
    return img

def generate_pong():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 0))
    rect(img, 4, 10, 6, 22, (255, 255, 255)) # left paddle
    rect(img, 26, 14, 28, 26, (255, 255, 255)) # right paddle
    rect(img, 14, 16, 16, 18, (255, 255, 255)) # ball
    for y in range(4, 30, 4):
        rect(img, 15, y, 16, y+2, (150, 150, 150)) # net
    return img

def generate_term():
    img = new_img()
    rect(img, 2, 4, 30, 28, (20, 20, 20)) # background
    rect(img, 2, 4, 30, 8, (200, 200, 200)) # title bar
    # >_
    rect(img, 4, 12, 6, 14, (0, 255, 0))
    rect(img, 6, 14, 8, 16, (0, 255, 0))
    rect(img, 4, 16, 6, 18, (0, 255, 0))
    rect(img, 10, 16, 16, 18, (0, 255, 0))
    return img

def generate_audio():
    img = new_img()
    rect(img, 2, 2, 30, 30, (255, 255, 255))
    # Beamed eighth notes
    rect(img, 8, 8, 24, 10, (0, 0, 0)) # beam
    rect(img, 8, 8, 10, 22, (0, 0, 0)) # left stem
    rect(img, 22, 8, 24, 22, (0, 0, 0)) # right stem
    circle(img, 6, 24, 4, (0, 0, 0))
    circle(img, 20, 24, 4, (0, 0, 0))
    return img

def generate_calendar():
    img = new_img()
    rect(img, 4, 6, 28, 28, (255, 255, 255)) # page
    rect(img, 4, 6, 28, 12, (255, 0, 0)) # header
    rect(img, 8, 2, 10, 8, (150, 150, 150)) # ring
    rect(img, 22, 2, 24, 8, (150, 150, 150)) # ring
    for y in range(16, 26, 4):
        for x in range(6, 26, 4):
            rect(img, x, y, x+2, y+2, (200, 200, 200))
    rect(img, 14, 16, 16, 18, (0, 0, 255)) # current day
    return img

def generate_mail():
    img = new_img()
    rect(img, 2, 8, 30, 24, (250, 250, 250)) # envelope
    # flaps
    line(img, 2, 8, 16, 16, (150, 150, 150))
    line(img, 30, 8, 16, 16, (150, 150, 150))
    line(img, 2, 24, 16, 16, (200, 200, 200))
    line(img, 30, 24, 16, 16, (200, 200, 200))
    return img

def generate_image():
    img = new_img()
    rect(img, 2, 4, 30, 28, (139, 69, 19)) # frame
    rect(img, 4, 6, 28, 26, (135, 206, 235)) # sky
    circle(img, 22, 10, 3, (255, 255, 0)) # sun
    # mountains
    for y in range(16, 26):
        for x in range(4, 28):
            if (x - 10) >= (16 - y) and (x - 10) <= (y - 16):
                rect(img, x, y, x, y, (34, 139, 34)) # left mountain
            if (x - 20) >= (18 - y) and (x - 20) <= (y - 18):
                rect(img, x, y, x, y, (0, 100, 0)) # right mountain
    return img

def generate_net():
    img = new_img()
    rect(img, 2, 2, 30, 30, (50, 50, 50))
    circle(img, 16, 16, 10, (0, 0, 255))
    circle(img, 16, 16, 10, (0, 0, 0, 0)) # transparent outline somehow?
    line(img, 16, 6, 16, 26, (150, 150, 255))
    line(img, 6, 16, 26, 16, (150, 150, 255))
    for x in range(8, 24):
        y1 = 16 - (x - 16)**2 // 4
        y2 = 16 + (x - 16)**2 // 4
        if 0 <= y1 < 32: img[y1][x] = (150, 150, 255)
        if 0 <= y2 < 32: img[y2][x] = (150, 150, 255)
    return img

def generate_script():
    img = new_img()
    rect(img, 4, 2, 28, 30, (40, 40, 40))
    # Draw {}
    art = [
        "  { }  ",
        " {   } ",
        "{     }",
        " {   } ",
        "  { }  "
    ]
    color_map = {'{': (255, 255, 0), '}': (255, 255, 0)}
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+8, y*2+10, x*2+9, y*2+11, color_map[char])
    return img

def generate_chart():
    img = new_img()
    rect(img, 2, 2, 30, 30, (255, 255, 255))
    rect(img, 4, 4, 6, 28, (0, 0, 0)) # y axis
    rect(img, 4, 26, 28, 28, (0, 0, 0)) # x axis
    rect(img, 8, 16, 12, 26, (255, 0, 0))
    rect(img, 14, 10, 18, 26, (0, 255, 0))
    rect(img, 20, 20, 24, 26, (0, 0, 255))
    return img

def generate_note():
    img = new_img()
    rect(img, 6, 6, 26, 26, (255, 255, 150))
    rect(img, 6, 6, 16, 10, (200, 200, 100)) # tape/fold
    for y in range(12, 24, 4):
        rect(img, 8, y, 24, y, (150, 150, 100))
    return img

def generate_pass():
    img = new_img()
    rect(img, 2, 2, 30, 30, (200, 200, 200))
    circle(img, 10, 16, 4, (255, 215, 0)) # key head
    circle(img, 10, 16, 2, (200, 200, 200)) # hole
    rect(img, 14, 15, 26, 17, (255, 215, 0)) # shaft
    rect(img, 22, 17, 24, 21, (255, 215, 0)) # bit
    rect(img, 26, 17, 28, 21, (255, 215, 0)) # bit
    return img

def generate_ping():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 0))
    circle(img, 16, 16, 12, (0, 100, 0))
    circle(img, 16, 16, 8, (0, 150, 0))
    circle(img, 16, 16, 4, (0, 255, 0))
    line(img, 16, 16, 24, 8, (0, 255, 0)) # sweep line
    circle(img, 20, 10, 2, (255, 255, 255)) # blip
    return img

def generate_hex():
    img = new_img()
    rect(img, 2, 2, 30, 30, (30, 30, 30))
    art = [
        " 00  XX ",
        "0  0  X ",
        "0  0   X",
        "0  0  X ",
        " 00  XX "
    ]
    color_map = {'0': (0, 255, 0), 'X': (0, 255, 0)}
    for y, row in enumerate(art):
        for x, char in enumerate(row):
            if char in color_map:
                rect(img, x*2+8, y*2+10, x*2+9, y*2+11, color_map[char])
    return img

def generate_sys():
    img = new_img()
    rect(img, 8, 8, 24, 24, (50, 50, 50))
    rect(img, 10, 10, 22, 22, (30, 30, 30))
    for i in range(10, 22, 4):
        rect(img, 4, i, 8, i+2, (150, 150, 150)) # pins left
        rect(img, 24, i, 28, i+2, (150, 150, 150)) # pins right
        rect(img, i, 4, i+2, 8, (150, 150, 150)) # pins top
        rect(img, i, 24, i+2, 28, (150, 150, 150)) # pins bottom
    return img

def generate_mandel():
    img = new_img()
    rect(img, 2, 2, 30, 30, (0, 0, 50))
    circle(img, 16, 16, 8, (0, 0, 0))
    circle(img, 10, 16, 4, (0, 0, 0))
    circle(img, 24, 16, 2, (0, 0, 0))
    return img

def generate_timer():
    img = new_img()
    rect(img, 4, 4, 28, 28, (255, 255, 255))
    rect(img, 10, 6, 22, 8, (100, 50, 0)) # top
    rect(img, 10, 24, 22, 26, (100, 50, 0)) # bottom
    for x in range(12, 20):
        y = 16 - abs(x - 16)
        rect(img, x, y, x, 16 + abs(x - 16), (200, 200, 255)) # glass
    # sand
    for x in range(14, 18):
        rect(img, x, 22, x, 24, (255, 200, 100))
    return img

def generate_synth():
    img = new_img()
    rect(img, 2, 8, 30, 24, (50, 50, 50))
    for x in range(4, 28, 4):
        rect(img, x, 12, x+2, 22, (255, 255, 255)) # white keys
    for x in [6, 10, 18, 22]:
        rect(img, x, 12, x+1, 16, (0, 0, 0)) # black keys
    return img

save_ico("icons/krogue.ico", generate_rogue())
save_ico("icons/ktetris.ico", generate_tetris())
save_ico("icons/kpong.ico", generate_pong())
save_ico("icons/kterm.ico", generate_term())
save_ico("icons/kaudio.ico", generate_audio())
save_ico("icons/kcalendar.ico", generate_calendar())
save_ico("icons/kmail.ico", generate_mail())
save_ico("icons/kimage.ico", generate_image())
save_ico("icons/knet.ico", generate_net())
save_ico("icons/kscript.ico", generate_script())
save_ico("icons/kchart.ico", generate_chart())
save_ico("icons/knote.ico", generate_note())
save_ico("icons/kpass.ico", generate_pass())
save_ico("icons/kping.ico", generate_ping())
save_ico("icons/khex.ico", generate_hex())
save_ico("icons/ksys.ico", generate_sys())
save_ico("icons/kmandel.ico", generate_mandel())
save_ico("icons/ktimer.ico", generate_timer())
save_ico("icons/ksynth.ico", generate_synth())
