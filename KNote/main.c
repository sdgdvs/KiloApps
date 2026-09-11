#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <wincrypt.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")

#define W 800
#define H 600

HWND hEdit, hList, hBtnNew, hBtnDel, hStatus, hSearch, hBtnPin, hBtnExportMd, hBtnExportJson, hBtnImport, hBtnLock, hTab, hBtnHelp;
HBRUSH bgBrush, sidebarBrush, g_hbrClass;
HFONT hFont;

#define ID_BTN_NEW 9005
#define ID_BTN_DEL 9006
#define ID_LIST 9007
#define ID_SEARCH 9008
#define ID_BTN_PIN 9009
#define ID_BTN_EXPORT_MD 9010
#define ID_BTN_EXPORT_JSON 9011
#define ID_BTN_IMPORT 9012
#define ID_BTN_LOCK 9013
#define ID_STATUS 9014
#define ID_TAB 9015
#define ID_TIMER_SAVE 9016
#define ID_BTN_HELP 9017

char notes[100][8192] = {0};
int pinned[100] = {0};
int encrypted[100] = {0};
char unlockedNotes[100][8192] = {0};

int displayToReal[100] = {0};
int displayCount = 0;
int numNotes = 0;
int activeNote = -1;

int tabs[100];
int numTabs = 0;
int isDirty = 0;

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void b64_encode(const unsigned char *in, int in_len, char *out) {
    int i=0, j=0;
    while(i < in_len) {
        unsigned int a = i < in_len ? in[i++] : 0;
        unsigned int b = i < in_len ? in[i++] : 0;
        unsigned int c = i < in_len ? in[i++] : 0;
        unsigned int n = (a << 16) | (b << 8) | c;
        out[j++] = b64[(n >> 18) & 63];
        out[j++] = b64[(n >> 12) & 63];
        out[j++] = (i > in_len + 1) ? '=' : b64[(n >> 6) & 63];
        out[j++] = (i > in_len) ? '=' : b64[n & 63];
    }
    out[j] = '\0';
}
void b64_decode(const char *in, unsigned char *out, int *out_len) {
    int map[256];
    for(int i=0; i<256; i++) map[i] = -1;
    for(int i=0; i<64; i++) map[(unsigned char)b64[i]] = i;
    int i=0, j=0;
    while(in[i] && in[i] != '=') {
        int c0 = (unsigned char)in[i++]; int a = (c0 < 256) ? map[c0] : -1; if(a==-1) break;
        if (!in[i] || in[i] == '=') break;
        int c1 = (unsigned char)in[i++]; int b = (c1 < 256) ? map[c1] : -1; if(b==-1) break;
        int c2 = (in[i] && in[i] != '=') ? (unsigned char)in[i++] : 0; int c = map[c2];
        int c3 = (in[i] && in[i] != '=') ? (unsigned char)in[i++] : 0; int d = map[c3];
        unsigned int n = (a << 18) | (b << 12) | ((c >= 0 ? c : 0) << 6) | (d >= 0 ? d : 0);
        out[j++] = (n >> 16) & 255;
        if(c >= 0) out[j++] = (n >> 8) & 255;
        if(d >= 0) out[j++] = n & 255;
    }
    *out_len = j;
}

int EncryptString(const char* text, const char* pass, char* out_b64) {
    HCRYPTPROV hProv; HCRYPTHASH hHash; HCRYPTKEY hKey;
    if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return 0;
    CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
    CryptHashData(hHash, (BYTE*)pass, lstrlenA(pass), 0);
    CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey);
    DWORD len = lstrlenA(text);
    BYTE buf[10000]; memcpy(buf, text, len + 1);
    DWORD dataLen = len + 1; DWORD bufLen = sizeof(buf);
    if(CryptEncrypt(hKey, 0, TRUE, 0, buf, &dataLen, bufLen)) b64_encode(buf, dataLen, out_b64);
    CryptDestroyKey(hKey); CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0);
    return 1;
}

