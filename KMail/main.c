#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#define W 900
#define H 600

#define ID_FOLDER_LIST 101
#define ID_EMAIL_LIST 102
#define ID_BTN_COMPOSE 103
#define ID_BTN_DELETE 104
#define ID_BTN_EMPTY_TRASH 105
#define ID_SEARCH_BOX 106
#define ID_TAG_FILTER 107
#define ID_BTN_IMPORT 108
#define ID_BTN_EXPORT 109
#define ID_TAB 110
#define ID_BTN_TAG 111
#define ID_BTN_DECRYPT 112

HWND hFolders, hEmails, hTitle, hBody, hBtnCompose, hBtnDelete, hBtnEmptyTrash, hSearchBox;
HWND hTagFilter, hBtnImport, hBtnExport, hTab, hBtnTag, hBtnDecrypt, hHelpLabel;

typedef struct {
    int id;
    int folder; // 0=inbox, 1=sent, 2=trash
    char subject[128];
    char sender[128];
    char body[2048];
    int unread;
    int encrypted;
    char tags[128]; // comma separated
} Email;

Email emails[200] = {
    {1, 0, "Welcome to KiloOS", "sysadmin@kilo.os", "Hello User,\r\n\r\nWelcome to KiloOS.\r\n\r\n- SysAdmin", 1, 0, "Work"},
    {2, 0, "Meeting at 3PM", "boss@kilo.os", "Don't forget our meeting at 3PM.", 0, 0, "Urgent,Work"},
    {3, 0, "Top Secret Info", "agent@kilo.os", "\x11\x14\x05\x5e\x06\x16\x16\x07\x16\x11\x5e\x1c\x19\x1f\x18", 1, 1, "Personal"}, // Mock XOR encrypted with pass "pass"
    {4, 0, "Newsletter #42", "news@kilo.os", "Weekly digest...", 0, 0, ""},
    {5, 1, "Re: Meeting at 3PM", "me@kilo.os", "I'll be there.", 0, 0, ""}
};
int num_emails = 5;

typedef struct {
    int id; // 0 for compose
    int emailId; // valid if id > 0
    char composeTo[128];
    char composeSub[128];
    char composeBody[2048];
    int composeEncrypted;
} TabData;

TabData tabs[20];
int num_tabs = 0;
int currentTabIdx = -1;

int currentFolder = 0; // 0=inbox, 1=sent, 2=trash
char searchQuery[128] = "";
char tagQuery[64] = "";
int nextId = 6;

#define my_tolower(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 32) : (c))

int contains_nocase(const char* haystack, const char* needle) {
    if (!needle[0]) return 1;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (needle[j] && my_tolower((unsigned char)haystack[i + j]) == my_tolower((unsigned char)needle[j])) j++;
        if (!needle[j]) return 1;
    }
    return 0;
}

HBRUSH hbgMain, hbgList;
HFONT hFont, hBold;
COLORREF textCol = RGB(248, 250, 252);
COLORREF bgMainCol = RGB(15, 23, 42);
COLORREF bgListCol = RGB(30, 41, 59);

void RenderPane();

void RefreshEmailList() {
    SendMessage(hEmails, LB_RESETCONTENT, 0, 0);
    for(int i = 0; i < num_emails; i++) {
        if(emails[i].folder == currentFolder) {
            if (searchQuery[0] == '\0' || 
                contains_nocase(emails[i].subject, searchQuery) || 
                contains_nocase(emails[i].sender, searchQuery)) {
                
                if (tagQuery[0] == '\0' || contains_nocase(emails[i].tags, tagQuery)) {
                    char displayStr[256];
                    wsprintfA(displayStr, "%s%s", emails[i].encrypted ? "[ENC] " : "", emails[i].subject);
                    int idx = SendMessageA(hEmails, LB_ADDSTRING, 0, (LPARAM)displayStr);
                    SendMessage(hEmails, LB_SETITEMDATA, idx, emails[i].id);
                }
            }
        }
    }
}

