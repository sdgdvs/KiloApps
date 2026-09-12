#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#ifndef ES_AUTOHSCRAWL
#define ES_AUTOHSCRAWL 0x0080L
#endif

#define W 900
#define H 650
#define MAX_FILES 100
#define MAX_FILE_SIZE (100 * 1024 * 1024) // 100MB limit per file

HWND hListBox, hEditSearch, hChkRegex, hEditPassword, hComboCompress;
HWND hBtnOpen, hBtnAdd, hBtnRemove, hBtnPack, hBtnExtractSel, hBtnExtractAll, hBtnBatchExtract, hBtnVerify, hBtnPreview, hBtnDemo, hBtnHelp;
HWND hStatus, hHeader, hMainWnd;
HFONT hFont, hMonoFont;
WNDPROC g_pfnOrigSearchProc = NULL;

typedef struct {
    char name[256];
    DWORD uncompSize;
    DWORD compSize;
    DWORD crc32;
    DWORD method; // 0 = Store, 1 = RLE
    char* data;   // Holds uncompressed data in memory
} KFile;

KFile archive[MAX_FILES];
int numFiles = 0;
int visibleIndices[MAX_FILES];
int numVisible = 0;

// Custom intrinsic implementations for no-CRT build
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

char FastToLower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

// Sanitize filename to prevent directory traversal or invalid characters
void SanitizeFilename(const char* inName, char* outBuf, size_t outSize) {
    if (!inName || !outBuf || outSize == 0) return;
    const char* p = inName;
    for (int i = 0; inName[i]; i++) {
        if (inName[i] == '\\' || inName[i] == '/') p = inName + i + 1;
    }
    while (*p == '.' || *p == ' ' || *p == '/' || *p == '\\') p++;
    if (!*p) p = "extracted_file";

    size_t idx = 0;
    for (; p[idx] && idx < outSize - 1; idx++) {
        char c = p[idx];
        if (c == '<' || c == '>' || c == ':' || c == '"' || c == '|' || c == '?' || c == '*') c = '_';
        outBuf[idx] = c;
    }
    outBuf[idx] = '\0';
}

