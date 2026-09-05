#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 860
#define H 780

// Control IDs
#define ID_HEX 1
#define ID_DEC 2
#define ID_BIN 3
#define ID_OCT 4
#define ID_ASC 5

#define ID_BTN_ENDIAN 10
#define ID_BTN_SWAP16 11
#define ID_BTN_SWAP32 12
#define ID_BTN_INVERT 13
#define ID_BTN_XORMASK 14
#define ID_BTN_CARRAY 15
#define ID_BTN_DUMP 16
#define ID_BTN_DISSECT 17

HWND hHex, hDec, hBin, hOct, hAscii;
HWND hInt8, hUint8, hInt16, hUint16, hInt32, hUint32, hFloat;
HWND hEntropy, hPopCount, hSig;
HWND hSum8, hSum16, hSum32, hXor8, hCRC32;
HWND hExportEdit, hEndianBtn;

BOOL updating = FALSE;
BOOL isLittleEndian = TRUE;
HBRUSH hBrushBg;
HFONT hFont;
HBRUSH hEditBrush;

// Helper String & Memory Functions (CRT-free)
#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void* custom_memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

size_t custom_strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

// Parsers
unsigned int parseHex(const char* s) {
    unsigned int res = 0;
    while (*s) {
        if (*s == 'x' || *s == 'X') { s++; continue; }
        res <<= 4;
        if (*s >= '0' && *s <= '9') res += *s - '0';
        else if (*s >= 'a' && *s <= 'f') res += *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') res += *s - 'A' + 10;
        s++;
    }
    return res;
}