void SelectTab(int tIdx) {
    currentTabIdx = tIdx;
    SendMessage(hTab, TCM_SETCURSEL, tIdx, 0);
    RenderPane();
}

void OpenEmailTab(int eid) {
    for(int i=0; i<num_tabs; i++) {
        if(tabs[i].emailId == eid) {
            SelectTab(i);
            return;
        }
    }
    if(num_tabs >= 20) return;
    
    Email* em = NULL;
    for(int i=0; i<num_emails; i++) if(emails[i].id == eid) em = &emails[i];
    if(!em) return;
    em->unread = 0;

    tabs[num_tabs].id = 1;
    tabs[num_tabs].emailId = eid;
    
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = em->subject;
    SendMessage(hTab, TCM_INSERTITEM, num_tabs, (LPARAM)&tie);
    
    num_tabs++;
    SelectTab(num_tabs - 1);
    RefreshEmailList();
}

void NewComposeTab() {
    if(num_tabs >= 20) return;
    tabs[num_tabs].id = 0;
    tabs[num_tabs].emailId = 0;
    tabs[num_tabs].composeTo[0] = 0;
    tabs[num_tabs].composeSub[0] = 0;
    tabs[num_tabs].composeBody[0] = 0;
    tabs[num_tabs].composeEncrypted = 0;

    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = "New Msg";
    SendMessage(hTab, TCM_INSERTITEM, num_tabs, (LPARAM)&tie);
    
    num_tabs++;
    SelectTab(num_tabs - 1);
}

void CloseCurrentTab() {
    if(currentTabIdx == -1) return;
    SendMessage(hTab, TCM_DELETEITEM, currentTabIdx, 0);
    for(int i = currentTabIdx; i < num_tabs - 1; i++) {
        tabs[i] = tabs[i+1];
    }
    num_tabs--;
    if(num_tabs == 0) {
        currentTabIdx = -1;
    } else {
        if(currentTabIdx >= num_tabs) currentTabIdx = num_tabs - 1;
    }
    SelectTab(currentTabIdx);
}

void RenderPane() {
    if(currentTabIdx == -1) {
        SetWindowTextA(hTitle, "No email selected");
        SetWindowTextA(hBody, "Select an email from the list to read, or click 'Compose' to write a new one.\r\n\r\nFeatures:\r\n- Switch between Inbox, Sent, and Trash folders.\r\n- Search and filter by tags.\r\n- Open multiple emails in tabs.\r\n- Encrypt your messages with a password.\r\n\r\nPress 'h' for help.");
        ShowWindow(hBtnTag, SW_HIDE);
        ShowWindow(hBtnDecrypt, SW_HIDE);
        SetWindowLong(hBody, GWL_STYLE, GetWindowLong(hBody, GWL_STYLE) | ES_READONLY);
        return;
    }
    
    TabData* t = &tabs[currentTabIdx];
    if(t->id == 1) {
        // Read mode
        Email* em = NULL;
        for(int i=0; i<num_emails; i++) if(emails[i].id == t->emailId) em = &emails[i];
        if(!em) { CloseCurrentTab(); return; }

        char tStr[512];
        wsprintfA(tStr, "%s: %s\r\nTags: %s", em->folder == 1 ? "To" : "From", em->sender, em->tags);
        SetWindowTextA(hTitle, tStr);
        SetWindowLong(hBody, GWL_STYLE, GetWindowLong(hBody, GWL_STYLE) | ES_READONLY);
        
        if(em->encrypted) {
            SetWindowTextA(hBody, "🔒 This message is encrypted. Click Decrypt to view.");
            ShowWindow(hBtnDecrypt, SW_SHOW);
        } else {
            SetWindowTextA(hBody, em->body);
            ShowWindow(hBtnDecrypt, SW_HIDE);
        }
        ShowWindow(hBtnTag, SW_SHOW);
        
    } else {
        // Compose mode
        SetWindowTextA(hTitle, "Compose Message (First line To, Second Subj)");
        SetWindowLong(hBody, GWL_STYLE, GetWindowLong(hBody, GWL_STYLE) & ~ES_READONLY);
        char compStr[2048];
        wsprintfA(compStr, "To: \r\nSub: \r\n\r\n(Write body here...)");
        SetWindowTextA(hBody, compStr);
        
        ShowWindow(hBtnTag, SW_HIDE);
        ShowWindow(hBtnDecrypt, SW_SHOW); // Reuse decrypt btn as 'Send' in compose mode
        SetWindowTextA(hBtnDecrypt, "Send");
    }
}

