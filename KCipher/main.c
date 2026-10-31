#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dst, const void* src, size_t sz) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dst;
}
#pragma function(memcpy)

static int k_strlen(const char* s) {
    int len = 0;
    while (s && s[len]) len++;
    return len;
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}

static int k_atoi(const char* s) {
    int v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        s++;
    }
    return v;
}

// Control IDs
#define ID_EDIT_INPUT       101
#define ID_EDIT_OUTPUT      102
#define ID_COMBO_CIPHER     103
#define ID_EDIT_KEY         104
#define ID_BTN_ENCRYPT      105
#define ID_BTN_DECRYPT      106
#define ID_BTN_SWAP         107
#define ID_BTN_CLEAR        108
#define ID_BTN_HELP         109
#define ID_BTN_COPY         110
#define ID_BTN_QSAVE        111
#define ID_BTN_QLOAD        112
#define ID_BTN_FILE_OPEN    113
#define ID_BTN_FILE_SAVE    114
#define ID_BTN_FREQ_STATS   115
#define ID_BTN_GENKEY       116

// Colors
static COLORREF COLOR_BG = RGB(9, 13, 22);
static COLORREF COLOR_CARD = RGB(15, 23, 42);
static COLORREF COLOR_TEXT = RGB(248, 250, 252);
static COLORREF COLOR_TEXT_MUTED = RGB(148, 163, 184);
static COLORREF COLOR_PRIMARY = RGB(56, 189, 248);
static COLORREF COLOR_ACCENT = RGB(245, 158, 11);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontNormal = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hEditInput = NULL;
static HWND g_hEditOutput = NULL;
static HWND g_hComboCipher = NULL;
static HWND g_hEditKey = NULL;
static HWND g_hBtnGenKey = NULL;
static HWND g_hBtnEncrypt = NULL;
static HWND g_hBtnDecrypt = NULL;
static HWND g_hBtnSwap = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnCopy = NULL;
static HWND g_hBtnQuickSave = NULL;
static HWND g_hBtnQuickLoad = NULL;
static HWND g_hBtnFileOpen = NULL;
static HWND g_hBtnFileSave = NULL;
static HWND g_hBtnFreq = NULL;
static HWND g_hStatusLabel = NULL;

static char g_szStatus[256] = "KCipher Ready. Select cipher, configure key/shift, and click Encrypt/Decrypt.";

// Cipher Logic
static void DoCaesar(const char* in, char* out, int shift, int decrypt) {
    if (decrypt) shift = (26 - (shift % 26)) % 26;
    shift = ((shift % 26) + 26) % 26;

    int i = 0;
    while (in[i]) {
        char c = in[i];
        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)(((c - 'A' + shift) % 26) + 'A');
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)(((c - 'a' + shift) % 26) + 'a');
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoVigenere(const char* in, char* out, const char* key, int decrypt) {
    int keyLen = k_strlen(key);
    if (keyLen == 0) {
        k_strcpy(out, in);
        return;
    }

    int kIdx = 0;
    int i = 0;
    while (in[i]) {
        char c = in[i];
        char kChar = key[kIdx % keyLen];
        int shift = 0;
        if (kChar >= 'A' && kChar <= 'Z') shift = kChar - 'A';
        else if (kChar >= 'a' && kChar <= 'z') shift = kChar - 'a';

        if (decrypt) shift = (26 - shift) % 26;

        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)(((c - 'A' + shift) % 26) + 'A');
            kIdx++;
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)(((c - 'a' + shift) % 26) + 'a');
            kIdx++;
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoAtbash(const char* in, char* out) {
    int i = 0;
    while (in[i]) {
        char c = in[i];
        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)('Z' - (c - 'A'));
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)('z' - (c - 'a'));
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoRailFence(const char* in, char* out, int rails, int decrypt) {
    if (rails < 2) rails = 2;
    if (rails > 10) rails = 10;
    int len = k_strlen(in);
    if (len <= rails) {
        k_strcpy(out, in);
        return;
    }

    if (!decrypt) {
        int pos = 0;
        for (int r = 0; r < rails; r++) {
            int step = 2 * (rails - 1);
            for (int i = r; i < len; i += step) {
                out[pos++] = in[i];
                if (r > 0 && r < rails - 1) {
                    int mid = i + step - 2 * r;
                    if (mid < len) out[pos++] = in[mid];
                }
            }
        }
        out[pos] = 0;
    } else {
        char matrix[10][4096];
        memset(matrix, 0, sizeof(matrix));

        int rail = 0;
        int dir = 1;
        for (int i = 0; i < len; i++) {
            matrix[rail][i] = '*';
            rail += dir;
            if (rail == 0 || rail == rails - 1) dir = -dir;
        }

        int idx = 0;
        for (int r = 0; r < rails; r++) {
            for (int c = 0; c < len; c++) {
                if (matrix[r][c] == '*' && idx < len) {
                    matrix[r][c] = in[idx++];
                }
            }
        }

        rail = 0;
        dir = 1;
        for (int i = 0; i < len; i++) {
            out[i] = matrix[rail][i];
            rail += dir;
            if (rail == 0 || rail == rails - 1) dir = -dir;
        }
        out[len] = 0;
    }
}

