#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#define W 420
#define H 320

HWND hListBox;
HWND hBtnOpen, hBtnAdd, hBtnPack, hBtnExtract;

#define MAX_FILES 50
typedef struct {
    char name[256];
    DWORD size;
    char* data;
} KFile;

KFile archive[MAX_FILES];
int numFiles = 0;

void AddFileToArchive(const char* filepath) {
    if (numFiles >= MAX_FILES) {
        MessageBoxA(NULL, "Archive full!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    DWORD size = GetFileSize(hFile, NULL);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    DWORD read;
    ReadFile(hFile, buf, size, &read, NULL);
    CloseHandle(hFile);
    
    const char* filename = filepath;
    for (int i = 0; filepath[i]; i++) {
        if (filepath[i] == '\\' || filepath[i] == '/') filename = filepath + i + 1;
    }
    
    lstrcpyA(archive[numFiles].name, filename);
    archive[numFiles].size = size;
    archive[numFiles].data = buf;
    
    SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)filename);
    numFiles++;
}

void PackArchive(const char* filepath) {
    if (numFiles == 0) return;
    HANDLE hFile = CreateFileA(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to create archive.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    DWORD written;
    WriteFile(hFile, "KZA\0", 4, &written, NULL);
    WriteFile(hFile, &numFiles, sizeof(DWORD), &written, NULL);
    
    for (int i = 0; i < numFiles; i++) {
        DWORD nameLen = (DWORD)lstrlenA(archive[i].name) + 1;
        WriteFile(hFile, &nameLen, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, archive[i].name, nameLen, &written, NULL);
        WriteFile(hFile, &archive[i].size, sizeof(DWORD), &written, NULL);
        WriteFile(hFile, archive[i].data, archive[i].size, &written, NULL);
    }
    
    CloseHandle(hFile);
    MessageBoxA(NULL, "Archive packed successfully!", "KZip", MB_OK | MB_ICONINFORMATION);
}

void OpenArchive(const char* filepath) {
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    char magic[4];
    DWORD read;
    ReadFile(hFile, magic, 4, &read, NULL);
    if (lstrcmpA(magic, "KZA") != 0) {
        CloseHandle(hFile);
        MessageBoxA(NULL, "Invalid KZA archive.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    for (int i=0; i<numFiles; i++) {
        HeapFree(GetProcessHeap(), 0, archive[i].data);
    }
    numFiles = 0;
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    
    DWORD fileCount;
    ReadFile(hFile, &fileCount, sizeof(DWORD), &read, NULL);
    
    for (DWORD i = 0; i < fileCount && i < MAX_FILES; i++) {
        DWORD nameLen;
        ReadFile(hFile, &nameLen, sizeof(DWORD), &read, NULL);
        ReadFile(hFile, archive[i].name, nameLen, &read, NULL);
        
        DWORD size;
        ReadFile(hFile, &size, sizeof(DWORD), &read, NULL);
        archive[i].size = size;
        archive[i].data = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
        ReadFile(hFile, archive[i].data, size, &read, NULL);
        
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)archive[i].name);
        numFiles++;
    }
    CloseHandle(hFile);
}

void ExtractAll() {
    if (numFiles == 0) return;
    for (int i=0; i<numFiles; i++) {
        HANDLE hFile = CreateFileA(archive[i].name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, archive[i].data, archive[i].size, &written, NULL);
            CloseHandle(hFile);
        }
    }
    MessageBoxA(NULL, "Extracted all files to current directory.", "KZip", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | LBS_STANDARD,
                10, 10, W - 35, H - 90, hwnd, NULL, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Open .kza", WS_CHILD | WS_VISIBLE, 10, H - 70, 80, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add File...", WS_CHILD | WS_VISIBLE, 100, H - 70, 90, 24, hwnd, (HMENU)2, NULL, NULL);
            hBtnPack = CreateWindowEx(0, "BUTTON", "Pack to .kza", WS_CHILD | WS_VISIBLE, 200, H - 70, 90, 24, hwnd, (HMENU)3, NULL, NULL);
            hBtnExtract = CreateWindowEx(0, "BUTTON", "Extract All", WS_CHILD | WS_VISIBLE, 300, H - 70, 80, 24, hwnd, (HMENU)4, NULL, NULL);

            SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPack, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExtract, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1) { // Open
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
            } else if (id == 4) { // Extract All
                ExtractAll();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hListBox, 10, 10, nw - 20, nh - 50, TRUE);
            MoveWindow(hBtnOpen, 10, nh - 35, 80, 24, TRUE);
            MoveWindow(hBtnAdd, 100, nh - 35, 90, 24, TRUE);
            MoveWindow(hBtnPack, 200, nh - 35, 90, 24, TRUE);
            MoveWindow(hBtnExtract, 300, nh - 35, 80, 24, TRUE);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
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

    HWND hwnd = CreateWindowEx(0, "KZipApp", "KZip", WS_OVERLAPPEDWINDOW,
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
