#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>

#define ID_BTN_ENCRYPT 101
#define ID_BTN_DECRYPT 102
#define ID_BTN_CLEAR 103
#define ID_EDIT_PASS 104
#define ID_EDIT_DATA 105
#define ID_BTN_LOAD 106
#define ID_BTN_SAVE 107
#define ID_EDIT_FIND 108
#define ID_BTN_FIND 109
#define ID_BTN_GENERATE 110
#define ID_COMBO_TIMEOUT 111
#define ID_STATIC_STRENGTH 112
#define ID_COMBO_TEMPLATE 113
#define ID_BTN_INSERT_TEMPLATE 114
#define ID_BTN_COPY_DATA 115
#define ID_BTN_CLEAR_CLIP 116
#define ID_COMBO_THEME 117
#define ID_BTN_HELP 118

HWND hPass, hData;
HBRUSH hbgBrush;
HBRUSH hDarkBrush;
HFONT hFont, hTitleFont;

DWORD g_lastActivity = 0;
int g_timeoutMs = 60000;
int g_theme = 0;

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

void GeneratePassword(HWND hTextEdit) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+~`|}{[]:;?><,./-=";
    int len = 16;
    char pass[17];
    for (int i = 0; i < len; i++) {
        pass[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    pass[len] = '\0';
    
    int textLen = GetWindowTextLengthA(hTextEdit);
    char* newText = (char*)malloc(textLen + len + 3);
    if (textLen > 0) {
        GetWindowTextA(hTextEdit, newText, textLen + 1);
        strcat(newText, "\r\n");
        strcat(newText, pass);
    } else {
        strcpy(newText, pass);
    }
    SetWindowTextA(hTextEdit, newText);
    free(newText);
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
            
            HWND hStatTimeout = CreateWindowA("STATIC", "Auto-lock:", WS_VISIBLE | WS_CHILD, 320, 22, 70, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatTimeout, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hComboTimeout = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 395, 20, 90, 100, hwnd, (HMENU)ID_COMBO_TIMEOUT, NULL, NULL);
            SendMessage(hComboTimeout, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"1 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"5 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"15 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"Never");
            SendMessage(hComboTimeout, CB_SETCURSEL, 0, 0);
            
            HWND hComboTheme = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 495, 20, 85, 100, hwnd, (HMENU)ID_COMBO_THEME, NULL, NULL);
            SendMessage(hComboTheme, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Dark");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Light");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Neon");
            SendMessage(hComboTheme, CB_SETCURSEL, 0, 0);
            
            SetTimer(hwnd, 1, 1000, NULL);
            
            HWND hStat = CreateWindowA("STATIC", "Master Key:", WS_VISIBLE | WS_CHILD, 15, 62, 80, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hStat, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 100, 60, 200, 25, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
            SendMessage(hPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hStrength = CreateWindowA("STATIC", "", WS_VISIBLE | WS_CHILD, 100, 85, 200, 15, hwnd, (HMENU)ID_STATIC_STRENGTH, NULL, NULL);
            SendMessage(hStrength, WM_SETFONT, (WPARAM)hFont, TRUE);
            
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
            HWND hBtnGen = CreateWindowA("BUTTON", "Gen Pass", WS_VISIBLE | WS_CHILD | BS_FLAT, 490, 320, 90, 25, hwnd, (HMENU)ID_BTN_GENERATE, NULL, NULL);
            SendMessage(hBtnGen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hComboTpl = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 15, 355, 100, 100, hwnd, (HMENU)ID_COMBO_TEMPLATE, NULL, NULL);
            SendMessage(hComboTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Login");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Note");
            SendMessage(hComboTpl, CB_SETCURSEL, 0, 0);
            
            HWND hBtnTpl = CreateWindowA("BUTTON", "Insert", WS_VISIBLE | WS_CHILD | BS_FLAT, 125, 355, 60, 25, hwnd, (HMENU)ID_BTN_INSERT_TEMPLATE, NULL, NULL);
            SendMessage(hBtnTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnCopyData = CreateWindowA("BUTTON", "Copy Data", WS_VISIBLE | WS_CHILD | BS_FLAT, 195, 355, 85, 25, hwnd, (HMENU)ID_BTN_COPY_DATA, NULL, NULL);
            SendMessage(hBtnCopyData, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnClearClip = CreateWindowA("BUTTON", "Clear Clip", WS_VISIBLE | WS_CHILD | BS_FLAT, 290, 355, 85, 25, hwnd, (HMENU)ID_BTN_CLEAR_CLIP, NULL, NULL);
            SendMessage(hBtnClearClip, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnHelp = CreateWindowA("BUTTON", "Help", WS_VISIBLE | WS_CHILD | BS_FLAT, 385, 355, 85, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            srand(GetTickCount());
            DragAcceptFiles(hwnd, TRUE);
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szFile[260];
            if (DragQueryFileA(hDrop, 0, szFile, sizeof(szFile))) {
                HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize > 0) {
                        char* buffer = (char*)malloc(fileSize + 1);
                        DWORD bytesRead;
                        if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                            buffer[bytesRead] = '\0';
                            SetWindowTextA(hData, buffer);
                        }
                        free(buffer);
                    } else {
                        SetWindowTextA(hData, "");
                    }
                    CloseHandle(hFile);
                    MessageBoxA(hwnd, "File dropped and loaded.", "KVault", MB_OK | MB_ICONINFORMATION);
                }
            }
            DragFinish(hDrop);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                if (g_timeoutMs > 0) {
                    DWORD idle = GetTickCount() - g_lastActivity;
                    if (idle > (DWORD)g_timeoutMs) {
                        int textLen = GetWindowTextLengthA(hData);
                        int passLen = GetWindowTextLengthA(hPass);
                        if (textLen > 0 || passLen > 0) {
                            SetWindowTextA(hData, "");
                            SetWindowTextA(hPass, "");
                            if (OpenClipboard(hwnd)) { EmptyClipboard(); CloseClipboard(); }
                            MessageBoxA(hwnd, "Vault locked due to inactivity.", "KVault", MB_OK | MB_ICONINFORMATION);
                        }
                        g_lastActivity = GetTickCount();
                    }
                }
            }
            break;
        }
        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_EDIT_PASS) {
                char pwd[256];
                GetWindowTextA((HWND)lParam, pwd, 256);
                int len = strlen(pwd);
                if (len == 0) {
                    SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "");
                } else {
                    int score = 0;
                    int hasUpper = 0, hasNum = 0, hasSym = 0;
                    for (int i = 0; i < len; i++) {
                        if (pwd[i] >= 'A' && pwd[i] <= 'Z') hasUpper = 1;
                        else if (pwd[i] >= '0' && pwd[i] <= '9') hasNum = 1;
                        else if (!(pwd[i] >= 'a' && pwd[i] <= 'z')) hasSym = 1;
                    }
                    if (len > 4) score++;
                    if (len >= 8) score++;
                    if (len >= 12) score++;
                    if (hasUpper) score++;
                    if (hasNum) score++;
                    if (hasSym) score++;
                    
                    if (score < 3) SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Weak");
                    else if (score < 5) SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Medium");
                    else SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Strong");
                    
                    InvalidateRect(GetDlgItem(hwnd, ID_STATIC_STRENGTH), NULL, TRUE);
                }
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_TIMEOUT) {
                int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                if (sel == 0) g_timeoutMs = 60000;
                else if (sel == 1) g_timeoutMs = 300000;
                else if (sel == 2) g_timeoutMs = 900000;
                else if (sel == 3) g_timeoutMs = 0;
                g_lastActivity = GetTickCount();
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_THEME) {
                g_theme = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                DeleteObject(hbgBrush);
                DeleteObject(hDarkBrush);
                if (g_theme == 0) {
                    hbgBrush = CreateSolidBrush(RGB(15, 15, 19));
                    hDarkBrush = CreateSolidBrush(RGB(25, 25, 30));
                } else if (g_theme == 1) {
                    hbgBrush = CreateSolidBrush(RGB(240, 240, 245));
                    hDarkBrush = CreateSolidBrush(RGB(255, 255, 255));
                } else if (g_theme == 2) {
                    hbgBrush = CreateSolidBrush(RGB(5, 5, 5));
                    hDarkBrush = CreateSolidBrush(RGB(10, 15, 10));
                }
                SetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbgBrush);
                InvalidateRect(hwnd, NULL, TRUE);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }
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
            } else if (LOWORD(wParam) == ID_BTN_GENERATE) {
                GeneratePassword(hData);
            } else if (LOWORD(wParam) == ID_BTN_INSERT_TEMPLATE) {
                int sel = SendMessage(GetDlgItem(hwnd, ID_COMBO_TEMPLATE), CB_GETCURSEL, 0, 0);
                const char* tpl = "";
                if (sel == 0) tpl = "\r\n--- Login ---\r\nURL: \r\nUsername: \r\nPassword: \r\n-------------\r\n";
                else if (sel == 1) tpl = "\r\n--- Finance ---\r\nBank: \r\nAccount: \r\nRouting: \r\nPIN: \r\n---------------\r\n";
                else if (sel == 2) tpl = "\r\n--- Secure Note ---\r\nTitle: \r\nNote: \r\n-------------------\r\n";
                SendMessage(hData, EM_REPLACESEL, TRUE, (LPARAM)tpl);
                SetFocus(hData);
            } else if (LOWORD(wParam) == ID_BTN_COPY_DATA) {
                int textLen = GetWindowTextLengthA(hData);
                if (textLen > 0) {
                    char* text = (char*)malloc(textLen + 1);
                    GetWindowTextA(hData, text, textLen + 1);
                    if (OpenClipboard(hwnd)) {
                        EmptyClipboard();
                        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, textLen + 1);
                        memcpy(GlobalLock(hMem), text, textLen + 1);
                        GlobalUnlock(hMem);
                        SetClipboardData(CF_TEXT, hMem);
                        CloseClipboard();
                        MessageBoxA(hwnd, "Data copied to clipboard.", "KVault", MB_OK | MB_ICONINFORMATION);
                    }
                    free(text);
                }
            } else if (LOWORD(wParam) == ID_BTN_CLEAR_CLIP) {
                if (OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    CloseClipboard();
                    MessageBoxA(hwnd, "Clipboard cleared.", "KVault", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBoxA(hwnd, 
                    "KVault Help & Tutorial\n\n"
                    "1. Master Password: Used to encrypt/decrypt data. It is NOT stored.\n"
                    "2. Encryption: Uses an XOR cipher for this native version (for AES use the Web version).\n"
                    "3. Auto-Lock: Automatically clears fields and clipboard after inactivity.\n"
                    "4. Shortcuts: Ctrl+S saves, Ctrl+L locks instantly.\n"
                    "5. Drag & Drop: Drop a file to load its contents.\n"
                    "6. Clear Clip: Click this after copying passwords to protect against clipboard snooping.",
                    "Help", MB_OK | MB_ICONINFORMATION);
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
            HWND hStatic = (HWND)lParam;
            if (GetDlgCtrlID(hStatic) == ID_STATIC_STRENGTH) {
                char text[64];
                GetWindowTextA(hStatic, text, 64);
                if (strstr(text, "Weak")) SetTextColor(hdc, RGB(255, 71, 87));
                else if (strstr(text, "Medium")) SetTextColor(hdc, RGB(255, 165, 2));
                else if (strstr(text, "Strong")) SetTextColor(hdc, RGB(46, 213, 115));
                else SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
                SetBkColor(hdc, g_theme == 1 ? RGB(240, 240, 245) : (g_theme == 2 ? RGB(5, 5, 5) : RGB(15, 15, 19)));
                return (LRESULT)hbgBrush;
            }
            SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
            SetBkColor(hdc, g_theme == 1 ? RGB(240, 240, 245) : (g_theme == 2 ? RGB(5, 5, 5) : RGB(15, 15, 19)));
            return (LRESULT)hbgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
            SetBkColor(hdc, g_theme == 1 ? RGB(255, 255, 255) : (g_theme == 2 ? RGB(10, 15, 10) : RGB(25, 25, 30)));
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowA("KVaultClass", "KVault", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 610, 435, NULL, NULL, hInstance, NULL);
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    g_lastActivity = GetTickCount();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
            g_lastActivity = GetTickCount();
            if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
                if (msg.wParam == 'S') {
                    SendMessage(hwnd, WM_COMMAND, ID_BTN_SAVE, 0);
                    continue;
                } else if (msg.wParam == 'L') {
                    SetWindowTextA(GetDlgItem(hwnd, ID_EDIT_DATA), "");
                    SetWindowTextA(GetDlgItem(hwnd, ID_EDIT_PASS), "");
                    if (OpenClipboard(hwnd)) { EmptyClipboard(); CloseClipboard(); }
                    MessageBoxA(hwnd, "Vault locked (Ctrl+L).", "KVault", MB_OK | MB_ICONINFORMATION);
                    continue;
                }
            }
        }
        if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
            g_lastActivity = GetTickCount();
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
