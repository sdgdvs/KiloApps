#!/usr/bin/env python3
r"""
KiloApps Tiered XML & Markdown Context Packer for Google Drive (E:\My Drive\Gemini-Code-Context\)
Generates:
  Tier 1: KiloApps-architecture-index (.txt, .xml, .md) (~250 KB)
          Core shell, directory structure, Cartridge Contract, and metadata catalog for all games.
  Tier 2: kiloapps-games/<game-slug>-context (.txt, .xml, .md) (100-999 KB each)
          Isolated per-game context bundles formatted with 4-tilde Markdown fences for NotebookLM.
"""
import os
import re
import sys
import glob
import xml.sax.saxutils as saxutils

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')

BUDGET_CEILING = 999000

def escape_xml(s):
    return saxutils.escape(str(s))

def detect_file_lang(rel_path: str) -> str:
    ext = os.path.splitext(rel_path)[1].lower()
    mapping = {
        ".html": "html",
        ".htm": "html",
        ".js": "javascript",
        ".jsx": "jsx",
        ".json": "json",
        ".c": "c",
        ".h": "c",
        ".md": "markdown",
        ".py": "python",
        ".css": "css",
        ".txt": "text",
        ".xml": "xml"
    }
    return mapping.get(ext, "text")

def format_file_for_notebooklm(rel_path: str, raw_content: str, lang: str = "text") -> str:
    """
    Formats a source file so NotebookLM's Markdown + HTML sanitizer
    never strips <script>, <style>, <![CDATA[, #include <...>, or JS backticks.
    """
    # Normalize CRLF -> LF so byte counts match across index and context files
    content = raw_content.replace("\r\n", "\n")

    # Prevent NotebookLM's HTML sanitizer from deleting <script>...</script>
    # and <style>...</style> blocks or treating <!DOCTYPE / <![CDATA[ as raw HTML blocks
    content = re.sub(r"(?i)<(/?)(script|style|!doctype|!\[cdata\[)", r"< \1\2", content)

    # Use 4 tildes (~~~~) so triple-backticks (```) and JS template literals (`)
    # inside the source file never close or corrupt the code block
    return (
        f"\n## FILE: {rel_path} ({len(content.encode('utf-8'))} bytes)\n\n"
        f"~~~~{lang}\n"
        f"{content}\n"
        f"~~~~\n"
    )

def get_gdrive_dir():
    candidates = [
        "E:/My Drive",
        "/e/My Drive",
        "G:/My Drive",
        "/g/My Drive",
        os.path.expanduser("~/Google Drive"),
    ]
    for c in candidates:
        if os.path.isdir(c):
            target = os.path.join(c, "Gemini-Code-Context")
            os.makedirs(target, exist_ok=True)
            return target
    # Fallback to local
    fallback = os.path.join(os.path.expanduser("~"), "Gemini-Code-Context")
    os.makedirs(fallback, exist_ok=True)
    return fallback

def read_file_safe(path, max_bytes=BUDGET_CEILING):
    try:
        if not os.path.exists(path):
            return None
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return f.read(max_bytes)
    except Exception as e:
        return None

def extract_apps_metadata(repo_root):
    app_jsx = os.path.join(repo_root, "KiloOS", "src", "App.jsx")
    apps = []
    if os.path.exists(app_jsx):
        content = read_file_safe(app_jsx)
        # Parse APPS array entries
        pattern = re.compile(r"\{\s*id:\s*'([^']+)',\s*title:\s*'([^']+)',\s*url:\s*'([^']+)',\s*exeUrl:\s*([^,]+),\s*icon:\s*'([^']*)',\s*w:\s*(\d+),\s*h:\s*(\d+),\s*folder:\s*'([^']+)'\s*\}")
        for match in pattern.finditer(content):
            app_id, title, url, exe_url, icon, w, h, folder = match.groups()
            clean_exe = exe_url.strip("'\" ") if exe_url != 'null' else None
            apps.append({
                "id": app_id,
                "title": title,
                "url": url,
                "exeUrl": clean_exe,
                "icon": icon,
                "w": int(w),
                "h": int(h),
                "folder": folder
            })
    return apps

