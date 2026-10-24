import os
import re

html_path = r"KiloOS\public\apps\kbase.html"
with open(html_path, "r", encoding="utf-8") as f:
    html = f.read()

# 1. Add Endianness Bar in Tab 1
old_tab1_top = """      <div class="card-title">
        <span>Simultaneous Multi-Base Live Converter</span>
        <button class="btn btn-sm" id="btn-clear-bases">Clear All</button>
      </div>

      <div class="preset-bar">"""

new_tab1_top = """      <div class="card-title">
        <span>Simultaneous Multi-Base Live Converter</span>
        <button class="btn btn-sm" id="btn-clear-bases">Clear All</button>
      </div>

      <!-- Endianness Mode & Byte Sequence Bar -->
      <div style="display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:8px; margin-bottom:12px; padding:10px 14px; background:rgba(0,240,255,0.05); border:1px solid rgba(0,240,255,0.2); border-radius:8px;">
        <div style="display:flex; align-items:center; gap:8px; flex-wrap:wrap;">
          <span style="font-size:0.85rem; font-weight:700; color:var(--accent-cyan);">Byte Order:</span>
          <span class="badge" id="badge-endian-mode" style="background:rgba(0,240,255,0.15); border:1px solid var(--accent-cyan);">Big-Endian (Network)</span>
          <button class="btn btn-xs btn-accent" id="btn-endian-toggle" title="Toggle Big-Endian / Little-Endian">🔄 Switch to Little-Endian (x86)</button>
          <button class="btn btn-xs" id="btn-endian-swap16" title="Byte Swap 16-bit word">Swap 16</button>
          <button class="btn btn-xs" id="btn-endian-swap32" title="Byte Swap 32-bit dword">Swap 32</button>
          <button class="btn btn-xs" id="btn-endian-swap64" title="Byte Swap 64-bit qword">Swap 64</button>
        </div>
        <div style="font-size:0.8rem; font-family:var(--font-mono); color:var(--text-muted);" id="endian-byte-seq">
          Raw Bytes: 00 00 00 00 00 00 00 2A
        </div>
      </div>

      <div class="preset-bar">"""

if old_tab1_top in html:
    html = html.replace(old_tab1_top, new_tab1_top)
    print("Added Endianness Bar in Tab 1")
else:
    print("Warning: old_tab1_top not found")

# 2. Add Base64, Base32, and Base58 Radix Inputs in Tab 1
old_tab1_grid_end = """        <div class="input-group">
          <div class="input-header">
            <label>Reflected Gray Code</label>
            <div style="display:flex; align-items:center; gap:6px;">
              <span class="badge">GRAY</span>
              <button class="btn btn-xs btn-copy-field" data-target="base-gray" data-label="Gray Code" title="Copy Gray">📋 Copy</button>
              <button class="btn btn-xs btn-clear-field" data-target="base-gray" title="Clear Gray">✕</button>
            </div>
          </div>
          <input type="text" id="base-gray" placeholder="Gray code result..." autocomplete="off">
        </div>
      </div>
    </div>"""

new_tab1_grid_end = """        <div class="input-group">
          <div class="input-header">
            <label>Reflected Gray Code</label>
            <div style="display:flex; align-items:center; gap:6px;">
              <span class="badge">GRAY</span>
              <button class="btn btn-xs btn-copy-field" data-target="base-gray" data-label="Gray Code" title="Copy Gray">📋 Copy</button>
              <button class="btn btn-xs btn-clear-field" data-target="base-gray" title="Clear Gray">✕</button>
            </div>
          </div>
          <input type="text" id="base-gray" placeholder="Gray code result..." autocomplete="off">
        </div>
      </div>

      <div class="grid-2" style="margin-top: 10px;">
        <div class="input-group">
          <div class="input-header">
            <label>Base64 Radix (64-bit int)</label>
            <div style="display:flex; align-items:center; gap:6px;">
              <span class="badge">B64</span>
              <button class="btn btn-xs btn-copy-field" data-target="base-b64" data-label="Base64 Radix" title="Copy Base64">📋 Copy</button>
              <button class="btn btn-xs btn-clear-field" data-target="base-b64" title="Clear Base64">✕</button>
            </div>
          </div>
          <input type="text" id="base-b64" placeholder="Base64 integer representation..." autocomplete="off">
        </div>

        <div class="input-group">
          <div class="input-header">
            <label>Base32 (RFC 4648) / Base58 (Bitcoin)</label>
            <div style="display:flex; align-items:center; gap:6px;">
              <span class="badge">B32 / B58</span>
              <button class="btn btn-xs btn-copy-field" data-target="base-b32" data-label="Base32" title="Copy Base32">📋 B32</button>
              <button class="btn btn-xs btn-copy-field" data-target="base-b58" data-label="Base58" title="Copy Base58">📋 B58</button>
              <button class="btn btn-xs btn-clear-field" data-target="base-b32" title="Clear Base32/58">✕</button>
            </div>
          </div>
          <div style="display:flex; gap:6px;">
            <input type="text" id="base-b32" placeholder="Base32..." style="flex:1;" autocomplete="off">
            <input type="text" id="base-b58" placeholder="Base58..." style="flex:1;" autocomplete="off">
          </div>
        </div>
      </div>
    </div>"""