int DecryptString(const char* b64_str, const char* pass, char* out_text) {
    HCRYPTPROV hProv; HCRYPTHASH hHash; HCRYPTKEY hKey;
    if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return 0;
    CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
    CryptHashData(hHash, (BYTE*)pass, lstrlenA(pass), 0);
    CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey);
    BYTE buf[10000]; int dataLen = 0; b64_decode(b64_str, buf, &dataLen);
    DWORD len = dataLen;
    if(CryptDecrypt(hKey, 0, TRUE, 0, buf, &len)) { memcpy(out_text, buf, len); out_text[len] = 0; }
    else out_text[0] = 0;
    CryptDestroyKey(hKey); CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0);
    return out_text[0] != 0;
}

char password_buf[64] = {0};
static WNDPROC g_oldPassEditProc = NULL;
LRESULT CALLBACK PassEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_RETURN) {
            SendMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)1, 0);
            return 0;
        } else if (wp == VK_ESCAPE) {
            SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
            return 0;
        }
    }
    return CallWindowProcA(g_oldPassEditProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK PassWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND hPassEdit;
    if(msg == WM_CREATE) {
        CreateWindow("STATIC", "Enter Password:", WS_CHILD|WS_VISIBLE, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
        hPassEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_BORDER|ES_PASSWORD|ES_AUTOHSCROLL, 10, 30, 150, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("BUTTON", "OK", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 170, 30, 50, 20, hwnd, (HMENU)1, NULL, NULL);
        g_oldPassEditProc = (WNDPROC)SetWindowLongPtrA(hPassEdit, GWLP_WNDPROC, (LONG_PTR)PassEditProc);
        SetFocus(hPassEdit);
    } else if(msg == WM_COMMAND && LOWORD(wp) == 1) {
        GetWindowTextA(hPassEdit, password_buf, sizeof(password_buf));
        DestroyWindow(hwnd);
    } else if(msg == WM_CLOSE) {
        password_buf[0] = 0; DestroyWindow(hwnd);
    } else return DefWindowProc(hwnd, msg, wp, lp);
    return 0;
}

int PromptPassword(HWND parent) {
    password_buf[0] = 0;
    if (parent) EnableWindow(parent, FALSE);
    HWND pw = CreateWindow("PassWnd", "Password", WS_POPUP|WS_CAPTION|WS_SYSMENU, 300, 300, 250, 100, parent, NULL, NULL, NULL);
    ShowWindow(pw, SW_SHOW);
    MSG pmsg;
    while(IsWindow(pw) && GetMessage(&pmsg, NULL, 0, 0)) { TranslateMessage(&pmsg); DispatchMessage(&pmsg); }
    if (parent) { EnableWindow(parent, TRUE); SetFocus(parent); }
    return password_buf[0] != 0;
}

int StrStrI(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (haystack[i + j] && needle[j]) {
            char c1 = haystack[i + j]; char c2 = needle[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) break;
            j++;
        }
        if (!needle[j]) return 1;
    }
    return 0;
}

void SaveToMemory() {
    if (activeNote >= 0) {
        if (!encrypted[activeNote]) {
            GetWindowTextA(hEdit, notes[activeNote], 6000);
        } else if (unlockedNotes[activeNote][0] != 0) {
            GetWindowTextA(hEdit, unlockedNotes[activeNote], 6000);
        }
    }
}

void LoadNotes() {
    HANDLE hFile = CreateFileA("knote_data_v2.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD fileSize = GetFileSize(hFile, NULL);
        if (fileSize != INVALID_FILE_SIZE && fileSize > 0 && fileSize < 10 * 1024 * 1024) {
            char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
            if (buf) {
                DWORD bytesRead = 0;
                ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
                buf[bytesRead] = 0;
                numNotes = 0; char* p = buf;
                while(p < buf + bytesRead && numNotes < 100) {
                    char* next = p; while(*next && *next != '\1') next++; *next = 0;
                    int l = (int)(next - p);
                    if (l > 0) {
                        if (l >= 2 && p[1] == '|') {
                            pinned[numNotes] = (p[0] == '1') ? 1 : 0;
                            encrypted[numNotes] = (p[0] == 'E') ? 1 : 0;
                            if(p[0] == 'P') { pinned[numNotes] = 1; encrypted[numNotes] = 1; }
                            p += 2; l -= 2;
                        } else { pinned[numNotes] = 0; encrypted[numNotes] = 0; }
                        if (l > (int)sizeof(notes[0])-1) l = (int)sizeof(notes[0])-1;
                        for(int i=0; i<l; i++) notes[numNotes][i] = p[i];
                        notes[numNotes][l] = 0;
                        numNotes++;
                    }
                    p = next + 1;
                }
                HeapFree(GetProcessHeap(), 0, buf);
            }
        }
        CloseHandle(hFile);
    }
    if (numNotes == 0) {
        numNotes = 1; pinned[0] = 0; encrypted[0] = 0;
        const char* def = "Welcome to KNote!\r\n- #tags supported\r\n- Tabs available\r\n- AES encryption\r\n- Click Help or press F1/H for Help";
        lstrcpyA(notes[0], def);
    }
}

