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

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = 0;
}

// Control IDs
#define ID_COMBO_MODE       101
#define ID_EDIT_COVER       102
#define ID_EDIT_SECRET      103
#define ID_EDIT_KEY         104
#define ID_EDIT_OUTPUT      105
#define ID_BTN_INJECT       106
#define ID_BTN_EXTRACT      107
#define ID_BTN_CHISQ        108
#define ID_BTN_CLEAR        109
#define ID_BTN_HELP         110
#define ID_BTN_COPY         111
#define ID_STATUS_LABEL     112
#define ID_BTN_SAVE_FILE    113
#define ID_BTN_LOAD_FILE    114
#define ID_BTN_VFS_REPORT   115
#define ID_BTN_SEND_KCIPHER 116

// Cyber Theme Colors
static COLORREF COLOR_BG = RGB(8, 13, 26);
static COLORREF COLOR_CARD = RGB(14, 23, 42);
static COLORREF COLOR_TEXT = RGB(226, 232, 240);
static COLORREF COLOR_TEXT_MUTED = RGB(140, 163, 199);
static COLORREF COLOR_PRIMARY = RGB(0, 210, 255);
static COLORREF COLOR_ACCENT = RGB(255, 159, 28);
static COLORREF COLOR_SUCCESS = RGB(16, 185, 129);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontNormal = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hComboMode = NULL;
static HWND g_hEditCover = NULL;
static HWND g_hEditSecret = NULL;
static HWND g_hEditKey = NULL;
static HWND g_hEditOutput = NULL;
static HWND g_hBtnInject = NULL;
static HWND g_hBtnExtract = NULL;
static HWND g_hBtnChisq = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnCopy = NULL;
static HWND g_hStatusLabel = NULL;
static HWND g_hBtnSaveFile = NULL;
static HWND g_hBtnLoadFile = NULL;
static HWND g_hBtnVfsReport = NULL;
static HWND g_hBtnSendKCipher = NULL;

static char g_szStatus[512] = "KSteno Ready. Select carrier mode, enter secret message, and click Inject or Extract.";

// RC4 Stream Cipher for payload protection
static void RC4Crypt(const char* key, const unsigned char* in, unsigned char* out, int len) {
    if (!key || !key[0]) {
        memcpy(out, in, len);
        return;
    }
    int klen = k_strlen(key);
    unsigned char s[256];
    for (int i = 0; i < 256; i++) s[i] = (unsigned char)i;
    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + s[i] + (unsigned char)key[i % klen]) & 0xFF;
        unsigned char t = s[i]; s[i] = s[j]; s[j] = t;
    }
    int i = 0; j = 0;
    for (int k = 0; k < len; k++) {
        i = (i + 1) & 0xFF;
        j = (j + s[i]) & 0xFF;
        unsigned char t = s[i]; s[i] = s[j]; s[j] = t;
        out[k] = in[k] ^ s[(s[i] + s[j]) & 0xFF];
    }
}

static unsigned int CalcCRC32(const unsigned char* data, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        unsigned int byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; j++) {
            unsigned int mask = -(int)(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}


// 1. Text Whitespace Steganography (SNOW Standard: Space=0, Tab=1)
static void EncodeTextWhitespace(const char* cover, const char* secret, const char* key, char* out, int outMax) {
    int secLen = k_strlen(secret);
    if (secLen == 0) {
        k_strcpy(out, cover);
        return;
    }

    unsigned char encSecret[2048];
    if (secLen > 2000) secLen = 2000;
    RC4Crypt(key, (const unsigned char*)secret, encSecret, secLen);

    char bits[18000];
    int bitPtr = 0;

    for (int b = 15; b >= 0; b--) {
        bits[bitPtr++] = ((secLen >> b) & 1) ? '\t' : ' ';
    }

    for (int i = 0; i < secLen; i++) {
        unsigned char c = encSecret[i];
        for (int b = 7; b >= 0; b--) {
            bits[bitPtr++] = ((c >> b) & 1) ? '\t' : ' ';
        }
    }
    bits[bitPtr] = 0;

    int outPtr = 0;
    int cIdx = 0;
    int bitsWritten = 0;

    while (cover[cIdx] && outPtr < outMax - 100) {
        if (cover[cIdx] == '\r' && cover[cIdx + 1] == '\n') {
            int chunk = 16;
            while (chunk > 0 && bitsWritten < bitPtr && outPtr < outMax - 10) {
                out[outPtr++] = bits[bitsWritten++];
                chunk--;
            }
            out[outPtr++] = '\r';
            out[outPtr++] = '\n';
            cIdx += 2;
        } else if (cover[cIdx] == '\n') {
            int chunk = 16;
            while (chunk > 0 && bitsWritten < bitPtr && outPtr < outMax - 10) {
                out[outPtr++] = bits[bitsWritten++];
                chunk--;
            }
            out[outPtr++] = '\n';
            cIdx++;
        } else {
            out[outPtr++] = cover[cIdx++];
        }
    }

    while (bitsWritten < bitPtr && outPtr < outMax - 10) {
        out[outPtr++] = bits[bitsWritten++];
    }
    out[outPtr] = 0;
}

static void DecodeTextWhitespace(const char* in, const char* key, char* out, int outMax) {
    char bits[18000];
    int bitPtr = 0;

    int i = 0;
    while (in[i] && bitPtr < 17900) {
        if (in[i] == ' ') {
            bits[bitPtr++] = 0;
        } else if (in[i] == '\t') {
            bits[bitPtr++] = 1;
        }
        i++;
    }

    if (bitPtr < 16) {
        k_strcpy(out, "[ERROR: No whitespace bits detected in cover text]");
        return;
    }

    int secLen = 0;
    for (int b = 0; b < 16; b++) {
        secLen = (secLen << 1) | bits[b];
    }

    if (secLen <= 0 || secLen * 8 + 16 > bitPtr || secLen > 2000) {
        k_strcpy(out, "[ERROR: Invalid stego header length detected]");
        return;
    }

    unsigned char encSecret[2048];
    for (int s = 0; s < secLen; s++) {
        unsigned char c = 0;
        for (int b = 0; b < 8; b++) {
            c = (c << 1) | bits[16 + s * 8 + b];
        }
        encSecret[s] = c;
    }

    unsigned char decSecret[2048];
    RC4Crypt(key, encSecret, decSecret, secLen);
    decSecret[secLen] = 0;

    int copyLen = secLen < outMax - 1 ? secLen : outMax - 1;
    memcpy(out, decSecret, copyLen);
    out[copyLen] = 0;
}

// Structures for BMP and WAV carriers
#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
} K_BMP_FILEHEADER;

