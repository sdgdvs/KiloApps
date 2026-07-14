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
    rect(img, 2, 10, 30, 26, (250, 200, 80)) # front folder
    rect(img, 4, 12, 28, 24, (240, 190, 70)) # inner highlight
    return img

def generate_pad():
    img = new_img()
    rect(img, 4, 2, 26, 30, (230, 230, 250)) # paper
    rect(img, 2, 2, 6, 30, (100, 100, 200)) # binding
    for y in range(6, 28, 4):
        rect(img, 8, y, 22, y, (150, 150, 150)) # lines
    return img

def generate_calc():
    img = new_img()
    rect(img, 4, 2, 28, 30, (180, 180, 180)) # body
    rect(img, 6, 4, 26, 10, (220, 240, 220)) # screen
    for y in range(14, 28, 4):
        for x in range(6, 26, 5):
            rect(img, x, y, x+3, y+2, (100, 100, 100)) # buttons
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
    rect(img, 6, 4, 26, 28, (200, 180, 50)) # outer ring
    rect(img, 8, 6, 24, 26, (255, 255, 255)) # face
    rect(img, 15, 15, 17, 17, (0, 0, 0)) # center
    rect(img, 15, 8, 17, 15, (0, 0, 0)) # hour hand
    rect(img, 15, 15, 22, 17, (0, 0, 0)) # min hand
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
    rect(img, 4, 6, 28, 22, (200, 200, 255)) # bubble
    rect(img, 8, 22, 12, 26, (200, 200, 255)) # tail
    rect(img, 10, 14, 12, 14, (0, 0, 0)) # dots
    rect(img, 16, 14, 18, 14, (0, 0, 0))
    rect(img, 22, 14, 24, 14, (0, 0, 0))
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
    for y in range(16, 24, 2):
        rect(img, 6, y, 10, y+1, (0, 255, 0))
        rect(img, 12, y-4, 16, y-3, (0, 255, 0))
        rect(img, 18, y-8, 22, y-7, (0, 255, 0))
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