void SaveNotes() {
    SaveToMemory();
    HANDLE hFile = CreateFileA("knote_data_v2.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        for(int i=0; i<numNotes; i++) {
            char flags[3] = {'0', '|', 0};
            if(pinned[i] && encrypted[i]) flags[0] = 'P';
            else if(pinned[i]) flags[0] = '1';
            else if(encrypted[i]) flags[0] = 'E';
            WriteFile(hFile, flags, 2, &bw, NULL);
            int l = lstrlenA(notes[i]);
            WriteFile(hFile, notes[i], l, &bw, NULL);
            char delim = '\1';
            WriteFile(hFile, &delim, 1, &bw, NULL);
        }
        CloseHandle(hFile);
    }
    isDirty = 0;
}

void ExportNoteMD() {
    if (activeNote < 0) return;
    SaveToMemory();
    OPENFILENAMEA ofn; char szFile[260] = "note.md";
    ZeroMemory(&ofn, sizeof(ofn)); ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hEdit; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Markdown (*.md)\0*.md\0All Files (*.*)\0*.*\0"; ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "md";
    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bw; 
            char* txt = encrypted[activeNote] ? unlockedNotes[activeNote] : notes[activeNote];
            WriteFile(hFile, txt, lstrlenA(txt), &bw, NULL);
            CloseHandle(hFile);
        }
    }
}

void ExportJSON() {
    SaveToMemory();
    OPENFILENAMEA ofn; char szFile[260] = "knote_export.json";
    ZeroMemory(&ofn, sizeof(ofn)); ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hEdit; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON (*.json)\0*.json\0All Files (*.*)\0*.*\0"; ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "json";
    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bw; WriteFile(hFile, "[\n", 2, &bw, NULL);
            for(int i=0; i<numNotes; i++) {
                char* txt = encrypted[i] ? unlockedNotes[i] : notes[i];
                char buf[8192];
                wsprintfA(buf, "  {\n    \"id\": \"n_%d\",\n    \"pinned\": %s,\n    \"text\": \"", i, pinned[i]?"true":"false");
                WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);
                for(int j=0; txt[j]; j++) {
                    if(txt[j]=='\n') WriteFile(hFile, "\\n", 2, &bw, NULL);
                    else if(txt[j]=='\r') WriteFile(hFile, "\\r", 2, &bw, NULL);
                    else if(txt[j]=='\\') WriteFile(hFile, "\\\\", 2, &bw, NULL);
                    else if(txt[j]=='"') WriteFile(hFile, "\\\"", 2, &bw, NULL);
                    else WriteFile(hFile, &txt[j], 1, &bw, NULL);
                }
                wsprintfA(buf, "\"\n  }%s\n", i==numNotes-1 ? "" : ",");
                WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);
            }
            WriteFile(hFile, "]\n", 2, &bw, NULL);
            CloseHandle(hFile);
        }
    }
}

void RefreshList();
void RenderTabs();

void ImportJSON() {
    OPENFILENAMEA ofn; char szFile[260] = "";
    ZeroMemory(&ofn, sizeof(ofn)); ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hEdit; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON (*.json)\0*.json\0All Files (*.*)\0*.*\0"; ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE && fileSize > 0 && fileSize < 10 * 1024 * 1024) {
                char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                if (buf) {
                    DWORD br = 0; ReadFile(hFile, buf, fileSize, &br, NULL);
                    buf[br] = 0;
                    char* p = buf;
                    while((p = strstr(p, "\"text\":")) != NULL) {
                        if (numNotes >= 100) break;
                        p += 7;
                        while(*p == ' ' || *p == '\t') p++;
                        if(*p == '"') {
                            p++; char* txt = notes[numNotes]; int j=0;
                            while(*p && *p != '"' && j < 5999) {
                                if(*p == '\\' && *(p+1) == 'n') { txt[j++] = '\n'; p+=2; }
                                else if(*p == '\\' && *(p+1) == 'r') { txt[j++] = '\r'; p+=2; }
                                else if(*p == '\\' && *(p+1) == '\\') { txt[j++] = '\\'; p+=2; }
                                else if(*p == '\\' && *(p+1) == '"') { txt[j++] = '"'; p+=2; }
                                else txt[j++] = *p++;
                            }
                            txt[j] = 0; pinned[numNotes] = 0; encrypted[numNotes] = 0; numNotes++;
                        }
                    }
                    HeapFree(GetProcessHeap(), 0, buf);
                }
            }
            CloseHandle(hFile);
            SaveNotes();
        }
    }
}