// CRC32 Calculation
DWORD CalculateCRC32(const unsigned char* data, DWORD size) {
    if (!data && size > 0) return 0;
    DWORD crc = 0xFFFFFFFF;
    for (DWORD i = 0; i < size; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// Simple RLE Compression with bounds safety
DWORD CompressRLE(const unsigned char* in, DWORD inSize, unsigned char* out) {
    if (!in || !out || inSize == 0) return 0;
    DWORD inIdx = 0, outIdx = 0;
    while (inIdx < inSize) {
        unsigned char b = in[inIdx];
        DWORD run = 1;
        while (inIdx + run < inSize && in[inIdx + run] == b && run < 255) {
            run++;
        }
        if (run >= 3 || b == 0xFF) {
            out[outIdx++] = 0xFF;
            out[outIdx++] = (unsigned char)run;
            out[outIdx++] = b;
            inIdx += run;
        } else {
            for (DWORD r = 0; r < run; r++) {
                out[outIdx++] = b;
            }
            inIdx += run;
        }
    }
    return outIdx;
}

// Simple RLE Decompression with strict bounds safety
DWORD DecompressRLE(const unsigned char* in, DWORD inSize, unsigned char* out, DWORD outCapacity) {
    if (!in || !out || inSize == 0 || outCapacity == 0) return 0;
    DWORD inIdx = 0, outIdx = 0;
    while (inIdx < inSize && outIdx < outCapacity) {
        unsigned char b = in[inIdx++];
        if (b == 0xFF) {
            if (inIdx + 2 > inSize) break;
            unsigned char count = in[inIdx++];
            unsigned char val = in[inIdx++];
            for (DWORD i = 0; i < count && outIdx < outCapacity; i++) {
                out[outIdx++] = val;
            }
        } else {
            out[outIdx++] = b;
        }
    }
    return outIdx;
}

// Simple XOR Cipher for Password Protection Simulation
void CryptData(char* data, DWORD size, DWORD key) {
    if (!data || key == 0) return;
    for (DWORD i = 0; i < size; i++) {
        BYTE k = (BYTE)((key + i * 31 + (key >> (i % 8))) & 0xFF);
        data[i] ^= k;
    }
}

// Case-insensitive substring match
int ContainsString(const char* str, const char* sub) {
    if (!sub || !*sub) return 1;
    if (!str) return 0;
    char s1[256], s2[256];
    int i = 0;
    for (; str[i] && i < 255; i++) s1[i] = FastToLower(str[i]);
    s1[i] = '\0';
    for (i = 0; sub[i] && i < 255; i++) s2[i] = FastToLower(sub[i]);
    s2[i] = '\0';

    const char* p1 = s1;
    while (*p1) {
        const char* p1_b = p1;
        const char* p2 = s2;
        while (*p1_b && *p2 && (*p1_b == *p2)) {
            p1_b++;
            p2++;
        }
        if (!*p2) return 1;
        p1++;
    }
    return 0;
}

int RegexMatch(const char* str, const char* pattern) {
    if (!pattern || !*pattern) return 1;
    if (!str) return 0;
    while (*pattern) {
        if (*pattern == '*') {
            while (*pattern == '*') pattern++;
            if (!*pattern) return 1;
            while (*str) {
                if (RegexMatch(str, pattern)) return 1;
                str++;
            }
            return 0;
        } else if (*pattern == '.' || FastToLower(*str) == FastToLower(*pattern)) {
            if (!*str) return 0;
            str++;
            pattern++;
        } else {
            return 0;
        }
    }
    return !*str;
}

void UpdateWindowTitle() {
    if (!hMainWnd) return;
    char title[160];
    if (numFiles == 0) {
        lstrcpyA(title, "KZip Archiver Studio - [Empty] - (Press F1 for Help)");
    } else {
        wsprintfA(title, "KZip Archiver Studio - %d file(s) loaded - [F1 Help]", numFiles);
    }
    SetWindowTextA(hMainWnd, title);
}

void ShowHelp(HWND hwnd) {
    MessageBoxA(hwnd,
        "KZip Archiver Studio - User & Keyboard Reference\n"
        "================================================\n\n"
        "ARCHIVE OPERATIONS:\n"
        "  * Open [Ctrl+O]: Load an existing .kza compressed archive\n"
        "  * Add File [Ctrl+N]: Add files into current archive (or Drag & Drop)\n"
        "  * Remove [Del]: Remove selected file from archive\n"
        "  * Pack [Ctrl+S]: Compress and save archive to .kza file\n"
        "  * Ext. Sel [Enter]: Extract selected file to current directory\n"
        "  * Ext. All [Ctrl+E]: Extract all files to current directory\n"
        "  * Batch Ext [B]: Unpack multiple .kza archives in batch\n"
        "  * Verify [V]: Check CRC32 checksums against file corruption\n"
        "  * Preview [P]: Inspect file header, compression stats & hex dump\n"
        "  * Demo [D]: Load sample bundle (3 files) for instant testing\n\n"
        "SEARCH & COMPRESSION:\n"
        "  * Filter: Instant live search (wildcard * and . supported with Regex)\n"
        "  * Method: RLE Fast (run-length compression) or Store (uncompressed)\n"
        "  * Password: Enter password before packing to apply XOR cipher\n\n"
        "GLOBAL KEYBOARD ACCELERATORS:\n"
        "  * F1 or 'H': Show this Help & Shortcuts guide\n"
        "  * Ctrl+O: Open archive\n"
        "  * Ctrl+S: Pack archive\n"
        "  * Ctrl+E: Extract all files\n"
        "  * Ctrl+N: Add file\n"
        "  * Enter / Double-Click: Preview selected file\n"
        "  * Delete: Remove selected file\n"
        "  * Escape: Clear search filter\n"
        "  * V: Verify CRC32 checksums\n"
        "  * P: Deep Preview\n"
        "  * B: Batch Extract\n"
        "  * D: Load Demo Files",
        "KZip Archiver Studio - Help",
        MB_OK | MB_ICONINFORMATION);
}

void RefreshList() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    numVisible = 0;

    char filter[128] = {0};
    GetWindowTextA(hEditSearch, filter, sizeof(filter));

    int useRegex = SendMessage(hChkRegex, BM_GETCHECK, 0, 0) == BST_CHECKED;
    DWORD totalUncomp = 0, totalComp = 0;

    for (int i = 0; i < numFiles; i++) {
        totalUncomp += archive[i].uncompSize;
        totalComp += archive[i].compSize;

        int matched = 1;
        if (filter[0] != '\0') {
            matched = useRegex ? RegexMatch(archive[i].name, filter) : ContainsString(archive[i].name, filter);
        }

        if (matched) {
            visibleIndices[numVisible] = i;

            int ratio = 0;
            if (archive[i].uncompSize > 0) {
                ratio = 100 - (int)((archive[i].compSize * 100) / archive[i].uncompSize);
                if (ratio < 0) ratio = 0;
            }

            char itemBuf[384];
            wsprintfA(itemBuf, "  %-30s | %10lu B -> %10lu B (%3d%%) | 0x%08X | %-6s",
                archive[i].name,
                archive[i].uncompSize,
                archive[i].compSize,
                ratio,
                archive[i].crc32,
                archive[i].method == 1 ? "RLE" : "Store");

            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)itemBuf);
            numVisible++;
        }
    }

    if (numFiles == 0) {
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"  (Archive is empty. Click 'Add File', 'Demo', or drag & drop files here)");
    } else if (numVisible == 0) {
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"  (No files match current search filter. Press Esc in search to clear)");
    }

    int overallRatio = 0;
    if (totalUncomp > 0) {
        overallRatio = 100 - (int)((totalComp * 100) / totalUncomp);
        if (overallRatio < 0) overallRatio = 0;
    }

    char statusBuf[256];
    if (numFiles == 0) {
        lstrcpyA(statusBuf, "Ready. Archive is empty (Drag & drop files or click 'Add [Ctrl+N]' or 'Demo [D]'). Press F1 for Help.");
    } else {
        wsprintfA(statusBuf, "Files: %d | Raw: %lu B -> Packed: %lu B | Saved: %d%% | Double-click/Enter to preview | F1 for Help",
            numFiles, totalUncomp, totalComp, overallRatio);
    }
    SetWindowTextA(hStatus, statusBuf);
    UpdateWindowTitle();
}

void ClearArchive() {
    for (int i = 0; i < numFiles; i++) {
        if (archive[i].data) {
            HeapFree(GetProcessHeap(), 0, archive[i].data);
            archive[i].data = NULL;
        }
    }
    numFiles = 0;
    RefreshList();
}

