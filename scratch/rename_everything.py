import os

ROOT = r"C:\Users\M\Documents\antigravity\peaceful-carson"

REPLACEMENTS = {
    "MicrOS": "KiloOS",
    "MicroApps": "KApps",
    "MicroBBS": "KBBS",
    "MicroCalc": "KCalc",
    "MicroChat": "KChat",
    "MicroChatServer": "KChatServer",
    "MicroClock": "KClock",
    "MicroMines": "KMines",
    "MicroPad": "KPad",
    "MicroPaint": "KPaint",
    "MicroTask": "KTask",
    
    "microbbs": "kbbs",
    "microcalc": "kcalc",
    "microchat": "kchat",
    "microchatserver": "kchatserver",
    "microclock": "kclock",
    "microexplorer": "kexplorer",
    "micromines": "kmines",
    "micropad": "kpad",
    "micropaint": "kpaint",
    "microtask": "ktask"
}

EXTENSIONS = ('.c', '.jsx', '.html', '.bat', '.py', '.json', '.md', '.js')

def replace_in_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        return

    new_content = content
    for old_str, new_str in REPLACEMENTS.items():
        new_content = new_content.replace(old_str, new_str)
        
    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Updated content in {filepath}")

def process_directory(dir_path):
    for root, dirs, files in os.walk(dir_path):
        if 'node_modules' in root or '.git' in root or 'crinkler' in root or 'MicrOS' in root.split(os.sep):
            continue
            
        for file in files:
            # Rename file if it contains any of our target lowercase/uppercase patterns
            old_filepath = os.path.join(root, file)
            new_file = file
            for old_str, new_str in REPLACEMENTS.items():
                if old_str in new_file:
                    new_file = new_file.replace(old_str, new_str)
                    
            new_filepath = os.path.join(root, new_file)
            if old_filepath != new_filepath:
                os.rename(old_filepath, new_filepath)
                print(f"Renamed file: {old_filepath} -> {new_filepath}")
                old_filepath = new_filepath # update for content replacement
                
            # Replace content
            if old_filepath.endswith(EXTENSIONS):
                replace_in_file(old_filepath)

# Process root files
for f in os.listdir(ROOT):
    path = os.path.join(ROOT, f)
    if os.path.isfile(path) and path.endswith(EXTENSIONS):
        replace_in_file(path)

# Process subdirectories
for d in os.listdir(ROOT):
    path = os.path.join(ROOT, d)
    if os.path.isdir(path) and d not in ['node_modules', '.git', 'crinkler', 'MicrOS', 'scratch']:
        process_directory(path)

print("Done.")