unsigned int parseDec(const char* s) {
    unsigned int res = 0;
    while (*s) {
        if (*s >= '0' && *s <= '9') res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

unsigned int parseBin(const char* s) {
    unsigned int res = 0;
    while (*s) {
        if (*s == '0' || *s == '1') {
            res <<= 1;
            if (*s == '1') res++;
        }
        s++;
    }
    return res;
}

unsigned int parseOct(const char* s) {
    unsigned int res = 0;
    while (*s) {
        if (*s >= '0' && *s <= '7') {
            res <<= 3;
            res += *s - '0';
        }
        s++;
    }
    return res;
}

unsigned int parseAscii(const char* s) {
    unsigned int res = 0;
    int count = 0;
    while (*s && count < 4) {
        res = (res << 8) | (unsigned char)(*s);
        s++;
        count++;
    }
    return res;
}

// Formatters
void fmtHex(unsigned int v, char* s) {
    wsprintfA(s, "0x%08X", v);
}

void fmtDec(unsigned int v, char* s) {
    wsprintfA(s, "%u", v);
}

void fmtBin(unsigned int v, char* s) {
    char tmp[33];
    int i = 0;
    for (i = 31; i >= 0; i--) {
        tmp[31 - i] = ((v >> i) & 1) ? '1' : '0';
    }
    tmp[32] = 0;
    custom_memcpy(s, tmp, 33);
}

void fmtOct(unsigned int v, char* s) {
    wsprintfA(s, "%o", v);
}

void fmtAscii(unsigned int v, char* s) {
    int i, j = 0;
    int started = 0;
    if (v == 0) { s[0] = '.'; s[1] = 0; return; }
    for (i = 3; i >= 0; i--) {
        unsigned char c = (v >> (i * 8)) & 0xFF;
        if (c != 0) started = 1;
        if (started) {
            if (c >= 32 && c <= 126) s[j++] = c;
            else s[j++] = '.';
        }
    }
    s[j] = 0;
    if (j == 0) { s[0] = '.'; s[1] = 0; }
}

// Byte Endian Swap Functions
unsigned int swap16(unsigned int v) {
    return ((v & 0xFF00FF00) >> 8) | ((v & 0x00FF00FF) << 8);
}

unsigned int swap32(unsigned int v) {
    return ((v & 0x000000FF) << 24) |
           ((v & 0x0000FF00) << 8)  |
           ((v & 0x00FF0000) >> 8)  |
           ((v & 0xFF000000) >> 24);
}

// CRC32 Calculation
unsigned int calcCRC32(unsigned int val) {
    unsigned char bytes[4];
    bytes[0] = val & 0xFF;
    bytes[1] = (val >> 8) & 0xFF;
    bytes[2] = (val >> 16) & 0xFF;
    bytes[3] = (val >> 24) & 0xFF;

    unsigned int crc = 0xFFFFFFFF;
    int i, j;
    for (i = 0; i < 4; i++) {
        crc ^= bytes[i];
        for (j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// Simple Float format to string
void fmtFloat(float f, char* buf) {
    int intPart = (int)f;
    float diff = f - (float)intPart;
    if (diff < 0) diff = -diff;
    int fracPart = (int)(diff * 1000.0f);
    wsprintfA(buf, "%d.%03d", intPart, fracPart);
}

// Popcount and Entropy
int popcount32(unsigned int v) {
    int count = 0;
    while (v) {
        count += (v & 1);
        v >>= 1;
    }
    return count;
}

void calcEntropy4(unsigned int val, char* outBuf, char* classBuf) {
    unsigned char b[4];
    b[0] = (val >> 24) & 0xFF;
    b[1] = (val >> 16) & 0xFF;
    b[2] = (val >> 8) & 0xFF;
    b[3] = val & 0xFF;

    unsigned char uVal[4];
    int counts[4] = {0, 0, 0, 0};
    int uniqueCount = 0;
    int i, j;
    for (i = 0; i < 4; i++) {
        int found = -1;
        for (j = 0; j < uniqueCount; j++) {
            if (b[i] == uVal[j]) {
                found = j;
                break;
            }
        }
        if (found >= 0) {
            counts[found]++;
        } else {
            uVal[uniqueCount] = b[i];
            counts[uniqueCount] = 1;
            uniqueCount++;
        }
    }

    if (uniqueCount == 1) {
        wsprintfA(outBuf, "0.000 b/B");
        wsprintfA(classBuf, "Zero / Constant");
    } else if (uniqueCount == 2) {
        if (counts[0] == 3 || counts[1] == 3) {
            wsprintfA(outBuf, "3.245 b/B");
            wsprintfA(classBuf, "Low (Repetitive)");
        } else {
            wsprintfA(outBuf, "4.000 b/B");
            wsprintfA(classBuf, "Medium (Structured)");
        }
    } else if (uniqueCount == 3) {
        wsprintfA(outBuf, "6.000 b/B");
        wsprintfA(classBuf, "High (Code / Text)");
    } else {
        wsprintfA(outBuf, "8.000 b/B");
        wsprintfA(classBuf, "Max (High Randomness)");
    }
}

const char* detectMagicSignature(unsigned int val, unsigned int swapVal) {
    unsigned int v = val;
    unsigned int s = swapVal;
    if (v == 0x89504E47 || s == 0x89504E47) return "PNG Image (.png) [0x89504E47]";
    if ((v & 0xFFFFFF00) == 0xFFD8FF00 || (s & 0xFFFFFF00) == 0xFFD8FF00) return "JPEG Image (.jpg) [0xFFD8FF]";
    if (v == 0x504B0304 || s == 0x504B0304) return "ZIP / JAR / APK (.zip) [0x504B0304]";
    if (v == 0x504B0506 || s == 0x504B0506) return "ZIP Empty Archive (.zip) [0x504B0506]";
    if (v == 0x7F454C46 || s == 0x7F454C46) return "ELF Executable (.elf) [0x7F454C46]";
    if (v == 0x25504446 || s == 0x25504446) return "PDF Document (.pdf) [0x25504446 (%PDF)]";
    if (v == 0x47494638 || s == 0x47494638) return "GIF Image (.gif) [0x47494638 (GIF8)]";
    if (v == 0x52494646 || s == 0x52494646) return "RIFF Multimedia / WAV (.wav) [0x52494646]";
    if (v == 0x53514C69 || s == 0x53514C69) return "SQLite 3 Database (.sqlite) [0x53514C69]";
    if (v == 0x0061736D || s == 0x0061736D) return "WebAssembly Binary (.wasm) [0x0061736D]";
    if (v == 0xCAFEBABE || s == 0xCAFEBABE) return "Java Class / Mach-O Fat [0xCAFEBABE]";
    if (v == 0xFD377A58 || s == 0xFD377A58) return "XZ Compressed Archive (.xz) [0xFD377A58]";
    if (v == 0x4F676753 || s == 0x4F676753) return "OGG Media Stream (.ogg) [0x4F676753]";
    if (v == 0x75737461 || s == 0x75737461) return "TAR Archive (.tar) [0x75737461]";
    if (v == 0x1A45DFA3 || s == 0x1A45DFA3) return "Matroska / WebM Video (.mkv) [0x1A45DFA3]";
    if (v == 0x464C5601 || s == 0x464C5601) return "Flash Video Container (.flv) [0x464C5601]";

    unsigned short w1 = (unsigned short)((v >> 16) & 0xFFFF);
    unsigned short w0 = (unsigned short)(v & 0xFFFF);
    unsigned short sw1 = (unsigned short)((s >> 16) & 0xFFFF);
    unsigned short sw0 = (unsigned short)(s & 0xFFFF);
    if (w1 == 0x4D5A || w0 == 0x4D5A || sw1 == 0x4D5A || sw0 == 0x4D5A) return "DOS / PE Executable (.exe, .dll) ['MZ']";
    if (w1 == 0x1F8B || w0 == 0x1F8B || sw1 == 0x1F8B || sw0 == 0x1F8B) return "GZIP Compressed (.gz) [0x1F8B]";
    if (w1 == 0x425A || w0 == 0x425A || sw1 == 0x425A || sw0 == 0x425A) return "BZip2 Compressed (.bz2) ['BZ']";
    if (w1 == 0x2321 || w0 == 0x2321 || sw1 == 0x2321 || sw0 == 0x2321) return "Unix Script Shebang ['#!']";
    if (w1 == 0x3C21 || w0 == 0x3C21 || sw1 == 0x3C21 || sw0 == 0x3C21) return "HTML / XML Document ['<!']";

    return "Raw Binary / Custom Pattern";
}

// Data Inspector Update
void UpdateInspector(unsigned int val) {
    char buf[128];
    unsigned int inspectVal = isLittleEndian ? val : swap32(val);

    // Int8 / Uint8
    char b8 = (char)(inspectVal & 0xFF);
    unsigned char ub8 = (unsigned char)(inspectVal & 0xFF);
    wsprintfA(buf, "%d", b8); SetWindowTextA(hInt8, buf);
    wsprintfA(buf, "%u", ub8); SetWindowTextA(hUint8, buf);

    // Int16 / Uint16
    short s16 = (short)(inspectVal & 0xFFFF);
    unsigned short us16 = (unsigned short)(inspectVal & 0xFFFF);
    wsprintfA(buf, "%d", s16); SetWindowTextA(hInt16, buf);
    wsprintfA(buf, "%u", us16); SetWindowTextA(hUint16, buf);

    // Int32 / Uint32
    int i32 = (int)inspectVal;
    wsprintfA(buf, "%d", i32); SetWindowTextA(hInt32, buf);
    wsprintfA(buf, "%u", inspectVal); SetWindowTextA(hUint32, buf);

    // Float 32
    union { unsigned int u; float f; } flt;
    flt.u = inspectVal;
    fmtFloat(flt.f, buf);
    SetWindowTextA(hFloat, buf);

    // Entropy & Diagnostics
    char entStr[32], entClass[32];
    calcEntropy4(inspectVal, entStr, entClass);
    wsprintfA(buf, "%s (%s)", entStr, entClass);
    SetWindowTextA(hEntropy, buf);

    int pop = popcount32(inspectVal);
    wsprintfA(buf, "%d / 32 (%s)", pop, (pop % 2 == 0) ? "Even" : "Odd");
    SetWindowTextA(hPopCount, buf);

    const char* sig = detectMagicSignature(val, swap32(val));
    SetWindowTextA(hSig, sig);

    // Hashes & Checksums
    unsigned char b0 = inspectVal & 0xFF;
    unsigned char b1 = (inspectVal >> 8) & 0xFF;
    unsigned char b2 = (inspectVal >> 16) & 0xFF;
    unsigned char b3 = (inspectVal >> 24) & 0xFF;

    unsigned int sum8 = (b0 + b1 + b2 + b3) & 0xFF;
    unsigned int sum16 = (b0 + b1 + b2 + b3) & 0xFFFF;
    unsigned int sum32 = inspectVal;
    unsigned int xor8 = b0 ^ b1 ^ b2 ^ b3;
    unsigned int crc32 = calcCRC32(inspectVal);

    wsprintfA(buf, "0x%02X", sum8); SetWindowTextA(hSum8, buf);
    wsprintfA(buf, "0x%04X", sum16); SetWindowTextA(hSum16, buf);
    wsprintfA(buf, "0x%08X", sum32); SetWindowTextA(hSum32, buf);
    wsprintfA(buf, "0x%02X", xor8); SetWindowTextA(hXor8, buf);
    wsprintfA(buf, "0x%08X", crc32); SetWindowTextA(hCRC32, buf);
}

void UpdateFields(HWND hSrc) {
    if (updating) return;
    updating = TRUE;

    char buf[128];
    GetWindowTextA(hSrc, buf, 128);

    unsigned int val = 0;
    if (hSrc == hHex) val = parseHex(buf);
    else if (hSrc == hDec) val = parseDec(buf);
    else if (hSrc == hBin) val = parseBin(buf);
    else if (hSrc == hOct) val = parseOct(buf);
    else if (hSrc == hAscii) val = parseAscii(buf);

    char hex[64], dec[64], bin[64], oct[64], asc[64];
    fmtHex(val, hex);
    fmtDec(val, dec);
    fmtBin(val, bin);
    fmtOct(val, oct);
    fmtAscii(val, asc);

    if (hSrc != hHex) SetWindowTextA(hHex, hex);
    if (hSrc != hDec) SetWindowTextA(hDec, dec);
    if (hSrc != hBin) SetWindowTextA(hBin, bin);
    if (hSrc != hOct) SetWindowTextA(hOct, oct);
    if (hSrc != hAscii) SetWindowTextA(hAscii, asc);

    UpdateInspector(val);

    updating = FALSE;
}

unsigned int GetCurrentVal() {
    char buf[64];
    GetWindowTextA(hHex, buf, 64);
    return parseHex(buf);
}

void SetCurrentVal(unsigned int val) {
    char buf[64];
    fmtHex(val, buf);
    SetWindowTextA(hHex, buf);
    UpdateFields(hHex);
}

void ExportCArray(unsigned int val) {
    char out[256];
    unsigned char b0 = (val >> 24) & 0xFF;
    unsigned char b1 = (val >> 16) & 0xFF;
    unsigned char b2 = (val >> 8) & 0xFF;
    unsigned char b3 = val & 0xFF;
    wsprintfA(out, "const unsigned char data[4] = { 0x%02X, 0x%02X, 0x%02X, 0x%02X };", b0, b1, b2, b3);
    SetWindowTextA(hExportEdit, out);
}

void ExportHexDump(unsigned int val) {
    char out[256];
    unsigned char b0 = (val >> 24) & 0xFF;
    unsigned char b1 = (val >> 16) & 0xFF;
    unsigned char b2 = (val >> 8) & 0xFF;
    unsigned char b3 = val & 0xFF;
    wsprintfA(out, "0x00000000  %02X %02X %02X %02X  |....|", b0, b1, b2, b3);
    SetWindowTextA(hExportEdit, out);
}

void ExportDissection(unsigned int val) {
    char out[4096];
    unsigned int swapVal = swap32(val);
    unsigned int inspectVal = isLittleEndian ? val : swapVal;
    
    unsigned char b0 = (inspectVal >> 24) & 0xFF;
    unsigned char b1 = (inspectVal >> 16) & 0xFF;
    unsigned char b2 = (inspectVal >> 8) & 0xFF;
    unsigned char b3 = inspectVal & 0xFF;

    int pop = popcount32(inspectVal);
    int popPct = (pop * 1000) / 32;

    char entStr[32], entClass[32];
    calcEntropy4(inspectVal, entStr, entClass);

    const char* sig = detectMagicSignature(val, swapVal);

    int nullBytes = (b0 == 0) + (b1 == 0) + (b2 == 0) + (b3 == 0);
    int asciiBytes = (b0 >= 32 && b0 <= 126) + (b1 >= 32 && b1 <= 126) + (b2 >= 32 && b2 <= 126) + (b3 >= 32 && b3 <= 126);
    int highBytes = (b0 >= 128) + (b1 >= 128) + (b2 >= 128) + (b3 >= 128);

    wsprintfA(out,
        "=======================================================================\r\n"
        " KHEX DEEP BINARY DISSECTION & SHANNON ENTROPY REPORT\r\n"
        "=======================================================================\r\n"
        "Raw Hex Value:        0x%08X  |  Endian Swapped: 0x%08X\r\n"
        "Active Endian Mode:   %s\r\n"
        "File Signature Match: %s\r\n"
        "Shannon Entropy:      %s (%s)\r\n"
        "Bit Population Count: %d / 32 bits set (%d.%d%% bit density, %s Parity)\r\n"
        "-----------------------------------------------------------------------\r\n"
        "BYTE DISTRIBUTION & CLASSIFICATION:\r\n"
        "  * Null Bytes (0x00):        %d / 4  (%d%%)\r\n"
        "  * Printable ASCII (32..126): %d / 4  (%d%%)\r\n"
        "  * High / Binary (>= 0x80):   %d / 4  (%d%%)\r\n"
        "-----------------------------------------------------------------------\r\n"
        "BYTE-BY-BYTE STRUCTURAL BREAKDOWN:\r\n"
        "  [0] 0x%02X (%3u)  Bin: %d%d%d%d%d%d%d%d  ASCII: '%c'  [%s]\r\n"
        "  [1] 0x%02X (%3u)  Bin: %d%d%d%d%d%d%d%d  ASCII: '%c'  [%s]\r\n"
        "  [2] 0x%02X (%3u)  Bin: %d%d%d%d%d%d%d%d  ASCII: '%c'  [%s]\r\n"
        "  [3] 0x%02X (%3u)  Bin: %d%d%d%d%d%d%d%d  ASCII: '%c'  [%s]\r\n"
        "-----------------------------------------------------------------------\r\n"
        "INTEGRITY & CHECKSUMS:\r\n"
        "  Sum8: 0x%02X  |  Sum16: 0x%04X  |  Sum32: 0x%08X\r\n"
        "  XOR8: 0x%02X  |  CRC32: 0x%08X\r\n"
        "=======================================================================",
        val, swapVal,
        isLittleEndian ? "Little Endian (LE)" : "Big Endian (BE)",
        sig,
        entStr, entClass,
        pop, popPct / 10, popPct % 10, (pop % 2 == 0) ? "Even" : "Odd",
        nullBytes, nullBytes * 25,
        asciiBytes, asciiBytes * 25,
        highBytes, highBytes * 25,
        b0, b0, (b0>>7)&1, (b0>>6)&1, (b0>>5)&1, (b0>>4)&1, (b0>>3)&1, (b0>>2)&1, (b0>>1)&1, b0&1, (b0>=32&&b0<=126)?b0:'.', (b0>=32&&b0<=126)?"Printable":(b0==0?"Null":"Binary"),
        b1, b1, (b1>>7)&1, (b1>>6)&1, (b1>>5)&1, (b1>>4)&1, (b1>>3)&1, (b1>>2)&1, (b1>>1)&1, b1&1, (b1>=32&&b1<=126)?b1:'.', (b1>=32&&b1<=126)?"Printable":(b1==0?"Null":"Binary"),
        b2, b2, (b2>>7)&1, (b2>>6)&1, (b2>>5)&1, (b2>>4)&1, (b2>>3)&1, (b2>>2)&1, (b2>>1)&1, b2&1, (b2>=32&&b2<=126)?b2:'.', (b2>=32&&b2<=126)?"Printable":(b2==0?"Null":"Binary"),
        b3, b3, (b3>>7)&1, (b3>>6)&1, (b3>>5)&1, (b3>>4)&1, (b3>>3)&1, (b3>>2)&1, (b3>>1)&1, b3&1, (b3>=32&&b3<=126)?b3:'.', (b3>=32&&b3<=126)?"Printable":(b3==0?"Null":"Binary"),
        (b0+b1+b2+b3)&0xFF, (b0+b1+b2+b3)&0xFFFF, inspectVal,
        b0^b1^b2^b3, calcCRC32(inspectVal)
    );

    SetWindowTextA(hExportEdit, out);
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "=== KHEX UTILITY SUITE — USER GUIDE ===\n\n"
        "CORE FEATURES:\n"
        " • Base Converter: Live sync between Hex, Dec, Bin, Oct, and ASCII.\n"
        " • Multi-Type Inspector: Signed/Unsigned Int8/16/32, Float32, PopCount, Shannon Entropy & File Signature.\n"
        " • Checksum Suite: Instant Sum8, Sum16, Sum32, XOR8, and IEEE CRC32.\n"
        " • Byte Operations: Swap16, Swap32, Invert (~), and XOR 0xFF mask.\n"
        " • Export Suite: C/C++ byte arrays, formatted HexDump, and deep Shannon Entropy reports.\n\n"
        "KEYBOARD SHORTCUTS:\n"
        " • [F1]               : Open this Help Dialog\n"
        " • [Ctrl+E]           : Toggle Endianness (LE / BE)\n"
        " • [Ctrl+1]           : Endian Swap 16-bit Words\n"
        " • [Ctrl+2]           : Endian Swap 32-bit Words\n"
        " • [Ctrl+3]           : Bitwise Invert (~)\n"
        " • [Ctrl+4]           : XOR 0xFF Mask\n"
        " • [Ctrl+5]           : Export as C Array\n"
        " • [Ctrl+6]           : Export as HexDump\n"
        " • [Ctrl+7]           : Deep Dissection & Entropy Report\n\n"
        "Tip: Type in any base field to instantly update all representations.",
        "KHex Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

// Fast integer trigonometry (0..360 deg -> -1000..1000)
int isin(int deg) {
    deg = (deg % 360 + 360) % 360;
    if (deg > 180) return -isin(deg - 180);
    int prod = deg * (180 - deg);
    return (4 * prod * 1000) / (40500 - prod);
}

int icos(int deg) {
    return isin(deg + 90);
}

static unsigned int g_rng = 0x12345678;
unsigned int k_rand() {
    g_rng = g_rng * 1664525u + 1013904223u;
    return g_rng;
}

// Particle & FX Engine Structures
typedef struct {
    int active;
    int layer; // 1: needle spark, 2: buoyant puff, 3: binary shard, 4: star
    int x, y;   // fixed point * 10
    int vx, vy; // fixed point * 10
    int life;
    int decay;
    int size;
    int rot, vRot;
    COLORREF color;
    char text[4];
} GdiParticle;

#define MAX_PARTICLES 96
static GdiParticle g_particles[MAX_PARTICLES];

typedef struct {
    int active;
    int x, y;
    int innerRadius, outerRadius;
    int innerSpeed, outerSpeed;
    int maxRadius;
    int life, decay;
    COLORREF color;
} GdiShockwave;

#define MAX_SHOCKWAVES 8
static GdiShockwave g_shockwaves[MAX_SHOCKWAVES];

typedef struct {
    int x, y;
    int vx, vy;
    int phase;
    char ch;
    COLORREF color;
} GdiMote;

#define MAX_MOTES 20
static GdiMote g_motes[MAX_MOTES];
static int g_motesInit = 0;

static int g_shake = 0;

void TriggerShake(int amt) {
    g_shake += amt;
    if (g_shake > 18) g_shake = 18;
}

void SpawnParticles(int x, int y, COLORREF colorScheme, int count) {
    TriggerShake(count / 4 + 2);
    int i, pIdx = 0;
    const char* shards[6] = { "0x", "FF", "00", "1", "0", "A5" };

    // Layer 1: Needle sparks
    for (i = 0; i < count; i++) {
        for (; pIdx < MAX_PARTICLES; pIdx++) {
            if (!g_particles[pIdx].active) break;
        }
        if (pIdx >= MAX_PARTICLES) break;
        int deg = (k_rand() % 360);
        int spd = 30 + (k_rand() % 50);
        g_particles[pIdx].active = 1;
        g_particles[pIdx].layer = 1;
        g_particles[pIdx].x = x * 10;
        g_particles[pIdx].y = y * 10;
        g_particles[pIdx].vx = (icos(deg) * spd) / 1000;
        g_particles[pIdx].vy = (isin(deg) * spd) / 1000;
        g_particles[pIdx].life = 100;
        g_particles[pIdx].decay = 4 + (k_rand() % 4);
        g_particles[pIdx].color = colorScheme;
        g_particles[pIdx].size = 4 + (k_rand() % 6);
    }

    // Layer 2: Buoyant puffs
    for (i = 0; i < count / 2; i++) {
        for (; pIdx < MAX_PARTICLES; pIdx++) {
            if (!g_particles[pIdx].active) break;
        }
        if (pIdx >= MAX_PARTICLES) break;
        int deg = (k_rand() % 360);
        int spd = 10 + (k_rand() % 25);
        g_particles[pIdx].active = 1;
        g_particles[pIdx].layer = 2;
        g_particles[pIdx].x = x * 10;
        g_particles[pIdx].y = y * 10;
        g_particles[pIdx].vx = (icos(deg) * spd) / 1000;
        g_particles[pIdx].vy = (isin(deg) * spd) / 1000 - 8;
        g_particles[pIdx].life = 100;
        g_particles[pIdx].decay = 3 + (k_rand() % 3);
        g_particles[pIdx].color = colorScheme;
        g_particles[pIdx].size = 6 + (k_rand() % 8);
    }

    // Layer 3: Binary shards
    for (i = 0; i < count / 3; i++) {
        for (; pIdx < MAX_PARTICLES; pIdx++) {
            if (!g_particles[pIdx].active) break;
        }
        if (pIdx >= MAX_PARTICLES) break;
        int deg = (k_rand() % 360);
        int spd = 20 + (k_rand() % 40);
        g_particles[pIdx].active = 1;
        g_particles[pIdx].layer = 3;
        g_particles[pIdx].x = x * 10;
        g_particles[pIdx].y = y * 10;
        g_particles[pIdx].vx = (icos(deg) * spd) / 1000;
        g_particles[pIdx].vy = (isin(deg) * spd) / 1000 - 15;
        g_particles[pIdx].life = 100;
        g_particles[pIdx].decay = 2 + (k_rand() % 3);
        g_particles[pIdx].color = RGB(103, 232, 249);
        const char* txt = shards[k_rand() % 6];
        g_particles[pIdx].text[0] = txt[0];
        g_particles[pIdx].text[1] = txt[1];
        g_particles[pIdx].text[2] = txt[2];
        g_particles[pIdx].text[3] = 0;
    }

    // Layer 4: Celebration stars
    for (i = 0; i < count / 4; i++) {
        for (; pIdx < MAX_PARTICLES; pIdx++) {
            if (!g_particles[pIdx].active) break;
        }
        if (pIdx >= MAX_PARTICLES) break;
        int deg = (k_rand() % 360);
        int spd = 15 + (k_rand() % 35);
        g_particles[pIdx].active = 1;
        g_particles[pIdx].layer = 4;
        g_particles[pIdx].x = x * 10;
        g_particles[pIdx].y = y * 10;
        g_particles[pIdx].vx = (icos(deg) * spd) / 1000;
        g_particles[pIdx].vy = (isin(deg) * spd) / 1000;
        g_particles[pIdx].life = 100;
        g_particles[pIdx].decay = 3;
        g_particles[pIdx].rot = k_rand() % 360;
        g_particles[pIdx].vRot = 10 + (k_rand() % 15);
        g_particles[pIdx].color = RGB(254, 240, 138);
        g_particles[pIdx].size = 5 + (k_rand() % 5);
    }
}

void SpawnShockwave(int x, int y, COLORREF color) {
    int i;
    for (i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) {
            g_shockwaves[i].active = 1;
            g_shockwaves[i].x = x;
            g_shockwaves[i].y = y;
            g_shockwaves[i].innerRadius = 4;
            g_shockwaves[i].outerRadius = 6;
            g_shockwaves[i].innerSpeed = 8;
            g_shockwaves[i].outerSpeed = 5;
            g_shockwaves[i].maxRadius = 60;
            g_shockwaves[i].life = 100;
            g_shockwaves[i].decay = 5;
            g_shockwaves[i].color = color;
            break;
        }
    }
}

void DrawCyberHudBracket(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    HPEN hPen = CreatePen(PS_SOLID, 2, color);
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    int arm = 8;
    // Top-Left
    MoveToEx(hdc, x, y + arm, NULL); LineTo(hdc, x, y); LineTo(hdc, x + arm, y);
    // Top-Right
    MoveToEx(hdc, x + w - arm, y, NULL); LineTo(hdc, x + w, y); LineTo(hdc, x + w, y + arm);
    // Bottom-Left
    MoveToEx(hdc, x, y + h - arm, NULL); LineTo(hdc, x, y + h); LineTo(hdc, x + arm, y + h);
    // Bottom-Right
    MoveToEx(hdc, x + w - arm, y + h, NULL); LineTo(hdc, x + w, y + h); LineTo(hdc, x + w, y + h - arm);
    
    // Gold Rivet Accents
    SetPixel(hdc, x + 2, y + 2, RGB(245, 158, 11));
    SetPixel(hdc, x + w - 3, y + 2, RGB(245, 158, 11));
    SetPixel(hdc, x + 2, y + h - 3, RGB(245, 158, 11));
    SetPixel(hdc, x + w - 3, y + h - 3, RGB(245, 158, 11));

    SelectObject(hdc, hOld);
    DeleteObject(hPen);
}

void DrawGdiStar(HDC hdc, int cx, int cy, int size, int rotDeg, COLORREF color) {
    POINT pts[8];
    int i;
    for (i = 0; i < 8; i++) {
        int r = (i % 2 == 0) ? size : (size / 2);
        int a = rotDeg + i * 45;
        pts[i].x = cx + (icos(a) * r) / 1000;
        pts[i].y = cy + (isin(a) * r) / 1000;
    }
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HBRUSH hBrush = CreateSolidBrush(color);
    HPEN hOldP = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hBrush);
    Polygon(hdc, pts, 8);
    SelectObject(hdc, hOldP);
    SelectObject(hdc, hOldB);
    DeleteObject(hPen);
    DeleteObject(hBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            SetTimer(hwnd, 1, 33, NULL); // 30 FPS smooth animation loop
            HDC hdc = GetDC(NULL);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
            int fontHeight = -MulDiv(12, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

            // Section 1: Base Converter
            CreateWindowEx(0, "STATIC", "--- BASE CONVERTER ---", WS_CHILD | WS_VISIBLE, 10, 8, 200, 16, hwnd, NULL, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 740, 8, 80, 24, hwnd, (HMENU)100, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Hex:", WS_CHILD | WS_VISIBLE, 10, 28, 35, 20, hwnd, NULL, NULL, NULL);
            hHex = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x00000000", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 50, 28, 160, 22, hwnd, (HMENU)ID_HEX, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Dec:", WS_CHILD | WS_VISIBLE, 10, 53, 35, 20, hwnd, NULL, NULL, NULL);
            hDec = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 50, 53, 160, 22, hwnd, (HMENU)ID_DEC, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Bin:", WS_CHILD | WS_VISIBLE, 10, 78, 35, 20, hwnd, NULL, NULL, NULL);
            hBin = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "00000000000000000000000000000000", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 50, 78, 260, 22, hwnd, (HMENU)ID_BIN, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Oct:", WS_CHILD | WS_VISIBLE, 320, 28, 35, 20, hwnd, NULL, NULL, NULL);
            hOct = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 360, 28, 180, 22, hwnd, (HMENU)ID_OCT, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Asc:", WS_CHILD | WS_VISIBLE, 320, 53, 35, 20, hwnd, NULL, NULL, NULL);
            hAscii = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", ".", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 360, 53, 180, 22, hwnd, (HMENU)ID_ASC, NULL, NULL);

            // Section 2: Data Inspector Panel
            CreateWindowEx(0, "STATIC", "--- MULTI-TYPE DATA INSPECTOR & ENTROPY ---", WS_CHILD | WS_VISIBLE, 10, 108, 320, 16, hwnd, NULL, NULL, NULL);
            hEndianBtn = CreateWindowEx(0, "BUTTON", "Endian: LE [^E]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 330, 104, 110, 22, hwnd, (HMENU)ID_BTN_ENDIAN, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Int8:", WS_CHILD | WS_VISIBLE, 10, 130, 40, 20, hwnd, NULL, NULL, NULL);
            hInt8 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 55, 130, 90, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Uint8:", WS_CHILD | WS_VISIBLE, 160, 130, 45, 20, hwnd, NULL, NULL, NULL);
            hUint8 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 210, 130, 90, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Int16:", WS_CHILD | WS_VISIBLE, 315, 130, 45, 20, hwnd, NULL, NULL, NULL);
            hInt16 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 365, 130, 90, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Uint16:", WS_CHILD | WS_VISIBLE, 470, 130, 55, 20, hwnd, NULL, NULL, NULL);
            hUint16 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 530, 130, 90, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Int32:", WS_CHILD | WS_VISIBLE, 10, 155, 45, 20, hwnd, NULL, NULL, NULL);
            hInt32 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 55, 155, 120, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Uint32:", WS_CHILD | WS_VISIBLE, 190, 155, 50, 20, hwnd, NULL, NULL, NULL);
            hUint32 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_READONLY, 245, 155, 120, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Float32:", WS_CHILD | WS_VISIBLE, 380, 155, 55, 20, hwnd, NULL, NULL, NULL);
            hFloat = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.000", WS_CHILD | WS_VISIBLE | ES_READONLY, 440, 155, 180, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Entropy:", WS_CHILD | WS_VISIBLE, 10, 180, 55, 20, hwnd, NULL, NULL, NULL);
            hEntropy = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.000 b/B", WS_CHILD | WS_VISIBLE | ES_READONLY, 70, 180, 165, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "PopCount:", WS_CHILD | WS_VISIBLE, 245, 180, 60, 20, hwnd, NULL, NULL, NULL);
            hPopCount = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0 / 32", WS_CHILD | WS_VISIBLE | ES_READONLY, 310, 180, 110, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Signature:", WS_CHILD | WS_VISIBLE, 430, 180, 60, 20, hwnd, NULL, NULL, NULL);
            hSig = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Raw Binary", WS_CHILD | WS_VISIBLE | ES_READONLY, 495, 180, 325, 22, hwnd, NULL, NULL, NULL);

            // Section 3: Hashes & Checksums
            CreateWindowEx(0, "STATIC", "--- CHECKSUM & HASH SUITE ---", WS_CHILD | WS_VISIBLE, 10, 210, 250, 16, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sum8:", WS_CHILD | WS_VISIBLE, 10, 230, 40, 20, hwnd, NULL, NULL, NULL);
            hSum8 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x00", WS_CHILD | WS_VISIBLE | ES_READONLY, 55, 230, 70, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sum16:", WS_CHILD | WS_VISIBLE, 135, 230, 45, 20, hwnd, NULL, NULL, NULL);
            hSum16 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x0000", WS_CHILD | WS_VISIBLE | ES_READONLY, 185, 230, 90, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "XOR8:", WS_CHILD | WS_VISIBLE, 285, 230, 40, 20, hwnd, NULL, NULL, NULL);
            hXor8 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x00", WS_CHILD | WS_VISIBLE | ES_READONLY, 330, 230, 70, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "CRC32:", WS_CHILD | WS_VISIBLE, 10, 255, 45, 20, hwnd, NULL, NULL, NULL);
            hCRC32 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x00000000", WS_CHILD | WS_VISIBLE | ES_READONLY, 60, 255, 120, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sum32:", WS_CHILD | WS_VISIBLE, 190, 255, 45, 20, hwnd, NULL, NULL, NULL);
            hSum32 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0x00000000", WS_CHILD | WS_VISIBLE | ES_READONLY, 240, 255, 120, 22, hwnd, NULL, NULL, NULL);

            // Section 4: Operations & Export
            CreateWindowEx(0, "STATIC", "--- BYTE OPERATIONS, EXPORT & DISSECTION ---", WS_CHILD | WS_VISIBLE, 10, 285, 340, 16, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "BUTTON", "Swap16 [^1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 305, 80, 24, hwnd, (HMENU)ID_BTN_SWAP16, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Swap32 [^2]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 95, 305, 80, 24, hwnd, (HMENU)ID_BTN_SWAP32, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Invert [^3]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 180, 305, 75, 24, hwnd, (HMENU)ID_BTN_INVERT, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "XOR FF [^4]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 260, 305, 80, 24, hwnd, (HMENU)ID_BTN_XORMASK, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "C Array [^5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 345, 305, 80, 24, hwnd, (HMENU)ID_BTN_CARRAY, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "HexDump [^6]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 430, 305, 85, 24, hwnd, (HMENU)ID_BTN_DUMP, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Dissect & Entropy [^7]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 520, 305, 140, 24, hwnd, (HMENU)ID_BTN_DISSECT, NULL, NULL);

            hExportEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Welcome to KHex Suite!\r\nPress F1 or click 'Help' for user guide & shortcuts.\r\n\r\nResult / Export & Deep Dissection preview area...", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | WS_TABSTOP, 10, 335, 810, 320, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Shortcuts: F1 (Help), Ctrl+E (Endian), Ctrl+1..7 (Operations)", WS_CHILD | WS_VISIBLE, 10, 665, 400, 16, hwnd, NULL, NULL, NULL);

            // Initialize Motes
            if (!g_motesInit) {
                g_motesInit = 1;
                int m;
                const char mChars[6] = { '0', '1', 'x', 'F', 'A', '•' };
                for (m = 0; m < MAX_MOTES; m++) {
                    g_motes[m].x = k_rand() % W;
                    g_motes[m].y = k_rand() % 100;
                    g_motes[m].vx = (k_rand() % 3) - 1;
                    g_motes[m].vy = -1 - (k_rand() % 2);
                    g_motes[m].phase = k_rand() % 360;
                    g_motes[m].ch = mChars[k_rand() % 6];
                    g_motes[m].color = (m % 2 == 0) ? RGB(6, 182, 212) : RGB(59, 130, 246);
                }
            }

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            UpdateFields(hHex);
            break;
        }
        case WM_COMMAND: {
            WORD code = HIWORD(wParam);
            WORD id = LOWORD(wParam);

            if (code == EN_CHANGE) {
                HWND hSrc = (HWND)lParam;
                UpdateFields(hSrc);
            } else if (code == BN_CLICKED) {
                if (id == ID_BTN_ENDIAN) {
                    isLittleEndian = !isLittleEndian;
                    SetWindowTextA(hEndianBtn, isLittleEndian ? "Endian: LE [^E]" : "Endian: BE [^E]");
                    UpdateInspector(GetCurrentVal());
                    SpawnParticles(380, 115, RGB(6, 182, 212), 24);
                    SpawnShockwave(380, 115, RGB(6, 182, 212));
                } else if (id == ID_BTN_SWAP16) {
                    SetCurrentVal(swap16(GetCurrentVal()));
                    SpawnParticles(50, 315, RGB(139, 92, 246), 24);
                    SpawnShockwave(50, 315, RGB(139, 92, 246));
                } else if (id == ID_BTN_SWAP32) {
                    SetCurrentVal(swap32(GetCurrentVal()));
                    SpawnParticles(135, 315, RGB(139, 92, 246), 24);
                    SpawnShockwave(135, 315, RGB(139, 92, 246));
                } else if (id == ID_BTN_INVERT) {
                    SetCurrentVal(~GetCurrentVal());
                    SpawnParticles(215, 315, RGB(245, 158, 11), 24);
                    SpawnShockwave(215, 315, RGB(245, 158, 11));
                } else if (id == ID_BTN_XORMASK) {
                    SetCurrentVal(GetCurrentVal() ^ 0xFFFFFFFF);
                    SpawnParticles(300, 315, RGB(245, 158, 11), 24);
                    SpawnShockwave(300, 315, RGB(245, 158, 11));
                } else if (id == ID_BTN_CARRAY) {
                    ExportCArray(GetCurrentVal());
                    SpawnParticles(385, 315, RGB(16, 185, 129), 24);
                    SpawnShockwave(385, 315, RGB(16, 185, 129));
                } else if (id == ID_BTN_DUMP) {
                    ExportHexDump(GetCurrentVal());
                    SpawnParticles(470, 315, RGB(59, 130, 246), 24);
                    SpawnShockwave(470, 315, RGB(59, 130, 246));
                } else if (id == ID_BTN_DISSECT) {
                    ExportDissection(GetCurrentVal());
                    SpawnParticles(590, 315, RGB(6, 182, 212), 32);
                    SpawnShockwave(590, 315, RGB(6, 182, 212));
                } else if (id == 100) {
                    SpawnParticles(780, 20, RGB(6, 182, 212), 24);
                    ShowHelpDialog(hwnd);
                }
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1) {
                ShowHelpDialog(hwnd);
            }
            break;
        }
        case WM_CHAR: {
            if (wParam == 'h' || wParam == 'H') {
                ShowHelpDialog(hwnd);
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                // Update particles
                int p;
                for (p = 0; p < MAX_PARTICLES; p++) {
                    if (g_particles[p].active) {
                        g_particles[p].x += g_particles[p].vx;
                        g_particles[p].y += g_particles[p].vy;
                        if (g_particles[p].layer == 3) {
                            g_particles[p].vy += 2; // gravity
                        }
                        g_particles[p].life -= g_particles[p].decay;
                        if (g_particles[p].life <= 0) g_particles[p].active = 0;
                    }
                }
                // Update shockwaves
                int s;
                for (s = 0; s < MAX_SHOCKWAVES; s++) {
                    if (g_shockwaves[s].active) {
                        g_shockwaves[s].innerRadius += g_shockwaves[s].innerSpeed;
                        g_shockwaves[s].outerRadius += g_shockwaves[s].outerSpeed;
                        g_shockwaves[s].life -= g_shockwaves[s].decay;
                        if (g_shockwaves[s].life <= 0 || g_shockwaves[s].innerRadius >= g_shockwaves[s].maxRadius) {
                            g_shockwaves[s].active = 0;
                        }
                    }
                }
                // Screen shake decay
                if (g_shake > 0) g_shake--;

                // Trigger repaint of top banner / particle area
                RECT rect = {0, 0, W, 100};
                InvalidateRect(hwnd, &rect, FALSE);
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;
            if (hCtl == hExportEdit) {
                SetTextColor(hdc, RGB(226, 232, 240));
                SetBkColor(hdc, RGB(15, 23, 42));
            } else if (hCtl == hEntropy || hCtl == hSig || hCtl == hCRC32) {
                SetTextColor(hdc, RGB(6, 182, 212));
                SetBkColor(hdc, RGB(15, 23, 42));
            } else {
                SetTextColor(hdc, RGB(148, 163, 184));
                SetBkColor(hdc, RGB(15, 23, 42));
            }
            return (LRESULT)hBrushBg;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(248, 250, 252));
            SetBkColor(hdc, RGB(30, 41, 59));
            if (!hEditBrush) hEditBrush = CreateSolidBrush(RGB(30, 41, 59));
            return (LRESULT)hEditBrush;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double-buffer top banner area
            int bannerW = W;
            int bannerH = 100;
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, bannerW, bannerH);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Background Fill
            HBRUSH bgB = CreateSolidBrush(RGB(15, 23, 42));
            RECT rcBg = { 0, 0, bannerW, bannerH };
            FillRect(memDC, &rcBg, bgB);
            DeleteObject(bgB);

            // Cyber HUD Corner Filigree Brackets on banner frame
            DrawCyberHudBracket(memDC, 6, 4, 830, 92, RGB(6, 182, 212));

            // Animated Traveling Specular Glint along top border
            DWORD ticks = GetTickCount();
            int glintX = (int)((ticks / 10) % (bannerW + 200)) - 100;
            HPEN hGlintPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            HPEN oldGP = (HPEN)SelectObject(memDC, hGlintPen);
            MoveToEx(memDC, glintX - 30, 4, NULL);
            LineTo(memDC, glintX + 30, 4);
            SelectObject(memDC, oldGP);
            DeleteObject(hGlintPen);

            // Ambient Floating Cyber Dust / Matrix Motes
            int m;
            SetBkMode(memDC, TRANSPARENT);
            for (m = 0; m < MAX_MOTES; m++) {
                g_motes[m].y += g_motes[m].vy;
                g_motes[m].phase = (g_motes[m].phase + 3) % 360;
                if (g_motes[m].y < 5) {
                    g_motes[m].y = bannerH - 10;
                    g_motes[m].x = k_rand() % bannerW;
                }
                int curX = g_motes[m].x + (isin(g_motes[m].phase) * 6) / 1000;
                SetTextColor(memDC, g_motes[m].color);
                char str[2] = { g_motes[m].ch, 0 };
                TextOutA(memDC, curX, g_motes[m].y, str, 1);
            }

            // Draw Rotating / Pulsing 3D Hexagon Logo with Specular Sheen
            int phase = (ticks / 40) % 360;
            int offset = isin(phase) / 100; // -10 to +10

            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(6, 182, 212));
            HBRUSH hBrush = CreateSolidBrush(RGB(59, 130, 246));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hBrush);

            POINT hexPts[6] = {
                { 710, 15 - offset },
                { 728 + offset, 25 - offset / 2 },
                { 728 + offset, 45 + offset / 2 },
                { 710, 55 + offset },
                { 692 - offset, 45 + offset / 2 },
                { 692 - offset, 25 - offset / 2 }
            };
            Polygon(memDC, hexPts, 6);

            // Specular sheen sweep line on hexagon
            HPEN hSheen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            HPEN oldSheen = (HPEN)SelectObject(memDC, hSheen);
            MoveToEx(memDC, 698 - offset, 28, NULL);
            LineTo(memDC, 722 + offset, 38);
            SelectObject(memDC, oldSheen);
            DeleteObject(hSheen);

            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, 703, 27, "0x", 2);

            SelectObject(memDC, hOldPen);
            SelectObject(memDC, hOldBrush);
            DeleteObject(hPen);
            DeleteObject(hBrush);

            // Dual-Tier Concentric Shockwaves
            int s;
            for (s = 0; s < MAX_SHOCKWAVES; s++) {
                if (g_shockwaves[s].active && g_shockwaves[s].y < bannerH + 50) {
                    HPEN hSw1 = CreatePen(PS_SOLID, 2, g_shockwaves[s].color);
                    HPEN oldSw = (HPEN)SelectObject(memDC, hSw1);
                    HBRUSH oldBr = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    
                    // Outer halo
                    int or = g_shockwaves[s].outerRadius;
                    Ellipse(memDC, g_shockwaves[s].x - or, g_shockwaves[s].y - or, g_shockwaves[s].x + or, g_shockwaves[s].y + or);
                    
                    // Inner compression ring
                    HPEN hSw2 = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    SelectObject(memDC, hSw2);
                    int ir = g_shockwaves[s].innerRadius;
                    Ellipse(memDC, g_shockwaves[s].x - ir, g_shockwaves[s].y - ir, g_shockwaves[s].x + ir, g_shockwaves[s].y + ir);

                    SelectObject(memDC, oldSw);
                    SelectObject(memDC, oldBr);
                    DeleteObject(hSw1);
                    DeleteObject(hSw2);
                }
            }

            // Multi-Layer Active Particles
            int p;
            for (p = 0; p < MAX_PARTICLES; p++) {
                if (g_particles[p].active && g_particles[p].y / 10 < bannerH + 40) {
                    int px = g_particles[p].x / 10;
                    int py = g_particles[p].y / 10;
                    if (g_particles[p].layer == 1) {
                        // Needle Spark
                        HPEN hSp = CreatePen(PS_SOLID, 2, g_particles[p].color);
                        HPEN oldSp = (HPEN)SelectObject(memDC, hSp);
                        MoveToEx(memDC, px, py, NULL);
                        LineTo(memDC, px - g_particles[p].vx / 8, py - g_particles[p].vy / 8);
                        SelectObject(memDC, oldSp);
                        DeleteObject(hSp);
                    } else if (g_particles[p].layer == 2) {
                        // Buoyant Puff
                        HBRUSH hPf = CreateSolidBrush(g_particles[p].color);
                        HBRUSH oldPf = (HBRUSH)SelectObject(memDC, hPf);
                        int sz = g_particles[p].size / 2;
                        Ellipse(memDC, px - sz, py - sz, px + sz, py + sz);
                        SelectObject(memDC, oldPf);
                        DeleteObject(hPf);
                    } else if (g_particles[p].layer == 3) {
                        // Binary Shard Text
                        SetTextColor(memDC, g_particles[p].color);
                        TextOutA(memDC, px, py, g_particles[p].text, 2);
                    } else if (g_particles[p].layer == 4) {
                        // Celebration Star
                        DrawGdiStar(memDC, px, py, g_particles[p].size, g_particles[p].rot, g_particles[p].color);
                    }
                }
            }

            // Apply Screen Shake Viewport Offset to BitBlt
            int shakeX = (g_shake > 0) ? (k_rand() % (g_shake * 2 + 1)) - g_shake : 0;
            int shakeY = (g_shake > 0) ? (k_rand() % (g_shake * 2 + 1)) - g_shake : 0;

            BitBlt(hdc, shakeX, shakeY, bannerW, bannerH, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hFont) DeleteObject(hFont);
            if (hEditBrush) DeleteObject(hEditBrush);
            if (hBrushBg) DeleteObject(hBrushBg);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDpi = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpi) setDpi();
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KHexApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBrushBg = CreateSolidBrush(RGB(15, 23, 42));
    wc.hbrBackground = hBrushBg;
    RegisterClass(&wc);

    RECT rect = {0, 0, W, H};
    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    AdjustWindowRect(&rect, style, FALSE);
    HWND hwnd = CreateWindowEx(0, "KHexApp", "KHex Utility Suite [Press F1 for Help]", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (msg.wParam == 'E' || msg.wParam == 'e') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ENDIAN, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '1') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SWAP16, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '2') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SWAP32, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '3') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INVERT, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '4') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_XORMASK, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '5') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CARRAY, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '6') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DUMP, BN_CLICKED), 0);
                    continue;
                } else if (msg.wParam == '7') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DISSECT, BN_CLICKED), 0);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    ExitProcess(0);
}