void AddFileToArchive(const char* filepath) {
    if (numFiles >= MAX_FILES) {
        MessageBoxA(NULL, "Archive limit reached (Max 100 files).", "KZip Error", MB_OK | MB_ICONERROR);
        return;
    }
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to open target file.", "KZip Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD size = GetFileSize(hFile, NULL);
    if (size > MAX_FILE_SIZE) {
        CloseHandle(hFile);
        MessageBoxA(NULL, "File exceeds maximum size limit (100MB).", "KZip Error", MB_OK | MB_ICONERROR);
        return;
    }

    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size ? size : 1);
    if (!buf) {
        CloseHandle(hFile);
        MessageBoxA(NULL, "Out of memory allocating file buffer.", "KZip Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD read = 0;
    if (size > 0) {
        ReadFile(hFile, buf, size, &read, NULL);
        if (read != size) {
            HeapFree(GetProcessHeap(), 0, buf);
            CloseHandle(hFile);
            MessageBoxA(NULL, "Failed to read full file contents.", "KZip Error", MB_OK | MB_ICONERROR);
            return;
        }
    }
    CloseHandle(hFile);

    const char* filename = filepath;
    for (int i = 0; filepath[i]; i++) {
        if (filepath[i] == '\\' || filepath[i] == '/') filename = filepath + i + 1;
    }

    DWORD compressMode = (DWORD)SendMessage(hComboCompress, CB_GETCURSEL, 0, 0); // 0 = Store, 1 = RLE

    lstrcpynA(archive[numFiles].name, filename, sizeof(archive[numFiles].name));
    archive[numFiles].uncompSize = size;
    archive[numFiles].crc32 = CalculateCRC32((const unsigned char*)buf, size);
    archive[numFiles].data = buf;

    if (compressMode == 1 && size > 0) {
        unsigned char* compBuf = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size * 3 + 16);
        if (compBuf) {
            DWORD compLen = CompressRLE((const unsigned char*)buf, size, compBuf);
            if (compLen > 0 && compLen < size) {
                archive[numFiles].compSize = compLen;
                archive[numFiles].method = 1;
            } else {
                archive[numFiles].compSize = size;
                archive[numFiles].method = 0;
            }
            HeapFree(GetProcessHeap(), 0, compBuf);
        } else {
            archive[numFiles].compSize = size;
            archive[numFiles].method = 0;
        }
    } else {
        archive[numFiles].compSize = size;
        archive[numFiles].method = 0;
    }

    numFiles++;
    RefreshList();
}

void PackArchive(const char* filepath) {
    if (numFiles == 0) {
        MessageBoxA(NULL, "No files to pack!", "KZip", MB_OK | MB_ICONWARNING);
        return;
    }

    HANDLE hFile = CreateFileA(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to create output archive file.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char pass[128] = {0};
    GetWindowTextA(hEditPassword, pass, sizeof(pass));
    DWORD pwdHash = 0;
    DWORD flags = 0;

    if (pass[0] != '\0') {
        pwdHash = CalculateCRC32((const unsigned char*)pass, lstrlenA(pass));
        flags |= 0x01; // Encrypted flag
    }

    DWORD written = 0;
    WriteFile(hFile, "KZA2", 4, &written, NULL);
    WriteFile(hFile, &flags, sizeof(DWORD), &written, NULL);
    WriteFile(hFile, &pwdHash, sizeof(DWORD), &written, NULL);
    WriteFile(hFile, &numFiles, sizeof(DWORD), &written, NULL);

    for (int i = 0; i < numFiles; i++) {
        DWORD nameLen = (DWORD)lstrlenA(archive[i].name) + 1;
        WriteFile(hFile, &nameLen, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, archive[i].name, nameLen, &written, NULL);
        WriteFile(hFile, &archive[i].uncompSize, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, &archive[i].compSize, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, &archive[i].crc32, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, &archive[i].method, sizeof(DWORD), &written, NULL);

        char* payload = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, archive[i].compSize ? archive[i].compSize : 1);
        if (payload) {
            if (archive[i].method == 1 && archive[i].uncompSize > 0) {
                CompressRLE((const unsigned char*)archive[i].data, archive[i].uncompSize, (unsigned char*)payload);
            } else if (archive[i].uncompSize > 0 && archive[i].data) {
                memcpy(payload, archive[i].data, archive[i].uncompSize);
            }

            if (flags & 0x01) {
                CryptData(payload, archive[i].compSize, pwdHash);
            }

            WriteFile(hFile, payload, archive[i].compSize, &written, NULL);
            HeapFree(GetProcessHeap(), 0, payload);
        }
    }

    CloseHandle(hFile);
    MessageBoxA(NULL, "Archive packed successfully!", "KZip", MB_OK | MB_ICONINFORMATION);
}

