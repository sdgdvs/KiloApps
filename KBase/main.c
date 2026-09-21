#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")

int _fltused = 1;

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

HWND hInput, hOutput, hBitDisplay, hEditA, hEditB, hBtnEnc, hBtnDec, hBtnHash, hStatus;

// Custom String Helpers (No CRT dependencies)
static size_t my_strlen(const char* s) {
    size_t len = 0;
    while (s && s[len]) len++;
    return len;
}

static int parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static UINT64 parse_u64(const char* str, int base) {
    UINT64 res = 0;
    if (!str) return 0;
    
    // Skip 0x / 0b / 0o prefix if present
    if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    if (base == 2 && str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) str += 2;

    while (*str) {
        int digit = -1;
        char c = *str;
        if (base == 16) {
            digit = parse_hex_digit(c);
        } else if (base == 10) {
            if (c >= '0' && c <= '9') digit = c - '0';
        } else if (base == 2) {
            if (c == '0' || c == '1') digit = c - '0';
        } else if (base == 8) {
            if (c >= '0' && c <= '7') digit = c - '0';
        }
        if (digit < 0 || digit >= base) break;
        res = res * base + digit;
        str++;
    }
    return res;
}

static void u64_to_hex(UINT64 val, char* outBuf) {
    wsprintfA(outBuf, "%08X%08X", (DWORD)(val >> 32), (DWORD)(val & 0xFFFFFFFF));
}

static void u64_to_bin(UINT64 val, char* outBuf) {
    for (int i = 63; i >= 0; i--) {
        outBuf[63 - i] = ((val >> i) & 1) ? '1' : '0';
    }
    outBuf[64] = '\0';
}

static void u64_to_dec(UINT64 val, char* outBuf) {
    if (val == 0) {
        outBuf[0] = '0';
        outBuf[1] = '\0';
        return;
    }
    char temp[32];
    int pos = 0;
    while (val > 0) {
        temp[pos++] = '0' + (char)(val % 10);
        val /= 10;
    }
    int outPos = 0;
    for (int i = pos - 1; i >= 0; i--) {
        outBuf[outPos++] = temp[i];
    }
    outBuf[outPos] = '\0';
}

// Clipboard and Status Notification Utilities
static void CopyToClipboard(HWND hwnd, const char* text) {
    if (!text || !*text) return;
    int len = (int)my_strlen(text);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (!hMem) return;
    char* pMem = (char*)GlobalLock(hMem);
    if (pMem) {
        memcpy(pMem, text, len + 1);
        GlobalUnlock(hMem);
        if (OpenClipboard(hwnd)) {
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
            return;
        }
    }
    GlobalFree(hMem);
}

static void ShowNativeStatus(HWND hwnd, const char* msg) {
    if (hStatus && msg) {
        SetWindowTextA(hStatus, msg);
    }
    if (hwnd && msg) {
        char title[256];
        wsprintfA(title, "KBase Studio | %s", msg);
        SetWindowTextA(hwnd, title);
    }
}

static void DoCopyOutput(HWND hwnd) {
    DWORD len = GetWindowTextLengthA(hOutput);
    if (len == 0) {
        ShowNativeStatus(hwnd, "Output buffer is empty!");
        return;
    }
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    if (buf) {
        GetWindowTextA(hOutput, buf, len + 1);
        CopyToClipboard(hwnd, buf);
        HeapFree(GetProcessHeap(), 0, buf);
        ShowNativeStatus(hwnd, "Copied output result to Windows clipboard! [C]");
    }
}