void UpdateStats() {
    int len = GetWindowTextLengthA(hEdit);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    if (buf) {
        GetWindowTextA(hEdit, buf, len + 1);
        int words = 0, chars = len, lines = (len > 0) ? 1 : 0;
        int inWord = 0;
        for (int i = 0; i < len; i++) {
            if (buf[i] == '\n') lines++;
            if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\t') inWord = 0;
            else if (!inWord) { inWord = 1; words++; }
        }
        HeapFree(GetProcessHeap(), 0, buf);
        char stat[96]; wsprintfA(stat, "  Lines: %d | Words: %d | Chars: %d / 6,000", lines, words, chars);
        SetWindowTextA(hStatus, stat);
    }
}

void LoadActiveNote() {
    if (activeNote >= 0) {
        if(encrypted[activeNote]) {
            if(unlockedNotes[activeNote][0] != 0) {
                EnableWindow(hEdit, TRUE);
                SetWindowTextA(hEdit, unlockedNotes[activeNote]);
                SetWindowTextA(hBtnLock, "Unlocked");
            } else {
                EnableWindow(hEdit, FALSE);
                SetWindowTextA(hEdit, "--- NOTE LOCKED ---");
                SetWindowTextA(hBtnLock, "Unlock");
            }
        } else {
            EnableWindow(hEdit, TRUE);
            SetWindowTextA(hEdit, notes[activeNote]);
            SetWindowTextA(hBtnLock, "Lock");
        }
        UpdateStats();
        SetWindowTextA(hBtnPin, pinned[activeNote] ? "Unpin [^P]" : "Pin [^P]");

        HWND hMain = GetAncestor(hEdit, GA_ROOT);
        if (hMain) {
            char title[128];
            char noteName[32] = "Untitled";
            char* raw = encrypted[activeNote] ? (unlockedNotes[activeNote][0] ? unlockedNotes[activeNote] : "Locked") : notes[activeNote];
            int k = 0; while(raw[k] && raw[k] != '\r' && raw[k] != '\n' && k < 24) { noteName[k] = raw[k]; k++; }
            noteName[k] = 0; if (k == 0) lstrcpyA(noteName, "Empty Note");
            wsprintfA(title, "KNote - [%s] - Press F1 or H for Help", noteName);
            SetWindowTextA(hMain, title);
        }
    } else {
        SetWindowTextA(hEdit, ""); EnableWindow(hEdit, FALSE);
        HWND hMain = GetAncestor(hEdit, GA_ROOT);
        if (hMain) SetWindowTextA(hMain, "KNote - Press F1 or H for Help");
    }
    RenderTabs();
}

void RenderTabs() {
    TabCtrl_DeleteAllItems(hTab);
    for(int i=0; i<numTabs; i++) {
        TCITEM tie; tie.mask = TCIF_TEXT; 
        char title[32] = "Empty";
        char* raw = encrypted[tabs[i]] ? (unlockedNotes[tabs[i]][0]?unlockedNotes[tabs[i]]:"Locked") : notes[tabs[i]];
        int j = 0;
        while(raw[j] && raw[j]!='\r' && raw[j]!='\n' && j<15) { title[j] = raw[j]; j++; }
        title[j] = 0; if(j==0) lstrcpyA(title, "Empty");
        tie.pszText = title;
        TabCtrl_InsertItem(hTab, i, &tie);
        if(tabs[i] == activeNote) SendMessage(hTab, TCM_SETCURSEL, i, 0);
    }
}

