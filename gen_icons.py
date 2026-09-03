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


def generate_sudoku():
    img = new_img()
    rect(img, 2, 2, 29, 29, (240, 240, 245))
    rect(img, 2, 2, 29, 3, (60, 60, 80))
    rect(img, 2, 28, 29, 29, (60, 60, 80))
    rect(img, 2, 2, 3, 29, (60, 60, 80))
    rect(img, 28, 2, 29, 29, (60, 60, 80))
    rect(img, 10, 2, 11, 29, (60, 60, 80))
    rect(img, 19, 2, 20, 29, (60, 60, 80))
    rect(img, 2, 10, 29, 11, (60, 60, 80))
    rect(img, 2, 19, 29, 20, (60, 60, 80))
    for p in [6, 15, 24]:
        line(img, p, 2, p, 29, (180, 180, 200))
        line(img, 2, p, 29, p, (180, 180, 200))
    rect(img, 5, 5, 8, 5, (30, 90, 220))
    rect(img, 5, 6, 5, 7, (30, 90, 220))
    rect(img, 5, 7, 8, 7, (30, 90, 220))
    rect(img, 8, 8, 8, 9, (30, 90, 220))
    rect(img, 5, 9, 8, 9, (30, 90, 220))
    rect(img, 14, 13, 17, 13, (220, 40, 40))
    rect(img, 17, 14, 17, 16, (220, 40, 40))
    rect(img, 14, 15, 16, 15, (220, 40, 40))
    rect(img, 14, 17, 17, 17, (220, 40, 40))
    rect(img, 23, 22, 26, 22, (40, 160, 60))
    rect(img, 23, 23, 23, 24, (40, 160, 60))
    rect(img, 26, 23, 26, 26, (40, 160, 60))
    rect(img, 23, 24, 26, 24, (40, 160, 60))
    rect(img, 23, 26, 26, 26, (40, 160, 60))
    return img

def generate_connect4():
    img = new_img()
    rect(img, 2, 2, 29, 29, (20, 80, 200))
    discs = [
        (7, 24, (240, 40, 40)),
        (13, 24, (240, 210, 40)),
        (19, 24, (240, 40, 40)),
        (25, 24, (240, 210, 40)),
        (7, 18, (240, 210, 40)),
        (13, 18, (240, 40, 40)),
        (19, 18, (240, 40, 40)),
        (13, 12, (240, 210, 40)),
    ]
    for cx, cy in [(7,6),(13,6),(19,6),(25,6),(7,12),(19,12),(25,12),(7,18),(25,18)]:
        circle(img, cx, cy, 2, (15, 30, 80))
    for cx, cy, col in discs:
        circle(img, cx, cy, 2, col)
    return img

def generate_hangman():
    img = new_img()
    rect(img, 2, 2, 29, 29, (35, 35, 45))
    rect(img, 4, 26, 20, 28, (140, 90, 40))
    rect(img, 8, 6, 11, 26, (140, 90, 40))
    rect(img, 8, 6, 24, 9, (140, 90, 40))
    line(img, 11, 14, 16, 9, (110, 70, 30))
    rect(img, 21, 9, 21, 13, (210, 180, 120))
    circle(img, 21, 15, 2, (210, 180, 120))
    circle(img, 21, 15, 1, (255, 220, 180))
    line(img, 21, 17, 21, 22, (255, 255, 255))
    line(img, 21, 19, 18, 21, (255, 255, 255))
    line(img, 21, 19, 24, 21, (255, 255, 255))
    line(img, 21, 22, 19, 25, (255, 255, 255))
    line(img, 21, 22, 23, 25, (255, 255, 255))
    return img

def generate_simon():
    img = new_img()
    rect(img, 2, 2, 29, 29, (20, 20, 25))
    circle(img, 16, 16, 13, (50, 50, 60))
    rect(img, 4, 4, 15, 15, (0, 220, 60))
    rect(img, 17, 4, 28, 15, (240, 40, 50))
    rect(img, 4, 17, 15, 28, (240, 220, 30))
    rect(img, 17, 17, 28, 28, (30, 120, 240))
    rect(img, 15, 4, 16, 28, (30, 30, 35))
    rect(img, 4, 15, 28, 16, (30, 30, 35))
    circle(img, 16, 16, 5, (30, 30, 40))
    circle(img, 16, 16, 3, (180, 180, 190))
    return img