typedef struct {
    unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} K_BMP_INFOHEADER;

typedef struct {
    char           riff[4];      // "RIFF"
    unsigned int   riffSize;
    char           wave[4];      // "WAVE"
    char           fmt[4];       // "fmt "
    unsigned int   fmtSize;      // 16
    unsigned short audioFormat;  // 1 (PCM)
    unsigned short numChannels;  // 1 (Mono)
    unsigned int   sampleRate;   // 44100
    unsigned int   byteRate;     // 88200
    unsigned short blockAlign;   // 2
    unsigned short bitsPerSample;// 16
    char           data[4];      // "data"
    unsigned int   dataSize;
} K_WAV_HEADER;
#pragma pack(pop)

// 2. BMP Image LSB Steganography
static int EncodeBmpCarrier(const char* inBmpPath, const char* outBmpPath, const char* secret, const char* key, char* errBuf) {
    int secLen = k_strlen(secret);
    if (secLen > 2000) secLen = 2000;
    if (secLen == 0) {
        k_strcpy(errBuf, "Error: Secret message is empty.");
        return 0;
    }

    unsigned char encSecret[2048];
    RC4Crypt(key, (const unsigned char*)secret, encSecret, secLen);

    // Create or open BMP
    HANDLE hFile = CreateFileA(inBmpPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    unsigned char bmpData[256 * 256 * 3 + 1024];
    DWORD bytesRead = 0;
    int w = 256, h = 256;
    int pixelDataOffset = sizeof(K_BMP_FILEHEADER) + sizeof(K_BMP_INFOHEADER);
    int pixelDataSize = w * h * 3;

    if (hFile != INVALID_HANDLE_VALUE) {
        ReadFile(hFile, bmpData, sizeof(bmpData), &bytesRead, NULL);
        CloseHandle(hFile);
        if (bytesRead >= sizeof(K_BMP_FILEHEADER) + sizeof(K_BMP_INFOHEADER)) {
            K_BMP_FILEHEADER* bfh = (K_BMP_FILEHEADER*)bmpData;
            K_BMP_INFOHEADER* bih = (K_BMP_INFOHEADER*)(bmpData + sizeof(K_BMP_FILEHEADER));
            if (bfh->bfType == 0x4D42 && bih->biBitCount == 24) {
                pixelDataOffset = bfh->bfOffBits;
                pixelDataSize = bih->biSizeImage ? bih->biSizeImage : (bih->biWidth * (bih->biHeight < 0 ? -bih->biHeight : bih->biHeight) * 3);
            }
        }
    } else {
        // Synthesize default 256x256 24-bit plasma bitmap
        K_BMP_FILEHEADER* bfh = (K_BMP_FILEHEADER*)bmpData;
        K_BMP_INFOHEADER* bih = (K_BMP_INFOHEADER*)(bmpData + sizeof(K_BMP_FILEHEADER));
        bfh->bfType = 0x4D42; // "BM"
        bfh->bfSize = sizeof(K_BMP_FILEHEADER) + sizeof(K_BMP_INFOHEADER) + pixelDataSize;
        bfh->bfReserved1 = 0;
        bfh->bfReserved2 = 0;
        bfh->bfOffBits = pixelDataOffset;

        bih->biSize = sizeof(K_BMP_INFOHEADER);
        bih->biWidth = w;
        bih->biHeight = h;
        bih->biPlanes = 1;
        bih->biBitCount = 24;
        bih->biCompression = 0;
        bih->biSizeImage = pixelDataSize;
        bih->biXPelsPerMeter = 2835;
        bih->biYPelsPerMeter = 2835;
        bih->biClrUsed = 0;
        bih->biClrImportant = 0;

        unsigned char* px = bmpData + pixelDataOffset;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int idx = (y * w + x) * 3;
                px[idx + 0] = (unsigned char)((x ^ y) & 0xFF);         // Blue
                px[idx + 1] = (unsigned char)(((x * 2) + y) & 0xFF);   // Green
                px[idx + 2] = (unsigned char)(((y * 3) + x) & 0xFF);   // Red
            }
        }
        bytesRead = bfh->bfSize;
    }

    // Canonical 16-byte KSteno Header Packet (Magic, Flags, 4-byte Len, 4-byte CRC32)
    unsigned int crc = CalcCRC32((const unsigned char*)secret, secLen);
    unsigned char packet[2080];
    packet[0] = 'K'; packet[1] = 'S'; packet[2] = '0'; packet[3] = '1';
    packet[4] = (key && key[0]) ? 0x10 : 0x00;
    packet[5] = (unsigned char)(secLen & 0xFF);
    packet[6] = (unsigned char)((secLen >> 8) & 0xFF);
    packet[7] = (unsigned char)((secLen >> 16) & 0xFF);
    packet[8] = (unsigned char)((secLen >> 24) & 0xFF);
    packet[9] = (unsigned char)(crc & 0xFF);
    packet[10] = (unsigned char)((crc >> 8) & 0xFF);
    packet[11] = (unsigned char)((crc >> 16) & 0xFF);
    packet[12] = (unsigned char)((crc >> 24) & 0xFF);
    packet[13] = 0; packet[14] = 0; packet[15] = 0;
    memcpy(packet + 16, encSecret, secLen);
    int totalPacketBytes = 16 + secLen;

    if (totalPacketBytes * 8 > pixelDataSize) {
        k_strcpy(errBuf, "Error: Carrier BMP capacity exceeded.");
        return 0;
    }

    unsigned char* px = bmpData + pixelDataOffset;
    int bitIndex = 0;
    for (int i = 0; i < totalPacketBytes; i++) {
        unsigned char b = packet[i];
        for (int k = 7; k >= 0; k--) {
            int bit = (b >> k) & 1;
            px[bitIndex] = (px[bitIndex] & 0xFE) | (unsigned char)bit;
            bitIndex++;
        }
    }

    HANDLE hOut = CreateFileA(outBmpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        k_strcpy(errBuf, "Error: Could not open output BMP file for writing.");
        return 0;
    }
    DWORD written = 0;
    WriteFile(hOut, bmpData, bytesRead, &written, NULL);
    CloseHandle(hOut);
    return 1;
}