// RC4 Stream Cipher
static void DoRC4(const char* in, char* out, const char* key, int decrypt) {
    int keyLen = k_strlen(key);
    if (keyLen == 0) key = "DEFAULT_KEY";
    keyLen = k_strlen(key);

    unsigned char s[256];
    for (int i = 0; i < 256; i++) s[i] = (unsigned char)i;

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + s[i] + (unsigned char)key[i % keyLen]) & 0xFF;
        unsigned char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }

    int i = 0;
    j = 0;

    if (!decrypt) {
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int k = 0; k < inLen; k++) {
            i = (i + 1) & 0xFF;
            j = (j + s[i]) & 0xFF;
            unsigned char tmp = s[i];
            s[i] = s[j];
            s[j] = tmp;
            unsigned char kByte = s[(s[i] + s[j]) & 0xFF];
            unsigned char c = ((unsigned char)in[k]) ^ kByte;

            const char hexChars[] = "0123456789ABCDEF";
            out[outPos++] = hexChars[(c >> 4) & 0x0F];
            out[outPos++] = hexChars[c & 0x0F];
        }
        out[outPos] = 0;
    } else {
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int k = 0; k < inLen; k += 2) {
            char h1 = in[k];
            char h2 = (k + 1 < inLen) ? in[k + 1] : '0';
            int v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : (h1 >= 'A' && h1 <= 'F' ? h1 - 'A' + 10 : (h1 >= 'a' && h1 <= 'f' ? h1 - 'a' + 10 : 0));
            int v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : (h2 >= 'A' && h2 <= 'F' ? h2 - 'A' + 10 : (h2 >= 'a' && h2 <= 'f' ? h2 - 'a' + 10 : 0));
            unsigned char encByte = (unsigned char)((v1 << 4) | v2);

            i = (i + 1) & 0xFF;
            j = (j + s[i]) & 0xFF;
            unsigned char tmp = s[i];
            s[i] = s[j];
            s[j] = tmp;
            unsigned char kByte = s[(s[i] + s[j]) & 0xFF];
            out[outPos++] = (char)(encByte ^ kByte);
        }
        out[outPos] = 0;
    }
}

// XOR Stream
static void DoXOR(const char* in, char* out, const char* key, int decrypt) {
    int keyLen = k_strlen(key);
    if (keyLen == 0) key = "SECRET";
    keyLen = k_strlen(key);

    const char hexChars[] = "0123456789ABCDEF";

    if (!decrypt) {
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int i = 0; i < inLen; i++) {
            unsigned char c = ((unsigned char)in[i]) ^ ((unsigned char)key[i % keyLen]);
            out[outPos++] = hexChars[(c >> 4) & 0x0F];
            out[outPos++] = hexChars[c & 0x0F];
        }
        out[outPos] = 0;
    } else {
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int i = 0; i < inLen; i += 2) {
            char h1 = in[i];
            char h2 = (i + 1 < inLen) ? in[i + 1] : '0';
            int v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : (h1 >= 'A' && h1 <= 'F' ? h1 - 'A' + 10 : (h1 >= 'a' && h1 <= 'f' ? h1 - 'a' + 10 : 0));
            int v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : (h2 >= 'A' && h2 <= 'F' ? h2 - 'A' + 10 : (h2 >= 'a' && h2 <= 'f' ? h2 - 'a' + 10 : 0));
            unsigned char encByte = (unsigned char)((v1 << 4) | v2);
            out[outPos++] = (char)(encByte ^ ((unsigned char)key[(i / 2) % keyLen]));
        }
        out[outPos] = 0;
    }
}