void CryptStr(char* data, const char* pass) {
    int pLen = lstrlenA(pass);
    if(pLen == 0) return;
    for(int i = 0; data[i]; i++) {
        data[i] ^= pass[i % pLen];
    }
}

// Simple JSON Export
void ExportJson() {
    HANDLE hFile = CreateFileA("mailbox.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(hFile, "[\r\n", 3, &written, NULL);
    for(int i=0; i<num_emails; i++) {
        char buf[2048];
        wsprintfA(buf, "  {\"id\":%d,\"folder\":%d,\"subject\":\"%s\",\"sender\":\"%s\",\"unread\":%d,\"encrypted\":%d,\"tags\":\"%s\"}%s\r\n",
            emails[i].id, emails[i].folder, emails[i].subject, emails[i].sender, emails[i].unread, emails[i].encrypted, emails[i].tags, (i==num_emails-1)?"":",");
        WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    }
    WriteFile(hFile, "]\r\n", 3, &written, NULL);
    CloseHandle(hFile);
    MessageBoxA(NULL, "Mailbox exported to mailbox.json", "Export", MB_OK);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitCommonControls();
            hbgMain = CreateSolidBrush(bgMainCol);
            hbgList = CreateSolidBrush(bgListCol);

            hFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hBold = CreateFontA(-18, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnCompose = CreateWindowEx(0, "BUTTON", "Compose", WS_CHILD | WS_VISIBLE, 10, 10, 100, 30, hwnd, (HMENU)ID_BTN_COMPOSE, NULL, NULL);
            SendMessage(hBtnCompose, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnImport = CreateWindowEx(0, "BUTTON", "Import", WS_CHILD | WS_VISIBLE, 115, 10, 60, 30, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
            hBtnExport = CreateWindowEx(0, "BUTTON", "Export", WS_CHILD | WS_VISIBLE, 180, 10, 60, 30, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            hHelpLabel = CreateWindowEx(0, "STATIC", "Press 'H' for Help", WS_CHILD | WS_VISIBLE, 250, 15, 120, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hHelpLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFolders = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 50, 100, H - 100, hwnd, (HMENU)ID_FOLDER_LIST, NULL, NULL);
            SendMessage(hFolders, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"Inbox");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"Sent");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"Trash");
            SendMessage(hFolders, LB_SETCURSEL, 0, 0);

            hBtnEmptyTrash = CreateWindowEx(0, "BUTTON", "Empty Trash", WS_CHILD | WS_VISIBLE, 10, H - 40, 100, 30, hwnd, (HMENU)ID_BTN_EMPTY_TRASH, NULL, NULL);
            SendMessage(hBtnEmptyTrash, WM_SETFONT, (WPARAM)hFont, TRUE);

            hSearchBox = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 120, 50, 200, 25, hwnd, (HMENU)ID_SEARCH_BOX, NULL, NULL);
            SendMessage(hSearchBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hSearchBox, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search...");

            hTagFilter = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 120, 80, 200, 25, hwnd, (HMENU)ID_TAG_FILTER, NULL, NULL);
            SendMessage(hTagFilter, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hTagFilter, EM_SETCUEBANNER, FALSE, (LPARAM)L"Tag filter...");

            hEmails = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                120, 110, 200, H - 160, hwnd, (HMENU)ID_EMAIL_LIST, NULL, NULL);
            SendMessage(hEmails, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnDelete = CreateWindowEx(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE, W - 120, 10, 90, 30, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            SendMessage(hBtnDelete, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTab = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | TCS_TABS, 330, 50, W-360, 25, hwnd, (HMENU)ID_TAB, NULL, NULL);
            SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTitle = CreateWindowEx(0, "STATIC", "No email selected",
                WS_CHILD | WS_VISIBLE,
                330, 85, W - 360, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hBold, TRUE);
            
            hBtnTag = CreateWindowEx(0, "BUTTON", "Add Tag", WS_CHILD, 330, 125, 80, 25, hwnd, (HMENU)ID_BTN_TAG, NULL, NULL);
            hBtnDecrypt = CreateWindowEx(0, "BUTTON", "Decrypt", WS_CHILD, 420, 125, 80, 25, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            SendMessage(hBtnTag, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDecrypt, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBody = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
                330, 155, W - 360, H - 200, hwnd, NULL, NULL, NULL);
            SendMessage(hBody, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            RefreshEmailList();
            RenderPane();
            break;
        }
        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if (nmhdr->idFrom == ID_TAB && nmhdr->code == TCN_SELCHANGE) {
                int iPage = SendMessage(hTab, TCM_GETCURSEL, 0, 0);
                SelectTab(iPage);
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_SEARCH_BOX && HIWORD(wParam) == EN_CHANGE) {
                GetWindowTextA(hSearchBox, searchQuery, sizeof(searchQuery));
                RefreshEmailList();
            }
            else if (LOWORD(wParam) == ID_TAG_FILTER && HIWORD(wParam) == EN_CHANGE) {
                GetWindowTextA(hTagFilter, tagQuery, sizeof(tagQuery));
                RefreshEmailList();
            }
            else if (LOWORD(wParam) == ID_FOLDER_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                currentFolder = SendMessage(hFolders, LB_GETCURSEL, 0, 0);
                RefreshEmailList();
            }
            else if (LOWORD(wParam) == ID_EMAIL_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessage(hEmails, LB_GETCURSEL, 0, 0);
                if (idx != LB_ERR) {
                    int id = SendMessage(hEmails, LB_GETITEMDATA, idx, 0);
                    OpenEmailTab(id);
                }
            }
            else if (LOWORD(wParam) == ID_BTN_DELETE) {
                if(currentTabIdx != -1) {
                    if(tabs[currentTabIdx].id == 1) { // read tab
                        int eid = tabs[currentTabIdx].emailId;
                        for(int i = 0; i < num_emails; i++) {
                            if(emails[i].id == eid) {
                                if(emails[i].folder == 2) emails[i].folder = 99; // hidden
                                else emails[i].folder = 2; // to trash
                                break;
                            }
                        }
                    }
                    CloseCurrentTab();
                    RefreshEmailList();
                }
            }
            else if (LOWORD(wParam) == ID_BTN_EMPTY_TRASH) {
                for(int i = 0; i < num_emails; i++) if(emails[i].folder == 2) emails[i].folder = 99;
                if(currentFolder == 2) RefreshEmailList();
                MessageBox(hwnd, "Trash emptied.", "KMail", MB_OK);
            }
            else if (LOWORD(wParam) == ID_BTN_COMPOSE) {
                NewComposeTab();
            }
            else if (LOWORD(wParam) == ID_BTN_EXPORT) {
                ExportJson();
            }
            else if (LOWORD(wParam) == ID_BTN_IMPORT) {
                MessageBox(hwnd, "Simulated JSON import. (Requires full parser for real import, but file selection would happen here.)", "Import", MB_OK);
            }
            else if (LOWORD(wParam) == ID_BTN_TAG) {
                if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
                    // Quick simulation: append "Important" tag
                    Email* em = NULL;
                    for(int i=0; i<num_emails; i++) if(emails[i].id == tabs[currentTabIdx].emailId) em = &emails[i];
                    if(em) {
                        if (lstrlenA(em->tags) + 11 < sizeof(em->tags)) {
                            lstrcatA(em->tags, ",Important");
                        }
                        MessageBox(hwnd, "Appended 'Important' tag.", "Tag", MB_OK);
                        RenderPane();
                        RefreshEmailList();
                    }
                }
            }
            else if (LOWORD(wParam) == ID_BTN_DECRYPT) {
                if(currentTabIdx != -1) {
                    if(tabs[currentTabIdx].id == 1) {
                        // Decrypt
                        Email* em = NULL;
                        for(int i=0; i<num_emails; i++) if(emails[i].id == tabs[currentTabIdx].emailId) em = &emails[i];
                        if(em && em->encrypted) {
                            char pwd[32] = "pass"; // Simplified password prompt simulation
                            char decStr[2048];
                            lstrcpyA(decStr, em->body);
                            CryptStr(decStr, pwd);
                            SetWindowTextA(hBody, decStr);
                            SetWindowTextA(hBtnDecrypt, "Decrypted!");
                            EnableWindow(hBtnDecrypt, FALSE);
                        }
                    } else if (tabs[currentTabIdx].id == 0) {
                        // Send
                        if(num_emails < 200) {
                            emails[num_emails].id = nextId++;
                            emails[num_emails].folder = 1; // sent
                            lstrcpyA(emails[num_emails].subject, "New Sent Email");
                            lstrcpyA(emails[num_emails].sender, "someone@kilo.os");
                            GetWindowTextA(hBody, emails[num_emails].body, 2048);
                            emails[num_emails].unread = 0;
                            emails[num_emails].encrypted = 0;
                            emails[num_emails].tags[0] = 0;
                            num_emails++;
                            if(currentFolder == 1) RefreshEmailList();
                            CloseCurrentTab();
                            MessageBox(hwnd, "Message Sent!", "KMail", MB_OK);
                        }
                    }
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hFolders, 10, 50, 100, nh - 100, TRUE);
            MoveWindow(hBtnEmptyTrash, 10, nh - 40, 100, 30, TRUE);
            
            MoveWindow(hSearchBox, 120, 50, 200, 25, TRUE);
            MoveWindow(hTagFilter, 120, 80, 200, 25, TRUE);
            MoveWindow(hEmails, 120, 110, 200, nh - 120, TRUE);
            
            MoveWindow(hBtnDelete, nw - 110, 10, 90, 30, TRUE);
            
            MoveWindow(hTab, 330, 50, nw - 340, 25, TRUE);
            MoveWindow(hTitle, 330, 85, nw - 340, 40, TRUE);
            
            MoveWindow(hBtnTag, 330, 125, 80, 25, TRUE);
            MoveWindow(hBtnDecrypt, 420, 125, 80, 25, TRUE);
            
            MoveWindow(hBody, 330, 155, nw - 340, nh - 165, TRUE);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetTextColor((HDC)wParam, textCol);
            SetBkColor((HDC)wParam, bgMainCol);
            return (LRESULT)hbgMain;
        }
        case WM_CTLCOLOREDIT: 
        case WM_CTLCOLORLISTBOX: {
            SetTextColor((HDC)wParam, textCol);
            SetBkColor((HDC)wParam, bgListCol);
            return (LRESULT)hbgList;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbgMain);
            return 1;
        }
        case WM_DESTROY:
            DeleteObject(hbgMain);
            DeleteObject(hbgList);
            DeleteObject(hFont);
            DeleteObject(hBold);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMailApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    RECT rect = {0, 0, W, H};
    AdjustWindowRect(&rect, style, FALSE);
    HWND hwnd = CreateWindowEx(0, "KMailApp", "KMail - Press H for Help", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && (msg.wParam == 'H' || msg.wParam == 'h')) {
            HWND hFocus = GetFocus();
            char cls[64] = {0};
            GetClassNameA(hFocus, cls, 64);
            if (lstrcmpiA(cls, "EDIT") != 0) {
                MessageBoxA(hwnd, "KMail Help:\n\n- Click 'Compose' to write.\n- Select folders on the left.\n- Search and filter by tags.\n- Tabs let you open multiple emails.", "Help", MB_OK);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