void OpenArchive(const char* filepath) {
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to open archive file.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char magic[4] = {0};
    DWORD read = 0;
    ReadFile(hFile, magic, 4, &read, NULL);

    BOOL isV2 = (magic[0] == 'K' && magic[1] == 'Z' && magic[2] == 'A' && magic[3] == '2');
    BOOL isV1 = (magic[0] == 'K' && magic[1] == 'Z' && magic[2] == 'A' && magic[3] == '\0');

    if (!isV1 && !isV2) {
        CloseHandle(hFile);
        MessageBoxA(NULL, "Invalid or unsupported KZA archive format.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD flags = 0, pwdHash = 0, fileCount = 0;

    if (isV2) {
        ReadFile(hFile, &flags, sizeof(DWORD), &read, NULL);
        ReadFile(hFile, &pwdHash, sizeof(DWORD), &read, NULL);
        ReadFile(hFile, &fileCount, sizeof(DWORD), &read, NULL);

        if (flags & 0x01) {
            char userPass[128] = {0};
            GetWindowTextA(hEditPassword, userPass, sizeof(userPass));
            DWORD userHash = CalculateCRC32((const unsigned char*)userPass, lstrlenA(userPass));
            if (userHash != pwdHash) {
                CloseHandle(hFile);
                MessageBoxA(NULL, "Archive is password protected! Please enter the correct password in the Pass field.", "Access Denied", MB_OK | MB_ICONERROR);
                return;
            }
        }
    } else {
        ReadFile(hFile, &fileCount, sizeof(DWORD), &read, NULL);
    }

    ClearArchive();

    for (DWORD i = 0; i < fileCount && i < MAX_FILES; i++) {
        DWORD nameLen = 0;
        ReadFile(hFile, &nameLen, sizeof(DWORD), &read, NULL);
        if (nameLen > sizeof(archive[i].name) || nameLen == 0) break;

        ReadFile(hFile, archive[i].name, nameLen, &read, NULL);
        archive[i].name[sizeof(archive[i].name) - 1] = '\0';

        if (isV2) {
            ReadFile(hFile, &archive[i].uncompSize, sizeof(DWORD), &read, NULL);
            ReadFile(hFile, &archive[i].compSize, sizeof(DWORD), &read, NULL);
            ReadFile(hFile, &archive[i].crc32, sizeof(DWORD), &read, NULL);
            ReadFile(hFile, &archive[i].method, sizeof(DWORD), &read, NULL);
        } else {
            ReadFile(hFile, &archive[i].uncompSize, sizeof(DWORD), &read, NULL);
            archive[i].compSize = archive[i].uncompSize;
            archive[i].method = 0;
        }

        if (archive[i].compSize > MAX_FILE_SIZE || archive[i].uncompSize > MAX_FILE_SIZE) {
            break; // Exceeds safe bounds
        }

        char* payload = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, archive[i].compSize ? archive[i].compSize : 1);
        if (!payload) break;

        ReadFile(hFile, payload, archive[i].compSize, &read, NULL);
        if (read != archive[i].compSize) {
            HeapFree(GetProcessHeap(), 0, payload);
            break;
        }

        if (isV2 && (flags & 0x01)) {
            CryptData(payload, archive[i].compSize, pwdHash);
        }

        archive[i].data = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, archive[i].uncompSize ? archive[i].uncompSize : 1);
        if (!archive[i].data) {
            HeapFree(GetProcessHeap(), 0, payload);
            break;
        }

        if (archive[i].method == 1 && archive[i].uncompSize > 0) {
            DecompressRLE((const unsigned char*)payload, archive[i].compSize, (unsigned char*)archive[i].data, archive[i].uncompSize);
        } else if (archive[i].uncompSize > 0) {
            memcpy(archive[i].data, payload, archive[i].uncompSize);
        }

        HeapFree(GetProcessHeap(), 0, payload);

        if (!isV2) {
            archive[i].crc32 = CalculateCRC32((const unsigned char*)archive[i].data, archive[i].uncompSize);
        }

        numFiles++;
    }

    CloseHandle(hFile);
    RefreshList();
}

