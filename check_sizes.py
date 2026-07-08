import os
import sys
import glob

# 999 KB in bytes
MAX_SIZE = 999 * 1024

def check_sizes():
    print("Checking KiloApps executable sizes...")
    failed = False
    
    # Check all executables in KiloOS/public/exe
    # (except .zip or bundled files if they existed, though only KApps.zip is exempted which isn't .exe)
    search_pattern = "KiloOS/public/exe/*.exe"
    
    for filepath in glob.glob(search_pattern):
        size = os.path.getsize(filepath)
        size_kb = size / 1024.0
        if size > MAX_SIZE:
            print(f"❌ FAIL: {filepath} is {size_kb:.2f} KB (Exceeds {MAX_SIZE/1024} KB limit!)")
            failed = True
        else:
            print(f"✅ PASS: {filepath} is {size_kb:.2f} KB")

    if failed:
        print("\nBuild failed due to size limit violations.")
        sys.exit(1)
    else:
        print("\nAll native KiloApps pass the 999KB size limit.")
        sys.exit(0)

if __name__ == "__main__":
    check_sizes()