// AES-128 CTR Mode Core
static const unsigned char AES_SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5e, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const unsigned char AES_RCON[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

static void aesKeyExpand(const unsigned char key[16], unsigned char w[44][4]) {
    for (int i = 0; i < 4; i++) {
        w[i][0] = key[4 * i];
        w[i][1] = key[4 * i + 1];
        w[i][2] = key[4 * i + 2];
        w[i][3] = key[4 * i + 3];
    }
    for (int i = 4; i < 44; i++) {
        unsigned char temp[4];
        temp[0] = w[i - 1][0];
        temp[1] = w[i - 1][1];
        temp[2] = w[i - 1][2];
        temp[3] = w[i - 1][3];
        if (i % 4 == 0) {
            unsigned char k0 = temp[0];
            temp[0] = (unsigned char)(AES_SBOX[temp[1]] ^ AES_RCON[i / 4]);
            temp[1] = AES_SBOX[temp[2]];
            temp[2] = AES_SBOX[temp[3]];
            temp[3] = AES_SBOX[k0];
        }
        w[i][0] = (unsigned char)(w[i - 4][0] ^ temp[0]);
        w[i][1] = (unsigned char)(w[i - 4][1] ^ temp[1]);
        w[i][2] = (unsigned char)(w[i - 4][2] ^ temp[2]);
        w[i][3] = (unsigned char)(w[i - 4][3] ^ temp[3]);
    }
}

static unsigned char gmul(unsigned char a, unsigned char b) {
    unsigned char p = 0;
    for (int c = 0; c < 8; c++) {
        if (b & 1) p ^= a;
        unsigned char hi = (unsigned char)(a & 0x80);
        a = (unsigned char)((a << 1) & 0xFF);
        if (hi) a ^= 0x1B;
        b >>= 1;
    }
    return p;
}

static void aesCipherBlock(const unsigned char inBlock[16], const unsigned char roundKeys[44][4], unsigned char outBlock[16]) {
    unsigned char state[4][4];
    for (int r = 0; r < 4; r++) {
        state[r][0] = inBlock[r];
        state[r][1] = inBlock[r + 4];
        state[r][2] = inBlock[r + 8];
        state[r][3] = inBlock[r + 12];
    }
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) state[r][c] ^= roundKeys[c][r];
    }
    for (int round = 1; round <= 9; round++) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) state[r][c] = AES_SBOX[state[r][c]];
        }
        unsigned char s1 = state[1][0];
        state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = s1;
        unsigned char s20 = state[2][0], s21 = state[2][1];
        state[2][0] = state[2][2]; state[2][1] = state[2][3]; state[2][2] = s20; state[2][3] = s21;
        unsigned char s33 = state[3][3];
        state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = state[3][0]; state[3][0] = s33;

        for (int c = 0; c < 4; c++) {
            unsigned char a0 = state[0][c], a1 = state[1][c], a2 = state[2][c], a3 = state[3][c];
            state[0][c] = (unsigned char)(gmul(a0, 2) ^ gmul(a1, 3) ^ a2 ^ a3);
            state[1][c] = (unsigned char)(a0 ^ gmul(a1, 2) ^ gmul(a2, 3) ^ a3);
            state[2][c] = (unsigned char)(a0 ^ a1 ^ gmul(a2, 2) ^ gmul(a3, 3));
            state[3][c] = (unsigned char)(gmul(a0, 3) ^ a1 ^ a2 ^ gmul(a3, 2));
        }
        int base = round * 4;
        for (int c = 0; c < 4; c++) {
            for (int r = 0; r < 4; r++) state[r][c] ^= roundKeys[base + c][r];
        }
    }
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) state[r][c] = AES_SBOX[state[r][c]];
    }
    unsigned char s1 = state[1][0];
    state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = s1;
    unsigned char s20 = state[2][0], s21 = state[2][1];
    state[2][0] = state[2][2]; state[2][1] = state[2][3]; state[2][2] = s20; state[2][3] = s21;
    unsigned char s33 = state[3][3];
    state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = state[3][0]; state[3][0] = s33;

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) state[r][c] ^= roundKeys[40 + c][r];
    }
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) outBlock[c * 4 + r] = state[r][c];
    }
}

static void DoAES128CTR(const char* in, char* out, const char* key, int decrypt) {
    unsigned char keyBytes[16];
    int klen = k_strlen(key);
    if (klen == 0) { key = "ARCHITECT_1999!"; klen = 15; }
    for (int i = 0; i < 16; i++) {
        keyBytes[i] = (i < klen) ? (unsigned char)key[i] : 0x5a;
    }
    unsigned char roundKeys[44][4];
    aesKeyExpand(keyBytes, roundKeys);

    const unsigned char nonce[8] = { 0x19, 0x99, 0x01, 0x01, 0xde, 0xad, 0xbe, 0xef };
    const char hexChars[] = "0123456789ABCDEF";

    if (!decrypt) {
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int i = 0; i < inLen; i += 16) {
            int blockIdx = i / 16;
            unsigned char counterBlock[16];
            memcpy(counterBlock, nonce, 8);
            counterBlock[8] = 0; counterBlock[9] = 0; counterBlock[10] = 0; counterBlock[11] = 0;
            counterBlock[12] = (unsigned char)((blockIdx >> 24) & 0xff);
            counterBlock[13] = (unsigned char)((blockIdx >> 16) & 0xff);
            counterBlock[14] = (unsigned char)((blockIdx >> 8) & 0xff);
            counterBlock[15] = (unsigned char)(blockIdx & 0xff);

            unsigned char keyStream[16];
            aesCipherBlock(counterBlock, roundKeys, keyStream);

            for (int b = 0; b < 16 && (i + b) < inLen; b++) {
                unsigned char c = (unsigned char)(((unsigned char)in[i + b]) ^ keyStream[b]);
                out[outPos++] = hexChars[(c >> 4) & 0x0f];
                out[outPos++] = hexChars[c & 0x0f];
            }
        }
        out[outPos] = 0;
    } else {
        int inLen = k_strlen(in);
        int hexLen = 0;
        unsigned char rawBytes[2048];
        for (int i = 0; i < inLen && hexLen < 2048; i += 2) {
            char h1 = in[i];
            char h2 = (i + 1 < inLen) ? in[i + 1] : '0';
            int v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : (h1 >= 'A' && h1 <= 'F' ? h1 - 'A' + 10 : (h1 >= 'a' && h1 <= 'f' ? h1 - 'a' + 10 : 0));
            int v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : (h2 >= 'A' && h2 <= 'F' ? h2 - 'A' + 10 : (h2 >= 'a' && h2 <= 'f' ? h2 - 'a' + 10 : 0));
            rawBytes[hexLen++] = (unsigned char)((v1 << 4) | v2);
        }

        int outPos = 0;
        for (int i = 0; i < hexLen; i += 16) {
            int blockIdx = i / 16;
            unsigned char counterBlock[16];
            memcpy(counterBlock, nonce, 8);
            counterBlock[8] = 0; counterBlock[9] = 0; counterBlock[10] = 0; counterBlock[11] = 0;
            counterBlock[12] = (unsigned char)((blockIdx >> 24) & 0xff);
            counterBlock[13] = (unsigned char)((blockIdx >> 16) & 0xff);
            counterBlock[14] = (unsigned char)((blockIdx >> 8) & 0xff);
            counterBlock[15] = (unsigned char)(blockIdx & 0xff);

            unsigned char keyStream[16];
            aesCipherBlock(counterBlock, roundKeys, keyStream);

            for (int b = 0; b < 16 && (i + b) < hexLen; b++) {
                out[outPos++] = (char)(rawBytes[i + b] ^ keyStream[b]);
            }
        }
        out[outPos] = 0;
    }
}

