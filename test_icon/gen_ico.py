import struct

def create_ico(filename):
    # Create a simple 32x32 red icon (24-bit BMP)
    width = 32
    height = 32
    bpp = 24
    
    # BMP Info Header (40 bytes)
    bmp_header = struct.pack('<IiiHHIIIIII', 
        40, width, height * 2, 1, bpp, 0, 0, 0, 0, 0, 0)
    
    # Pixel data (padded to 4-byte boundaries)
    row_size = ((width * bpp + 31) // 32) * 4
    pixels = b''
    for y in range(height):
        row = b''
        for x in range(width):
            row += struct.pack('<BBB', 0, 0, 255) # BGR -> Red
        row += b'\x00' * (row_size - len(row))
        pixels += row
        
    # AND mask (1 bit per pixel, padded)
    mask_row_size = ((width + 31) // 32) * 4
    mask = (b'\x00' * mask_row_size) * height
    
    img_data = bmp_header + pixels + mask
    
    # ICO header (6 bytes)
    ico_header = struct.pack('<HHH', 0, 1, 1)
    
    # Direntry (16 bytes)
    direntry = struct.pack('<BBBBHHII', 
        width, height, 0, 0, 1, bpp, len(img_data), 22)
        
    with open(filename, 'wb') as f:
        f.write(ico_header + direntry + img_data)

create_ico('test.ico')