if old_tab1_grid_end in html:
    html = html.replace(old_tab1_grid_end, new_tab1_grid_end)
    print("Added Base64/Base32/Base58 Radix Inputs in Tab 1")
else:
    print("Warning: old_tab1_grid_end not found")

# 3. Add String Suite Base32 and Base58 buttons
old_str_btns = """        <button class="btn btn-accent" id="btn-hex-enc">Hex Enc</button>
        <button class="btn" id="btn-hex-dec">Hex Dec</button>
        <button class="btn btn-primary" id="btn-sha256">SHA-256 Hash</button>"""

new_str_btns = """        <button class="btn btn-accent" id="btn-hex-enc">Hex Enc</button>
        <button class="btn" id="btn-hex-dec">Hex Dec</button>
        <button class="btn btn-accent" id="btn-b32-enc">Base32 Enc</button>
        <button class="btn" id="btn-b32-dec">Base32 Dec</button>
        <button class="btn btn-accent" id="btn-b58-enc">Base58 Enc</button>
        <button class="btn" id="btn-b58-dec">Base58 Dec</button>
        <button class="btn btn-primary" id="btn-sha256">SHA-256 Hash</button>"""

if old_str_btns in html:
    html = html.replace(old_str_btns, new_str_btns)
    print("Added Base32/Base58 string buttons")
else:
    print("Warning: old_str_btns not found")

# 4. Add Word Swap and Dword Swap to Bitboard buttons
old_bb_btns = """        <button class="btn" id="bb-byteswap">Byte Swap (Endianness)</button>
        <button class="btn btn-accent" id="bb-swap-nybbles">Swap Nybbles</button>"""

new_bb_btns = """        <button class="btn" id="bb-byteswap">Byte Swap (64-bit)</button>
        <button class="btn btn-accent" id="bb-swap-word">Word Swap (16-bit)</button>
        <button class="btn btn-accent" id="bb-swap-dword">Dword Swap (32-bit)</button>
        <button class="btn btn-accent" id="bb-swap-nybbles">Swap Nybbles</button>"""

if old_bb_btns in html:
    html = html.replace(old_bb_btns, new_bb_btns)
    print("Added Word/Dword swap buttons")
else:
    print("Warning: old_bb_btns not found")

# 5. Add Granular Bitwise Mask & Subnet / CIDR Suite Card in Tab 2
old_bb_matrix_end = """      <table class="matrix-table">
        <thead>
          <tr>
            <th style="width: 100px;">Target</th>
            <th>Hexadecimal / Decimal Value</th>
            <th>64-Bit Binary Stream</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td><strong>Operand A</strong></td>
            <td id="mat-a-val" style="color:var(--accent-green);">0x0F0F0F0F0F0F0F0F</td>
            <td id="mat-a-bin" class="bit-stream"></td>
          </tr>
          <tr>
            <td><strong>Operand B</strong></td>
            <td id="mat-b-val" style="color:var(--accent-amber);">0x00FF00FF00FF00FF</td>
            <td id="mat-b-bin" class="bit-stream"></td>
          </tr>
          <tr style="background: rgba(0, 240, 255, 0.05);">
            <td><strong id="mat-op-label" style="color:var(--accent-cyan);">Result (AND)</strong></td>
            <td style="color:var(--accent-cyan); font-weight:bold;">
              <span id="mat-res-val">0x000F00F00F00F00</span>
              <button class="btn btn-xs btn-accent" id="btn-copy-mat-res" style="margin-left:8px;" title="Copy Bitwise Result">📋 Copy</button>
            </td>
            <td id="mat-res-bin" class="bit-stream"></td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>"""

