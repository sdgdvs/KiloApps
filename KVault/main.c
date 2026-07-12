#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ID_BTN_ENCRYPT 101
#define ID_BTN_DECRYPT 102
#define ID_BTN_CLEAR 103
#define ID_EDIT_PASS 104
#define ID_EDIT_DATA 105
#define ID_BTN_LOAD 106
#define ID_BTN_SAVE 107
#define ID_EDIT_FIND 108
#define ID_BTN_FIND 109

HWND hPass, hData;
HBRUSH hbgBrush;
HBRUSH hDarkBrush;
HFONT hFont, hTitleFont;

void EncryptData(HWND hTextEdit, HWND hPassEdit) {
    int textLen = GetWindowTextLengthA(hTextEdit);
    int passLen = GetWindowTextLengthA(hPassEdit);
    if (textLen == 0 || passLen == 0) return;
    
    char* text = (char*)malloc(textLen + 1);
    char* pass = (char*)malloc(passLen + 1);
    GetWindowTextA(hTextEdit, text, textLen + 1);
    GetWindowTextA(hPassEdit, pass, passLen + 1);
    
    char* hexOut = (char*)malloc(textLen * 2 + 1);
    for (int i = 0; i < textLen; i++) {
        unsigned char cipher = (unsigned char)text[i] ^ (unsigned char)pass[i % passLen];
        sprintf(&hexOut[i * 2], "%02X", cipher);
    }
    hexOut[textLen * 2] = '\0';
    
    SetWindowTextA(hTextEdit, hexOut);
    free(hexOut);
    free(text);
    free(pass);
}

void DecryptData(HWND hTextEdit, HWND hPassEdit) {
    int textLen = GetWindowTextLengthA(hTextEdit);
    int passLen = GetWindowTextLengthA(hPassEdit);
    if (textLen == 0 || passLen == 0 || textLen % 2 != 0) return;
    
    char* hexText = (char*)malloc(textLen + 1);
    char* pass = (char*)malloc(passLen + 1);
    GetWindowTextA(hTextEdit, hexText, textLen + 1);
    GetWindowTextA(hPassEdit, pass, passLen + 1);
    
    char* plainOut = (char*)malloc(textLen / 2 + 1);
    for (int i = 0; i < textLen / 2; i++) {
        char hexByte[3] = { hexText[i * 2], hexText[i * 2 + 1], '\0' };
        unsigned char cipher = (unsigned char)strtol(hexByte, NULL, 16);
        plainOut[i] = cipher ^ (unsigned char)pass[i % passLen];
    }
    plainOut[textLen / 2] = '\0';
    
    SetWindowTextA(hTextEdit, plainOut);
    free(plainOut);
    free(hexText);
    free(pass);
}

void LoadFromFile(HWND hwnd, HWND hTextEdit) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize > 0) {
                char* buffer = (char*)malloc(fileSize + 1);
                DWORD bytesRead;
                if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                    buffer[bytesRead] = '\0';
                    SetWindowTextA(hTextEdit, buffer);
                }
                free(buffer);
            } else {
                SetWindowTextA(hTextEdit, "");
            }
            CloseHandle(hFile);
        }
    }
}