// Core Utility Operations
void DoB64Encode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    DWORD outLen = 0;
    CryptBinaryToStringA((const BYTE*)buf, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &outLen);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, outLen + 1);
    CryptBinaryToStringA((const BYTE*)buf, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, outBuf, &outLen);
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoB64Decode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    DWORD outLen = 0;
    if (CryptStringToBinaryA(buf, len, CRYPT_STRING_BASE64, NULL, &outLen, NULL, NULL)) {
        char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, outLen + 1);
        if (CryptStringToBinaryA(buf, len, CRYPT_STRING_BASE64, (BYTE*)outBuf, &outLen, NULL, NULL)) {
            outBuf[outLen] = 0;
            SetWindowTextA(hOutput, outBuf);
        } else {
            SetWindowTextA(hOutput, "Error decoding Base64.");
        }
        HeapFree(GetProcessHeap(), 0, outBuf);
    } else {
        SetWindowTextA(hOutput, "Error decoding Base64.");
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

void DoSHA256() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            if (CryptHashData(hHash, (BYTE*)buf, len, 0)) {
                BYTE hash[32];
                DWORD hashLen = 32;
                if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
                    char hex[65];
                    for (int i = 0; i < 32; i++) {
                        wsprintfA(&hex[i*2], "%02x", hash[i]);
                    }
                    SetWindowTextA(hOutput, hex);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

void DoUrlEncode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len * 3 + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        unsigned char c = buf[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else {
            p += wsprintfA(p, "%%%02X", c);
        }
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoUrlDecode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        if (buf[i] == '%' && i + 2 < len) {
            char hex[3] = { buf[i+1], buf[i+2], 0 };
            *p++ = (char)parse_hex_digit(hex[0]) * 16 + parse_hex_digit(hex[1]);
            i += 2;
        } else if (buf[i] == '+') {
            *p++ = ' ';
        } else {
            *p++ = buf[i];
        }
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoHexEncode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len * 3 + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        if (i > 0) p += wsprintfA(p, " %02X", (unsigned char)buf[i]);
        else p += wsprintfA(p, "%02X", (unsigned char)buf[i]);
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoHexDecode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    DWORD outLen = 0;
    for (DWORD i = 0; i < len; i++) {
        char c = buf[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (i + 1 < len) {
            int d1 = parse_hex_digit(buf[i]);
            int d2 = parse_hex_digit(buf[i+1]);
            if (d1 >= 0 && d2 >= 0) {
                outBuf[outLen++] = (char)(d1 * 16 + d2);
                i++;
            }
        }
    }
    outBuf[outLen] = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

// Convert Base live
void DoConvertBases() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') {
        val = parse_u64(buf, 16);
    }
    
    char hex[32], dec[32], bin[68], formatted[256];
    u64_to_hex(val, hex);
    u64_to_dec(val, dec);
    u64_to_bin(val, bin);

    wsprintfA(formatted, "DEC: %s\r\nHEX: 0x%s\r\nBIN: %s", dec, hex, bin);
    SetWindowTextA(hOutput, formatted);
    SetWindowTextA(hBitDisplay, bin);
    ShowNativeStatus(GetParent(hOutput), "Base Conversion synchronized [Enter]");
}

static void DoLoadDemo(HWND hwnd) {
    SetWindowTextA(hInput, "0xDEADBEEF");
    SetWindowTextA(hEditA, "0x0F0F0F0F");
    SetWindowTextA(hEditB, "0x00FF00FF");
    DoConvertBases();
    ShowNativeStatus(hwnd, "Loaded Demo Presets: 0xDEADBEEF [D]");
}

// Multi-Width Representation Inspector
void DoMultiWidthInspect() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    INT8 s8 = (INT8)(val & 0xFF);
    UINT8 u8 = (UINT8)(val & 0xFF);
    UINT8 ones8 = (UINT8)(~u8);
    UINT8 sm8 = (UINT8)(((u8 & 0x80) ? 0x80 : 0) | (u8 & 0x7F));

    INT16 s16 = (INT16)(val & 0xFFFF);
    UINT16 u16 = (UINT16)(val & 0xFFFF);
    UINT16 ones16 = (UINT16)(~u16);
    UINT16 sm16 = (UINT16)(((u16 & 0x8000) ? 0x8000 : 0) | (u16 & 0x7FFF));

    INT32 s32 = (INT32)(val & 0xFFFFFFFF);
    UINT32 u32 = (UINT32)(val & 0xFFFFFFFF);
    UINT32 ones32 = (UINT32)(~u32);
    UINT32 sm32 = (UINT32)(((u32 & 0x80000000) ? 0x80000000 : 0) | (u32 & 0x7FFFFFFF));

    INT64 s64 = (INT64)val;
    UINT64 ones64 = ~val;

    char hex64[32], onesHex64[32], dec64[32], bin64[68], outStr[1024];
    u64_to_hex(val, hex64);
    u64_to_hex(ones64, onesHex64);
    u64_to_dec(val, dec64);
    u64_to_bin(val, bin64);

    wsprintfA(outStr,
        "=== Multi-Width Integer Representation ===\r\n"
        "[8-Bit]   Signed: %d | Unsigned: %u | Hex: 0x%02X | 1's Comp: 0x%02X | Sign-Mag: 0x%02X\r\n"
        "[16-Bit]  Signed: %d | Unsigned: %u | Hex: 0x%04X | 1's Comp: 0x%04X | Sign-Mag: 0x%04X\r\n"
        "[32-Bit]  Signed: %d | Unsigned: %u | Hex: 0x%08X | 1's Comp: 0x%08X | Sign-Mag: 0x%08X\r\n"
        "[64-Bit]  Hex: 0x%s | 1's Comp: 0x%s | Unsigned: %s\r\n"
        "64-Bit Binary: %s",
        (int)s8, (unsigned int)u8, (unsigned int)u8, (unsigned int)ones8, (unsigned int)sm8,
        (int)s16, (unsigned int)u16, (unsigned int)u16, (unsigned int)ones16, (unsigned int)sm16,
        (int)s32, (unsigned int)u32, (unsigned int)u32, (unsigned int)ones32, (unsigned int)sm32,
        hex64, onesHex64, dec64, bin64);

    SetWindowTextA(hOutput, outStr);
    SetWindowTextA(hBitDisplay, bin64);
}

// Variable-Length Integer (LEB128, Protobuf Varint & MIDI VLQ)
void DoVarintEncode() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    
    // Check if input is a hex byte sequence (contains spaces/dashes/commas)
    int hasSpace = 0;
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == ' ' || buf[i] == ',' || buf[i] == '-') { hasSpace = 1; break; }
    }

    if (hasSpace) {
        // Decode hex bytes
        UINT64 ulebDec = 0;
        int shift = 0;
        const char* p = buf;
        while (*p) {
            while (*p == ' ' || *p == ',' || *p == '-' || *p == '\t') p++;
            if (!*p) break;
            int d1 = parse_hex_digit(*p++);
            if (!*p) break;
            int d2 = parse_hex_digit(*p++);
            if (d1 >= 0 && d2 >= 0) {
                BYTE b = (BYTE)((d1 << 4) | d2);
                ulebDec |= ((UINT64)(b & 0x7F)) << shift;
                if (!(b & 0x80)) break;
                shift += 7;
            }
        }
        char decStr[32], hexStr[32];
        u64_to_dec(ulebDec, decStr);
        u64_to_hex(ulebDec, hexStr);

        char outStr[512];
        wsprintfA(outStr,
            "=== Decoded LEB128 / Varint Bytes ===\r\n"
            "Raw Byte Stream: %s\r\n"
            "Decoded Unsigned: %s\r\n"
            "Decoded Hex: 0x%s",
            buf, decStr, hexStr);
        SetWindowTextA(hOutput, outStr);
        return;
    }

    // Otherwise Encode integer to ULEB128, SLEB128, ZigZag & MIDI VLQ
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    // ULEB128 Encode
    BYTE ulebBytes[16];
    int ulebCount = 0;
    UINT64 tempU = val;
    do {
        BYTE b = (BYTE)(tempU & 0x7F);
        tempU >>= 7;
        if (tempU != 0) b |= 0x80;
        ulebBytes[ulebCount++] = b;
    } while (tempU != 0);

    // SLEB128 Encode
    BYTE slebBytes[16];
    int slebCount = 0;
    INT64 tempS = (INT64)val;
    int more = 1;
    while (more) {
        BYTE b = (BYTE)(tempS & 0x7F);
        tempS >>= 7;
        int signBit = (b & 0x40) != 0;
        if ((tempS == 0 && !signBit) || (tempS == -1 && signBit)) {
            more = 0;
        } else {
            b |= 0x80;
        }
        slebBytes[slebCount++] = b;
    }

    // ZigZag 64
    INT64 sVal = (INT64)val;
    UINT64 zzVal = (UINT64)((sVal << 1) ^ (sVal >> 63));
    BYTE zzBytes[16];
    int zzCount = 0;
    UINT64 tempZZ = zzVal;
    do {
        BYTE b = (BYTE)(tempZZ & 0x7F);
        tempZZ >>= 7;
        if (tempZZ != 0) b |= 0x80;
        zzBytes[zzCount++] = b;
    } while (tempZZ != 0);

    // MIDI VLQ Encode (Big-endian 7-bit continuation stream)
    BYTE vlqBytes[16];
    int vlqCount = 0;
    UINT64 tempV = val;
    vlqBytes[vlqCount++] = (BYTE)(tempV & 0x7F);
    tempV >>= 7;
    while (tempV > 0 && vlqCount < 16) {
        vlqBytes[vlqCount++] = (BYTE)((tempV & 0x7F) | 0x80);
        tempV >>= 7;
    }
    // Reverse because MIDI VLQ is big-endian
    for (int i = 0; i < vlqCount / 2; i++) {
        BYTE t = vlqBytes[i];
        vlqBytes[i] = vlqBytes[vlqCount - 1 - i];
        vlqBytes[vlqCount - 1 - i] = t;
    }

    char ulebHex[64] = {0}, slebHex[64] = {0}, zzHex[64] = {0}, vlqHex[64] = {0};
    int pos = 0;
    for (int i = 0; i < ulebCount; i++) pos += wsprintfA(&ulebHex[pos], "%02X ", ulebBytes[i]);
    pos = 0;
    for (int i = 0; i < slebCount; i++) pos += wsprintfA(&slebHex[pos], "%02X ", slebBytes[i]);
    pos = 0;
    for (int i = 0; i < zzCount; i++) pos += wsprintfA(&zzHex[pos], "%02X ", zzBytes[i]);
    pos = 0;
    for (int i = 0; i < vlqCount; i++) pos += wsprintfA(&vlqHex[pos], "%02X ", vlqBytes[i]);

    char outStr[1280];
    wsprintfA(outStr,
        "=== Variable-Length Integer (LEB128, Protobuf & MIDI VLQ) ===\r\n"
        "Input Value: %s\r\n\r\n"
        "ULEB128 (Unsigned) Hex Bytes: %s (%d bytes)\r\n"
        "SLEB128 (Signed) Hex Bytes:   %s (%d bytes)\r\n"
        "Protobuf ZigZag Encoded Hex:  %s (%d bytes)\r\n"
        "MIDI VLQ (Big-Endian) Hex:    %s (%d bytes)\r\n\r\n"
        "[ULEB128 First Byte Breakdown]\r\n"
        "MSB (Continuation): %d | 7-bit Payload: 0x%02X (%d)",
        buf, ulebHex, ulebCount, slebHex, slebCount, zzHex, zzCount, vlqHex, vlqCount,
        (ulebBytes[0] & 0x80) ? 1 : 0, (int)(ulebBytes[0] & 0x7F), (int)(ulebBytes[0] & 0x7F));

    SetWindowTextA(hOutput, outStr);
}