new_bb_matrix_end = """      <table class="matrix-table">
        <thead>
          <tr>
            <th style="width: 100px;">Target</th>
            <th>Hexadecimal / Decimal Value</th>
            <th>64-Bit Binary Stream</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td><strong>Operand A</strong></td>
            <td id="mat-a-val" style="color:var(--accent-green);">0x0F0F0F0F0F0F0F0F</td>
            <td id="mat-a-bin" class="bit-stream"></td>
          </tr>
          <tr>
            <td><strong>Operand B</strong></td>
            <td id="mat-b-val" style="color:var(--accent-amber);">0x00FF00FF00FF00FF</td>
            <td id="mat-b-bin" class="bit-stream"></td>
          </tr>
          <tr style="background: rgba(0, 240, 255, 0.05);">
            <td><strong id="mat-op-label" style="color:var(--accent-cyan);">Result (AND)</strong></td>
            <td style="color:var(--accent-cyan); font-weight:bold;">
              <span id="mat-res-val">0x000F00F00F00F00</span>
              <button class="btn btn-xs btn-accent" id="btn-copy-mat-res" style="margin-left:8px;" title="Copy Bitwise Result">📋 Copy</button>
            </td>
            <td id="mat-res-bin" class="bit-stream"></td>
          </tr>
        </tbody>
      </table>
    </div>

    <!-- Granular Bitwise Mask & Subnet / CIDR Suite -->
    <div class="card">
      <div class="card-title">
        <span>Granular Bitwise Mask &amp; Subnet / CIDR Suite</span>
        <div style="display:flex; align-items:center; gap:8px;">
          <span class="badge" id="mask-alignment-badge">MSB-Aligned (Subnet)</span>
          <button class="btn btn-xs btn-accent" id="btn-copy-mask" title="Copy Bitmask Hex">📋 Copy Mask</button>
        </div>
      </div>

      <div class="grid-2">
        <div class="input-group">
          <div class="input-header">
            <label>CIDR Prefix / Mask Width: <strong id="mask-cidr-label" style="color:var(--accent-cyan);">/32</strong></label>
            <input type="range" id="mask-cidr-range" min="0" max="64" value="32" style="width:160px;">
          </div>
        </div>
        <div class="input-group">
          <div class="input-header">
            <label>Mask Alignment &amp; Polarity</label>
            <div class="btn-group">
              <button class="btn btn-xs btn-accent" id="btn-mask-align">MSB-Aligned (Subnet)</button>
              <button class="btn btn-xs" id="btn-mask-invert">Invert Mask (~)</button>
              <button class="btn btn-xs btn-primary" id="btn-mask-load-opb">Load Mask to Op B</button>
            </div>
          </div>
        </div>
      </div>

      <div class="preset-bar" style="margin-top:6px;">
        <span style="font-size:0.75rem; font-weight:bold; color:var(--text-muted); margin-right:4px;">🎭 Mask Presets:</span>
        <button class="preset-chip mask-preset" data-bits="8" data-msb="1">/8 (Class A)</button>
        <button class="preset-chip mask-preset" data-bits="16" data-msb="1">/16 (Class B)</button>
        <button class="preset-chip mask-preset" data-bits="24" data-msb="1">/24 (Class C)</button>
        <button class="preset-chip mask-preset" data-bits="32" data-msb="1">/32 (Host)</button>
        <button class="preset-chip mask-preset" data-bits="48" data-msb="1">/48 (IPv6)</button>
        <button class="preset-chip mask-preset" data-custom="0x0F0F0F0F0F0F0F0F">Nybble Alt</button>
        <button class="preset-chip mask-preset" data-custom="0x00FF00FF00FF00FF">Byte Alt</button>
        <button class="preset-chip mask-preset" data-custom="0x0000FFFF0000FFFF">Word Alt</button>
        <button class="preset-chip mask-preset" data-custom="0xAAAAAAAAAAAAAAAA">Odd Bits</button>
        <button class="preset-chip mask-preset" data-custom="0x5555555555555555">Even Bits</button>
      </div>

      <div class="grid-2" style="font-family: var(--font-mono); font-size: 0.9rem; margin-top: 10px;">
        <div style="background:#090c13; padding:12px; border-radius:6px; border:1px solid var(--card-border);">
          <p style="margin-bottom:6px;"><strong>Active Mask (Hex):</strong> <span id="mask-hex-val" style="color:var(--accent-cyan); font-weight:bold;">0xFFFFFFFF00000000</span></p>
          <p style="margin-bottom:6px;"><strong>Wildcard / Inverse:</strong> <span id="mask-wildcard-val" style="color:var(--accent-pink);">0x00000000FFFFFFFF</span></p>
          <p style="margin-bottom:6px;"><strong>Hamming Weight:</strong> <span id="mask-popcount-val" style="color:var(--accent-green);">32 / 64</span></p>
          <p><strong>Contiguity Check:</strong> <span id="mask-contiguity-val" style="color:var(--accent-amber);">Contiguous CIDR Subnet</span></p>
        </div>
        <div style="background:#090c13; padding:12px; border-radius:6px; border:1px solid var(--card-border);">
          <p style="margin-bottom:6px;"><strong>Apply Mask to Operand A:</strong></p>
          <div class="btn-group" style="margin-bottom:8px;">
            <button class="btn btn-xs btn-accent" id="btn-mask-apply-and">A &amp; MASK</button>
            <button class="btn btn-xs btn-accent" id="btn-mask-apply-or">A | MASK</button>
            <button class="btn btn-xs btn-accent" id="btn-mask-apply-xor">A ^ MASK</button>
            <button class="btn btn-xs btn-accent" id="btn-mask-apply-clear">A &amp; ~MASK</button>
          </div>
          <p style="font-size:0.85rem;"><strong>Result:</strong> <span id="mask-apply-result" style="color:var(--accent-cyan); font-weight:bold;">0x0000000000000000</span></p>
        </div>
      </div>
    </div>
  </div>"""