// Toy RSA Core (32-bit arithmetic, mod = 3233, e = 17, d = 2753)
static unsigned int rsa_modpow(unsigned int base, unsigned int exp, unsigned int mod) {
    unsigned int res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) res = (res * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return res;
}

static void DoRSADemo(const char* in, char* out, const char* key, int decrypt) {
    unsigned int e = 17, d = 2753, n = 3233;
    if (!decrypt) {
        int inLen = k_strlen(in);
        out[0] = 0;
        char numBuf[32];
        for (int i = 0; i < inLen; i++) {
            unsigned int m = (unsigned char)in[i];
            unsigned int c = rsa_modpow(m, e, n);
            wsprintfA(numBuf, "%u ", c);
            int curLen = k_strlen(out);
            if (curLen + 10 < 4000) {
                k_strcpy(out + curLen, numBuf);
            }
        }
    } else {
        int outPos = 0;
        const char* pStr = in;
        while (*pStr && outPos < 4000) {
            while (*pStr == ' ' || *pStr == '\t' || *pStr == '\r' || *pStr == '\n') pStr++;
            if (!*pStr) break;
            unsigned int val = 0;
            while (*pStr >= '0' && *pStr <= '9') {
                val = val * 10 + (*pStr - '0');
                pStr++;
            }
            unsigned int m = rsa_modpow(val, d, n);
            out[outPos++] = (char)(m & 0xFF);
        }
        out[outPos] = 0;
    }
}

static void OpenFileDlg(HWND hwnd) {
    char szFile[MAX_PATH];
    szFile[0] = 0;
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Text / Cipher Files (*.txt;*.enc)\0*.txt;*.enc\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buf[4096];
            memset(buf, 0, sizeof(buf));
            DWORD dwRead = 0;
            ReadFile(hFile, buf, sizeof(buf) - 1, &dwRead, NULL);
            CloseHandle(hFile);
            SetWindowTextA(g_hEditInput, buf);
            wsprintfA(g_szStatus, "Loaded %d bytes from %s", (int)dwRead, szFile);
            SetWindowTextA(g_hStatusLabel, g_szStatus);
        }
    }
}

static void SaveFileDlg(HWND hwnd) {
    char szFile[MAX_PATH];
    k_strcpy(szFile, "cipher_export.enc");
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Cipher Files (*.enc)\0*.enc\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&ofn)) {
        char buf[4096];
        memset(buf, 0, sizeof(buf));
        GetWindowTextA(g_hEditOutput, buf, sizeof(buf) - 1);
        int len = k_strlen(buf);
        if (len == 0) {
            GetWindowTextA(g_hEditInput, buf, sizeof(buf) - 1);
            len = k_strlen(buf);
        }
        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwWritten = 0;
            WriteFile(hFile, buf, len, &dwWritten, NULL);
            CloseHandle(hFile);
            wsprintfA(g_szStatus, "Exported %d bytes to %s", len, szFile);
            SetWindowTextA(g_hStatusLabel, g_szStatus);
        }
    }
}

static void ShowFrequencyStats(HWND hwnd) {
    char text[4096];
    memset(text, 0, sizeof(text));
    GetWindowTextA(g_hEditOutput, text, sizeof(text) - 1);
    if (k_strlen(text) == 0) {
        GetWindowTextA(g_hEditInput, text, sizeof(text) - 1);
    }
    int len = k_strlen(text);
    if (len == 0) {
        MessageBoxA(hwnd, "No text to analyze. Enter plaintext or generate ciphertext first.", "Frequency Analysis", MB_OK | MB_ICONWARNING);
        return;
    }
    int counts[26];
    memset(counts, 0, sizeof(counts));
    int totalLetters = 0;
    for (int i = 0; i < len; i++) {
        char c = text[i];
        if (c >= 'A' && c <= 'Z') { counts[c - 'A']++; totalLetters++; }
        else if (c >= 'a' && c <= 'z') { counts[c - 'a']++; totalLetters++; }
    }
    
    int iocScaled = 0;
    if (totalLetters > 1) {
        int sumProd = 0;
        for (int i = 0; i < 26; i++) {
            sumProd += counts[i] * (counts[i] - 1);
        }
        iocScaled = (sumProd * 1000) / (totalLetters * (totalLetters - 1));
    }

    char msg[1024];
    wsprintfA(msg,
        "Text Length: %d chars | English Letters: %d\n"
        "Index of Coincidence (IoC): 0.%03d\n"
        "(Typical English: ~0.067 | Polyalphabetic/Random: ~0.038)\n\n"
        "Letter Frequency Breakdown:\n"
        "A:%d B:%d C:%d D:%d E:%d F:%d G:%d\n"
        "H:%d I:%d J:%d K:%d L:%d M:%d N:%d\n"
        "O:%d P:%d Q:%d R:%d S:%d T:%d U:%d\n"
        "V:%d W:%d X:%d Y:%d Z:%d",
        len, totalLetters, iocScaled,
        counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6],
        counts[7], counts[8], counts[9], counts[10], counts[11], counts[12], counts[13],
        counts[14], counts[15], counts[16], counts[17], counts[18], counts[19], counts[20],
        counts[21], counts[22], counts[23], counts[24], counts[25]);

    MessageBoxA(hwnd, msg, "Frequency & IoC Analysis", MB_OK | MB_ICONINFORMATION);
}

