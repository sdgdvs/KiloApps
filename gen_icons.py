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