if old_bb_matrix_end in html:
    html = html.replace(old_bb_matrix_end, new_bb_matrix_end)
    print("Added Granular Bitwise Mask Card in Tab 2")
else:
    print("Warning: old_bb_matrix_end not found")

# 6. Add JS Logic for Base64, Base32, Base58, Endianness, Masks, and string functions
# Find position to insert helpers
insert_pos_marker = "  // ==========================================\n  // TAB 1: SIMULTANEOUS MULTI-BASE CONVERTER"
if insert_pos_marker not in html:
    insert_pos_marker = "  // ==========================================\r\n  // TAB 1: SIMULTANEOUS MULTI-BASE CONVERTER"

js_helpers = """  // --- Radix Conversion Helpers (Base64, Base32, Base58, Endianness) ---
  const B64_RADIX_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const B32_RADIX_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  const B58_RADIX_CHARS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

  let isLittleEndianMode = false;

  function uint64ToBase64(u64) {
    if (u64 === 0n) return "A";
    let res = "";
    let v = u64 & 0xFFFFFFFFFFFFFFFFn;
    while (v > 0n) {
      res = B64_RADIX_CHARS[Number(v % 64n)] + res;
      v /= 64n;
    }
    return res;
  }

  function parseBase64Radix(str) {
    str = str.trim();
    if (!str) return null;
    let res = 0n;
    for (let i = 0; i < str.length; i++) {
      const idx = B64_RADIX_CHARS.indexOf(str[i]);
      if (idx < 0) return null;
      res = res * 64n + BigInt(idx);
    }
    return res;
  }

  function uint64ToBase32(u64) {
    if (u64 === 0n) return "A";
    let res = "";
    let v = u64 & 0xFFFFFFFFFFFFFFFFn;
    while (v > 0n) {
      res = B32_RADIX_CHARS[Number(v % 32n)] + res;
      v /= 32n;
    }
    return res;
  }

  function parseBase32(str) {
    str = str.trim().toUpperCase();
    if (!str) return null;
    let res = 0n;
    for (let i = 0; i < str.length; i++) {
      const idx = B32_RADIX_CHARS.indexOf(str[i]);
      if (idx < 0) return null;
      res = res * 32n + BigInt(idx);
    }
    return res;
  }

  function uint64ToBase58(u64) {
    if (u64 === 0n) return "1";
    let res = "";
    let v = u64 & 0xFFFFFFFFFFFFFFFFn;
    while (v > 0n) {
      res = B58_RADIX_CHARS[Number(v % 58n)] + res;
      v /= 58n;
    }
    return res;
  }

  function parseBase58(str) {
    str = str.trim();
    if (!str) return null;
    let res = 0n;
    for (let i = 0; i < str.length; i++) {
      const idx = B58_RADIX_CHARS.indexOf(str[i]);
      if (idx < 0) return null;
      res = res * 58n + BigInt(idx);
    }
    return res;
  }

  function byteSwap64(val) {
    let swapped = 0n;
    for (let i = 0n; i < 8n; i++) {
      const byte = (val >> (i * 8n)) & 0xFFn;
      swapped |= (byte << ((7n - i) * 8n));
    }
    return swapped;
  }

  function wordSwap64(val) {
    let swapped = 0n;
    for (let i = 0n; i < 4n; i++) {
      const word = (val >> (i * 16n)) & 0xFFFFn;
      const sw = ((word & 0xFFn) << 8n) | ((word >> 8n) & 0xFFn);
      swapped |= (sw << (i * 16n));
    }
    return swapped;
  }

  function dwordSwap64(val) {
    const lo = val & 0xFFFFFFFFn;
    const hi = (val >> 32n) & 0xFFFFFFFFn;
    return (lo << 32n) | hi;
  }

  function updateEndianDisplay(val) {
    const elSeq = document.getElementById('endian-byte-seq');
    const badge = document.getElementById('badge-endian-mode');
    const btnToggle = document.getElementById('btn-endian-toggle');
    if (!elSeq || !badge || !btnToggle) return;

    const u64 = val & 0xFFFFFFFFFFFFFFFFn;
    const bytes = [];
    if (isLittleEndianMode) {
      badge.textContent = 'Little-Endian (x86 Host)';
      btnToggle.textContent = '🔄 Switch to Big-Endian (Network)';
      for (let i = 0n; i < 8n; i++) {
        bytes.push(((u64 >> (i * 8n)) & 0xFFn).toString(16).padStart(2, '0').toUpperCase());
      }
    } else {
      badge.textContent = 'Big-Endian (Network)';
      btnToggle.textContent = '🔄 Switch to Little-Endian (x86)';
      for (let i = 7n; i >= 0n; i--) {
        bytes.push(((u64 >> (i * 8n)) & 0xFFn).toString(16).padStart(2, '0').toUpperCase());
      }
    }
    elSeq.textContent = `Raw Bytes: ${bytes.join(' ')}`;
  }

"""

if insert_pos_marker in html:
    html = html.replace(insert_pos_marker, js_helpers + insert_pos_marker)
    print("Added JS Radix & Endian Helpers")