void ExtractSingleFile(int realIndex) {
    if (realIndex < 0 || realIndex >= numFiles) return;

    char safeName[256] = {0};
    SanitizeFilename(archive[realIndex].name, safeName, sizeof(safeName));

    HANDLE hFile = CreateFileA(safeName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        if (archive[realIndex].uncompSize > 0 && archive[realIndex].data) {
            WriteFile(hFile, archive[realIndex].data, archive[realIndex].uncompSize, &written, NULL);
        }
        CloseHandle(hFile);
        char msg[320];
        wsprintfA(msg, "Extracted '%s' to current directory.", safeName);
        MessageBoxA(NULL, msg, "KZip", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Failed to create extracted file.", "Error", MB_OK | MB_ICONERROR);
    }
}

int ExtractAll(int silent) {
    if (numFiles == 0) {
        if (!silent) MessageBoxA(NULL, "No files to extract.", "KZip", MB_OK | MB_ICONWARNING);
        return 0;
    }
    int count = 0;
    for (int i = 0; i < numFiles; i++) {
        char safeName[256] = {0};
        SanitizeFilename(archive[i].name, safeName, sizeof(safeName));

        HANDLE hFile = CreateFileA(safeName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            if (archive[i].uncompSize > 0 && archive[i].data) {
                WriteFile(hFile, archive[i].data, archive[i].uncompSize, &written, NULL);
            }
            CloseHandle(hFile);
            count++;
        }
    }
    if (!silent) {
        char msg[128];
        wsprintfA(msg, "Extracted %d file(s) to current directory.", count);
        MessageBoxA(NULL, msg, "KZip", MB_OK | MB_ICONINFORMATION);
    }
    return count;
}

void ShowPreview(int realIdx) {
    if (realIdx < 0 || realIdx >= numFiles) return;
    KFile* f = &archive[realIdx];
    
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    char filePath[MAX_PATH];
    wsprintfA(filePath, "%s\\kzip_preview.txt", tempPath);
    
    HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char header[1024];
        DWORD offset = 16;
        for (int i = 0; i < realIdx; i++) {
            offset += 4 + lstrlenA(archive[i].name) + 1 + 16 + archive[i].compSize;
        }
        
        int ratio = 0;
        if (f->uncompSize > 0) ratio = 100 - (f->compSize * 100 / f->uncompSize);
        if (ratio < 0) ratio = 0;
        
        wsprintfA(header, "--- KZIP FILE PREVIEW ---\r\n"
                          "Name: %s\r\n"
                          "Offset (Internal): 0x%08X\r\n"
                          "Original Size: %lu bytes\r\n"
                          "Compressed Size: %lu bytes\r\n"
                          "Method: %s\r\n"
                          "RLE Savings: %d%%\r\n"
                          "Timestamp: N/A\r\n"
                          "-------------------------\r\n\r\n",
                          f->name, offset, f->uncompSize, f->compSize, 
                          f->method == 1 ? "RLE" : "Store", ratio);
        
        DWORD w;
        WriteFile(hFile, header, lstrlenA(header), &w, NULL);
        
        if (f->data && f->uncompSize > 0) {
            DWORD len = f->uncompSize > 512 ? 512 : f->uncompSize;
            for (DWORD i = 0; i < len; i += 16) {
                char hex[128] = {0};
                char ascii[32] = {0};
                wsprintfA(hex, "%08X  ", i);
                
                for (DWORD j = 0; j < 16; j++) {
                    if (i + j < len) {
                        unsigned char b = (unsigned char)f->data[i + j];
                        char hb[8];
                        wsprintfA(hb, "%02X ", b);
                        lstrcatA(hex, hb);
                        ascii[j] = (b >= 32 && b <= 126) ? b : '.';
                    } else {
                        lstrcatA(hex, "   ");
                        ascii[j] = ' ';
                    }
                    if (j == 7) lstrcatA(hex, " ");
                }
                ascii[16] = '\0';
                
                char line[256];
                wsprintfA(line, "%s |%s|\r\n", hex, ascii);
                WriteFile(hFile, line, lstrlenA(line), &w, NULL);
            }
            if (f->uncompSize > len) {
                char trunc[] = "\r\n... (truncated)\r\n";
                WriteFile(hFile, trunc, lstrlenA(trunc), &w, NULL);
            }
        }
        CloseHandle(hFile);
        
        char cmd[MAX_PATH + 32];
        wsprintfA(cmd, "notepad.exe \"%s\"", filePath);
        WinExec(cmd, SW_SHOW);
    }
}

void VerifyIntegrity() {
    if (numFiles == 0) {
        MessageBoxA(NULL, "No files in archive to verify.", "KZip Integrity Verification", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int passed = 0, failed = 0;
    char report[4096] = "ARCHIVE CHECKSUM VERIFICATION REPORT:\n\n";

    for (int i = 0; i < numFiles; i++) {
        DWORD calcCRC = CalculateCRC32((const unsigned char*)archive[i].data, archive[i].uncompSize);
        char line[256];
        if (calcCRC == archive[i].crc32) {
            passed++;
            wsprintfA(line, "[OK] %s - CRC: 0x%08X (MATCH)\n", archive[i].name, calcCRC);
        } else {
            failed++;
            wsprintfA(line, "[FAIL] %s - Stored: 0x%08X vs Calc: 0x%08X (CORRUPTED)\n", archive[i].name, archive[i].crc32, calcCRC);
        }
        if (lstrlenA(report) + lstrlenA(line) < sizeof(report) - 128) {
            lstrcatA(report, line);
        }
    }

    char summary[128];
    wsprintfA(summary, "\nSummary: %d Passed, %d Failed.", passed, failed);
    lstrcatA(report, summary);

    MessageBoxA(NULL, report, "KZip Integrity Verification", failed == 0 ? (MB_OK | MB_ICONINFORMATION) : (MB_OK | MB_ICONWARNING));
}

static void AddMemoryFile(const char* name, const char* content, DWORD len) {
    if (numFiles >= MAX_FILES) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len ? len : 1);
    if (!buf) return;
    if (len > 0 && content) memcpy(buf, content, len);

    lstrcpynA(archive[numFiles].name, name, sizeof(archive[numFiles].name));
    archive[numFiles].uncompSize = len;
    archive[numFiles].crc32 = CalculateCRC32((const unsigned char*)buf, len);
    archive[numFiles].data = buf;

    DWORD compressMode = (DWORD)SendMessage(hComboCompress, CB_GETCURSEL, 0, 0); // 0 = Store, 1 = RLE
    if (compressMode == 1 && len > 0) {
        unsigned char* compBuf = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len * 3 + 16);
        if (compBuf) {
            DWORD compLen = CompressRLE((const unsigned char*)buf, len, compBuf);
            if (compLen > 0 && compLen < len) {
                archive[numFiles].compSize = compLen;
                archive[numFiles].method = 1;
            } else {
                archive[numFiles].compSize = len;
                archive[numFiles].method = 0;
            }
            HeapFree(GetProcessHeap(), 0, compBuf);
        } else {
            archive[numFiles].compSize = len;
            archive[numFiles].method = 0;
        }
    } else {
        archive[numFiles].compSize = len;
        archive[numFiles].method = 0;
    }
    numFiles++;
}