def generate_asteroids():
    img = new_img()
    rect(img, 2, 2, 29, 29, (5, 5, 15))
    rect(img, 5, 6, 5, 6, (255, 255, 255))
    rect(img, 27, 8, 27, 8, (200, 200, 255))
    rect(img, 7, 25, 7, 25, (255, 255, 200))
    rect(img, 24, 26, 24, 26, (255, 255, 255))
    line(img, 10, 24, 6, 27, (0, 255, 255))
    line(img, 6, 27, 14, 27, (0, 255, 255))
    line(img, 14, 27, 10, 24, (0, 255, 255))
    line(img, 10, 23, 10, 12, (255, 255, 0))
    rect(img, 18, 5, 26, 13, (120, 110, 100))
    rect(img, 20, 3, 24, 15, (120, 110, 100))
    rect(img, 20, 7, 22, 9, (80, 75, 70))
    rect(img, 6, 10, 10, 14, (140, 130, 120))
    return img

def generate_freecell():
    img = new_img()
    rect(img, 2, 2, 29, 29, (10, 110, 50))
    for x in [4, 9, 14, 19]:
        rect(img, x, 4, x+3, 8, (0, 180, 140))
        rect(img, x+1, 5, x+2, 7, (10, 110, 50))
    rect(img, 25, 4, 28, 8, (240, 200, 40))
    cards = [(4, 11, (255,255,255), (220,30,30)), (10, 13, (255,255,255), (20,20,20)), (16, 10, (255,255,255), (220,30,30)), (22, 12, (255,255,255), (20,20,20))]
    for cx, cy, bg, pip in cards:
        rect(img, cx, cy, cx+5, cy+14, bg)
        rect(img, cx+2, cy+3, cx+3, cy+5, pip)
    return img

def generate_match3():
    img = new_img()
    rect(img, 2, 2, 29, 29, (25, 20, 35))
    circle(img, 7, 7, 3, (240, 40, 60))
    rect(img, 6, 6, 7, 7, (255, 180, 190))
    rect(img, 13, 5, 19, 9, (40, 220, 80))
    rect(img, 15, 6, 16, 7, (200, 255, 210))
    circle(img, 25, 7, 3, (40, 120, 240))
    rect(img, 24, 6, 25, 7, (180, 220, 255))
    line(img, 7, 13, 10, 16, (250, 210, 40))
    line(img, 10, 16, 7, 19, (250, 210, 40))
    line(img, 7, 19, 4, 16, (250, 210, 40))
    line(img, 4, 16, 7, 13, (250, 210, 40))
    rect(img, 5, 15, 9, 17, (250, 210, 40))
    circle(img, 16, 16, 4, (180, 50, 230))
    rect(img, 15, 14, 16, 15, (240, 190, 255))
    circle(img, 25, 16, 3, (40, 220, 240))
    line(img, 3, 16, 28, 16, (255, 255, 255))
    return img

def generate_words():
    img = new_img()
    rect(img, 2, 2, 29, 29, (45, 30, 20))
    tiles = [
        (4, 8, 'K'),
        (10, 8, 'W'),
        (16, 8, 'O'),
        (22, 8, 'R')
    ]
    for tx, ty, char in tiles:
        rect(img, tx, ty, tx+5, ty+14, (245, 235, 210))
        rect(img, tx+1, ty+1, tx+4, ty+13, (255, 250, 230))
        rect(img, tx, ty, tx+5, ty, (180, 160, 130))
        rect(img, tx, ty+14, tx+5, ty+14, (180, 160, 130))
        rect(img, tx, ty, tx, ty+14, (180, 160, 130))
        rect(img, tx+5, ty, tx+5, ty+14, (180, 160, 130))
    line(img, 6, 11, 6, 17, (60, 40, 20))
    line(img, 6, 14, 8, 11, (60, 40, 20))
    line(img, 6, 14, 8, 17, (60, 40, 20))
    line(img, 11, 11, 11, 17, (60, 40, 20))
    line(img, 11, 17, 12, 14, (60, 40, 20))
    line(img, 12, 14, 13, 17, (60, 40, 20))
    line(img, 13, 17, 13, 11, (60, 40, 20))
    rect(img, 17, 12, 19, 16, (60, 40, 20))
    rect(img, 18, 13, 18, 15, (255, 250, 230))
    line(img, 23, 11, 23, 17, (60, 40, 20))
    rect(img, 23, 11, 25, 14, (60, 40, 20))
    rect(img, 24, 12, 24, 13, (255, 250, 230))
    line(img, 24, 14, 25, 17, (60, 40, 20))
    return img