else:
    print("Warning: insert_pos_marker not found")

# 7. Update updateBasesFromValue to handle base-b64, base-b32, base-b58 and endianness
old_update_bases = """    if (elGray && sourceEl !== elGray) {
      const u64 = bigVal & 0xFFFFFFFFFFFFFFFFn;
      const gray = u64 ^ (u64 >> 1n);
      elGray.value = '0x' + gray.toString(16).toUpperCase();
    }"""

new_update_bases = """    if (elGray && sourceEl !== elGray) {
      const u64 = bigVal & 0xFFFFFFFFFFFFFFFFn;
      const gray = u64 ^ (u64 >> 1n);
      elGray.value = '0x' + gray.toString(16).toUpperCase();
    }

    const elB64 = document.getElementById('base-b64');
    if (elB64 && sourceEl !== elB64) {
      elB64.value = uint64ToBase64(bigVal);
    }

    const elB32 = document.getElementById('base-b32');
    if (elB32 && sourceEl !== elB32) {
      elB32.value = uint64ToBase32(bigVal);
    }

    const elB58 = document.getElementById('base-b58');
    if (elB58 && sourceEl !== elB58) {
      elB58.value = uint64ToBase58(bigVal);
    }

    updateEndianDisplay(bigVal);"""

if old_update_bases in html:
    html = html.replace(old_update_bases, new_update_bases)
    print("Updated updateBasesFromValue with b64, b32, b58, endian")
else:
    print("Warning: old_update_bases not found")

# 8. Add event listeners for base-b64, base-b32, base-b58 and endianness buttons
old_endian_listen = """  if (elGray) {
    elGray.addEventListener('input', (e) => {
      let val = e.target.value.trim();
      if (!val) { updateBasesFromValue(null, e.target); return; }
      let grayVal = parseOpVal(val);
      let bin = 0n;
      let temp = grayVal;
      while (temp > 0n) {
        bin ^= temp;
        temp >>= 1n;
      }
      updateBasesFromValue(bin, e.target);
      bitboardVal = bin & 0xFFFFFFFFFFFFFFFFn;
      renderBitboard();
    });
  }"""

new_endian_listen = """  if (elGray) {
    elGray.addEventListener('input', (e) => {
      let val = e.target.value.trim();
      if (!val) { updateBasesFromValue(null, e.target); return; }
      let grayVal = parseOpVal(val);
      let bin = 0n;
      let temp = grayVal;
      while (temp > 0n) {
        bin ^= temp;
        temp >>= 1n;
      }
      updateBasesFromValue(bin, e.target);
      bitboardVal = bin & 0xFFFFFFFFFFFFFFFFn;
      renderBitboard();
    });
  }

  const elB64 = document.getElementById('base-b64');
  if (elB64) {
    elB64.addEventListener('input', (e) => {
      const v = parseBase64Radix(e.target.value);
      updateBasesFromValue(v, e.target);
      if (v !== null) {
        bitboardVal = v & 0xFFFFFFFFFFFFFFFFn;
        renderBitboard();
      }
    });
  }

  const elB32 = document.getElementById('base-b32');
  if (elB32) {
    elB32.addEventListener('input', (e) => {
      const v = parseBase32(e.target.value);
      updateBasesFromValue(v, e.target);
      if (v !== null) {
        bitboardVal = v & 0xFFFFFFFFFFFFFFFFn;
        renderBitboard();
      }
    });
  }

  const elB58 = document.getElementById('base-b58');
  if (elB58) {
    elB58.addEventListener('input', (e) => {
      const v = parseBase58(e.target.value);
      updateBasesFromValue(v, e.target);
      if (v !== null) {
        bitboardVal = v & 0xFFFFFFFFFFFFFFFFn;
        renderBitboard();
      }
    });
  }

  const btnEndianToggle = document.getElementById('btn-endian-toggle');
  if (btnEndianToggle) {
    btnEndianToggle.onclick = () => {
      isLittleEndianMode = !isLittleEndianMode;
      updateEndianDisplay(bitboardVal);
      showToast(isLittleEndianMode ? 'Byte Order: Little-Endian (x86 Host)' : 'Byte Order: Big-Endian (Network)');
    };
  }

  const btnSwap16 = document.getElementById('btn-endian-swap16');
  if (btnSwap16) {
    btnSwap16.onclick = () => {
      bitboardVal = wordSwap64(bitboardVal);
      renderBitboard();
      updateBasesFromValue(bitboardVal, null);
      showToast('Swapped 16-bit words');
    };
  }

  const btnSwap32 = document.getElementById('btn-endian-swap32');
  if (btnSwap32) {
    btnSwap32.onclick = () => {
      bitboardVal = dwordSwap64(bitboardVal);
      renderBitboard();
      updateBasesFromValue(bitboardVal, null);
      showToast('Swapped 32-bit dwords');
    };
  }

  const btnSwap64 = document.getElementById('btn-endian-swap64');
  if (btnSwap64) {
    btnSwap64.onclick = () => {
      bitboardVal = byteSwap64(bitboardVal);
      renderBitboard();
      updateBasesFromValue(bitboardVal, null);
      showToast('Swapped 64-bit bytes');
    };
  }"""