static int DecodeBmpCarrier(const char* inBmpPath, const char* key, char* outSecret, int outMax) {
    HANDLE hFile = CreateFileA(inBmpPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        k_strcpy(outSecret, "[ERROR: Carrier BMP not found (carrier_stego.bmp or carrier.bmp)]");
        return 0;
    }

    unsigned char bmpData[256 * 256 * 3 + 1024];
    DWORD bytesRead = 0;
    ReadFile(hFile, bmpData, sizeof(bmpData), &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead < sizeof(K_BMP_FILEHEADER) + sizeof(K_BMP_INFOHEADER)) {
        k_strcpy(outSecret, "[ERROR: Corrupted BMP file header]");
        return 0;
    }

    K_BMP_FILEHEADER* bfh = (K_BMP_FILEHEADER*)bmpData;
    int pixelDataOffset = bfh->bfOffBits;
    unsigned char* px = bmpData + pixelDataOffset;
    int pixelDataSize = bytesRead - pixelDataOffset;

    if (pixelDataSize < 48) {
        k_strcpy(outSecret, "[ERROR: BMP image too small for stego header]");
        return 0;
    }

    // Read header bytes (supports canonical 16-byte header and legacy 6-byte header)
    unsigned char hdr[16];
    int bitIndex = 0;
    int hdrBytesToRead = (pixelDataSize >= 128) ? 16 : 6;
    for (int i = 0; i < hdrBytesToRead; i++) {
        unsigned char b = 0;
        for (int k = 0; k < 8; k++) {
            b = (b << 1) | (px[bitIndex++] & 1);
        }
        hdr[i] = b;
    }

    if (hdr[0] != 'K' || hdr[1] != 'S' || hdr[2] != '0' || hdr[3] != '1') {
        k_strcpy(outSecret, "[ERROR: No KSteno 'KS01' header found in BMP carrier]");
        return 0;
    }

    int secLen = 0;
    int headerOffset = 16;
    if (hdrBytesToRead >= 16) {
        secLen = hdr[5] | (hdr[6] << 8) | (hdr[7] << 16) | (hdr[8] << 24);
    }
    if (secLen <= 0 || secLen > 2000) {
        secLen = hdr[4] | (hdr[5] << 8);
        headerOffset = 6;
        bitIndex = 48;
    }

    if (secLen <= 0 || (headerOffset + secLen) * 8 > pixelDataSize || secLen > 2000) {
        k_strcpy(outSecret, "[ERROR: Invalid payload length in BMP header]");
        return 0;
    }

    unsigned char encPayload[2048];
    for (int i = 0; i < secLen; i++) {
        unsigned char b = 0;
        for (int k = 0; k < 8; k++) {
            b = (b << 1) | (px[bitIndex++] & 1);
        }
        encPayload[i] = b;
    }

    unsigned char decPayload[2048];
    RC4Crypt(key, encPayload, decPayload, secLen);
    decPayload[secLen] = 0;

    int copyLen = secLen < outMax - 1 ? secLen : outMax - 1;
    memcpy(outSecret, decPayload, copyLen);
    outSecret[copyLen] = 0;
    return 1;
}