def generate_go():
    img = new_img()
    rect(img, 2, 2, 29, 29, (220, 160, 70))
    for p in [7, 12, 16, 20, 24]:
        line(img, p, 5, p, 26, (80, 50, 20))
        line(img, 5, p, 26, p, (80, 50, 20))
    rect(img, 16, 16, 16, 16, (80, 50, 20))
    circle(img, 12, 12, 3, (20, 20, 25))
    rect(img, 11, 11, 11, 11, (100, 100, 110))
    circle(img, 20, 20, 3, (20, 20, 25))
    rect(img, 19, 19, 19, 19, (100, 100, 110))
    circle(img, 20, 12, 3, (245, 245, 250))
    rect(img, 19, 11, 19, 11, (255, 255, 255))
    circle(img, 12, 20, 3, (245, 245, 250))
    rect(img, 11, 19, 11, 19, (255, 255, 255))
    return img

def generate_darts():
    img = new_img()
    rect(img, 2, 2, 29, 29, (25, 25, 30))
    circle(img, 16, 16, 13, (20, 20, 20))
    circle(img, 16, 16, 11, (220, 210, 180))
    circle(img, 16, 16, 10, (200, 40, 40))
    circle(img, 16, 16, 8, (220, 210, 180))
    circle(img, 16, 16, 6, (40, 160, 60))
    circle(img, 16, 16, 4, (220, 210, 180))
    circle(img, 16, 16, 3, (40, 160, 60))
    circle(img, 16, 16, 1, (220, 40, 40))
    line(img, 24, 8, 17, 15, (220, 180, 40))
    rect(img, 23, 7, 27, 10, (40, 200, 240))
    return img

def generate_towers():
    img = new_img()
    rect(img, 2, 2, 29, 29, (10, 15, 30))
    rect(img, 6, 5, 6, 5, (255, 255, 200))
    rect(img, 25, 6, 25, 6, (255, 255, 255))
    rect(img, 4, 14, 10, 28, (50, 70, 100))
    for y in range(16, 27, 3):
        rect(img, 6, y, 8, y+1, (250, 220, 100))
    rect(img, 12, 6, 20, 28, (30, 90, 150))
    rect(img, 15, 2, 16, 6, (220, 40, 40))
    for y in range(8, 27, 3):
        rect(img, 14, y, 15, y+1, (100, 240, 255))
        rect(img, 17, y, 18, y+1, (250, 220, 100))
    rect(img, 22, 10, 28, 28, (40, 60, 90))
    for y in range(12, 27, 3):
        rect(img, 24, y, 26, y+1, (250, 220, 100))
    return img

def generate_reversi():
    img = new_img()
    rect(img, 2, 2, 29, 29, (20, 100, 40))
    rect(img, 2, 2, 29, 3, (120, 70, 30))
    rect(img, 2, 28, 29, 29, (120, 70, 30))
    rect(img, 2, 2, 3, 29, (120, 70, 30))
    rect(img, 28, 2, 29, 29, (120, 70, 30))
    for p in [9, 15, 21]:
        line(img, p, 4, p, 27, (15, 75, 30))
        line(img, 4, p, 27, p, (15, 75, 30))
    circle(img, 12, 12, 2, (245, 245, 250))
    circle(img, 18, 12, 2, (20, 20, 25))
    circle(img, 12, 18, 2, (20, 20, 25))
    circle(img, 18, 18, 2, (245, 245, 250))
    circle(img, 24, 18, 1, (240, 240, 60))
    return img