void SaveToFile(HWND hwnd, HWND hTextEdit) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            int textLen = GetWindowTextLengthA(hTextEdit);
            if (textLen > 0) {
                char* buffer = (char*)malloc(textLen + 1);
                GetWindowTextA(hTextEdit, buffer, textLen + 1);
                DWORD bytesWritten;
                WriteFile(hFile, buffer, textLen, &bytesWritten, NULL);
                free(buffer);
            }
            CloseHandle(hFile);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            hbgBrush = CreateSolidBrush(RGB(15, 15, 19)); // Darker background
            hDarkBrush = CreateSolidBrush(RGB(25, 25, 30));
            hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            HWND hTitle = CreateWindowA("STATIC", "KVault - Secure Storage", WS_VISIBLE | WS_CHILD, 15, 15, 300, 30, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            
            HWND hStat = CreateWindowA("STATIC", "Master Key:", WS_VISIBLE | WS_CHILD, 15, 62, 80, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hStat, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 100, 60, 200, 25, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
            SendMessage(hPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnEnc = CreateWindowA("BUTTON", "Encrypt", WS_VISIBLE | WS_CHILD | BS_FLAT, 315, 60, 85, 25, hwnd, (HMENU)ID_BTN_ENCRYPT, NULL, NULL);
            HWND hBtnDec = CreateWindowA("BUTTON", "Decrypt", WS_VISIBLE | WS_CHILD | BS_FLAT, 410, 60, 85, 25, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            HWND hBtnClr = CreateWindowA("BUTTON", "Clear", WS_VISIBLE | WS_CHILD | BS_FLAT, 505, 60, 75, 25, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            
            SendMessage(hBtnEnc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDec, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnClr, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hData = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 15, 100, 565, 210, hwnd, (HMENU)ID_EDIT_DATA, NULL, NULL);
            SendMessage(hData, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnLoad = CreateWindowA("BUTTON", "Load File", WS_VISIBLE | WS_CHILD | BS_FLAT, 15, 320, 100, 25, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            HWND hBtnSave = CreateWindowA("BUTTON", "Save File", WS_VISIBLE | WS_CHILD | BS_FLAT, 125, 320, 100, 25, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hFindEdit = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 240, 320, 150, 25, hwnd, (HMENU)ID_EDIT_FIND, NULL, NULL);
            SendMessage(hFindEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND hBtnFind = CreateWindowA("BUTTON", "Find", WS_VISIBLE | WS_CHILD | BS_FLAT, 400, 320, 80, 25, hwnd, (HMENU)ID_BTN_FIND, NULL, NULL);
            SendMessage(hBtnFind, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_ENCRYPT) {
                EncryptData(hData, hPass);
            } else if (LOWORD(wParam) == ID_BTN_DECRYPT) {
                DecryptData(hData, hPass);
            } else if (LOWORD(wParam) == ID_BTN_CLEAR) {
                SetWindowTextA(hData, "");
            } else if (LOWORD(wParam) == ID_BTN_LOAD) {
                LoadFromFile(hwnd, hData);
            } else if (LOWORD(wParam) == ID_BTN_SAVE) {
                SaveToFile(hwnd, hData);
            } else if (LOWORD(wParam) == ID_BTN_FIND) {
                char findText[256];
                GetDlgItemTextA(hwnd, ID_EDIT_FIND, findText, 256);
                if (strlen(findText) > 0) {
                    int textLen = GetWindowTextLengthA(hData);
                    char* text = (char*)malloc(textLen + 1);
                    GetWindowTextA(hData, text, textLen + 1);
                    
                    DWORD startSel = 0, endSel = 0;
                    SendMessage(hData, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);
                    
                    char* pos = strstr(text + endSel, findText);
                    if (!pos && endSel > 0) {
                        pos = strstr(text, findText);
                    }
                    
                    if (pos) {
                        int index = (int)(pos - text);
                        SendMessage(hData, EM_SETSEL, index, index + strlen(findText));
                        SendMessage(hData, EM_SCROLLCARET, 0, 0);
                        SetFocus(hData);
                    } else {
                        MessageBoxA(hwnd, "Text not found.", "Find", MB_OK | MB_ICONINFORMATION);
                    }
                    free(text);
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(240, 240, 245));
            SetBkColor(hdc, RGB(15, 15, 19));
            return (LRESULT)hbgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(240, 240, 245));
            SetBkColor(hdc, RGB(25, 25, 30));
            return (LRESULT)hDarkBrush;
        }
        case WM_DESTROY: {
            DeleteObject(hbgBrush);
            DeleteObject(hDarkBrush);
            DeleteObject(hFont);
            DeleteObject(hTitleFont);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KVaultClass";
    wc.hbrBackground = CreateSolidBrush(RGB(15, 15, 19));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowA("KVaultClass", "KVault", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