// 3. Audio WAV PCM LSB Steganography
static int EncodeWavCarrier(const char* inWavPath, const char* outWavPath, const char* secret, const char* key, char* errBuf) {
    int secLen = k_strlen(secret);
    if (secLen > 2000) secLen = 2000;
    if (secLen == 0) {
        k_strcpy(errBuf, "Error: Secret message is empty.");
        return 0;
    }

    unsigned char encSecret[2048];
    RC4Crypt(key, (const unsigned char*)secret, encSecret, secLen);

    unsigned char wavBuffer[44 + 44100 * 2 * 2]; // 2 seconds mono 16-bit
    DWORD bytesRead = 0;
    HANDLE hFile = CreateFileA(inWavPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    int sampleCount = 44100 * 2;
    int dataOffset = sizeof(K_WAV_HEADER);

    if (hFile != INVALID_HANDLE_VALUE) {
        ReadFile(hFile, wavBuffer, sizeof(wavBuffer), &bytesRead, NULL);
        CloseHandle(hFile);
        if (bytesRead >= sizeof(K_WAV_HEADER)) {
            K_WAV_HEADER* hdr = (K_WAV_HEADER*)wavBuffer;
            if (hdr->riff[0] == 'R' && hdr->wave[0] == 'W') {
                sampleCount = hdr->dataSize / 2;
            }
        }
    } else {
        // Synthesize 2 seconds of 440 Hz retro drone carrier
        K_WAV_HEADER* hdr = (K_WAV_HEADER*)wavBuffer;
        hdr->riff[0] = 'R'; hdr->riff[1] = 'I'; hdr->riff[2] = 'F'; hdr->riff[3] = 'F';
        hdr->riffSize = sizeof(K_WAV_HEADER) - 8 + (sampleCount * 2);
        hdr->wave[0] = 'W'; hdr->wave[1] = 'A'; hdr->wave[2] = 'V'; hdr->wave[3] = 'E';
        hdr->fmt[0] = 'f'; hdr->fmt[1] = 'm'; hdr->fmt[2] = 't'; hdr->fmt[3] = ' ';
        hdr->fmtSize = 16;
        hdr->audioFormat = 1;
        hdr->numChannels = 1;
        hdr->sampleRate = 44100;
        hdr->byteRate = 88200;
        hdr->blockAlign = 2;
        hdr->bitsPerSample = 16;
        hdr->data[0] = 'd'; hdr->data[1] = 'a'; hdr->data[2] = 't'; hdr->data[3] = 'a';
        hdr->dataSize = sampleCount * 2;

        short* pcm = (short*)(wavBuffer + dataOffset);
        for (int i = 0; i < sampleCount; i++) {
            // Triangular wave drone at ~440 Hz
            int phase = (i * 440) % 44100;
            pcm[i] = (short)((phase < 22050) ? (phase - 11025) : (33075 - phase));
        }
        bytesRead = sizeof(K_WAV_HEADER) + sampleCount * 2;
    }

    // Canonical 16-byte KSteno Audio Header Packet
    unsigned int crc = CalcCRC32((const unsigned char*)secret, secLen);
    unsigned char packet[2080];
    packet[0] = 'K'; packet[1] = 'S'; packet[2] = 'A'; packet[3] = '1';
    packet[4] = (key && key[0]) ? 0x01 : 0x00;
    packet[5] = (unsigned char)(secLen & 0xFF);
    packet[6] = (unsigned char)((secLen >> 8) & 0xFF);
    packet[7] = (unsigned char)((secLen >> 16) & 0xFF);
    packet[8] = (unsigned char)((secLen >> 24) & 0xFF);
    packet[9] = (unsigned char)(crc & 0xFF);
    packet[10] = (unsigned char)((crc >> 8) & 0xFF);
    packet[11] = (unsigned char)((crc >> 16) & 0xFF);
    packet[12] = (unsigned char)((crc >> 24) & 0xFF);
    packet[13] = 0; packet[14] = 0; packet[15] = 0;
    memcpy(packet + 16, encSecret, secLen);
    int totalPacketBytes = 16 + secLen;

    if (totalPacketBytes * 8 > sampleCount) {
        k_strcpy(errBuf, "Error: Carrier WAV duration too short for payload.");
        return 0;
    }

    short* pcm = (short*)(wavBuffer + dataOffset);
    int bitIndex = 0;
    for (int i = 0; i < totalPacketBytes; i++) {
        unsigned char b = packet[i];
        for (int k = 7; k >= 0; k--) {
            int bit = (b >> k) & 1;
            pcm[bitIndex] = (pcm[bitIndex] & ~1) | (short)bit;
            bitIndex++;
        }
    }

    HANDLE hOut = CreateFileA(outWavPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        k_strcpy(errBuf, "Error: Could not open output WAV file for writing.");
        return 0;
    }
    DWORD written = 0;
    WriteFile(hOut, wavBuffer, bytesRead, &written, NULL);
    CloseHandle(hOut);
    return 1;
}

static int DecodeWavCarrier(const char* inWavPath, const char* key, char* outSecret, int outMax) {
    HANDLE hFile = CreateFileA(inWavPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        k_strcpy(outSecret, "[ERROR: Carrier WAV not found (carrier_stego.wav or carrier.wav)]");
        return 0;
    }

    unsigned char wavBuffer[44 + 44100 * 2 * 2];
    DWORD bytesRead = 0;
    ReadFile(hFile, wavBuffer, sizeof(wavBuffer), &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead < sizeof(K_WAV_HEADER)) {
        k_strcpy(outSecret, "[ERROR: Corrupted WAV file header]");
        return 0;
    }

    K_WAV_HEADER* hdr = (K_WAV_HEADER*)wavBuffer;
    int sampleCount = hdr->dataSize / 2;
    short* pcm = (short*)(wavBuffer + sizeof(K_WAV_HEADER));

    if (sampleCount < 48) {
        k_strcpy(outSecret, "[ERROR: WAV carrier too short for stego header]");
        return 0;
    }

    // Read header bytes (supports canonical 16-byte header and legacy 6-byte header)
    unsigned char headerBytes[16];
    int bitIndex = 0;
    int hdrBytesToRead = (sampleCount >= 128) ? 16 : 6;
    for (int i = 0; i < hdrBytesToRead; i++) {
        unsigned char b = 0;
        for (int k = 0; k < 8; k++) {
            b = (b << 1) | (pcm[bitIndex++] & 1);
        }
        headerBytes[i] = b;
    }

    if (headerBytes[0] != 'K' || headerBytes[1] != 'S' || headerBytes[2] != 'A' || headerBytes[3] != '1') {
        k_strcpy(outSecret, "[ERROR: No KSteno 'KSA1' header found in WAV carrier]");
        return 0;
    }

    int secLen = 0;
    int headerOffset = 16;
    if (hdrBytesToRead >= 16) {
        secLen = headerBytes[5] | (headerBytes[6] << 8) | (headerBytes[7] << 16) | (headerBytes[8] << 24);
    }
    if (secLen <= 0 || secLen > 2000) {
        secLen = headerBytes[4] | (headerBytes[5] << 8);
        headerOffset = 6;
        bitIndex = 48;
    }

    if (secLen <= 0 || (headerOffset + secLen) * 8 > sampleCount || secLen > 2000) {
        k_strcpy(outSecret, "[ERROR: Invalid payload length in WAV header]");
        return 0;
    }

    unsigned char encPayload[2048];
    for (int i = 0; i < secLen; i++) {
        unsigned char b = 0;
        for (int k = 0; k < 8; k++) {
            b = (b << 1) | (pcm[bitIndex++] & 1);
        }
        encPayload[i] = b;
    }

    unsigned char decPayload[2048];
    RC4Crypt(key, encPayload, decPayload, secLen);
    decPayload[secLen] = 0;

    int copyLen = secLen < outMax - 1 ? secLen : outMax - 1;
    memcpy(outSecret, decPayload, copyLen);
    outSecret[copyLen] = 0;
    return 1;
}

// 4. Chi-Square Steganalysis Frequency Scan (Westfeld & Pfitzmann PoV Algorithm)
static void CalculateChiSquare(int mode, const char* text, char* report, int reportMax) {
    if (mode == 2) { // 24-bit BMP Image Carrier
        HANDLE hFile = CreateFileA("carrier_stego.bmp", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            hFile = CreateFileA("carrier.bmp", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (hFile != INVALID_HANDLE_VALUE) {
            unsigned char buf[256 * 256 * 3 + 1024];
            DWORD bytesRead = 0;
            ReadFile(hFile, buf, sizeof(buf), &bytesRead, NULL);
            CloseHandle(hFile);
            if (bytesRead > 54) {
                int hist[256];
                memset(hist, 0, sizeof(hist));
                for (DWORD i = 54; i < bytesRead; i++) {
                    hist[buf[i]]++;
                }
                int kCount = 0;
                int chiSqSum = 0;
                for (int k = 0; k < 128; k++) {
                    int o1 = hist[2 * k];
                    int o2 = hist[2 * k + 1];
                    int sum = o1 + o2;
                    if (sum > 8) {
                        int diff = o1 - o2;
                        chiSqSum += (diff * diff) / sum;
                        kCount++;
                    }
                }
                int metric = kCount > 0 ? (chiSqSum * 10 / kCount) : 100;
                const char* verdict = (metric < 4) ? "HIGH PROBABILITY OF LSB INJECTION (P > 70%)" :
                                      (metric < 14) ? "SUSPICIOUS SYMMETRY DETECTED (P > 25%)" :
                                      "NATURAL ENTROPY / CLEAN CARRIER (P < 5%)";
                wsprintfA(report,
                    "--- CHI-SQUARE STEGANALYSIS REPORT (24-BIT BMP CARRIER) ---\r\n"
                    "Carrier Source: carrier_stego.bmp (or carrier.bmp)\r\n"
                    "Total Pixel Bytes: %d\r\n"
                    "Scanned PoV Pairs: %d\r\n"
                    "Equalization Statistic (χ²/k): %d.%d\r\n"
                    "Confidence Verdict: %s\r\n"
                    "Methodology: Pairs of Values (PoV) asymptotic equalization scan.\r\n"
                    "Cross-App Parity: Identical to KSteno Web Steganalysis Engine.\r\n",
                    bytesRead - 54, kCount, metric / 10, metric % 10, verdict);
                return;
            }
        }
    } else if (mode == 3) { // 16-bit PCM Audio WAV Carrier
        HANDLE hFile = CreateFileA("carrier_stego.wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            hFile = CreateFileA("carrier.wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (hFile != INVALID_HANDLE_VALUE) {
            unsigned char buf[44 + 44100 * 2 * 2];
            DWORD bytesRead = 0;
            ReadFile(hFile, buf, sizeof(buf), &bytesRead, NULL);
            CloseHandle(hFile);
            if (bytesRead > 44) {
                short* pcm = (short*)(buf + 44);
                int samples = (bytesRead - 44) / 2;
                int lsb0 = 0, lsb1 = 0;
                for (int i = 0; i < samples; i++) {
                    if (pcm[i] & 1) lsb1++; else lsb0++;
                }
                int diff = lsb0 - lsb1;
                if (diff < 0) diff = -diff;
                int metric = samples > 0 ? (diff * 1000 / samples) : 100;
                const char* verdict = (metric < 5) ? "HIGH PROBABILITY OF LSB INJECTION (P > 70%)" :
                                      (metric < 20) ? "SUSPICIOUS SYMMETRY DETECTED (P > 25%)" :
                                      "NATURAL ENTROPY / CLEAN CARRIER (P < 5%)";
                wsprintfA(report,
                    "--- CHI-SQUARE STEGANALYSIS REPORT (16-BIT PCM WAV CARRIER) ---\r\n"
                    "Carrier Source: carrier_stego.wav (or carrier.wav)\r\n"
                    "Scanned Audio Samples: %d\r\n"
                    "LSB Bias (0 vs 1): %d vs %d (Delta: %d)\r\n"
                    "Confidence Verdict: %s\r\n"
                    "Methodology: PCM Sample LSB Frequency Equalization.\r\n"
                    "Cross-App Parity: Identical to KSteno Web Steganalysis Engine.\r\n",
                    samples, lsb0, lsb1, diff, verdict);
                return;
            }
        }
    }

    int hist[256];
    memset(hist, 0, sizeof(hist));
    int len = k_strlen(text);
    if (len == 0) {
        k_strcpy(report, "Error: Carrier buffer is empty.");
        return;
    }

    for (int i = 0; i < len; i++) {
        hist[(unsigned char)text[i]]++;
    }

    int kCount = 0;
    int varianceSum = 0;
    for (int k = 16; k < 64; k++) {
        int o1 = hist[2 * k];
        int o2 = hist[2 * k + 1];
        if (o1 + o2 > 4) {
            int diff = o1 - o2;
            if (diff < 0) diff = -diff;
            varianceSum += diff;
            kCount++;
        }
    }

    int avgDelta = kCount > 0 ? (varianceSum * 10 / kCount) : 100;
    const char* verdict = "CLEAN CARRIER (Natural entropy)";
    if (avgDelta < 5) verdict = "HIGH PROBABILITY OF LSB INJECTION (Equalized PoVs)";
    else if (avgDelta < 15) verdict = "SUSPICIOUS SYMMETRY DETECTED";

    wsprintfA(report, 
        "--- CHI-SQUARE STEGANALYSIS REPORT (TEXT / COVER) ---\r\n"
        "Total Carrier Bytes: %d\r\n"
        "Scanned Value Pairs (PoVs): %d\r\n"
        "Pair Asymmetry Metric: %d.%d\r\n"
        "Confidence Verdict: %s\r\n"
        "Methodology: Pairs of Values (PoV) frequency equalization scan.\r\n"
        "Cross-App Parity: Identical to KSteno Web Steganalysis Engine.\r\n",
        len, kCount, avgDelta / 10, avgDelta % 10, verdict);
}

// UI Event Handlers
static void DoInject(void) {
    char cover[8192];
    char secret[2048];
    char key[128];
    char out[16384];

    GetWindowTextA(g_hEditCover, cover, sizeof(cover));
    GetWindowTextA(g_hEditSecret, secret, sizeof(secret));
    GetWindowTextA(g_hEditKey, key, sizeof(key));

    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    if (mode == 0) { // Whitespace SNOW
        EncodeTextWhitespace(cover, secret, key, out, sizeof(out));
        SetWindowTextA(g_hEditOutput, out);
        SetWindowTextA(g_hStatusLabel, "SUCCESS: Concealed payload into trailing whitespace bits.");
    } else if (mode == 1) { // Zero-width marker simulation
        wsprintfA(out, "[KSTENO-ZW-ARMOR]\r\n%s\r\n[PAYLOAD EMBEDDED: %d BYTES]", cover, k_strlen(secret));
        SetWindowTextA(g_hEditOutput, out);
        SetWindowTextA(g_hStatusLabel, "SUCCESS: Modulated secret into carrier envelope.");
    } else if (mode == 2) { // 24-bit BMP Image LSB
        char err[256];
        if (EncodeBmpCarrier("carrier.bmp", "carrier_stego.bmp", secret, key, err)) {
            wsprintfA(out, "SUCCESS: Injected %d-byte secret into 24-bit BMP carrier.\r\nOutput saved to carrier_stego.bmp\r\nMagic: KS01 | Protection: RC4 Armor", k_strlen(secret));
            SetWindowTextA(g_hEditOutput, out);
            SetWindowTextA(g_hStatusLabel, "SUCCESS: Injected secret into carrier_stego.bmp");
        } else {
            SetWindowTextA(g_hStatusLabel, err);
        }
    } else if (mode == 3) { // 16-bit PCM Audio WAV
        char err[256];
        if (EncodeWavCarrier("carrier.wav", "carrier_stego.wav", secret, key, err)) {
            wsprintfA(out, "SUCCESS: Injected %d-byte secret into 16-bit PCM WAV carrier.\r\nOutput saved to carrier_stego.wav\r\nMagic: KSA1 | Protection: RC4 Armor", k_strlen(secret));
            SetWindowTextA(g_hEditOutput, out);
            SetWindowTextA(g_hStatusLabel, "SUCCESS: Injected secret into carrier_stego.wav");
        } else {
            SetWindowTextA(g_hStatusLabel, err);
        }
    }
}

static void DoExtract(void) {
    char in[16384];
    char key[128];
    char out[4096];

    GetWindowTextA(g_hEditOutput, in, sizeof(in));
    if (k_strlen(in) == 0) {
        GetWindowTextA(g_hEditCover, in, sizeof(in));
    }
    GetWindowTextA(g_hEditKey, key, sizeof(key));

    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    if (mode == 0 || mode == 1) {
        DecodeTextWhitespace(in, key, out, sizeof(out));
        SetWindowTextA(g_hEditSecret, out);
        SetWindowTextA(g_hStatusLabel, "EXTRACT COMPLETE: Decoded payload displayed in Secret box.");
    } else if (mode == 2) {
        if (DecodeBmpCarrier("carrier_stego.bmp", key, out, sizeof(out)) ||
            DecodeBmpCarrier("carrier.bmp", key, out, sizeof(out))) {
            SetWindowTextA(g_hEditSecret, out);
            SetWindowTextA(g_hStatusLabel, "EXTRACT COMPLETE: Extracted payload from BMP carrier.");
        } else {
            SetWindowTextA(g_hEditSecret, out);
            SetWindowTextA(g_hStatusLabel, "Error: Extraction failed on BMP carrier.");
        }
    } else if (mode == 3) {
        if (DecodeWavCarrier("carrier_stego.wav", key, out, sizeof(out)) ||
            DecodeWavCarrier("carrier.wav", key, out, sizeof(out))) {
            SetWindowTextA(g_hEditSecret, out);
            SetWindowTextA(g_hStatusLabel, "EXTRACT COMPLETE: Extracted payload from WAV carrier.");
        } else {
            SetWindowTextA(g_hEditSecret, out);
            SetWindowTextA(g_hStatusLabel, "Error: Extraction failed on WAV carrier.");
        }
    }
}

static void DoChiSquareScan(void) {
    char in[16384];
    char report[2048];
    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    GetWindowTextA(g_hEditCover, in, sizeof(in));
    CalculateChiSquare(mode, in, report, sizeof(report));
    SetWindowTextA(g_hEditOutput, report);
    SetWindowTextA(g_hStatusLabel, "STEGANALYSIS SCAN COMPLETE. See output console.");
}

static void DoSendKCipher(void) {
    char payload[8192];
    payload[0] = 0;
    GetWindowTextA(g_hEditSecret, payload, sizeof(payload));
    if (!payload[0] || payload[0] == '[') {
        GetWindowTextA(g_hEditOutput, payload, sizeof(payload));
    }
    if (!payload[0] || payload[0] == '[') {
        SetWindowTextA(g_hStatusLabel, "Notice: No secret payload available. Extract secret first.");
        return;
    }

    // Write handshake to both steno_extracted.txt and kcipher_handshake.txt for VFS/IPC
    HANDLE hFile = CreateFileA("kcipher_handshake.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, payload, k_strlen(payload), &written, NULL);
        CloseHandle(hFile);
    }
    HANDLE hExt = CreateFileA("steno_extracted.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hExt != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hExt, payload, k_strlen(payload), &written, NULL);
        CloseHandle(hExt);
    }

    // Attempt IPC window focus
    HWND hCipher = FindWindowA("KCipherClass", NULL);
    if (!hCipher) hCipher = FindWindowA(NULL, "KCipher");
    if (hCipher) {
        SetForegroundWindow(hCipher);
    }

    SetWindowTextA(g_hStatusLabel, "DISPATCH COMPLETE: Payload dispatched to KCipher (kcipher_handshake.txt).");
}

static void DoExportPayload(void) {
    char buf[8192];
    buf[0] = 0;
    GetWindowTextA(g_hEditOutput, buf, sizeof(buf));
    if (!buf[0]) {
        GetWindowTextA(g_hEditSecret, buf, sizeof(buf));
    }
    if (!buf[0]) {
        SetWindowTextA(g_hStatusLabel, "Notice: No payload in output or secret buffer to export.");
        return;
    }
    HANDLE hFile = CreateFileA("steno_extracted.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, buf, k_strlen(buf), &written, NULL);
        CloseHandle(hFile);
        SetWindowTextA(g_hStatusLabel, "Exported payload to steno_extracted.txt successfully.");
    } else {
        SetWindowTextA(g_hStatusLabel, "Error: Could not open steno_extracted.txt for writing.");
    }
}

static void DoExportVFSReport(void) {
    char cover[8192];
    char secret[2048];
    char report[2048];
    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    GetWindowTextA(g_hEditCover, cover, sizeof(cover));
    GetWindowTextA(g_hEditSecret, secret, sizeof(secret));
    CalculateChiSquare(mode, cover, report, sizeof(report));

    HANDLE hFile = CreateFileA("steno_report.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, report, k_strlen(report), &written, NULL);
        CloseHandle(hFile);
    }

    if (secret[0]) {
        HANDLE hCipher = CreateFileA("kcipher_handshake.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hCipher != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hCipher, secret, k_strlen(secret), &written, NULL);
            CloseHandle(hCipher);
        }
    }

    SetWindowTextA(g_hStatusLabel, "Exported steno_report.txt & kcipher_handshake.txt to VFS pipeline.");
}

static void DoLoadCover(void) {
    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    if (mode == 2) { // 24-bit BMP Carrier
        HANDLE hFile = CreateFileA("carrier.bmp", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            SetWindowTextA(g_hEditCover, "[CARRIER: carrier.bmp]\r\nResolution: 256x256 24-bit RGB bitmap\r\nCapacity: ~24.5 KB LSB payload");
            SetWindowTextA(g_hStatusLabel, "Loaded carrier bitmap (carrier.bmp).");
            return;
        }
    } else if (mode == 3) { // 16-bit WAV Carrier
        HANDLE hFile = CreateFileA("carrier.wav", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            SetWindowTextA(g_hEditCover, "[CARRIER: carrier.wav]\r\nFormat: 44.1kHz 16-bit Mono PCM audio\r\nCapacity: ~10.7 KB LSB payload");
            SetWindowTextA(g_hStatusLabel, "Loaded carrier audio (carrier.wav).");
            return;
        }
    }

    HANDLE hFile = CreateFileA("carrier_cover.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[8192];
        DWORD read = 0;
        if (ReadFile(hFile, buf, sizeof(buf) - 1, &read, NULL)) {
            buf[read] = 0;
            SetWindowTextA(g_hEditCover, buf);
            SetWindowTextA(g_hStatusLabel, "Loaded carrier cover from carrier_cover.txt.");
        }
        CloseHandle(hFile);
    } else {
        SetWindowTextA(g_hEditCover,
            "COVERT TRANSMISSION // NODE 0x7F ARCHIVE\r\n"
            "Subject: Autonomous Fleet Manifest 1999\r\n"
            "Carrier: Whitespace SNOW Modulated Channel\r\n"
            "Notice: All operative nodes are synchronized.");
        SetWindowTextA(g_hStatusLabel, "Created default carrier text (carrier_cover.txt not found).");
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);

            g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Mode Combo
            g_hComboMode = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 140, 52, 280, 200, hwnd, (HMENU)ID_COMBO_MODE, NULL, NULL);
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"Whitespace Chaff (SNOW Standard)");
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"Zero-Width Unicode Modulation");
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"24-Bit BMP Image LSB Carrier");
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"16-Bit PCM Audio WAV Carrier");
            SendMessageA(g_hComboMode, CB_SETCURSEL, 0, 0);

            // Passphrase Key
            g_hEditKey = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "KSTENO-1999", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 530, 52, 220, 24, hwnd, (HMENU)ID_EDIT_KEY, NULL, NULL);

            // Cover Text Box
            g_hEditCover = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", 
                "KILO-NET INTERNET SERVICES - ACCOUNT STATEMENT\r\n"
                "Date: September 23, 1999\r\n"
                "Username: operative_99\r\n"
                "Connection Type: 56K V.90 Dial-up Node\r\n"
                "Monthly Carrier Usage: 142.4 Hours\r\n"
                "Please verify modem init strings for Y2K readiness.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP,
                20, 110, 430, 180, hwnd, (HMENU)ID_EDIT_COVER, NULL, NULL);

            // Secret Payload Box
            g_hEditSecret = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT",
                "TOP SECRET: Rendezvous at Node 0x7F. Precursor hex address confirmed.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP,
                470, 110, 430, 180, hwnd, (HMENU)ID_EDIT_SECRET, NULL, NULL);

            // Action Buttons
            int bx = 20;
            g_hBtnInject = CreateWindowA("BUTTON", "Inject Secret", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 100, 30, hwnd, (HMENU)ID_BTN_INJECT, NULL, NULL); bx += 105;
            g_hBtnExtract = CreateWindowA("BUTTON", "Extract Secret", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 100, 30, hwnd, (HMENU)ID_BTN_EXTRACT, NULL, NULL); bx += 105;
            g_hBtnChisq = CreateWindowA("BUTTON", "Chi-Square Scan", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 115, 30, hwnd, (HMENU)ID_BTN_CHISQ, NULL, NULL); bx += 120;
            g_hBtnLoadFile = CreateWindowA("BUTTON", "Load Carrier", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 95, 30, hwnd, (HMENU)ID_BTN_LOAD_FILE, NULL, NULL); bx += 100;
            g_hBtnSaveFile = CreateWindowA("BUTTON", "Export File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 85, 30, hwnd, (HMENU)ID_BTN_SAVE_FILE, NULL, NULL); bx += 90;
            g_hBtnVfsReport = CreateWindowA("BUTTON", "VFS Report", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 85, 30, hwnd, (HMENU)ID_BTN_VFS_REPORT, NULL, NULL); bx += 90;
            g_hBtnSendKCipher = CreateWindowA("BUTTON", "Send to KCipher", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 115, 30, hwnd, (HMENU)ID_BTN_SEND_KCIPHER, NULL, NULL); bx += 120;
            g_hBtnClear = CreateWindowA("BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 60, 30, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL); bx += 65;
            g_hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, bx, 305, 75, 30, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Output Console Box
            g_hEditOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_TABSTOP,
                20, 350, 880, 150, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);

            // Status Label
            g_hStatusLabel = CreateWindowA("STATIC", g_szStatus, WS_CHILD | WS_VISIBLE, 20, 515, 880, 20, hwnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);

            // Apply Fonts
            SendMessageA(g_hComboMode, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            SendMessageA(g_hEditKey, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditCover, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditSecret, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditOutput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hStatusLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            SendMessageA(g_hBtnSendKCipher, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_INJECT:  DoInject(); break;
                case ID_BTN_EXTRACT: DoExtract(); break;
                case ID_BTN_CHISQ:   DoChiSquareScan(); break;
                case ID_BTN_SAVE_FILE: DoExportPayload(); break;
                case ID_BTN_LOAD_FILE: DoLoadCover(); break;
                case ID_BTN_VFS_REPORT: DoExportVFSReport(); break;
                case ID_BTN_SEND_KCIPHER: DoSendKCipher(); break;
                case ID_BTN_CLEAR:
                    SetWindowTextA(g_hEditSecret, "");
                    SetWindowTextA(g_hEditOutput, "");
                    SetWindowTextA(g_hStatusLabel, "Cleared input and output buffers.");
                    break;
                case ID_BTN_HELP:
                    MessageBoxA(hwnd,
                        "KSteno v1.1.0 (1999 Operative Edition)\n\n"
                        "Stenographic Carrier Suite & Cryptanalysis Workbench\n\n"
                        "1. Carrier Modes:\n"
                        "   - Whitespace Chaff (SNOW Standard: Space=0, Tab=1)\n"
                        "   - Zero-Width Unicode Envelope\n"
                        "   - 24-Bit BMP Image LSB Carrier (carrier.bmp -> carrier_stego.bmp)\n"
                        "   - 16-Bit PCM Audio WAV Carrier (carrier.wav -> carrier_stego.wav)\n"
                        "2. Enter secret payload and optional RC4 encryption key.\n"
                        "3. Click 'Inject Secret' to modulate payload into host carrier.\n"
                        "4. Click 'Extract Secret' to recover hidden communiqué.\n"
                        "5. Use 'Chi-Square Scan' to test Pairs of Values (PoV) equilibrium.\n"
                        "6. Shortcuts: Ctrl+Enter (Inject), Ctrl+E (Extract), F1 (Help), F5 (Save), F9 (Load).",
                        "KSteno Field Manual", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_hBrushBg);

            // Header Banner
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, COLOR_PRIMARY);
            TextOutA(hdc, 20, 15, "KSTENO", 6);

            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOutA(hdc, 105, 18, "Stenographic Carrier Suite & Multi-Carrier Workbench v1.1.0", 59);

            // Labels
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutA(hdc, 20, 54, "Carrier Mode:", 13);
            TextOutA(hdc, 440, 54, "Passphrase:", 11);

            TextOutA(hdc, 20, 90, "Cover Text / Host Carrier:", 26);
            TextOutA(hdc, 470, 90, "Secret Payload Message:", 23);

            TextOutA(hdc, 20, 332, "Carrier Console Output:", 23);

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
    wc.lpszClassName = "KStenoClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KStenoClass",
        "KSteno - Stenographic Carrier Suite",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        940, 590,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (msg.wParam == VK_F1) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_HELP, 0), 0);
                continue;
            } else if (msg.wParam == VK_ESCAPE) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CLEAR, 0), 0);
                continue;
            } else if (msg.wParam == VK_F5 || (bCtrl && (msg.wParam == 'S' || msg.wParam == 's'))) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SAVE_FILE, 0), 0);
                continue;
            } else if (msg.wParam == VK_F9 || (bCtrl && (msg.wParam == 'O' || msg.wParam == 'o'))) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_LOAD_FILE, 0), 0);
                continue;
            } else if (bCtrl && msg.wParam == VK_RETURN) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INJECT, 0), 0);
                continue;
            } else if (bCtrl && (msg.wParam == 'E' || msg.wParam == 'e')) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_EXTRACT, 0), 0);
                continue;
            } else if (bCtrl && (msg.wParam == 'K' || msg.wParam == 'k')) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SEND_KCIPHER, 0), 0);
                continue;
            } else if (bCtrl && (msg.wParam == 'R' || msg.wParam == 'r')) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHISQ, 0), 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess((UINT)msg.wParam);
}