def generate_quest():
    img = new_img()
    rect(img, 2, 2, 29, 29, (30, 20, 40))
    line(img, 6, 25, 25, 6, (240, 200, 40))
    rect(img, 5, 24, 7, 26, (180, 130, 30))
    line(img, 25, 25, 6, 6, (140, 90, 200))
    circle(img, 6, 6, 3, (80, 220, 255))
    circle(img, 16, 22, 4, (220, 30, 40))
    rect(img, 15, 16, 17, 18, (200, 200, 220))
    return img

def generate_starship():
    img = new_img()
    rect(img, 2, 2, 29, 29, (5, 8, 22))
    rect(img, 4, 5, 4, 5, (255, 255, 255))
    rect(img, 26, 24, 26, 24, (255, 255, 255))
    rect(img, 14, 4, 17, 24, (210, 215, 230))
    rect(img, 12, 10, 19, 20, (170, 175, 190))
    rect(img, 7, 16, 24, 22, (130, 135, 150))
    rect(img, 9, 23, 11, 26, (40, 220, 255))
    rect(img, 20, 23, 22, 26, (40, 220, 255))
    rect(img, 15, 25, 16, 28, (40, 220, 255))
    circle(img, 15, 12, 2, (255, 60, 60))
    return img

def generate_alchemy():
    img = new_img()
    rect(img, 2, 2, 29, 29, (20, 15, 30))
    circle(img, 16, 20, 8, (200, 200, 220))
    circle(img, 16, 20, 6, (40, 220, 120))
    rect(img, 14, 6, 18, 14, (200, 200, 220))
    rect(img, 13, 4, 19, 6, (180, 180, 200))
    circle(img, 12, 5, 1, (240, 100, 255))
    circle(img, 18, 3, 1, (100, 240, 255))
    circle(img, 21, 6, 1, (250, 240, 100))
    return img

def generate_fortress():
    img = new_img()
    rect(img, 2, 2, 29, 29, (130, 180, 230))
    rect(img, 3, 12, 28, 28, (120, 120, 130))
    for x in range(3, 28, 5):
        rect(img, x, 8, x+2, 12, (120, 120, 130))
    rect(img, 12, 18, 19, 28, (90, 55, 25))
    line(img, 12, 18, 19, 18, (60, 35, 15))
    line(img, 5, 2, 5, 8, (60, 60, 60))
    rect(img, 6, 3, 10, 6, (220, 30, 30))
    return img

def generate_colosseum():
    img = new_img()
    rect(img, 2, 2, 29, 29, (139, 0, 0)) # Imperial Roman Crimson
    # Colosseum sandstone arches
    rect(img, 4, 18, 27, 28, (215, 185, 140))
    for x in (6, 12, 18, 24):
        rect(img, x, 20, x+2, 28, (60, 40, 20))
    # Roman helmet / golden laurel & crossed gladius
    circle(img, 15, 11, 6, (212, 175, 55)) # Golden helmet/shield
    rect(img, 13, 8, 17, 10, (180, 20, 20)) # Crest
    line(img, 7, 5, 23, 21, (240, 240, 250)) # Crossed Gladius 1
    line(img, 23, 5, 7, 21, (240, 240, 250)) # Crossed Gladius 2
    circle(img, 15, 11, 2, (139, 0, 0))
    return img

save_ico("icons/ksudoku.ico", generate_sudoku())
save_ico("icons/kconnect4.ico", generate_connect4())
save_ico("icons/khangman.ico", generate_hangman())
save_ico("icons/ksimon.ico", generate_simon())
save_ico("icons/kasteroids.ico", generate_asteroids())
save_ico("icons/kfreecell.ico", generate_freecell())
save_ico("icons/kmatch3.ico", generate_match3())
save_ico("icons/kwords.ico", generate_words())
save_ico("icons/kgo.ico", generate_go())
save_ico("icons/kdarts.ico", generate_darts())
save_ico("icons/ktowers.ico", generate_towers())
save_ico("icons/kreversi.ico", generate_reversi())
save_ico("icons/kquest.ico", generate_quest())
save_ico("icons/kstarship.ico", generate_starship())
save_ico("icons/kalchemy.ico", generate_alchemy())
save_ico("icons/kfortress.ico", generate_fortress())
save_ico("icons/kcolosseum.ico", generate_colosseum())
save_ico("KiloOS/public/assets/icons/kcolosseum.ico", generate_colosseum())