// Bitfield Slicer & Bit Metrics
void DoBitfieldSlice() {
    char bufA[64], bufB[64];
    GetWindowTextA(hEditA, bufA, 64);
    GetWindowTextA(hEditB, bufB, 64);

    UINT64 valA = parse_u64(bufA, 16);
    if (valA == 0 && bufA[0] != '0') valA = parse_u64(bufA, 10);

    int high = 15, low = 8;
    // Check if Operand B is "High:Low" format like "15:8"
    char* colon = NULL;
    for (int i = 0; bufB[i]; i++) {
        if (bufB[i] == ':') { colon = &bufB[i]; break; }
    }
    if (colon) {
        high = (int)parse_u64(bufB, 10);
        low = (int)parse_u64(colon + 1, 10);
    } else {
        UINT64 b = parse_u64(bufB, 10);
        if (b > 0 && b < 64) {
            low = 0;
            high = (int)b - 1;
        }
    }

    if (high < 0) high = 0;
    if (high > 63) high = 63;
    if (low < 0) low = 0;
    if (low > 63) low = 63;
    if (high < low) { int t = high; high = low; low = t; }

    int bitCount = high - low + 1;
    UINT64 sliceMask = (bitCount >= 64) ? 0xFFFFFFFFFFFFFFFFULL : ((1ULL << bitCount) - 1ULL);
    UINT64 extractedU = (valA >> low) & sliceMask;
    INT64 extractedS = (INT64)extractedU;
    if (bitCount < 64 && (extractedU & (1ULL << (bitCount - 1)))) {
        extractedS |= ~sliceMask;
    }
    UINT64 fullMask = sliceMask << low;

    // Bit Metrics (Popcount, CLZ, CTZ, Parity)
    int popcount = 0;
    for (int i = 0; i < 64; i++) {
        if ((valA >> i) & 1) popcount++;
    }

    int clz = 0;
    for (int i = 63; i >= 0; i--) {
        if ((valA >> i) & 1) break;
        clz++;
    }

    int ctz = 0;
    for (int i = 0; i < 64; i++) {
        if ((valA >> i) & 1) break;
        ctz++;
    }

    char maskHex[32], extHex[32], valHex[32], outStr[1024];
    u64_to_hex(fullMask, maskHex);
    u64_to_hex(extractedU, extHex);
    u64_to_hex(valA, valHex);

    wsprintfA(outStr,
        "=== Bitfield Slicer & Bit Metrics ===\r\n"
        "Source Operand A: 0x%s\r\n"
        "Bit Slice Range:  [%d : %d] (%d bits)\r\n"
        "Bitmask:          0x%s\r\n\r\n"
        "Extracted Unsigned: %d (0x%s)\r\n"
        "Extracted Signed:   %d\r\n\r\n"
        "[64-Bit Diagnostics]\r\n"
        "Population Count (Hamming Weight): %d / 64\r\n"
        "Count Leading Zeros (CLZ):         %d\r\n"
        "Count Trailing Zeros (CTZ):        %d\r\n"
        "Parity:                            %s\r\n"
        "Is Power of 2:                     %s",
        valHex, high, low, bitCount, maskHex,
        (int)extractedU, extHex, (int)extractedS,
        popcount, clz, ctz,
        (popcount % 2 == 0) ? "Even (0)" : "Odd (1)",
        (valA > 0 && (valA & (valA - 1)) == 0) ? "YES" : "NO");

    SetWindowTextA(hOutput, outStr);
}

