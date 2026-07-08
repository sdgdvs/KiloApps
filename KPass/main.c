#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 300
#define H 350

HWND hEdit;
HWND hBtnGen;
HWND hBtnCopy;
HWND hLabel;
HWND hBtnSave;
HWND hListBox;

const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+";
int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

void LoadVault() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    HANDLE hFile = CreateFileA("kpass_vault.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD size = GetFileSize(hFile, NULL);
        char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 1);
        if (buf) {
            DWORD read;
            ReadFile(hFile, buf, size, &read, NULL);
            char* line = strtok(buf, "\r\n");
            while (line) {
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)line);
                line = strtok(NULL, "\r\n");
            }
            HeapFree(GetProcessHeap(), 0, buf);
        }
        CloseHandle(hFile);
    }
}

void SaveToVault() {
    char label[64];
    char pass[32];
    GetWindowTextA(hLabel, label, 64);
    GetWindowTextA(hEdit, pass, 32);
    if (lstrlenA(label) == 0 || lstrlenA(pass) == 0) {
        MessageBoxA(NULL, "Please enter a label and generate a password.", "Error", MB_OK);
        return;
    }
    
    char entry[128];
    wsprintfA(entry, "%s: %s\r\n", label, pass);
    
    HANDLE hFile = CreateFileA("kpass_vault.txt", FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, entry, lstrlenA(entry), &written, NULL);
        CloseHandle(hFile);
    }
    SetWindowTextA(hLabel, "");
    LoadVault();
}

void GeneratePassword() {
    char pass[17];
    int len = lstrlenA(alphabet);
    for (int i = 0; i < 16; i++) {
        pass[i] = alphabet[MyRand() % len];
    }
    pass[16] = 0;
    SetWindowTextA(hEdit, pass);
}

void CopyToClipboard(HWND hwnd) {
    char pass[32];
    GetWindowTextA(hEdit, pass, 32);
    int len = lstrlenA(pass);
    if (len == 0) return;
    
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, len + 1);
        if (hg) {
            char* dest = (char*)GlobalLock(hg);
            for (int i = 0; i <= len; i++) dest[i] = pass[i];
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
        }
        CloseClipboard();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFont) hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
            
            hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
                20, 20, W - 55, 30, hwnd, NULL, NULL, NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HFONT hFontBtn = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hBtnGen = CreateWindowEx(0, "BUTTON", "Generate",
                WS_CHILD | WS_VISIBLE,
                20, 60, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnGen, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            
            hBtnCopy = CreateWindowEx(0, "BUTTON", "Copy",
                WS_CHILD | WS_VISIBLE,
                W - 135, 60, 100, 30, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnCopy, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            
            hLabel = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE,
                20, 100, 150, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hLabel, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            
            hBtnSave = CreateWindowEx(0, "BUTTON", "Save",
                WS_CHILD | WS_VISIBLE,
                180, 100, 85, 25, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD,
                20, 140, W - 55, 150, hwnd, NULL, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            
            LoadVault();
            GeneratePassword();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                GeneratePassword();
            } else if (LOWORD(wParam) == 2) {
                CopyToClipboard(hwnd);
            } else if (LOWORD(wParam) == 3) {
                SaveToVault();
            }
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
    wc.lpszClassName = "KPassApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KPassApp", "KPass", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