void OpenTab(int noteIdx) {
    for(int i=0; i<numTabs; i++) if(tabs[i] == noteIdx) { activeNote = noteIdx; LoadActiveNote(); return; }
    if(numTabs<100) { tabs[numTabs++] = noteIdx; activeNote = noteIdx; LoadActiveNote(); }
}

void CloseTab(int tabIdx) {
    if(tabIdx < 0 || tabIdx >= numTabs) return;
    int closingNote = tabs[tabIdx];
    for(int i=tabIdx; i<numTabs-1; i++) tabs[i] = tabs[i+1];
    numTabs--;
    if(activeNote == closingNote) {
        if(numTabs > 0) activeNote = tabs[numTabs-1];
        else activeNote = -1;
    }
    LoadActiveNote();
    RefreshList();
}

void RefreshList() {
    char query[64] = {0}; if (hSearch) GetWindowTextA(hSearch, query, 64);
    SendMessage(hList, LB_RESETCONTENT, 0, 0); displayCount = 0;
    
    for (int pPass = 1; pPass >= 0; pPass--) {
        for (int i = 0; i < numNotes; i++) {
            if ((pinned[i] ? 1 : 0) != pPass) continue;
            char* txt = encrypted[i] ? (unlockedNotes[i][0]?unlockedNotes[i]:"Locked") : notes[i];
            if (query[0] != 0 && !StrStrI(txt, query)) continue;
            displayToReal[displayCount] = i;
            char title[64] = {0}; int offset = 0;
            if (pinned[i]) { lstrcpyA(title, "[P] "); offset = 4; }
            if (encrypted[i] && unlockedNotes[i][0]==0) { lstrcpyA(title+offset, "[L] "); offset += 4; }
            int j = 0;
            while (txt[j] && txt[j] != '\r' && txt[j] != '\n' && j < 30) { title[offset + j] = txt[j]; j++; }
            if (j == 0) lstrcpyA(title+offset, "Empty");
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)title);
            if (i == activeNote) SendMessage(hList, LB_SETCURSEL, displayCount, 0);
            displayCount++;
        }
    }
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KNote - Personal Notepad & Organizer\r\n"
        "======================================\r\n\r\n"
        "KEYBOARD SHORTCUTS:\r\n"
        "  - Ctrl+N        : Create new note\r\n"
        "  - Ctrl+F        : Focus search & tags box\r\n"
        "  - Ctrl+P        : Pin / unpin current note\r\n"
        "  - Ctrl+S        : Save notes immediately\r\n"
        "  - Ctrl+W        : Close active tab\r\n"
        "  - Ctrl+Tab      : Cycle next open tab\r\n"
        "  - Ctrl+1..9     : Switch directly to tab 1 to 9\r\n"
        "  - Ctrl+D / Del  : Delete selected note\r\n"
        "  - Esc           : Clear search / return to note list\r\n"
        "  - F1 or H       : Open this Help manual\r\n\r\n"
        "FEATURES:\r\n"
        "  - Tags: Type #tag anywhere in notes to categorize\r\n"
        "  - AES-256: Lock notes with secure password encryption\r\n"
        "  - Backups: Export Markdown or standard JSON\r\n"
        "  - Multi-tab: Open and edit multiple notes seamlessly",
        "KNote Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