static void ProcessCipher(int decrypt) {
    char inText[4096];
    char outText[4096];
    char keyText[128];
    memset(inText, 0, sizeof(inText));
    memset(outText, 0, sizeof(outText));
    memset(keyText, 0, sizeof(keyText));

    GetWindowTextA(decrypt ? g_hEditOutput : g_hEditInput, inText, sizeof(inText) - 1);
    GetWindowTextA(g_hEditKey, keyText, sizeof(keyText) - 1);

    int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);

    switch (sel) {
        case 0: { // Caesar
            int shift = k_atoi(keyText);
            if (shift == 0) shift = 13;
            DoCaesar(inText, outText, shift, decrypt);
            break;
        }
        case 1: { // Vigenere
            if (keyText[0] == 0) k_strcpy(keyText, "KILO1999");
            DoVigenere(inText, outText, keyText, decrypt);
            break;
        }
        case 2: { // Rail Fence
            int rails = k_atoi(keyText);
            if (rails < 2) rails = 3;
            DoRailFence(inText, outText, rails, decrypt);
            break;
        }
        case 3: { // Atbash
            DoAtbash(inText, outText);
            break;
        }
        case 4: { // RC4
            if (keyText[0] == 0) k_strcpy(keyText, "ARCHITECT_1999");
            DoRC4(inText, outText, keyText, decrypt);
            break;
        }
        case 5: { // XOR Stream
            if (keyText[0] == 0) k_strcpy(keyText, "SECRET");
            DoXOR(inText, outText, keyText, decrypt);
            break;
        }
        case 6: { // AES-128 CTR
            if (keyText[0] == 0) k_strcpy(keyText, "ARCHITECT_1999!");
            DoAES128CTR(inText, outText, keyText, decrypt);
            break;
        }
        case 7: { // RSA Demo
            DoRSADemo(inText, outText, keyText, decrypt);
            break;
        }
        default:
            k_strcpy(outText, inText);
            break;
    }

    SetWindowTextA(decrypt ? g_hEditInput : g_hEditOutput, outText);

    int inLen = k_strlen(inText);
    int outLen = k_strlen(outText);
    wsprintfA(g_szStatus, "Processed %d chars -> %d chars. Mode: %s.", inLen, outLen, decrypt ? "Decryption" : "Encryption");
    SetWindowTextA(g_hStatusLabel, g_szStatus);
}

static void DoGenKey(HWND hwnd) {
    int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);
    char keyBuf[128];
    keyBuf[0] = 0;
    static unsigned int seed = 199973;
    seed = seed * 1103515245 + 12345;

    switch (sel) {
        case 0: { // Caesar
            int shift = ((seed >> 4) % 25) + 1;
            wsprintfA(keyBuf, "%d", shift);
            break;
        }
        case 1: { // Vigenere
            const char* words[] = { "KILO1999", "CIPHER99", "CYBERSYS", "PARADIGM", "ENIGMA99", "SPECTRE9" };
            k_strcpy(keyBuf, words[(seed >> 4) % 6]);
            break;
        }
        case 2: { // Rail Fence
            int rails = ((seed >> 4) % 5) + 2;
            wsprintfA(keyBuf, "%d", rails);
            break;
        }
        case 3: { // Atbash
            k_strcpy(keyBuf, "(None - Fixed Mirror)");
            break;
        }
        case 4: { // RC4
            const char* keys[] = { "ARCHITECT_1999", "KILO_STREAM_KEY", "DEEP_CHOPPER99", "SYS_CIPHER_CORE" };
            k_strcpy(keyBuf, keys[(seed >> 4) % 4]);
            break;
        }
        case 5: { // XOR Stream
            const char* xors[] = { "SECRET", "SHADOW1999", "KILO_KEY", "CYBER_XOR" };
            k_strcpy(keyBuf, xors[(seed >> 4) % 4]);
            break;
        }
        case 6: { // AES-128 CTR
            const char* aesKeys[] = { "ARCHITECT_1999!", "KILO_AES128_SPEC", "KILO_BLOCK_1999", "CYBER_KEY_128BIT" };
            k_strcpy(keyBuf, aesKeys[(seed >> 4) % 4]);
            break;
        }
        case 7: { // RSA Demo
            k_strcpy(keyBuf, "p=61, q=53, e=17 (n=3233, d=2753)");
            break;
        }
    }
    SetWindowTextA(g_hEditKey, keyBuf);
    ProcessCipher(0);
}