def build_architecture_index(repo_root, apps, target_dir):
    out_txt_path = os.path.join(target_dir, "KiloApps-architecture-index.txt")
    out_xml_path = os.path.join(target_dir, "KiloApps-architecture-index.xml")
    out_md_path = os.path.join(target_dir, "KiloApps-architecture-index.md")
    apps_dir = os.path.join(repo_root, "KiloOS", "public", "apps")
    
    xml_parts = []
    xml_parts.append('<?xml version="1.0" encoding="UTF-8"?>')
    xml_parts.append('<project_context name="KiloApps" tier="Architecture-Index" budget="sub-999kb">')
    xml_parts.append('  <overview>')
    xml_parts.append('    <description>KiloApps is an open-source sub-999KB retro minigame arcade and web desktop platform.</description>')
    xml_parts.append('    <cartridge_contract>Every game is an isolated cartridge (<999KB) bootable standalone at /play/:id or inside the sandboxed KiloOS desktop.</cartridge_contract>')
    xml_parts.append('    <sdk>@kiloapps/sdk provides zero-byte ZzFX audio synthesis, touch/gamepad input, and VFS/localStorage persistence.</sdk>')
    xml_parts.append('  </overview>\n')
    
    # Core Architecture Files
    core_files = [
        "architecture.md",
        "docs/CARTRIDGE_CONTRACT.md",
        "packages/sdk/cartridge.schema.json",
        "packages/sdk/kilo-sdk.js",
        "KiloOS/package.json",
        "KiloOS/src/App.jsx",
        "KiloOS/public/apps/catalog.json",
        "scripts/byte_budget_gate.py"
    ]
    
    xml_parts.append('  <core_architecture_files>')
    for rel_path in core_files:
        full_path = os.path.join(repo_root, rel_path)
        content = read_file_safe(full_path, max_bytes=BUDGET_CEILING)
        if content:
            xml_parts.append(format_file_for_notebooklm(rel_path, content, detect_file_lang(rel_path)))
    xml_parts.append('  </core_architecture_files>\n')
    
    # 40+ Cartridge Catalog (10-line metadata header each)
    xml_parts.append('  <cartridges_catalog total="' + str(len(apps)) + '">')
    for app in apps:
        slug = app["id"]
        title = app["title"]
        folder = app["folder"]
        
        # Check web html size
        if app["url"] == "#":
            html_rel = f"apps/{slug}.html"
        else:
            html_rel = app["url"].lstrip("/")
        html_path = os.path.join(repo_root, "KiloOS", "public", html_rel)
        web_bytes = os.path.getsize(html_path) if os.path.exists(html_path) else 0
        pct = round((web_bytes / BUDGET_CEILING) * 100, 1)
        
        # Check native c source
        native_folder_candidates = [slug.capitalize(), "K" + slug[1:].capitalize() if slug.startswith("k") else slug]
        c_source = None
        c_bytes = 0
        for cand in native_folder_candidates:
            c_path = os.path.join(repo_root, cand, "main.c")
            if os.path.exists(c_path):
                c_source = f"{cand}/main.c"
                c_bytes = os.path.getsize(c_path)
                break
                
        status = "COMPLIANT" if web_bytes <= BUDGET_CEILING else "OVER_BUDGET"
        
        xml_parts.append(f'    <cartridge id="{escape_xml(slug)}">')
        xml_parts.append(f'      <title>{escape_xml(title)}</title>')
        xml_parts.append(f'      <category>{escape_xml(folder)}</category>')
        xml_parts.append(f'      <web_entrypoint>{escape_xml(app["url"])}</web_entrypoint>')
        xml_parts.append(f'      <web_size_bytes>{web_bytes}</web_size_bytes>')
        xml_parts.append(f'      <budget_percent>{pct}%</budget_percent>')
        xml_parts.append(f'      <native_c_source>{escape_xml(c_source or "web-native")}</native_c_source>')
        xml_parts.append(f'      <native_c_bytes>{c_bytes}</native_c_bytes>')
        xml_parts.append(f'      <native_exe>{escape_xml(app.get("exeUrl") or "none")}</native_exe>')
        xml_parts.append(f'      <status>{status}</status>')
        xml_parts.append(f'    </cartridge>')
    xml_parts.append('  </cartridges_catalog>')
    xml_parts.append('</project_context>')
    
    body = "\n".join(xml_parts)
    with open(out_txt_path, "w", encoding="utf-8") as f:
        f.write(body)
    with open(out_xml_path, "w", encoding="utf-8") as f:
        f.write(body)
    with open(out_md_path, "w", encoding="utf-8") as f:
        f.write(body)
    return out_txt_path

