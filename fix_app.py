import re
content = open('KiloOS/src/App.jsx').read()
content = re.sub(r"exeUrl:\s*'[^']+'", "exeUrl: '/exe/KApps.zip'", content)
open('KiloOS/src/App.jsx', 'w').write(content)