static void SaveSnapshot(HWND hwnd) {
    char inText[4096];
    char outText[4096];
    char keyText[128];
    memset(inText, 0, sizeof(inText));
    memset(outText, 0, sizeof(outText));
    memset(keyText, 0, sizeof(keyText));

    GetWindowTextA(g_hEditInput, inText, sizeof(inText) - 1);
    GetWindowTextA(g_hEditOutput, outText, sizeof(outText) - 1);
    GetWindowTextA(g_hEditKey, keyText, sizeof(keyText) - 1);
    int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);

    HANDLE hFile = CreateFileA("kcipher_quicksave.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &sel, sizeof(sel), &written, NULL);
        int inLen = k_strlen(inText);
        WriteFile(hFile, &inLen, sizeof(inLen), &written, NULL);
        if (inLen > 0) WriteFile(hFile, inText, inLen, &written, NULL);
        int outLen = k_strlen(outText);
        WriteFile(hFile, &outLen, sizeof(outLen), &written, NULL);
        if (outLen > 0) WriteFile(hFile, outText, outLen, &written, NULL);
        int keyLen = k_strlen(keyText);
        WriteFile(hFile, &keyLen, sizeof(keyLen), &written, NULL);
        if (keyLen > 0) WriteFile(hFile, keyText, keyLen, &written, NULL);
        CloseHandle(hFile);
        wsprintfA(g_szStatus, "Session snapshot saved [F5] to kcipher_quicksave.dat");
        SetWindowTextA(g_hStatusLabel, g_szStatus);
    } else {
        SetWindowTextA(g_hStatusLabel, "Failed to save session snapshot.");
    }
}

static void LoadSnapshot(HWND hwnd) {
    HANDLE hFile = CreateFileA("kcipher_quicksave.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        int sel = 0;
        ReadFile(hFile, &sel, sizeof(sel), &read, NULL);
        int inLen = 0;
        char inText[4096];
        memset(inText, 0, sizeof(inText));
        ReadFile(hFile, &inLen, sizeof(inLen), &read, NULL);
        if (inLen > 0 && inLen < (int)sizeof(inText)) ReadFile(hFile, inText, inLen, &read, NULL);

        int outLen = 0;
        char outText[4096];
        memset(outText, 0, sizeof(outText));
        ReadFile(hFile, &outLen, sizeof(outLen), &read, NULL);
        if (outLen > 0 && outLen < (int)sizeof(outText)) ReadFile(hFile, outText, outLen, &read, NULL);

        int keyLen = 0;
        char keyText[128];
        memset(keyText, 0, sizeof(keyText));
        ReadFile(hFile, &keyLen, sizeof(keyLen), &read, NULL);
        if (keyLen > 0 && keyLen < (int)sizeof(keyText)) ReadFile(hFile, keyText, keyLen, &read, NULL);
        CloseHandle(hFile);

        if (sel >= 0 && sel <= 7) {
            SendMessageA(g_hComboCipher, CB_SETCURSEL, sel, 0);
        }
        SetWindowTextA(g_hEditInput, inText);
        SetWindowTextA(g_hEditOutput, outText);
        SetWindowTextA(g_hEditKey, keyText);

        wsprintfA(g_szStatus, "Session snapshot restored [F9] (%d chars in, %d chars out)", inLen, outLen);
        SetWindowTextA(g_hStatusLabel, g_szStatus);
    } else {
        SetWindowTextA(g_hStatusLabel, "No quicksave file found. Press F5 to save snapshot.");
    }
}