static WNDPROC g_oldSearchEditProc = NULL;
LRESULT CALLBACK SearchEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_ESCAPE) {
            SetWindowTextA(hwnd, "");
            RefreshList();
            SetFocus(hList);
            return 0;
        } else if (wp == VK_RETURN || wp == VK_DOWN) {
            SetFocus(hList);
            return 0;
        }
    }
    return CallWindowProcA(g_oldSearchEditProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex; icex.dwSize = sizeof(INITCOMMONCONTROLSEX); icex.dwICC = ICC_TAB_CLASSES;
            InitCommonControlsEx(&icex);
            
            WNDCLASS pc = {0}; pc.lpfnWndProc = PassWndProc; pc.lpszClassName = "PassWnd"; pc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
            RegisterClass(&pc);

            bgBrush = CreateSolidBrush(RGB(255, 253, 231));
            sidebarBrush = CreateSolidBrush(RGB(255, 249, 196));
            HDC hdcScreen = GetDC(NULL);
            int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSY);
            ReleaseDC(NULL, hdcScreen);
            int fontHeight = -MulDiv(12, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
            
            hBtnNew = CreateWindow("BUTTON", "New [^N]", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 0, 0, 100, 26, hwnd, (HMENU)ID_BTN_NEW, NULL, NULL);
            hBtnDel = CreateWindow("BUTTON", "Del [Del]", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 100, 0, 100, 26, hwnd, (HMENU)ID_BTN_DEL, NULL, NULL);
            hSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, 0, 26, 200, 22, hwnd, (HMENU)ID_SEARCH, NULL, NULL);
            SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search tags... (Ctrl+F, Esc clear)");
            hList = CreateWindowEx(0, "LISTBOX", NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|LBS_NOTIFY, 0, 48, 200, H-48, hwnd, (HMENU)ID_LIST, NULL, NULL);
            
            hBtnPin = CreateWindow("BUTTON", "Pin [^P]", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 200, 0, 70, 26, hwnd, (HMENU)ID_BTN_PIN, NULL, NULL);
            hBtnLock = CreateWindow("BUTTON", "Lock", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 270, 0, 65, 26, hwnd, (HMENU)ID_BTN_LOCK, NULL, NULL);
            hBtnExportMd = CreateWindow("BUTTON", "Exp MD", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 335, 0, 75, 26, hwnd, (HMENU)ID_BTN_EXPORT_MD, NULL, NULL);
            hBtnExportJson = CreateWindow("BUTTON", "Exp JSON", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 410, 0, 80, 26, hwnd, (HMENU)ID_BTN_EXPORT_JSON, NULL, NULL);
            hBtnImport = CreateWindow("BUTTON", "Imp JSON", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 490, 0, 80, 26, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
            hBtnHelp = CreateWindow("BUTTON", "Help [F1]", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON, 570, 0, 80, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            g_oldSearchEditProc = (WNDPROC)SetWindowLongPtrA(hSearch, GWLP_WNDPROC, (LONG_PTR)SearchEditProc);

            hTab = CreateWindow(WC_TABCONTROL, "", WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|WS_TABSTOP, 200, 26, W-200, 24, hwnd, (HMENU)ID_TAB, NULL, NULL);

            hEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL,
                200, 50, W-200, H-70, hwnd, NULL, NULL, NULL);
            SendMessage(hEdit, EM_LIMITTEXT, 6000, 0);
            hStatus = CreateWindowEx(0, "STATIC", "  Ready. Press F1 or H for Help. | Lines: 0 | Words: 0 | Chars: 0 / 6,000", WS_CHILD|WS_VISIBLE, 200, H-20, W-200, 20, hwnd, (HMENU)ID_STATUS, NULL, NULL);
                
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnNew, WM_SETFONT, (WPARAM)hFont, TRUE); SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPin, WM_SETFONT, (WPARAM)hFont, TRUE); SendMessage(hBtnLock, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExportMd, WM_SETFONT, (WPARAM)hFont, TRUE); SendMessage(hBtnExportJson, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE); SendMessage(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LoadNotes();
            if(numNotes>0) OpenTab(0);
            RefreshList();
            
            SetTimer(hwnd, ID_TIMER_SAVE, 3000, NULL);
            break;
        }
        case WM_TIMER:
            if(wParam == ID_TIMER_SAVE && isDirty) SaveNotes();
            break;
        case WM_NOTIFY: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            if(lpnm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(hTab);
                if(sel>=0 && sel<numTabs) { activeNote = tabs[sel]; LoadActiveNote(); RefreshList(); }
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_HELP) {
                ShowHelpDialog(hwnd);
            }
            else if (LOWORD(wParam) == ID_BTN_EXPORT_MD) { ExportNoteMD(); }
            else if (LOWORD(wParam) == ID_BTN_EXPORT_JSON) { ExportJSON(); RefreshList(); RenderTabs(); }
            else if (LOWORD(wParam) == ID_BTN_IMPORT) { ImportJSON(); RefreshList(); RenderTabs(); }
            else if (LOWORD(wParam) == ID_BTN_NEW) {
                if (numNotes < 100) {
                    SaveToMemory();
                    notes[numNotes][0] = 0; pinned[numNotes] = 0; encrypted[numNotes] = 0;
                    OpenTab(numNotes); numNotes++;
                    RefreshList(); isDirty = 1;
                }
            } else if (LOWORD(wParam) == ID_BTN_DEL) {
                if (activeNote>=0) {
                    if (MessageBoxA(hwnd, "Are you sure you want to delete this note?\nThis action cannot be undone.", "Confirm Delete", MB_YESNO | MB_ICONQUESTION) != IDYES) break;
                    int toDel = activeNote;
                    for(int i=0; i<numTabs; i++) if(tabs[i]==toDel) { CloseTab(i); break; }
                    for(int i=toDel; i<numNotes-1; i++) {
                        lstrcpyA(notes[i], notes[i+1]);
                        pinned[i] = pinned[i+1]; encrypted[i] = encrypted[i+1];
                        lstrcpyA(unlockedNotes[i], unlockedNotes[i+1]);
                    }
                    numNotes--;
                    for(int i=0; i<numTabs; i++) if(tabs[i]>toDel) tabs[i]--;
                    RefreshList(); isDirty = 1;
                }
            } else if (LOWORD(wParam) == ID_BTN_PIN) {
                if (activeNote >= 0) { pinned[activeNote] = !pinned[activeNote]; RefreshList(); isDirty=1; }
            } else if (LOWORD(wParam) == ID_BTN_LOCK) {
                if (activeNote >= 0) {
                    if (encrypted[activeNote]) {
                        if(unlockedNotes[activeNote][0]) {
                            encrypted[activeNote] = 0; lstrcpyA(notes[activeNote], unlockedNotes[activeNote]);
                            unlockedNotes[activeNote][0] = 0;
                            LoadActiveNote(); RefreshList(); isDirty=1;
                        } else {
                            if(PromptPassword(hwnd)) {
                                if(DecryptString(notes[activeNote], password_buf, unlockedNotes[activeNote])) {
                                    LoadActiveNote(); RefreshList();
                                } else MessageBox(hwnd, "Wrong password!", "Error", MB_OK);
                                SecureZeroMemory(password_buf, sizeof(password_buf));
                            }
                        }
                    } else {
                        if(PromptPassword(hwnd)) {
                            encrypted[activeNote] = 1;
                            GetWindowTextA(hEdit, unlockedNotes[activeNote], 6000);
                            EncryptString(unlockedNotes[activeNote], password_buf, notes[activeNote]);
                            SecureZeroMemory(password_buf, sizeof(password_buf));
                            LoadActiveNote(); RefreshList(); isDirty=1;
                        }
                    }
                }
            } else if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                SaveToMemory();
                int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < displayCount) { OpenTab(displayToReal[sel]); }
            }
            else if ((HWND)lParam == hSearch && HIWORD(wParam) == EN_CHANGE) { RefreshList(); }
            else if ((HWND)lParam == hEdit && HIWORD(wParam) == EN_CHANGE) {
                UpdateStats(); isDirty = 1;
                if(encrypted[activeNote]) GetWindowTextA(hEdit, unlockedNotes[activeNote], 6000);
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam; SetBkColor(hdc, RGB(255, 253, 231)); return (LRESULT)bgBrush;
        }
        case WM_CTLCOLORSTATIC: {
            if ((HWND)lParam == hStatus) { HDC hdc = (HDC)wParam; SetBkColor(hdc, RGB(255, 249, 196)); return (LRESULT)sidebarBrush; }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_CTLCOLORLISTBOX: { HDC hdc = (HDC)wParam; SetBkColor(hdc, RGB(255, 249, 196)); return (LRESULT)sidebarBrush; }
        case WM_SIZE: {
            int nw = LOWORD(lParam), nh = HIWORD(lParam); int sideW = 200, topH = 26;
            MoveWindow(hBtnNew, 0, 0, 100, topH, TRUE); MoveWindow(hBtnDel, 100, 0, 100, topH, TRUE);
            MoveWindow(hSearch, 0, topH, sideW, 22, TRUE); MoveWindow(hList, 0, topH + 22, sideW, nh - (topH + 22), TRUE);
            int bx = sideW;
            MoveWindow(hBtnPin, bx, 0, 70, topH, TRUE); bx += 70;
            MoveWindow(hBtnLock, bx, 0, 65, topH, TRUE); bx += 65;
            MoveWindow(hBtnExportMd, bx, 0, 75, topH, TRUE); bx += 75;
            MoveWindow(hBtnExportJson, bx, 0, 80, topH, TRUE); bx += 80;
            MoveWindow(hBtnImport, bx, 0, 80, topH, TRUE); bx += 80;
            MoveWindow(hBtnHelp, bx, 0, 80, topH, TRUE);
            MoveWindow(hTab, sideW, topH, nw - sideW, 24, TRUE);
            MoveWindow(hEdit, sideW, topH + 24, nw - sideW, nh - topH - 44, TRUE);
            MoveWindow(hStatus, sideW, nh - 20, nw - sideW, 20, TRUE);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, ID_TIMER_SAVE);
            SaveNotes();
            DeleteObject(bgBrush);
            DeleteObject(sidebarBrush);
            if (hFont) DeleteObject(hFont);
            if (g_hbrClass) DeleteObject(g_hbrClass);
            PostQuitMessage(0);
            break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) { char* b = (char*)dest; while (count--) *b++ = (char)c; return dest; }
