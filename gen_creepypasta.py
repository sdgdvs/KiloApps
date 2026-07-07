import os

def generate_creepypasta():
    base_text = """Day 1: I found this old operating system. It's called KiloOS. It looks normal, but there's something off about the way the icons draw themselves. 
Day 3: The clock app... it doesn't match my system time anymore. It's counting down.
Day 4: I tried to delete this file but it keeps coming back. The File Browser is showing directories that don't exist on my hard drive.
Day 7: The minesweeper game... the mines aren't random. They form a pattern. A face. It's looking at me.
Day 12: I can hear the hard drive spinning even when the computer is unplugged. 
Day 15: Don't look behind you. It's in the room.
"""
    
    repeated_phrase = "It sees you. "
    glitch_text = "T H E Y A R E I N T H E W A L L S . "
    
    content = base_text
    
    # build up to 50k characters
    while len(content) < 10000:
        content += repeated_phrase * 10 + "\n"
        
    while len(content) < 30000:
        content += "I CAN'T WAKE UP " * 5 + "\n"
        
    while len(content) < 55000:
        content += glitch_text
        
    return content

vfs_content = generate_creepypasta()

# Escape for JS string
js_content = vfs_content.replace("\\", "\\\\").replace("'", "\\'").replace("\n", "\\n")

with open(os.path.join('KiloOS', 'src', 'defaultVfs.js'), 'w') as f:
    f.write(f"""export const DEFAULT_VFS = {{
  'readme.txt': '{js_content}'
}};
""")

print("Generated defaultVfs.js with", len(vfs_content), "characters.")