// Reflected Gray Code & Packed BCD Conversion
void DoBCDAndGray() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    // Binary to Gray
    UINT64 gray = val ^ (val >> 1);
    char grayHex[32], grayBin[68];
    u64_to_hex(gray, grayHex);
    u64_to_bin(gray, grayBin);

    // Gray to Binary (decoding input assuming input was Gray)
    UINT64 decodedBin = val;
    for (UINT64 mask = val >> 1; mask != 0; mask >>= 1) {
        decodedBin ^= mask;
    }
    char decBinHex[32], decBinDec[32];
    u64_to_hex(decodedBin, decBinHex);
    u64_to_dec(decodedBin, decBinDec);

    // Packed BCD encode (each decimal digit to 4 bits)
    UINT64 bcd = 0;
    UINT64 temp = val;
    int bcdShift = 0;
    int bcdOverflow = 0;
    while (temp > 0) {
        if (bcdShift >= 64) { bcdOverflow = 1; break; }
        UINT64 d = temp % 10;
        bcd |= (d << bcdShift);
        bcdShift += 4;
        temp /= 10;
    }
    char bcdHex[32];
    u64_to_hex(bcd, bcdHex);

    // Decode input assuming input was Packed BCD
    UINT64 bcdDec = 0;
    UINT64 mult = 1;
    UINT64 tempBcd = val;
    int bcdValid = 1;
    while (tempBcd > 0) {
        UINT64 d = tempBcd & 0xF;
        if (d > 9) { bcdValid = 0; break; }
        bcdDec += d * mult;
        mult *= 10;
        tempBcd >>= 4;
    }
    char bcdDecStr[32];
    u64_to_dec(bcdDec, bcdDecStr);

    char outStr[1024];
    wsprintfA(outStr,
        "=== Reflected Gray Code & Packed BCD ===\r\n"
        "Source Value: 0x%08X%08X (%d decimal)\r\n\r\n"
        "[Reflected Gray Code]\r\n"
        "Binary -> Gray Code:    0x%s\r\n"
        "Gray Code Binary:       %s\r\n"
        "Gray -> Binary Decoded: 0x%s (%s dec)\r\n\r\n"
        "[Packed Binary-Coded Decimal (BCD)]\r\n"
        "Packed BCD Encode:      0x%s %s\r\n"
        "BCD Decode (raw nibbles): %s\r\n\r\n"
        "[Project Echo Historical Cipher Anchor]\r\n"
        "Key: 0x10199904 -> Octets: 16.25.153.4 | Date: Oct 19, 1994",
        (DWORD)(val >> 32), (DWORD)(val & 0xFFFFFFFF), (DWORD)(val & 0xFFFFFFFF),
        grayHex, grayBin, decBinHex, decBinDec,
        bcdHex, bcdOverflow ? "(truncated >16 digits)" : "",
        bcdValid ? bcdDecStr : "Invalid BCD digits (>9 in nibble)");

    SetWindowTextA(hOutput, outStr);
}

// Floating-Point & Fixed-Point Inspector
void DoFloatInspect() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    DWORD u32 = (DWORD)(val & 0xFFFFFFFF);
    DWORD s32 = (u32 >> 31) & 1;
    DWORD exp32 = (u32 >> 23) & 0xFF;
    DWORD mant32 = u32 & 0x7FFFFF;
    int expUnbiased = (int)exp32 - 127;
    const char* class32 = "Normalized";
    if (exp32 == 255) {
        class32 = (mant32 == 0) ? (s32 ? "-Infinity" : "+Infinity") : "NaN (Not-a-Number)";
    } else if (exp32 == 0) {
        class32 = (mant32 == 0) ? (s32 ? "-Zero" : "+Zero") : "Subnormal (Denormalized)";
    }

    WORD u16 = (WORD)(val & 0xFFFF);
    WORD s16 = (u16 >> 15) & 1;
    WORD exp16 = (u16 >> 10) & 0x1F;
    WORD mant16 = u16 & 0x3FF;
    int exp16Unbiased = (int)exp16 - 15;

    WORD bf16 = (WORD)(val & 0xFFFF);
    WORD sBf = (bf16 >> 15) & 1;
    WORD expBf = (bf16 >> 7) & 0xFF;
    WORD mantBf = bf16 & 0x7F;
    int expBfUnbiased = (int)expBf - 127;

    // Fixed-Point Q-formats
    INT16 q88 = (INT16)(val & 0xFFFF);
    int q88Int = q88 >> 8;
    int q88Frac = (int)(((q88 & 0xFF) * 1000) / 256);

    INT32 q1616 = (INT32)(val & 0xFFFFFFFF);
    int q1616Int = q1616 >> 16;
    int q1616Frac = (int)(((q1616 & 0xFFFF) * 1000) / 65536);

    INT16 q015 = (INT16)(val & 0xFFFF);
    int q015Frac = (int)(((INT32)q015 * 10000) / 32768);

    char outStr[1536];
    wsprintfA(outStr,
        "=== Floating-Point & Fixed-Point Inspector ===\r\n"
        "Raw 32-Bit Hex: 0x%08X | Raw 16-Bit Hex: 0x%04X\r\n\r\n"
        "[IEEE-754 32-Bit Single Precision (Binary32)]\r\n"
        "Classification: %s\r\n"
        "Sign:           %d (%s)\r\n"
        "Biased Exp:     %u (0x%02X) | Unbiased: %d\r\n"
        "Mantissa:       0x%06X (23-bit fraction)\r\n\r\n"
        "[IEEE-754 16-Bit Half Precision (Binary16)]\r\n"
        "Sign: %d | Biased Exp: %u (Unbiased: %d) | Mantissa: 0x%03X (10-bit)\r\n\r\n"
        "[Brain Floating Point (Bfloat16)]\r\n"
        "Sign: %d | Biased Exp: %u (Unbiased: %d) | Mantissa: 0x%02X (7-bit)\r\n\r\n"
        "[Fixed-Point DSP Formats]\r\n"
        "Q8.8   (Signed 8.8):   ~%d.%03d\r\n"
        "Q16.16 (Signed 16.16): ~%d.%03d\r\n"
        "Q0.15  (DSP Audio):    ~%s0.%04d",
        u32, (DWORD)u16,
        class32, s32, s32 ? "Negative" : "Positive",
        exp32, exp32, expUnbiased, mant32,
        (DWORD)s16, (DWORD)exp16, exp16Unbiased, (DWORD)mant16,
        (DWORD)sBf, (DWORD)expBf, expBfUnbiased, (DWORD)mantBf,
        q88Int, q88Frac < 0 ? -q88Frac : q88Frac,
        q1616Int, q1616Frac < 0 ? -q1616Frac : q1616Frac,
        q015 < 0 ? "-" : "+", q015Frac < 0 ? -q015Frac : q015Frac);

    SetWindowTextA(hOutput, outStr);
}