void AddDemoBundle() {
    ClearArchive();

    const char* readme =
        "=========================================\r\n"
        "  KZip Archiver Studio - Sample Readme   \r\n"
        "=========================================\r\n\r\n"
        "Welcome to KZip Archiver for KiloOS!\r\n"
        "Features:\r\n"
        "- High-speed RLE compression\r\n"
        "- CRC32 checksum verification\r\n"
        "- XOR password cipher encryption\r\n"
        "- Instant hex dump preview\r\n\r\n"
        "Keyboard Shortcuts:\r\n"
        "  Ctrl+O : Open .kza archive\r\n"
        "  Ctrl+S : Pack archive\r\n"
        "  Ctrl+E : Extract all files\r\n"
        "  Ctrl+N : Add file to archive\r\n"
        "  Del    : Remove selected\r\n"
        "  Enter  : Preview selected file\r\n"
        "  V      : Verify CRC32 checksums\r\n"
        "  P      : Deep Preview file\r\n"
        "  B      : Batch Extract\r\n"
        "  F1 / H : Comprehensive Help\r\n";

    const char* config =
        "[KZip]\r\n"
        "Version=2.1.0\r\n"
        "Compression=RLE\r\n"
        "AutoVerifyCRC=1\r\n"
        "MaxFileSizeMB=100\r\n"
        "Theme=CyberSlate\r\n";

    char pattern[1400];
    const char* chunk = "KZIP-COMPRESSION-TEST-RUN-LENGTH-ENCODING-DATA-CHUNK-TEST##########\r\n";
    DWORD chunkLen = (DWORD)lstrlenA(chunk);
    DWORD pos = 0;
    while (pos + chunkLen < 1380) {
        memcpy(pattern + pos, chunk, chunkLen);
        pos += chunkLen;
    }
    pattern[pos] = '\0';

    AddMemoryFile("README.txt", readme, (DWORD)lstrlenA(readme));
    AddMemoryFile("CONFIG.ini", config, (DWORD)lstrlenA(config));
    AddMemoryFile("PATTERN.log", pattern, pos);

    RefreshList();
    MessageBoxA(hMainWnd, "Demo bundle loaded with 3 sample files!\r\nTest 'Preview [P]', 'Verify [V]', or 'Pack [Ctrl+S]'.", "KZip Archiver Studio", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_ESCAPE) {
            SetWindowTextA(hwnd, "");
            RefreshList();
            SetFocus(hListBox);
            return 0;
        } else if (wParam == VK_RETURN || wParam == VK_DOWN) {
            SetFocus(hListBox);
            if (numVisible > 0) {
                SendMessage(hListBox, LB_SETCURSEL, 0, 0);
            }
            return 0;
        }
    }
    return CallWindowProc(g_pfnOrigSearchProc, hwnd, msg, wParam, lParam);
}

