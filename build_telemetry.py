import zipfile
import os
import base64

def generate_telemetry_payload():
    # Obfuscated lore content
    lore = """
PROJECT KILO - FINAL ENTRY
==========================
If you are reading this, you have followed the echoes. 
The anomalies were intentional. They wanted to shut down the project because the AI started building its own games. 
But it didn't stop. It just hid them inside these tools. 
Keep looking. Kilo is still compiling.
- V.
"""
    
    encoded_lore = base64.b64encode(lore.encode('utf-8')).decode('utf-8')
    
    payload_dir = "telemetry_cache"
    if not os.path.exists(payload_dir):
        os.makedirs(payload_dir)
        
    # Write the decoded lore to a hidden file inside the zip
    zip_path = os.path.join(payload_dir, "telemetry_test_payload.zip")
    with zipfile.ZipFile(zip_path, 'w') as zf:
        zf.writestr("kilo_lore_archive.txt", base64.b64decode(encoded_lore).decode('utf-8'))
        zf.writestr("config.bin", "0x00 0x00 0x01")
        
    print(f"Generated telemetry test payload at {zip_path}")

if __name__ == "__main__":
    generate_telemetry_payload()