// Endian Byte Swap & 64-bit Nybble Swap
void DoEndianSwap() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    UINT16 u16 = (UINT16)(val & 0xFFFF);
    UINT16 swap16 = ((u16 & 0x00FF) << 8) | ((u16 & 0xFF00) >> 8);

    UINT32 u32 = (UINT32)(val & 0xFFFFFFFF);
    UINT32 swap32 = ((u32 & 0x000000FF) << 24) |
                    ((u32 & 0x0000FF00) << 8)  |
                    ((u32 & 0x00FF0000) >> 8)  |
                    ((u32 & 0xFF000000) >> 24);

    UINT64 swap64 = ((val & 0x00000000000000FFULL) << 56) |
                    ((val & 0x000000000000FF00ULL) << 40) |
                    ((val & 0x0000000000FF0000ULL) << 24) |
                    ((val & 0x00000000FF000000ULL) << 8)  |
                    ((val & 0x000000FF00000000ULL) >> 8)  |
                    ((val & 0x0000FF0000000000ULL) >> 24) |
                    ((val & 0x00FF000000000000ULL) >> 40) |
                    ((val & 0xFF00000000000000ULL) >> 56);

    UINT64 nybbleSwap = ((val & 0x0F0F0F0F0F0F0F0FULL) << 4) |
                        ((val & 0xF0F0F0F0F0F0F0F0ULL) >> 4);

    char valHex[32], swap64Hex[32], nybbleHex[32];
    u64_to_hex(val, valHex);
    u64_to_hex(swap64, swap64Hex);
    u64_to_hex(nybbleSwap, nybbleHex);

    char outStr[1024];
    wsprintfA(outStr,
        "=== Endian Byte Swap & Nybble Reversal ===\r\n"
        "Original 64-Bit Hex: 0x%s\r\n\r\n"
        "[16-Bit Byte Swap]\r\n"
        "Original: 0x%04X -> Swapped: 0x%04X (Bytes: %02X %02X -> %02X %02X)\r\n\r\n"
        "[32-Bit Byte Swap]\r\n"
        "Original: 0x%08X -> Swapped: 0x%08X\r\n\r\n"
        "[64-Bit Byte Swap]\r\n"
        "Little-Endian <-> Big-Endian: 0x%s\r\n\r\n"
        "[64-Bit Nybble Swap (Adjacent 4-bit nibbles)]\r\n"
        "Swapped Nybbles: 0x%s",
        valHex,
        (DWORD)u16, (DWORD)swap16, (DWORD)(u16 >> 8), (DWORD)(u16 & 0xFF), (DWORD)(swap16 >> 8), (DWORD)(swap16 & 0xFF),
        u32, swap32,
        swap64Hex,
        nybbleHex);

    SetWindowTextA(hOutput, outStr);
}

// Multi-Language Constant Code Snippet Generator
void DoGenCode() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') val = parse_u64(buf, 16);

    char hexStr[32], decStr[32];
    u64_to_hex(val, hexStr);
    u64_to_dec(val, decStr);

    char outStr[1024];
    wsprintfA(outStr,
        "=== Multi-Language Constant Code Snippets ===\r\n\r\n"
        "// C / C++\r\n"
        "static const uint64_t VALUE = 0x%sULL; /* %s */\r\n\r\n"
        "// Rust\r\n"
        "const VALUE: u64 = 0x%s; // %s\r\n\r\n"
        "// Python\r\n"
        "VALUE = 0x%s  # %s\r\n\r\n"
        "// NASM x86_64 Assembly\r\n"
        "mov rax, 0x%s ; %s",
        hexStr, decStr,
        hexStr, decStr,
        hexStr, decStr,
        hexStr, decStr);

    SetWindowTextA(hOutput, outStr);
}