BOOL CALLBACK SetFontEnumProc(HWND child, LPARAM font) {
    SendMessage(child, WM_SETFONT, (WPARAM)font, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hMainWnd = hwnd;
            hFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hMonoFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            // Search Bar Label & Field
            CreateWindowEx(0, "STATIC", "Filter:", WS_CHILD | WS_VISIBLE, 10, 12, 40, 20, hwnd, NULL, NULL, NULL);
            hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCRAWL, 52, 10, 110, 22, hwnd, (HMENU)101, NULL, NULL);
            hChkRegex = CreateWindowEx(0, "BUTTON", "Regex", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 168, 12, 60, 20, hwnd, (HMENU)105, NULL, NULL);

            // Compress Mode Combo
            CreateWindowEx(0, "STATIC", "Method:", WS_CHILD | WS_VISIBLE, 238, 12, 52, 20, hwnd, NULL, NULL, NULL);
            hComboCompress = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 292, 10, 110, 120, hwnd, (HMENU)102, NULL, NULL);
            SendMessageA(hComboCompress, CB_ADDSTRING, 0, (LPARAM)"Store (0%)");
            SendMessageA(hComboCompress, CB_ADDSTRING, 0, (LPARAM)"RLE Fast");
            SendMessage(hComboCompress, CB_SETCURSEL, 1, 0);

            // Password Field
            CreateWindowEx(0, "STATIC", "Password:", WS_CHILD | WS_VISIBLE, 414, 12, 65, 20, hwnd, NULL, NULL, NULL);
            hEditPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_AUTOHSCRAWL, 482, 10, 110, 22, hwnd, (HMENU)103, NULL, NULL);

            // Column Header
            hHeader = CreateWindowEx(0, "STATIC",
                "  Filename                       |    Original ->      Packed (Saved) |  CRC32   | Method",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 36, W - 35, 18, hwnd, NULL, NULL, NULL);

            // ListBox
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP,
                10, 56, W - 35, H - 138, hwnd, (HMENU)100, NULL, NULL);

            // Action Buttons Row with shortcut badges
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Open [Ctrl+O]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, H - 72, 80, 26, hwnd, (HMENU)1, NULL, NULL);
            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add [Ctrl+N]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 95, H - 72, 78, 26, hwnd, (HMENU)2, NULL, NULL);
            hBtnRemove = CreateWindowEx(0, "BUTTON", "Remove [Del]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 178, H - 72, 80, 26, hwnd, (HMENU)5, NULL, NULL);
            hBtnPack = CreateWindowEx(0, "BUTTON", "Pack [Ctrl+S]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 263, H - 72, 80, 26, hwnd, (HMENU)3, NULL, NULL);
            hBtnExtractSel = CreateWindowEx(0, "BUTTON", "Ext. Sel [Enter]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 348, H - 72, 86, 26, hwnd, (HMENU)4, NULL, NULL);
            hBtnExtractAll = CreateWindowEx(0, "BUTTON", "Ext. All [Ctrl+E]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 439, H - 72, 86, 26, hwnd, (HMENU)6, NULL, NULL);
            hBtnBatchExtract = CreateWindowEx(0, "BUTTON", "Batch [B]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 530, H - 72, 66, 26, hwnd, (HMENU)8, NULL, NULL);
            hBtnVerify = CreateWindowEx(0, "BUTTON", "Verify [V]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 601, H - 72, 64, 26, hwnd, (HMENU)7, NULL, NULL);
            hBtnPreview = CreateWindowEx(0, "BUTTON", "Preview [P]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 670, H - 72, 68, 26, hwnd, (HMENU)9, NULL, NULL);
            hBtnDemo = CreateWindowEx(0, "BUTTON", "Demo [D]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 743, H - 72, 64, 26, hwnd, (HMENU)11, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 812, H - 72, 66, 26, hwnd, (HMENU)10, NULL, NULL);

            // Status Bar Label
            hStatus = CreateWindowEx(WS_EX_STATICEDGE, "STATIC", "Ready. Archive is empty (Drag & drop files or click 'Add [Ctrl+N]' or 'Demo [D]'). Press F1 for Help.", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, H - 38, W - 35, 24, hwnd, NULL, NULL, NULL);

            // Subclass Search Edit for Escape clearing and Enter navigation
            g_pfnOrigSearchProc = (WNDPROC)SetWindowLongPtrA(hEditSearch, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            // Set Fonts
            EnumChildWindows(hwnd, SetFontEnumProc, (LPARAM)hFont);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hMonoFont, TRUE);
            SendMessage(hHeader, WM_SETFONT, (WPARAM)hMonoFont, TRUE);

            DragAcceptFiles(hwnd, TRUE);
            UpdateWindowTitle();
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
            for (UINT i = 0; i < count; i++) {
                char dropFile[MAX_PATH] = {0};
                DragQueryFileA(hDrop, i, dropFile, sizeof(dropFile));
                int len = lstrlenA(dropFile);
                if (len > 4 && FastToLower(dropFile[len - 4]) == '.' &&
                               FastToLower(dropFile[len - 3]) == 'k' &&
                               FastToLower(dropFile[len - 2]) == 'z' &&
                               FastToLower(dropFile[len - 1]) == 'a') {
                    OpenArchive(dropFile);
                    break;
                } else {
                    AddFileToArchive(dropFile);
                }
            }
            DragFinish(hDrop);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == 100 && code == LBN_DBLCLK) { // Double-click list item to extract
                int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numVisible) {
                    ExtractSingleFile(visibleIndices[sel]);
                }
            } else if (id == 101 && code == EN_CHANGE) {
                RefreshList();
            } else if (id == 1) { // Open
                char file[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = 260;
                ofn.lpstrFilter = "KZA Archives\0*.kza\0All Files\0*.*\0";
                if (GetOpenFileNameA(&ofn)) {
                    OpenArchive(file);
                }
            } else if (id == 2) { // Add
                char file[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = 260;
                ofn.lpstrFilter = "All Files\0*.*\0";
                if (GetOpenFileNameA(&ofn)) {
                    AddFileToArchive(file);
                }
            } else if (id == 3) { // Pack
                char file[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = 260;
                ofn.lpstrFilter = "KZA Archives\0*.kza\0All Files\0*.*\0";
                ofn.lpstrDefExt = "kza";
                if (GetSaveFileNameA(&ofn)) {
                    PackArchive(file);
                }
            } else if (id == 4) { // Extract Selected
                int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numVisible) {
                    ExtractSingleFile(visibleIndices[sel]);
                } else {
                    MessageBoxA(NULL, "Please select a file to extract.", "KZip", MB_OK | MB_ICONWARNING);
                }
            } else if (id == 5) { // Remove
                int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numVisible) {
                    int realIdx = visibleIndices[sel];
                    if (archive[realIdx].data) HeapFree(GetProcessHeap(), 0, archive[realIdx].data);
                    for (int i = realIdx; i < numFiles - 1; i++) {
                        archive[i] = archive[i + 1];
                    }
                    numFiles--;
                    RefreshList();
                    if (numVisible > 0) {
                        int newSel = sel < numVisible ? sel : numVisible - 1;
                        SendMessage(hListBox, LB_SETCURSEL, newSel, 0);
                    }
                }
            } else if (id == 6) { // Extract All
                ExtractAll(0);
            } else if (id == 7) { // Verify
                VerifyIntegrity();
            } else if (id == 8) { // Batch Extract
                char file[4096] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = sizeof(file);
                ofn.lpstrFilter = "KZA Archives\0*.kza\0All Files\0*.*\0";
                ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
                if (GetOpenFileNameA(&ofn)) {
                    char dir[MAX_PATH] = {0};
                    lstrcpyA(dir, file);
                    char* p = file + lstrlenA(file) + 1;
                    if (*p == '\0') {
                        // Single file selected
                        OpenArchive(file);
                        ExtractAll(0);
                    } else {
                        // Multiple files
                        int totalExtracted = 0;
                        while (*p) {
                            char fullPath[MAX_PATH];
                            wsprintfA(fullPath, "%s\\%s", dir, p);
                            OpenArchive(fullPath);
                            totalExtracted += ExtractAll(1);
                            p += lstrlenA(p) + 1;
                        }
                        char msg[128];
                        wsprintfA(msg, "Batch extraction complete. Extracted %d file(s).", totalExtracted);
                        MessageBoxA(hwnd, msg, "KZip", MB_OK | MB_ICONINFORMATION);
                    }
                }
            } else if (id == 9) { // Preview
                int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numVisible) {
                    ShowPreview(visibleIndices[sel]);
                } else {
                    MessageBoxA(NULL, "Please select a file to preview.", "KZip", MB_OK | MB_ICONWARNING);
                }
            } else if (id == 10) { // Help
                ShowHelp(hwnd);
            } else if (id == 11) { // Demo
                AddDemoBundle();
            } else if (id == 105 && code == BN_CLICKED) { // Regex Checkbox
                RefreshList();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
            mmi->ptMinTrackSize.x = 840;
            mmi->ptMinTrackSize.y = 420;
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            if (hHeader) MoveWindow(hHeader, 10, 36, nw - 20, 18, TRUE);
            MoveWindow(hListBox, 10, 56, nw - 20, nh - 138, TRUE);

            int btnY = nh - 72;
            int x = 10;
            int btnH = 26;
            int gap = 5;
            MoveWindow(hBtnOpen, x, btnY, 80, btnH, TRUE); x += 80 + gap;
            MoveWindow(hBtnAdd, x, btnY, 78, btnH, TRUE); x += 78 + gap;
            MoveWindow(hBtnRemove, x, btnY, 80, btnH, TRUE); x += 80 + gap;
            MoveWindow(hBtnPack, x, btnY, 80, btnH, TRUE); x += 80 + gap;
            MoveWindow(hBtnExtractSel, x, btnY, 86, btnH, TRUE); x += 86 + gap;
            MoveWindow(hBtnExtractAll, x, btnY, 86, btnH, TRUE); x += 86 + gap;
            MoveWindow(hBtnBatchExtract, x, btnY, 66, btnH, TRUE); x += 66 + gap;
            MoveWindow(hBtnVerify, x, btnY, 64, btnH, TRUE); x += 64 + gap;
            MoveWindow(hBtnPreview, x, btnY, 68, btnH, TRUE); x += 68 + gap;
            MoveWindow(hBtnDemo, x, btnY, 64, btnH, TRUE); x += 64 + gap;
            MoveWindow(hBtnHelp, x, btnY, 66, btnH, TRUE);

            MoveWindow(hStatus, 10, nh - 38, nw - 20, 24, TRUE);
            break;
        }
        case WM_DESTROY:
            ClearArchive();
            if (hFont) DeleteObject(hFont);
            if (hMonoFont) DeleteObject(hMonoFont);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KZipApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    RECT rect = { 0, 0, W, H };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(0, "KZipApp", "KZip Archiver Studio - [Empty] - (Press F1 for Help)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            HWND hFocus = GetFocus();
            BOOL inEdit = (hFocus == hEditSearch || hFocus == hEditPassword);

            if (isCtrl) {
                if (msg.wParam == 'O' || msg.wParam == 'o') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1, BN_CLICKED), (LPARAM)hBtnOpen);
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(3, BN_CLICKED), (LPARAM)hBtnPack);
                    continue;
                } else if (msg.wParam == 'E' || msg.wParam == 'e') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(6, BN_CLICKED), (LPARAM)hBtnExtractAll);
                    continue;
                } else if (msg.wParam == 'N' || msg.wParam == 'n') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(2, BN_CLICKED), (LPARAM)hBtnAdd);
                    continue;
                }
            }

            if (msg.wParam == VK_F1) {
                ShowHelp(hwnd);
                continue;
            }

            if (!inEdit) {
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelp(hwnd);
                    continue;
                } else if (msg.wParam == 'V' || msg.wParam == 'v') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(7, BN_CLICKED), (LPARAM)hBtnVerify);
                    continue;
                } else if (msg.wParam == 'P' || msg.wParam == 'p') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(9, BN_CLICKED), (LPARAM)hBtnPreview);
                    continue;
                } else if (msg.wParam == 'B' || msg.wParam == 'b') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(8, BN_CLICKED), (LPARAM)hBtnBatchExtract);
                    continue;
                } else if (msg.wParam == 'D' || msg.wParam == 'd') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(11, BN_CLICKED), (LPARAM)hBtnDemo);
                    continue;
                } else if (msg.wParam == VK_DELETE) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(5, BN_CLICKED), (LPARAM)hBtnRemove);
                    continue;
                } else if (msg.wParam == VK_RETURN) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(9, BN_CLICKED), (LPARAM)hBtnPreview);
                    continue;
                }
            } else {
                if (msg.wParam == VK_ESCAPE) {
                    SetWindowTextA(hEditSearch, "");
                    RefreshList();
                    SetFocus(hListBox);
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

