import struct
import os

def save_ico(filename, pixels):
    width = 32
    height = 32
    bpp = 24
    
    bmp_header = struct.pack('<IiiHHIIIIII', 40, width, height * 2, 1, bpp, 0, 0, 0, 0, 0, 0)
    
    row_size = ((width * bpp + 31) // 32) * 4
    img_data = b''
    for y in range(height - 1, -1, -1):
        row = b''
        for x in range(width):
            val = pixels[y][x]
            if len(val) == 4:
                r, g, b, a = val
            else:
                r, g, b = val
            row += struct.pack('<BBB', b, g, r)
        row += b'\x00' * (row_size - len(row))
        img_data += row
        
    mask_row_size = ((width + 31) // 32) * 4
    mask_data = b''
    for y in range(height - 1, -1, -1):
        row_bits = 0
        for x in range(width):
            if pixels[y][x] == (0, 0, 0, 0):
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
    return [[(0,0,0,0) for _ in range(32)] for _ in range(32)]

def rect(img, x1, y1, x2, y2, color):
    for y in range(y1, y2+1):
        for x in range(x1, x2+1):
            if 0 <= y < 32 and 0 <= x < 32:
                img[y][x] = color

def generate_rogue():
    img = new_img()
    # background
    rect(img, 2, 2, 29, 29, (40, 40, 40))
    # "@" symbol in green
    # draw roughly a circle
    rect(img, 8, 10, 24, 12, (0, 255, 0))
    rect(img, 8, 22, 24, 24, (0, 255, 0))
    rect(img, 6, 12, 10, 22, (0, 255, 0))
    # a in middle
    rect(img, 14, 14, 20, 16, (0, 255, 0))
    rect(img, 14, 16, 16, 20, (0, 255, 0))
    rect(img, 14, 18, 20, 20, (0, 255, 0))
    rect(img, 20, 14, 22, 26, (0, 255, 0)) # right tail of @
    rect(img, 10, 26, 22, 28, (0, 255, 0)) # bottom curve
    return img

save_ico('icon.ico', generate_rogue())