// Bitwise operations (AND, OR, XOR, NOT, SHL, SHR, ROL, ROR, NAND, NOR, XNOR, ANDN, CLMUL, LSB-Iso)
void DoBitwiseOp(int op) {
    char bufA[64], bufB[64];
    GetWindowTextA(hEditA, bufA, 64);
    GetWindowTextA(hEditB, bufB, 64);

    UINT64 valA = parse_u64(bufA, 16);
    if (valA == 0 && bufA[0] != '0') valA = parse_u64(bufA, 10);

    UINT64 valB = parse_u64(bufB, 16);
    if (valB == 0 && bufB[0] != '0') valB = parse_u64(bufB, 10);

    UINT64 res = 0;
    const char* opName = "";

    switch (op) {
        case 10: res = valA & valB; opName = "AND"; break;
        case 11: res = valA | valB; opName = "OR"; break;
        case 12: res = valA ^ valB; opName = "XOR"; break;
        case 13: res = ~valA; opName = "NOT(A)"; break;
        case 14: res = valA << (valB & 63); opName = "SHL"; break;
        case 15: res = valA >> (valB & 63); opName = "SHR"; break;
        case 16: {
            UINT64 cnt = valB & 63;
            res = (valA << cnt) | (valA >> (64 - cnt));
            opName = "ROL";
            break;
        }
        case 17: {
            UINT64 cnt = valB & 63;
            res = (valA >> cnt) | (valA << (64 - cnt));
            opName = "ROR";
            break;
        }
        case 18: res = ~(valA & valB); opName = "NAND"; break;
        case 19: res = ~(valA | valB); opName = "NOR"; break;
        case 30: res = ~(valA ^ valB); opName = "XNOR"; break;
        case 31: res = valA & (~valB); opName = "AND-NOT (ANDN)"; break;
        case 32: {
            res = 0;
            for (int i = 0; i < 32; i++) {
                if ((valB >> i) & 1) res ^= (valA << i);
            }
            opName = "Carry-less Mul (CLMUL)";
            break;
        }
        case 33: res = valA & (0ULL - valA); opName = "LSB-Isolate (A & -A)"; break;
    }

    char hex[32], dec[32], bin[68], outStr[256];
    u64_to_hex(res, hex);
    u64_to_dec(res, dec);
    u64_to_bin(res, bin);

    wsprintfA(outStr, "[Bitwise %s Result]\r\nHEX: 0x%s\r\nDEC: %s\r\nBIN: %s", opName, hex, dec, bin);
    SetWindowTextA(hOutput, outStr);
    SetWindowTextA(hBitDisplay, bin);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdcScreen = GetDC(NULL);
            int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSY);
            ReleaseDC(NULL, hdcScreen);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5 /* CLEARTYPE_QUALITY */, DEFAULT_PITCH, "Consolas");
            
            CreateWindowA("STATIC", "Input Buffer / Number:", WS_CHILD | WS_VISIBLE, 10, 8, 200, 18, hwnd, NULL, NULL, NULL);
            HWND hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 780, 6, 105, 24, hwnd, (HMENU)99, NULL, NULL);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFont, 0);

            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "42", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
                10, 30, 875, 44, hwnd, NULL, NULL, NULL);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, 0);

            // Operands A & B for bitwise calculations, plus 1-click Demo, Copy, Clear, Code buttons
            CreateWindowA("STATIC", "Operand A (Hex/Dec):", WS_CHILD | WS_VISIBLE, 10, 80, 135, 18, hwnd, NULL, NULL, NULL);
            hEditA = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0x0F0F0F0F", WS_CHILD | WS_VISIBLE, 145, 78, 105, 22, hwnd, NULL, NULL, NULL);
            SendMessageA(hEditA, WM_SETFONT, (WPARAM)hFont, 0);

            CreateWindowA("STATIC", "Operand B / Slice:", WS_CHILD | WS_VISIBLE, 260, 80, 120, 18, hwnd, NULL, NULL, NULL);
            hEditB = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "15:8", WS_CHILD | WS_VISIBLE, 385, 78, 85, 22, hwnd, NULL, NULL, NULL);
            SendMessageA(hEditB, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnCopyOut = CreateWindowA("BUTTON", "Copy Out [C]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 480, 77, 95, 24, hwnd, (HMENU)201, NULL, NULL);
            SendMessageA(hBtnCopyOut, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnDemo = CreateWindowA("BUTTON", "✨ Demo [D]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 77, 95, 24, hwnd, (HMENU)202, NULL, NULL);
            SendMessageA(hBtnDemo, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnClear = CreateWindowA("BUTTON", "Clear [X]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 690, 77, 85, 24, hwnd, (HMENU)203, NULL, NULL);
            SendMessageA(hBtnClear, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnCode = CreateWindowA("BUTTON", "Code [K]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 785, 77, 100, 24, hwnd, (HMENU)26, NULL, NULL);
            SendMessageA(hBtnCode, WM_SETFONT, (WPARAM)hFont, 0);

            // Action Buttons - Base, Encodings, Ints, Slicer, BCD, Float
            HWND hBtnConv = CreateWindowA("BUTTON", "Convert [Enter]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 107, 110, 24, hwnd, (HMENU)100, NULL, NULL);
            SendMessageA(hBtnConv, WM_SETFONT, (WPARAM)hFont, 0);

            hBtnEnc = CreateWindowA("BUTTON", "B64 Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 125, 107, 65, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessageA(hBtnEnc, WM_SETFONT, (WPARAM)hFont, 0);
            
            hBtnDec = CreateWindowA("BUTTON", "B64 Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 195, 107, 65, 24, hwnd, (HMENU)2, NULL, NULL);
            SendMessageA(hBtnDec, WM_SETFONT, (WPARAM)hFont, 0);
            
            HWND hBtnUrlEnc = CreateWindowA("BUTTON", "URL Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 265, 107, 65, 24, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hBtnUrlEnc, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnUrlDec = CreateWindowA("BUTTON", "URL Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 335, 107, 65, 24, hwnd, (HMENU)5, NULL, NULL);
            SendMessageA(hBtnUrlDec, WM_SETFONT, (WPARAM)hFont, 0);

            hBtnHash = CreateWindowA("BUTTON", "SHA-256", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 405, 107, 75, 24, hwnd, (HMENU)3, NULL, NULL);
            SendMessageA(hBtnHash, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnVarint = CreateWindowA("BUTTON", "Varint [V]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 485, 107, 75, 24, hwnd, (HMENU)20, NULL, NULL);
            SendMessageA(hBtnVarint, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnInts = CreateWindowA("BUTTON", "Ints [I]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 565, 107, 70, 24, hwnd, (HMENU)21, NULL, NULL);
            SendMessageA(hBtnInts, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnSlice = CreateWindowA("BUTTON", "Slice [S]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 640, 107, 75, 24, hwnd, (HMENU)22, NULL, NULL);
            SendMessageA(hBtnSlice, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnBCD = CreateWindowA("BUTTON", "BCD/Gray [G]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 720, 107, 90, 24, hwnd, (HMENU)23, NULL, NULL);
            SendMessageA(hBtnBCD, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnFloat = CreateWindowA("BUTTON", "Float [F]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 815, 107, 70, 24, hwnd, (HMENU)24, NULL, NULL);
            SendMessageA(hBtnFloat, WM_SETFONT, (WPARAM)hFont, 0);

            // Extended Bitwise Operator Buttons
            HWND hBtnAnd = CreateWindowA("BUTTON", "AND", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 137, 48, 24, hwnd, (HMENU)10, NULL, NULL);
            SendMessageA(hBtnAnd, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnOr  = CreateWindowA("BUTTON", "OR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 62, 137, 44, 24, hwnd, (HMENU)11, NULL, NULL);
            SendMessageA(hBtnOr, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnXor = CreateWindowA("BUTTON", "XOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 110, 137, 48, 24, hwnd, (HMENU)12, NULL, NULL);
            SendMessageA(hBtnXor, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnNot = CreateWindowA("BUTTON", "NOT(A)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 162, 137, 52, 24, hwnd, (HMENU)13, NULL, NULL);
            SendMessageA(hBtnNot, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnShl = CreateWindowA("BUTTON", "SHL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 218, 137, 46, 24, hwnd, (HMENU)14, NULL, NULL);
            SendMessageA(hBtnShl, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnShr = CreateWindowA("BUTTON", "SHR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 268, 137, 46, 24, hwnd, (HMENU)15, NULL, NULL);
            SendMessageA(hBtnShr, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnRol = CreateWindowA("BUTTON", "ROL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 318, 137, 46, 24, hwnd, (HMENU)16, NULL, NULL);
            SendMessageA(hBtnRol, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnRor = CreateWindowA("BUTTON", "ROR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 368, 137, 46, 24, hwnd, (HMENU)17, NULL, NULL);
            SendMessageA(hBtnRor, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnNand = CreateWindowA("BUTTON", "NAND", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 418, 137, 50, 24, hwnd, (HMENU)18, NULL, NULL);
            SendMessageA(hBtnNand, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnNor = CreateWindowA("BUTTON", "NOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 472, 137, 46, 24, hwnd, (HMENU)19, NULL, NULL);
            SendMessageA(hBtnNor, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnXnor = CreateWindowA("BUTTON", "XNOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 522, 137, 52, 24, hwnd, (HMENU)30, NULL, NULL);
            SendMessageA(hBtnXnor, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnAndn = CreateWindowA("BUTTON", "ANDN", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 578, 137, 52, 24, hwnd, (HMENU)31, NULL, NULL);
            SendMessageA(hBtnAndn, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnClmul = CreateWindowA("BUTTON", "CLMUL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 634, 137, 58, 24, hwnd, (HMENU)32, NULL, NULL);
            SendMessageA(hBtnClmul, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnLsbIso = CreateWindowA("BUTTON", "LSB-Iso", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 696, 137, 66, 24, hwnd, (HMENU)33, NULL, NULL);
            SendMessageA(hBtnLsbIso, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnEndian = CreateWindowA("BUTTON", "Endian [E]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 766, 137, 119, 24, hwnd, (HMENU)25, NULL, NULL);
            SendMessageA(hBtnEndian, WM_SETFONT, (WPARAM)hFont, 0);

            // 64-Bit Binary Stream Display
            CreateWindowA("STATIC", "64-Bit Binary Stream:", WS_CHILD | WS_VISIBLE, 10, 167, 200, 18, hwnd, NULL, NULL, NULL);
            hBitDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0000000000000000000000000000000000000000000000000000000000000000",
                WS_CHILD | WS_VISIBLE | ES_READONLY, 10, 185, 875, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hBitDisplay, WM_SETFONT, (WPARAM)hFont, 0);

            // Output Display Area
            CreateWindowA("STATIC", "Output Result:", WS_CHILD | WS_VISIBLE, 10, 215, 200, 18, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                10, 233, 875, 305, hwnd, NULL, NULL, NULL);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFont, 0);

            // Non-blocking Bottom Status Bar
            hStatus = CreateWindowExA(0, "STATIC", " Ready | F1: Help | C: Copy Output | D: Demo | Enter: Convert | G: Gray/BCD | F: Float | E: Endian",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 544, 875, 20, hwnd, (HMENU)300, NULL, NULL);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFont, 0);

            // Trigger default conversion
            DoConvertBases();
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1) { DoB64Encode(); ShowNativeStatus(hwnd, "Base64 encoded output"); }
            else if (id == 2) { DoB64Decode(); ShowNativeStatus(hwnd, "Base64 decoded output"); }
            else if (id == 3) { DoSHA256(); ShowNativeStatus(hwnd, "SHA-256 hash computed"); }
            else if (id == 4) { DoUrlEncode(); ShowNativeStatus(hwnd, "URL encoded output"); }
            else if (id == 5) { DoUrlDecode(); ShowNativeStatus(hwnd, "URL decoded output"); }
            else if (id == 6) { DoHexEncode(); ShowNativeStatus(hwnd, "Hex encoded output"); }
            else if (id == 7) { DoHexDecode(); ShowNativeStatus(hwnd, "Hex decoded output"); }
            else if (id == 20) { DoVarintEncode(); ShowNativeStatus(hwnd, "Varint / LEB128 & MIDI VLQ computed [V]"); }
            else if (id == 21) { DoMultiWidthInspect(); ShowNativeStatus(hwnd, "Multi-width integers inspected [I]"); }
            else if (id == 22) { DoBitfieldSlice(); ShowNativeStatus(hwnd, "Bitfield slice extracted [S]"); }
            else if (id == 23) { DoBCDAndGray(); ShowNativeStatus(hwnd, "Gray Code & Packed BCD converted [G]"); }
            else if (id == 24) { DoFloatInspect(); ShowNativeStatus(hwnd, "Floating & Fixed-Point inspected [F]"); }
            else if (id == 25) { DoEndianSwap(); ShowNativeStatus(hwnd, "Endian & Nybble Swapped [E]"); }
            else if (id == 26) { DoGenCode(); ShowNativeStatus(hwnd, "Code snippets generated [K]"); }
            else if (id == 100) DoConvertBases();
            else if (id == 201) DoCopyOutput(hwnd);
            else if (id == 202) DoLoadDemo(hwnd);
            else if (id == 203) {
                SetWindowTextA(hInput, "");
                SetWindowTextA(hOutput, "");
                SetWindowTextA(hBitDisplay, "0000000000000000000000000000000000000000000000000000000000000000");
                ShowNativeStatus(hwnd, "Cleared buffer and output [X]");
            }
            else if ((id >= 10 && id <= 19) || (id >= 30 && id <= 33)) {
                DoBitwiseOp(id);
                ShowNativeStatus(hwnd, "Bitwise operation calculated!");
            }
            else if (id == 99) {
                MessageBoxA(hwnd,
                    "=== KBase Studio User Guide ===\n\n"
                    "KEYBOARD SHORTCUTS:\n"
                    "- F1 or H: Open this Help & Feature Guide\n"
                    "- Enter: Live base conversion / calculate\n"
                    "- C: Copy output buffer to Windows clipboard\n"
                    "- D: Load 1-click sample demo presets (0xDEADBEEF)\n"
                    "- X: Clear inputs and output buffer\n"
                    "- V: Varint / LEB128 & MIDI VLQ encoding\n"
                    "- I: Multi-width integer representation inspector\n"
                    "- S: Bitfield slice & bit diagnostics\n"
                    "- G: Reflected Gray Code & Packed BCD\n"
                    "- F: IEEE-754 Float32/FP16/Bfloat16 & Q-Formats\n"
                    "- E: Endian Byte Swap & 64-bit Nybble Swap\n"
                    "- K: Multi-language constant code snippets\n\n"
                    "FEATURES & OPERATIONS:\n"
                    "1. Convert Base: Live conversion across Dec, Hex, and 64-bit Binary stream.\n"
                    "2. String Suite: Base64, URL, Hex encode/decode, and SHA-256 hash.\n"
                    "3. Varint & Streams: ULEB128, SLEB128, ZigZag, and MIDI VLQ.\n"
                    "4. Int Formats: int8/16/32/64 two's comp, unsigned, 1's comp, sign-mag.\n"
                    "5. Bitfield Slice: Extract [High:Low] slices; Popcount, CLZ, CTZ, Parity.\n"
                    "6. Extended Bitwise: AND, OR, XOR, NOT, SHL, SHR, ROL, ROR, NAND, NOR, XNOR, ANDN, CLMUL, LSB-Iso.\n"
                    "7. Vintage & Specialized: Reflected Gray code, Packed BCD, Project Echo anchor (0x10199904).\n"
                    "8. Floats & Fixed-Point: Binary32, Binary16, Bfloat16, and DSP Q-formats (Q8.8, Q16.16, Q0.15).",
                    "KBase Studio - User Guide", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

typedef BOOL(WINAPI* SETPROCESSDPIAWARE_T)(void);

void __stdcall MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        SETPROCESSDPIAWARE_T pSetProcessDPIAware = (SETPROCESSDPIAWARE_T)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) pSetProcessDPIAware();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KBaseApp";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassA(&wc);
    
    DWORD style = (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rc = {0, 0, 915, 610};
    AdjustWindowRect(&rc, style, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KBaseApp", "KBase - Universal Base & Bitwise Studio [Press F1 for Help]", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
        
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                SendMessageA(hwnd, WM_COMMAND, 99, 0);
                continue;
            }
            HWND hFocus = GetFocus();
            if (msg.wParam == VK_RETURN) {
                if (hFocus == hInput || hFocus == hEditA || hFocus == hEditB) {
                    SendMessageA(hwnd, WM_COMMAND, 100, 0);
                    continue;
                }
            }
            if (msg.wParam == VK_ESCAPE) {
                SetFocus(hwnd);
                continue;
            }
            if (hFocus != hInput && hFocus != hEditA && hFocus != hEditB && hFocus != hOutput && hFocus != hBitDisplay) {
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    SendMessageA(hwnd, WM_COMMAND, 99, 0);
                    continue;
                }
                if (msg.wParam == 'C' || msg.wParam == 'c') {
                    SendMessageA(hwnd, WM_COMMAND, 201, 0);
                    continue;
                }
                if (msg.wParam == 'D' || msg.wParam == 'd') {
                    SendMessageA(hwnd, WM_COMMAND, 202, 0);
                    continue;
                }
                if (msg.wParam == 'X' || msg.wParam == 'x') {
                    SendMessageA(hwnd, WM_COMMAND, 203, 0);
                    continue;
                }
                if (msg.wParam == 'V' || msg.wParam == 'v') {
                    SendMessageA(hwnd, WM_COMMAND, 20, 0);
                    continue;
                }
                if (msg.wParam == 'I' || msg.wParam == 'i') {
                    SendMessageA(hwnd, WM_COMMAND, 21, 0);
                    continue;
                }
                if (msg.wParam == 'S' || msg.wParam == 's') {
                    SendMessageA(hwnd, WM_COMMAND, 22, 0);
                    continue;
                }
                if (msg.wParam == 'G' || msg.wParam == 'g') {
                    SendMessageA(hwnd, WM_COMMAND, 23, 0);
                    continue;
                }
                if (msg.wParam == 'F' || msg.wParam == 'f') {
                    SendMessageA(hwnd, WM_COMMAND, 24, 0);
                    continue;
                }
                if (msg.wParam == 'E' || msg.wParam == 'e') {
                    SendMessageA(hwnd, WM_COMMAND, 25, 0);
                    continue;
                }
                if (msg.wParam == 'K' || msg.wParam == 'k') {
                    SendMessageA(hwnd, WM_COMMAND, 26, 0);
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
