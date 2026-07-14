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