def build_per_game_bundles(repo_root, apps, target_dir):
    games_out_dir = os.path.join(target_dir, "kiloapps-games")
    os.makedirs(games_out_dir, exist_ok=True)
    generated = []
    
    for app in apps:
        slug = app["id"]
        out_txt_path = os.path.join(games_out_dir, f"{slug}-context.txt")
        out_xml_path = os.path.join(games_out_dir, f"{slug}-context.xml")
        out_md_path = os.path.join(games_out_dir, f"{slug}-context.md")
        
        # Web app html
        if app["url"] == "#":
            html_rel = f"apps/{slug}.html"
        else:
            html_rel = app["url"].lstrip("/")
        html_path = os.path.join(repo_root, "KiloOS", "public", html_rel)
        html_content = read_file_safe(html_path, max_bytes=BUDGET_CEILING)
        
        # Native C source
        native_folder_candidates = [slug.capitalize(), "K" + slug[1:].capitalize() if slug.startswith("k") else slug]
        c_source_name = None
        c_content = None
        for cand in native_folder_candidates:
            c_path = os.path.join(repo_root, cand, "main.c")
            if os.path.exists(c_path):
                c_source_name = f"{cand}/main.c"
                c_content = read_file_safe(c_path, max_bytes=BUDGET_CEILING)
                break
                
        xml = []
        xml.append('<?xml version="1.0" encoding="UTF-8"?>')
        xml.append(f'<cartridge_context id="{escape_xml(slug)}" title="{escape_xml(app["title"])}" category="{escape_xml(app["folder"])}">')
        xml.append('  <metadata>')
        xml.append(f'    <slug>{escape_xml(slug)}</slug>')
        xml.append(f'    <title>{escape_xml(app["title"])}</title>')
        xml.append(f'    <category>{escape_xml(app["folder"])}</category>')
        xml.append(f'    <window_dimensions width="{app["w"]}" height="{app["h"]}" />')
        xml.append(f'    <budget_ceiling>999000 bytes</budget_ceiling>')
        xml.append(f'    <web_size_bytes>{len(html_content.encode("utf-8")) if html_content else 0}</web_size_bytes>')
        xml.append('  </metadata>\n')
        
        xml.append('  <files>')
        if html_content:
            xml.append(format_file_for_notebooklm(html_rel, html_content, detect_file_lang(html_rel)))
        if c_content:
            xml.append(format_file_for_notebooklm(c_source_name, c_content, detect_file_lang(c_source_name)))
        xml.append('  </files>')
        xml.append('</cartridge_context>')
        
        body = "\n".join(xml)
        with open(out_txt_path, "w", encoding="utf-8") as f:
            f.write(body)
        with open(out_xml_path, "w", encoding="utf-8") as f:
            f.write(body)
        with open(out_md_path, "w", encoding="utf-8") as f:
            f.write(body)
        generated.append(out_txt_path)
        
    return generated

def main():
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    target_dir = get_gdrive_dir()
    
    print(f"📦 Packing Tiered Context for KiloApps...")
    print(f"Target Directory: {target_dir}")
    
    apps = extract_apps_metadata(repo_root)
    print(f"Extracted metadata for {len(apps)} registered cartridges.")
    
    # 1. Tier 1 Architecture Index
    index_path = build_architecture_index(repo_root, apps, target_dir)
    index_size = os.path.getsize(index_path)
    print(f"✅ Tier 1 Generated: {os.path.basename(index_path)} ({index_size:,} bytes / {round(index_size/1024, 1)} KB)")
    
    # 2. Tier 2 Per-Game Bundles
    game_bundles = build_per_game_bundles(repo_root, apps, target_dir)
    print(f"✅ Tier 2 Generated: {len(game_bundles)} per-game bundles in kiloapps-games/")
    
    # Clean up old monolithic files if present
    for old_file in ["KiloApps-main-context.xml", "peaceful-carson-main-context.xml"]:
        old_path = os.path.join(target_dir, old_file)
        if os.path.exists(old_path):
            try:
                os.remove(old_path)
                print(f"🗑️ Removed obsolete monolithic context: {old_file}")
            except Exception:
                pass
                
    print("✨ Tiered packing complete! Google Drive is organized and lightweight.")

if __name__ == "__main__":
    main()