#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest; const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}
char* __cdecl strstr(const char* h, const char* n) {
    if (!*n) return (char*)h;
    for (; *h; h++) {
        const char* n1 = n; const char* p = h;
        while (*n1 && *p && *p == *n1) { p++; n1++; }
        if (!*n1) return (char*)h;
    }
    return NULL;
}

void MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if(hUser32) {
        typedef BOOL (WINAPI *SetProcessDPIAwareFunc)(void);
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if(setDpiAware) setDpiAware();
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    g_hbrClass = CreateSolidBrush(RGB(255, 253, 231));
    WNDCLASS wc = {0}; wc.lpfnWndProc = WndProc; wc.hInstance = hInstance; wc.lpszClassName = "KNoteApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1)); wc.hbrBackground = g_hbrClass;
    RegisterClass(&wc);
    
    RECT rc = {0, 0, W, H};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowEx(0, "KNoteApp", "KNote - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW); UpdateWindow(hwnd);
    MSG msg; 
    while (GetMessage(&msg, NULL, 0, 0) > 0) { 
        if (msg.message == WM_KEYDOWN) {
            int isEdit = 0;
            HWND hFocus = GetFocus();
            if (hFocus) {
                char cls[32] = {0};
                GetClassNameA(hFocus, cls, sizeof(cls));
                if (lstrcmpiA(cls, "EDIT") == 0) isEdit = 1;
            }
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (msg.wParam == 'N' || msg.wParam == 'n') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_NEW, BN_CLICKED), (LPARAM)hBtnNew);
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    SaveNotes();
                    SetWindowTextA(hStatus, "  Saved locally [Ctrl+S]");
                    continue;
                } else if (msg.wParam == 'F' || msg.wParam == 'f') {
                    SetFocus(hSearch);
                    SendMessage(hSearch, EM_SETSEL, 0, -1);
                    continue;
                } else if (msg.wParam == 'P' || msg.wParam == 'p') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PIN, BN_CLICKED), (LPARAM)hBtnPin);
                    continue;
                } else if (msg.wParam == 'D' || msg.wParam == 'd') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DEL, BN_CLICKED), (LPARAM)hBtnDel);
                    continue;
                } else if (msg.wParam == 'W' || msg.wParam == 'w') {
                    int sel = (int)TabCtrl_GetCurSel(hTab);
                    if (sel >= 0 && sel < numTabs) CloseTab(sel);
                    continue;
                } else if (msg.wParam == VK_TAB) {
                    if (numTabs > 1) {
                        int sel = (int)TabCtrl_GetCurSel(hTab);
                        sel = (sel + 1) % numTabs;
                        TabCtrl_SetCurSel(hTab, sel);
                        activeNote = tabs[sel];
                        LoadActiveNote();
                        RefreshList();
                    }
                    continue;
                } else if (msg.wParam >= '1' && msg.wParam <= '9') {
                    int targetTab = (int)(msg.wParam - '1');
                    if (targetTab < numTabs) {
                        TabCtrl_SetCurSel(hTab, targetTab);
                        activeNote = tabs[targetTab];
                        LoadActiveNote();
                        RefreshList();
                    }
                    continue;
                }
            }
            if (msg.wParam == VK_DELETE && hFocus == hList) {
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DEL, BN_CLICKED), (LPARAM)hBtnDel);
                continue;
            }
            if (msg.wParam == VK_F1 || (!isEdit && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                ShowHelpDialog(hwnd);
                continue;
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    ExitProcess(0);
}