static void CheckFirstRun(HWND hwnd) {
    HANDLE hSave = CreateFileA("kcipher_quicksave.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSave != INVALID_HANDLE_VALUE) {
        CloseHandle(hSave);
        return;
    }
    HANDLE hTut = CreateFileA("kcipher_tutorialSeen.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        CloseHandle(hTut);
        return;
    }
    HANDLE hNew = CreateFileA("kcipher_tutorialSeen.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hNew != INVALID_HANDLE_VALUE) {
        DWORD wr = 0;
        WriteFile(hNew, "1", 1, &wr, NULL);
        CloseHandle(hNew);
    }
    MessageBoxA(hwnd,
        "Welcome to KCipher - Cryptographic Cipher Suite (1999 Edition)\n\n"
        "Supported Ciphers:\n"
        "1. Caesar / ROT-N, Vigenere, Rail Fence, Atbash\n"
        "2. RC4 & XOR Stream Ciphers\n"
        "3. AES-128 CTR Block Cipher (Hex Keystream)\n"
        "4. RSA Public-Key Demo (p=61, q=53, e=17, n=3233)\n\n"
        "Ergonomics & Hotkeys:\n"
        "- Ctrl+E: Encrypt | Ctrl+D: Decrypt\n"
        "- Gen Key: Instant random key/prime generation\n"
        "- Ctrl+S / F5: QuickSave snapshot | Ctrl+O / F9: QuickLoad snapshot\n"
        "- F1 / H: Cipher Manual | Esc: Clear Input/Output",
        "KCipher - First Run Guide", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);

            g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");

            // Controls
            g_hComboCipher = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 120, 48, 210, 240, hwnd, (HMENU)ID_COMBO_CIPHER, NULL, NULL);
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Caesar / ROT-N (Key = Shift)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Vigenere (Key = Word)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Rail Fence (Key = Rails)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Atbash Mirror");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"RC4 Stream (Key = Passphrase)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"XOR Stream (Key = Passphrase)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"AES-128 CTR (Key = Passphrase)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"RSA Demo (p=61, q=53, e=17)");
            SendMessageA(g_hComboCipher, CB_SETCURSEL, 0, 0);
            SendMessageA(g_hComboCipher, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hEditKey = CreateWindowA("EDIT", "13", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 410, 48, 140, 24, hwnd, (HMENU)ID_EDIT_KEY, NULL, NULL);
            SendMessageA(g_hEditKey, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            g_hBtnGenKey = CreateWindowA("BUTTON", "Gen Key", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 555, 47, 75, 26, hwnd, (HMENU)ID_BTN_GENKEY, NULL, NULL);
            SendMessageA(g_hBtnGenKey, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnEncrypt = CreateWindowA("BUTTON", "Encrypt [Ctrl+E]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 635, 47, 125, 26, hwnd, (HMENU)ID_BTN_ENCRYPT, NULL, NULL);
            SendMessageA(g_hBtnEncrypt, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            g_hBtnDecrypt = CreateWindowA("BUTTON", "Decrypt [Ctrl+D]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 47, 125, 26, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            SendMessageA(g_hBtnDecrypt, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            // Text Areas
            g_hEditInput = CreateWindowA("EDIT", "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 20, 110, 430, 360, hwnd, (HMENU)ID_EDIT_INPUT, NULL, NULL);
            SendMessageA(g_hEditInput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            g_hEditOutput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 470, 110, 430, 360, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
            SendMessageA(g_hEditOutput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Action Buttons
            g_hBtnSwap = CreateWindowA("BUTTON", "Swap Panes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 485, 85, 26, hwnd, (HMENU)ID_BTN_SWAP, NULL, NULL);
            SendMessageA(g_hBtnSwap, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnClear = CreateWindowA("BUTTON", "Clear All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 110, 485, 75, 26, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            SendMessageA(g_hBtnClear, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnCopy = CreateWindowA("BUTTON", "Copy Output", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 485, 90, 26, hwnd, (HMENU)ID_BTN_COPY, NULL, NULL);
            SendMessageA(g_hBtnCopy, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnFileOpen = CreateWindowA("BUTTON", "Open File...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 285, 485, 85, 26, hwnd, (HMENU)ID_BTN_FILE_OPEN, NULL, NULL);
            SendMessageA(g_hBtnFileOpen, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnFileSave = CreateWindowA("BUTTON", "Export...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 375, 485, 80, 26, hwnd, (HMENU)ID_BTN_FILE_SAVE, NULL, NULL);
            SendMessageA(g_hBtnFileSave, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnFreq = CreateWindowA("BUTTON", "Freq / IoC", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 460, 485, 85, 26, hwnd, (HMENU)ID_BTN_FREQ_STATS, NULL, NULL);
            SendMessageA(g_hBtnFreq, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnQuickSave = CreateWindowA("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 550, 485, 80, 26, hwnd, (HMENU)ID_BTN_QSAVE, NULL, NULL);
            SendMessageA(g_hBtnQuickSave, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnQuickLoad = CreateWindowA("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 635, 485, 80, 26, hwnd, (HMENU)ID_BTN_QLOAD, NULL, NULL);
            SendMessageA(g_hBtnQuickLoad, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnHelp = CreateWindowA("BUTTON", "Manual (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 720, 485, 90, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessageA(g_hBtnHelp, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hStatusLabel = CreateWindowA("STATIC", g_szStatus, WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 525, 880, 20, hwnd, NULL, NULL, NULL);
            SendMessageA(g_hStatusLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            ProcessCipher(0);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == ID_BTN_ENCRYPT) {
                ProcessCipher(0);
            } else if (wmId == ID_BTN_DECRYPT) {
                ProcessCipher(1);
            } else if (wmId == ID_BTN_GENKEY) {
                DoGenKey(hwnd);
            } else if (wmId == ID_BTN_SWAP) {
                char t1[4096], t2[4096];
                GetWindowTextA(g_hEditInput, t1, sizeof(t1));
                GetWindowTextA(g_hEditOutput, t2, sizeof(t2));
                SetWindowTextA(g_hEditInput, t2);
                SetWindowTextA(g_hEditOutput, t1);
            } else if (wmId == ID_BTN_CLEAR) {
                SetWindowTextA(g_hEditInput, "");
                SetWindowTextA(g_hEditOutput, "");
            } else if (wmId == ID_BTN_COPY) {
                char outText[4096];
                GetWindowTextA(g_hEditOutput, outText, sizeof(outText));
                int len = k_strlen(outText);
                if (len > 0 && OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
                    if (hGlob) {
                        char* p = (char*)GlobalLock(hGlob);
                        memcpy(p, outText, len + 1);
                        GlobalUnlock(hGlob);
                        SetClipboardData(CF_TEXT, hGlob);
                    }
                    CloseClipboard();
                    SetWindowTextA(g_hStatusLabel, "Output copied to Windows clipboard.");
                }
            } else if (wmId == ID_BTN_QSAVE) {
                SaveSnapshot(hwnd);
            } else if (wmId == ID_BTN_QLOAD) {
                LoadSnapshot(hwnd);
            } else if (wmId == ID_BTN_FILE_OPEN) {
                OpenFileDlg(hwnd);
            } else if (wmId == ID_BTN_FILE_SAVE) {
                SaveFileDlg(hwnd);
            } else if (wmId == ID_BTN_FREQ_STATS) {
                ShowFrequencyStats(hwnd);
            } else if (wmId == ID_BTN_HELP) {
                MessageBoxA(hwnd,
                    "KCipher - Cryptographic Cipher Suite (v1.0.0 - 1999)\n\n"
                    "Supported Ciphers:\n"
                    "1. Caesar / ROT-N: Shift letter by integer N (1-25).\n"
                    "2. Vigenere: Polyalphabetic key phrase shifting.\n"
                    "3. Rail Fence: Zig-zag transposition with N rails.\n"
                    "4. Atbash: Mirror substitution (A<->Z, B<->Y).\n"
                    "5. RC4: Stream cipher with passphrase (hex output).\n"
                    "6. XOR Stream: Byte-wise XOR keystream (hex output).\n"
                    "7. AES-128 CTR: Block cipher in counter mode (hex output).\n"
                    "8. RSA Demo: Asymmetric key encryption (space-separated integers).\n\n"
                    "Shortcuts:\n"
                    "F1 / H: Manual | Esc: Clear Panes | Ctrl+E / Ctrl+Enter: Encrypt | Ctrl+D: Decrypt\n"
                    "Ctrl+S / F5: QuickSave snapshot | Ctrl+O / F9: QuickLoad snapshot",
                    "KCipher Manual", MB_OK | MB_ICONINFORMATION);
            } else if (wmId == ID_COMBO_CIPHER && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);
                if (sel == 0) SetWindowTextA(g_hEditKey, "13");
                else if (sel == 1) SetWindowTextA(g_hEditKey, "KILO1999");
                else if (sel == 2) SetWindowTextA(g_hEditKey, "3");
                else if (sel == 3) SetWindowTextA(g_hEditKey, "(None)");
                else if (sel == 4) SetWindowTextA(g_hEditKey, "ARCHITECT_1999");
                else if (sel == 5) SetWindowTextA(g_hEditKey, "SECRET");
                else if (sel == 6) SetWindowTextA(g_hEditKey, "ARCHITECT_1999!");
                else if (sel == 7) SetWindowTextA(g_hEditKey, "p=61, q=53, e=17 (n=3233)");
                ProcessCipher(0);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_hBrushBg);

            SetBkMode(hdc, TRANSPARENT);

            // Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, COLOR_PRIMARY);
            TextOutA(hdc, 20, 12, "KCipher - Cryptographic Cipher Suite", 36);

            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOutA(hdc, 400, 18, "v1.0.0 (1999 Edition)", 21);

            // Labels
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutA(hdc, 20, 51, "Cipher Engine:", 14);
            TextOutA(hdc, 335, 51, "Key / Shift:", 12);

            TextOutA(hdc, 20, 88, "Plaintext Input:", 16);
            TextOutA(hdc, 470, 88, "Ciphertext Output:", 18);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COLOR_TEXT);
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)g_hBrushBg;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, COLOR_TEXT);
            SetBkColor(hdcEdit, COLOR_CARD);
            return (LRESULT)g_hBrushCard;
        }

        case WM_DESTROY: {
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushCard) DeleteObject(g_hBrushCard);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KCipherClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KCipherClass",
        "KCipher - Cryptographic Cipher Suite",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        940, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    CheckFirstRun(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            int isEdit = (hFocus == g_hEditInput || hFocus == g_hEditOutput || hFocus == g_hEditKey);
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            if (msg.wParam == VK_ESCAPE) {
                SetWindowTextA(g_hEditInput, "");
                SetWindowTextA(g_hEditOutput, "");
                SetWindowTextA(g_hStatusLabel, "Panes Cleared [Esc].");
                continue;
            }
            if (msg.wParam == VK_F5 || (ctrl && (msg.wParam == 'S' || msg.wParam == 's'))) {
                SaveSnapshot(hwnd);
                continue;
            }
            if (msg.wParam == VK_F9 || (ctrl && (msg.wParam == 'O' || msg.wParam == 'o'))) {
                LoadSnapshot(hwnd);
                continue;
            }
            if (msg.wParam == VK_F1 || (!isEdit && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_HELP, 0), (LPARAM)g_hBtnHelp);
                continue;
            }
            if (ctrl && (msg.wParam == 'E' || msg.wParam == 'e')) {
                ProcessCipher(0);
                continue;
            }
            if (ctrl && (msg.wParam == 'D' || msg.wParam == 'd')) {
                ProcessCipher(1);
                continue;
            }
            if (ctrl && (msg.wParam == 'I' || msg.wParam == 'i')) {
                ShowFrequencyStats(hwnd);
                continue;
            }
            if (ctrl && msg.wParam == VK_RETURN) {
                ProcessCipher(0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess((UINT)msg.wParam);
}