if old_endian_listen in html:
    html = html.replace(old_endian_listen, new_endian_listen)
    print("Added event listeners for b64/b32/b58 and endian buttons")
else:
    print("Warning: old_endian_listen not found")

# 9. Add String Suite Base32 and Base58 implementation
old_sha_handler = """  document.getElementById('btn-sha256').onclick = async () => {
    try {
      const msgUint8 = new TextEncoder().encode(strInput.value);
      const hashBuffer = await crypto.subtle.digest('SHA-256', msgUint8);
      const hashArray = Array.from(new Uint8Array(hashBuffer));
      strOutput.value = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
      addHistory('SHA-256', strInput.value, strOutput.value);
    } catch(e) { strOutput.value = 'Error calculating SHA-256'; }
  };"""

new_sha_handler = """  // Base32 RFC 4648 String Encode / Decode
  function encodeBase32String(str) {
    const bytes = new TextEncoder().encode(str);
    let binary = "";
    for (let b of bytes) binary += b.toString(2).padStart(8, '0');
    let out = "";
    for (let i = 0; i < binary.length; i += 5) {
      let chunk = binary.substring(i, i + 5);
      if (chunk.length < 5) chunk = chunk.padEnd(5, '0');
      out += B32_RADIX_CHARS[parseInt(chunk, 2)];
    }
    return out;
  }

  function decodeBase32String(str) {
    str = str.replace(/=+$/, '').toUpperCase();
    let binary = "";
    for (let c of str) {
      let idx = B32_RADIX_CHARS.indexOf(c);
      if (idx < 0) throw new Error("Invalid Base32");
      binary += idx.toString(2).padStart(5, '0');
    }
    const bytes = [];
    for (let i = 0; i + 8 <= binary.length; i += 8) {
      bytes.push(parseInt(binary.substring(i, i + 8), 2));
    }
    return new TextDecoder().decode(new Uint8Array(bytes));
  }

  // Base58 Bitcoin String Encode / Decode
  function encodeBase58String(str) {
    const bytes = new TextEncoder().encode(str);
    if (bytes.length === 0) return "";
    let digits = [0];
    for (let i = 0; i < bytes.length; i++) {
      for (let j = 0; j < digits.length; j++) digits[j] <<= 8;
      digits[0] += bytes[i];
      let carry = 0;
      for (let j = 0; j < digits.length; j++) {
        digits[j] += carry;
        carry = (digits[j] / 58) | 0;
        digits[j] %= 58;
      }
      while (carry) {
        digits.push(carry % 58);
        carry = (carry / 58) | 0;
      }
    }
    let res = "";
    for (let i = 0; i < bytes.length && bytes[i] === 0; i++) res += '1';
    for (let i = digits.length - 1; i >= 0; i--) res += B58_RADIX_CHARS[digits[i]];
    return res;
  }

  function decodeBase58String(str) {
    if (!str) return "";
    let bytes = [0];
    for (let i = 0; i < str.length; i++) {
      let c = B58_RADIX_CHARS.indexOf(str[i]);
      if (c < 0) throw new Error("Invalid Base58");
      for (let j = 0; j < bytes.length; j++) bytes[j] *= 58;
      bytes[0] += c;
      let carry = 0;
      for (let j = 0; j < bytes.length; j++) {
        bytes[j] += carry;
        carry = bytes[j] >> 8;
        bytes[j] &= 0xFF;
      }
      while (carry) {
        bytes.push(carry & 0xFF);
        carry >>= 8;
      }
    }
    for (let i = 0; i < str.length && str[i] === '1'; i++) bytes.push(0);
    bytes.reverse();
    return new TextDecoder().decode(new Uint8Array(bytes));
  }

  const btnB32Enc = document.getElementById('btn-b32-enc');
  if (btnB32Enc) {
    btnB32Enc.onclick = () => {
      try {
        strOutput.value = encodeBase32String(strInput.value);
        addHistory('Base32 Encode', strInput.value, strOutput.value);
      } catch(e) { strOutput.value = 'Error encoding Base32'; }
    };
  }

  const btnB32Dec = document.getElementById('btn-b32-dec');
  if (btnB32Dec) {
    btnB32Dec.onclick = () => {
      try {
        strOutput.value = decodeBase32String(strInput.value.trim());
        addHistory('Base32 Decode', strInput.value, strOutput.value);
      } catch(e) { strOutput.value = 'Error decoding Base32'; }
    };
  }

  const btnB58Enc = document.getElementById('btn-b58-enc');
  if (btnB58Enc) {
    btnB58Enc.onclick = () => {
      try {
        strOutput.value = encodeBase58String(strInput.value);
        addHistory('Base58 Encode', strInput.value, strOutput.value);
      } catch(e) { strOutput.value = 'Error encoding Base58'; }
    };
  }

  const btnB58Dec = document.getElementById('btn-b58-dec');
  if (btnB58Dec) {
    btnB58Dec.onclick = () => {
      try {
        strOutput.value = decodeBase58String(strInput.value.trim());
        addHistory('Base58 Decode', strInput.value, strOutput.value);
      } catch(e) { strOutput.value = 'Error decoding Base58'; }
    };
  }

  document.getElementById('btn-sha256').onclick = async () => {
    try {
      const msgUint8 = new TextEncoder().encode(strInput.value);
      const hashBuffer = await crypto.subtle.digest('SHA-256', msgUint8);
      const hashArray = Array.from(new Uint8Array(hashBuffer));
      strOutput.value = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
      addHistory('SHA-256', strInput.value, strOutput.value);
    } catch(e) { strOutput.value = 'Error calculating SHA-256'; }
  };"""

