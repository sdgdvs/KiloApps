with open('KiloOS/public/web/webring.html', 'r', encoding='utf-8') as f:
    text = f.read()

target = '<a href="javascript:openNodeModal(\'${node.id}\')" style="color:var(--cyan); text-decoration:underline;">Inspect Node Modal [Enter]</a>'
replacement = '<button class="btn-action" onclick="openNodeModal(\'${node.id}\')" style="padding:1px 6px; font-size:10px;">Inspect Node Modal [Enter]</button>'

if target in text:
    text = text.replace(target, replacement)
    with open('KiloOS/public/web/webring.html', 'w', encoding='utf-8') as f:
        f.write(text)
    print("Success: replaced target in webring.html")
else:
    print("Target not found directly, trying re.sub...")
    import re
    text, n = re.subn(r'href=["\']javascript:openNodeModal\(\\\'(\$\{node\.id\}|\w+)\\\'\)["\']', r'href="#" onclick="openNodeModal(\'\1\'); return false;"', text)
    with open('KiloOS/public/web/webring.html', 'w', encoding='utf-8') as f:
        f.write(text)
    print(f"Substituted {n} occurrences")
