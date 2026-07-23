#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#ifndef ES_AUTOHSCRAWL
#define ES_AUTOHSCRAWL 0x0080L
#endif

#define W 680
#define H 460

HWND hListBox, hEditSearch, hEditPassword, hComboCompress;
HWND hBtnOpen, hBtnAdd, hBtnRemove, hBtnPack, hBtnExtractSel, hBtnExtractAll, hBtnVerify;
HWND hStatus;
HFONT hFont;

#define MAX_FILES 100

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

// CRC32 Calculation
DWORD CalculateCRC32(const unsigned char* data, DWORD size) {
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

// Simple RLE Compression
DWORD CompressRLE(const unsigned char* in, DWORD inSize, unsigned char* out) {
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

// Simple RLE Decompression
DWORD DecompressRLE(const unsigned char* in, DWORD inSize, unsigned char* out, DWORD outCapacity) {
    DWORD inIdx = 0, outIdx = 0;
    while (inIdx < inSize && outIdx < outCapacity) {
        unsigned char b = in[inIdx++];
        if (b == 0xFF) {
            if (inIdx + 1 >= inSize) break;
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
    if (key == 0) return;
    for (DWORD i = 0; i < size; i++) {
        BYTE k = (BYTE)((key + i * 31 + (key >> (i % 8))) & 0xFF);
        data[i] ^= k;
    }
}

// Case-insensitive substring match
int ContainsString(const char* str, const char* sub) {
    if (!sub || !*sub) return 1;
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

void RefreshList() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    numVisible = 0;

    char filter[128] = {0};
    GetWindowTextA(hEditSearch, filter, sizeof(filter));

    DWORD totalUncomp = 0, totalComp = 0;

    for (int i = 0; i < numFiles; i++) {
        totalUncomp += archive[i].uncompSize;
        totalComp += archive[i].compSize;

        if (ContainsString(archive[i].name, filter)) {
            visibleIndices[numVisible] = i;

            int ratio = 0;
            if (archive[i].uncompSize > 0) {
                ratio = 100 - (int)((archive[i].compSize * 100) / archive[i].uncompSize);
                if (ratio < 0) ratio = 0;
            }

            char itemBuf[384];
            wsprintfA(itemBuf, "%-24s | %lu B -> %lu B (%d%%) | CRC: 0x%08X | %s",
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

    int overallRatio = 0;
    if (totalUncomp > 0) {
        overallRatio = 100 - (int)((totalComp * 100) / totalUncomp);
        if (overallRatio < 0) overallRatio = 0;
    }

    char statusBuf[256];
    wsprintfA(statusBuf, "Files: %d | Total Raw: %lu B | Compressed: %lu B | Savings: %d%%",
        numFiles, totalUncomp, totalComp, overallRatio);
    SetWindowTextA(hStatus, statusBuf);
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
        MessageBoxA(NULL, "Archive is full!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD size = GetFileSize(hFile, NULL);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size ? size : 1);
    DWORD read;
    if (size > 0) ReadFile(hFile, buf, size, &read, NULL);
    CloseHandle(hFile);

    const char* filename = filepath;
    for (int i = 0; filepath[i]; i++) {
        if (filepath[i] == '\\' || filepath[i] == '/') filename = filepath + i + 1;
    }

    DWORD compressMode = (DWORD)SendMessage(hComboCompress, CB_GETCURSEL, 0, 0); // 0 = Store, 1 = RLE

    lstrcpyA(archive[numFiles].name, filename);
    archive[numFiles].uncompSize = size;
    archive[numFiles].crc32 = CalculateCRC32((const unsigned char*)buf, size);
    archive[numFiles].data = buf;

    if (compressMode == 1 && size > 0) {
        unsigned char* compBuf = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size * 2 + 16);
        DWORD compLen = CompressRLE((const unsigned char*)buf, size, compBuf);
        if (compLen < size) {
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
        MessageBoxA(NULL, "Failed to create archive file.", "Error", MB_OK | MB_ICONERROR);
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

    DWORD written;
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
        if (archive[i].method == 1 && archive[i].uncompSize > 0) {
            CompressRLE((const unsigned char*)archive[i].data, archive[i].uncompSize, (unsigned char*)payload);
        } else if (archive[i].uncompSize > 0) {
            memcpy(payload, archive[i].data, archive[i].uncompSize);
        }

        if (flags & 0x01) {
            CryptData(payload, archive[i].compSize, pwdHash);
        }

        WriteFile(hFile, payload, archive[i].compSize, &written, NULL);
        HeapFree(GetProcessHeap(), 0, payload);
    }

    CloseHandle(hFile);
    MessageBoxA(NULL, "Archive packed successfully!", "KZip", MB_OK | MB_ICONINFORMATION);
}

void OpenArchive(const char* filepath) {
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    char magic[4] = {0};
    DWORD read;
    ReadFile(hFile, magic, 4, &read, NULL);

    BOOL isV2 = (magic[0] == 'K' && magic[1] == 'Z' && magic[2] == 'A' && magic[3] == '2');
    BOOL isV1 = (magic[0] == 'K' && magic[1] == 'Z' && magic[2] == 'A' && magic[3] == '\0');

    if (!isV1 && !isV2) {
        CloseHandle(hFile);
        MessageBoxA(NULL, "Invalid KZA archive format.", "Error", MB_OK | MB_ICONERROR);
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
                MessageBoxA(NULL, "Archive is password protected! Please enter the correct password in the Password field.", "Access Denied", MB_OK | MB_ICONERROR);
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

        char* payload = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, archive[i].compSize ? archive[i].compSize : 1);
        ReadFile(hFile, payload, archive[i].compSize, &read, NULL);

        if (isV2 && (flags & 0x01)) {
            CryptData(payload, archive[i].compSize, pwdHash);
        }

        archive[i].data = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, archive[i].uncompSize ? archive[i].uncompSize : 1);
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
    HANDLE hFile = CreateFileA(archive[realIndex].name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        if (archive[realIndex].uncompSize > 0) {
            WriteFile(hFile, archive[realIndex].data, archive[realIndex].uncompSize, &written, NULL);
        }
        CloseHandle(hFile);
        char msg[300];
        wsprintfA(msg, "Extracted '%s' to current directory.", archive[realIndex].name);
        MessageBoxA(NULL, msg, "KZip", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Failed to create extracted file.", "Error", MB_OK | MB_ICONERROR);
    }
}

void ExtractAll() {
    if (numFiles == 0) return;
    int count = 0;
    for (int i = 0; i < numFiles; i++) {
        HANDLE hFile = CreateFileA(archive[i].name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            if (archive[i].uncompSize > 0) {
                WriteFile(hFile, archive[i].data, archive[i].uncompSize, &written, NULL);
            }
            CloseHandle(hFile);
            count++;
        }
    }
    char msg[128];
    wsprintfA(msg, "Extracted %d file(s) to current directory.", count);
    MessageBoxA(NULL, msg, "KZip", MB_OK | MB_ICONINFORMATION);
}

void VerifyIntegrity() {
    if (numFiles == 0) {
        MessageBoxA(NULL, "No files in archive to verify.", "KZip Integrity Verification", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int passed = 0, failed = 0;
    char report[1024] = "ARCHIVE CHECKSUM VERIFICATION REPORT:\n\n";

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
        if (lstrlenA(report) + lstrlenA(line) < sizeof(report) - 100) {
            lstrcatA(report, line);
        }
    }

    char summary[128];
    wsprintfA(summary, "\nSummary: %d Passed, %d Failed.", passed, failed);
    lstrcatA(report, summary);

    MessageBoxA(NULL, report, "KZip Integrity Verification", failed == 0 ? (MB_OK | MB_ICONINFORMATION) : (MB_OK | MB_ICONWARNING));
}

BOOL CALLBACK SetFontEnumProc(HWND child, LPARAM font) {
    SendMessage(child, WM_SETFONT, (WPARAM)font, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");

            // Search Bar Label & Field
            CreateWindowEx(0, "STATIC", "Filter:", WS_CHILD | WS_VISIBLE, 10, 12, 40, 20, hwnd, NULL, NULL, NULL);
            hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCRAWL, 55, 10, 140, 22, hwnd, (HMENU)101, NULL, NULL);

            // Compress Mode Combo
            CreateWindowEx(0, "STATIC", "Method:", WS_CHILD | WS_VISIBLE, 205, 12, 50, 20, hwnd, NULL, NULL, NULL);
            hComboCompress = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 260, 10, 100, 120, hwnd, (HMENU)102, NULL, NULL);
            SendMessageA(hComboCompress, CB_ADDSTRING, 0, (LPARAM)"Store (0%)");
            SendMessageA(hComboCompress, CB_ADDSTRING, 0, (LPARAM)"RLE Fast");
            SendMessage(hComboCompress, CB_SETCURSEL, 1, 0);

            // Password Field
            CreateWindowEx(0, "STATIC", "Pass:", WS_CHILD | WS_VISIBLE, 370, 12, 35, 20, hwnd, NULL, NULL, NULL);
            hEditPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_AUTOHSCRAWL, 410, 10, 100, 22, hwnd, (HMENU)103, NULL, NULL);

            // Action Buttons Row
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Open .kza", WS_CHILD | WS_VISIBLE, 10, H - 75, 75, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add File", WS_CHILD | WS_VISIBLE, 90, H - 75, 70, 24, hwnd, (HMENU)2, NULL, NULL);
            hBtnRemove = CreateWindowEx(0, "BUTTON", "Remove", WS_CHILD | WS_VISIBLE, 165, H - 75, 65, 24, hwnd, (HMENU)5, NULL, NULL);
            hBtnPack = CreateWindowEx(0, "BUTTON", "Pack .kza", WS_CHILD | WS_VISIBLE, 235, H - 75, 75, 24, hwnd, (HMENU)3, NULL, NULL);
            hBtnExtractSel = CreateWindowEx(0, "BUTTON", "Extract Sel", WS_CHILD | WS_VISIBLE, 315, H - 75, 80, 24, hwnd, (HMENU)4, NULL, NULL);
            hBtnExtractAll = CreateWindowEx(0, "BUTTON", "Extract All", WS_CHILD | WS_VISIBLE, 400, H - 75, 75, 24, hwnd, (HMENU)6, NULL, NULL);
            hBtnVerify = CreateWindowEx(0, "BUTTON", "Verify CRC", WS_CHILD | WS_VISIBLE, 480, H - 75, 75, 24, hwnd, (HMENU)7, NULL, NULL);

            // ListBox
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_HSCROLL,
                10, 40, W - 35, H - 125, hwnd, NULL, NULL, NULL);

            // Status Bar Label
            hStatus = CreateWindowEx(WS_EX_STATICEDGE, "STATIC", "Ready", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, H - 45, W - 35, 20, hwnd, NULL, NULL, NULL);

            // Set Fonts
            EnumChildWindows(hwnd, SetFontEnumProc, (LPARAM)hFont);

            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == 101 && code == EN_CHANGE) {
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
                }
            } else if (id == 6) { // Extract All
                ExtractAll();
            } else if (id == 7) { // Verify
                VerifyIntegrity();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hListBox, 10, 40, nw - 20, nh - 120, TRUE);

            int btnY = nh - 70;
            MoveWindow(hBtnOpen, 10, btnY, 75, 24, TRUE);
            MoveWindow(hBtnAdd, 90, btnY, 70, 24, TRUE);
            MoveWindow(hBtnRemove, 165, btnY, 65, 24, TRUE);
            MoveWindow(hBtnPack, 235, btnY, 75, 24, TRUE);
            MoveWindow(hBtnExtractSel, 315, btnY, 80, 24, TRUE);
            MoveWindow(hBtnExtractAll, 400, btnY, 75, 24, TRUE);
            MoveWindow(hBtnVerify, 480, btnY, 75, 24, TRUE);

            MoveWindow(hStatus, 10, nh - 35, nw - 20, 22, TRUE);
            break;
        }
        case WM_DESTROY:
            ClearArchive();
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KZipApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KZipApp", "KZip Archiver", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