if old_sha_handler in html:
    html = html.replace(old_sha_handler, new_sha_handler)
    print("Added string Base32 and Base58 handlers")
else:
    print("Warning: old_sha_handler not found")

# 10. Add Bitboard Word & Dword Swap handlers, plus Granular Mask Suite Logic
old_mat_res_copy = """  const btnCopyMatRes = document.getElementById('btn-copy-mat-res');
  if (btnCopyMatRes) {
    btnCopyMatRes.onclick = () => {
      const resVal = document.getElementById('mat-res-val').textContent;
      copyToClipboard(resVal, 'Bitwise Result');
    };
  }"""

new_mat_res_copy = """  const btnCopyMatRes = document.getElementById('btn-copy-mat-res');
  if (btnCopyMatRes) {
    btnCopyMatRes.onclick = () => {
      const resVal = document.getElementById('mat-res-val').textContent;
      copyToClipboard(resVal, 'Bitwise Result');
    };
  }

  const btnBbSwapWord = document.getElementById('bb-swap-word');
  if (btnBbSwapWord) {
    btnBbSwapWord.onclick = () => {
      bitboardVal = wordSwap64(bitboardVal);
      renderBitboard();
      updateBasesFromValue(bitboardVal, null);
      showToast('Swapped 16-bit words across 64-bit value');
    };
  }

  const btnBbSwapDword = document.getElementById('bb-swap-dword');
  if (btnBbSwapDword) {
    btnBbSwapDword.onclick = () => {
      bitboardVal = dwordSwap64(bitboardVal);
      renderBitboard();
      updateBasesFromValue(bitboardVal, null);
      showToast('Swapped 32-bit dwords across 64-bit value');
    };
  }

  // --- Granular Bitwise Mask & Subnet / CIDR Suite Logic ---
  let maskCIDR = 32;
  let isMaskMSB = true;
  let isMaskInverted = false;
  let customMaskVal = null;

  function computeActiveMask() {
    if (customMaskVal !== null) {
      let m = customMaskVal & 0xFFFFFFFFFFFFFFFFn;
      return isMaskInverted ? (~m & 0xFFFFFFFFFFFFFFFFn) : m;
    }
    let m = 0n;
    const k = BigInt(maskCIDR);
    if (k > 0n) {
      if (isMaskMSB) {
        m = (0xFFFFFFFFFFFFFFFFn << (64n - k)) & 0xFFFFFFFFFFFFFFFFn;
      } else {
        m = (k >= 64n) ? 0xFFFFFFFFFFFFFFFFn : ((1n << k) - 1n);
      }
    }
    return isMaskInverted ? (~m & 0xFFFFFFFFFFFFFFFFn) : m;
  }

  function updateMaskSuite() {
    const mask = computeActiveMask();
    const hex = '0x' + mask.toString(16).padStart(16, '0').toUpperCase();
    const inv = '0x' + (~mask & 0xFFFFFFFFFFFFFFFFn).toString(16).padStart(16, '0').toUpperCase();

    let popcount = 0;
    for (let i = 0n; i < 64n; i++) {
      if ((mask & (1n << i)) !== 0n) popcount++;
    }

    // Contiguity test
    let isContig = false;
    let invMask = (~mask & 0xFFFFFFFFFFFFFFFFn);
    if (mask !== 0n && (invMask & (invMask + 1n)) === 0n) {
      isContig = true;
    } else if (mask !== 0n && (mask & (mask + 1n)) === 0n) {
      isContig = true;
    }

    const elHex = document.getElementById('mask-hex-val');
    const elWild = document.getElementById('mask-wildcard-val');
    const elPop = document.getElementById('mask-popcount-val');
    const elContig = document.getElementById('mask-contiguity-val');
    const elLabel = document.getElementById('mask-cidr-label');
    const elBadge = document.getElementById('mask-alignment-badge');

    if (elHex) elHex.textContent = hex;
    if (elWild) elWild.textContent = inv;
    if (elPop) elPop.textContent = `${popcount} / 64`;
    if (elContig) elContig.textContent = isContig ? 'Contiguous CIDR Subnet' : 'Non-Contiguous Arbitrary Mask';
    if (elLabel) elLabel.textContent = customMaskVal !== null ? 'Custom' : `/${maskCIDR}`;
    if (elBadge) elBadge.textContent = isMaskMSB ? 'MSB-Aligned (Subnet)' : 'LSB-Aligned (Width)';
  }

  const rangeCIDR = document.getElementById('mask-cidr-range');
  if (rangeCIDR) {
    rangeCIDR.oninput = () => {
      customMaskVal = null;
      maskCIDR = parseInt(rangeCIDR.value, 10);
      updateMaskSuite();
    };
  }

  const btnMaskAlign = document.getElementById('btn-mask-align');
  if (btnMaskAlign) {
    btnMaskAlign.onclick = () => {
      isMaskMSB = !isMaskMSB;
      btnMaskAlign.textContent = isMaskMSB ? 'MSB-Aligned (Subnet)' : 'LSB-Aligned (Width)';
      updateMaskSuite();
    };
  }

  const btnMaskInvert = document.getElementById('btn-mask-invert');
  if (btnMaskInvert) {
    btnMaskInvert.onclick = () => {
      isMaskInverted = !isMaskInverted;
      btnMaskInvert.classList.toggle('btn-accent', isMaskInverted);
      updateMaskSuite();
    };
  }

  const btnMaskLoadOpB = document.getElementById('btn-mask-load-opb');
  if (btnMaskLoadOpB) {
    btnMaskLoadOpB.onclick = () => {
      const hex = '0x' + computeActiveMask().toString(16).padStart(16, '0').toUpperCase();
      opBInput.value = hex;
      executeBitwiseOp(currentBitwiseOp, true);
      showToast(`Loaded mask into Operand B: ${hex}`);
    };
  }

  const btnCopyMask = document.getElementById('btn-copy-mask');
  if (btnCopyMask) {
    btnCopyMask.onclick = () => {
      const hex = '0x' + computeActiveMask().toString(16).padStart(16, '0').toUpperCase();
      copyToClipboard(hex, 'Bitmask Hex');
    };
  }

  document.querySelectorAll('.mask-preset').forEach(btn => {
    btn.onclick = () => {
      if (btn.dataset.bits) {
        customMaskVal = null;
        maskCIDR = parseInt(btn.dataset.bits, 10);
        if (rangeCIDR) rangeCIDR.value = maskCIDR;
        isMaskMSB = (btn.dataset.msb === '1');
        if (btnMaskAlign) btnMaskAlign.textContent = isMaskMSB ? 'MSB-Aligned (Subnet)' : 'LSB-Aligned (Width)';
      } else if (btn.dataset.custom) {
        customMaskVal = parseOpVal(btn.dataset.custom);
      }
      updateMaskSuite();
      showToast(`Applied Mask Preset: ${btn.textContent}`);
    };
  });

  function applyMaskToA(mode) {
    const valA = parseOpVal(opAInput.value) & 0xFFFFFFFFFFFFFFFFn;
    const mask = computeActiveMask();
    let res = 0n;
    if (mode === 'AND') res = valA & mask;
    else if (mode === 'OR') res = valA | mask;
    else if (mode === 'XOR') res = valA ^ mask;
    else if (mode === 'CLEAR') res = valA & (~mask & 0xFFFFFFFFFFFFFFFFn);

    const hex = '0x' + res.toString(16).padStart(16, '0').toUpperCase();
    const elRes = document.getElementById('mask-apply-result');
    if (elRes) elRes.textContent = hex;
    addHistory(`Mask Apply (${mode})`, `A: 0x${valA.toString(16).toUpperCase()} | MASK: 0x${mask.toString(16).toUpperCase()}`, hex);
    showToast(`Applied Mask ${mode}: ${hex}`);
  }

  document.getElementById('btn-mask-apply-and').onclick = () => applyMaskToA('AND');
  document.getElementById('btn-mask-apply-or').onclick = () => applyMaskToA('OR');
  document.getElementById('btn-mask-apply-xor').onclick = () => applyMaskToA('XOR');
  document.getElementById('btn-mask-apply-clear').onclick = () => applyMaskToA('CLEAR');"""

if old_mat_res_copy in html:
    html = html.replace(old_mat_res_copy, new_mat_res_copy)
    print("Added Mask suite JS and Word/Dword swap handlers")
else:
    print("Warning: old_mat_res_copy not found")

# 11. Add initial call to updateMaskSuite() in initialization
old_init_call = "  initBitboardGrid();"
new_init_call = "  initBitboardGrid();\n  if (typeof updateMaskSuite === 'function') updateMaskSuite();"
if old_init_call in html:
    html = html.replace(old_init_call, new_init_call)
    print("Added updateMaskSuite call in init")

with open(html_path, "w", encoding="utf-8") as f:
    f.write(html)

print("KBase HTML successfully updated!")
