#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <commctrl.h>

#define W 700
#define H 500

#define ID_FOLDER_LIST 101
#define ID_EMAIL_LIST 102
#define ID_BTN_COMPOSE 103
#define ID_BTN_DELETE 104
#define ID_BTN_EMPTY_TRASH 105
#define ID_SEARCH_BOX 106

HWND hFolders, hEmails, hTitle, hBody, hBtnCompose, hBtnDelete, hBtnEmptyTrash, hSearchBox;

typedef struct {
    int id;
    int folder; // 0=inbox, 1=sent, 2=trash
    char subject[128];
    char sender[128];
    char body[1024];
    int unread;
} Email;

Email emails[100] = {
    {1, 0, "Welcome to KiloOS", "sysadmin@kilo.os", "Hello User,\r\n\r\nWelcome to KiloOS, your new minimalist operating environment.\r\n\r\nEnjoy your stay!\r\n- SysAdmin", 1},
    {2, 0, "Meeting at 3PM", "boss@kilo.os", "Don't forget our meeting at 3PM in the conference room. We need to discuss the Q3 roadmap.\r\n\r\nRegards,\r\nBoss", 0},
    {3, 0, "URGENT: Server down", "alerts@kilo.os", "The primary database server is unresponsive. Please investigate immediately. This is not a drill.", 1},
    {4, 0, "Newsletter #42", "news@kilo.os", "Here is your weekly digest of what's new in the world of retro computing...", 0},
    {5, 1, "Re: Meeting at 3PM", "me@kilo.os", "I'll be there.\r\n\r\n- Me", 0}
};
int num_emails = 5;

int currentFolder = 0; // 0=inbox, 1=sent, 2=trash
int currentEmailId = -1;
char searchQuery[128] = "";

int contains_nocase(const char* haystack, const char* needle) {
    if (!needle[0]) return 1;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (needle[j] && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (!needle[j]) return 1;
    }
    return 0;
}

HBRUSH hbgMain, hbgList;
HFONT hFont, hBold;
COLORREF textCol = RGB(248, 250, 252);
COLORREF bgMainCol = RGB(15, 23, 42);
COLORREF bgListCol = RGB(30, 41, 59);

void RefreshEmailList() {
    SendMessage(hEmails, LB_RESETCONTENT, 0, 0);
    for(int i = 0; i < num_emails; i++) {
        if(emails[i].folder == currentFolder) {
            if (searchQuery[0] == '\0' || 
                contains_nocase(emails[i].subject, searchQuery) || 
                contains_nocase(emails[i].sender, searchQuery) || 
                contains_nocase(emails[i].body, searchQuery)) {
                
                int idx = SendMessageA(hEmails, LB_ADDSTRING, 0, (LPARAM)emails[i].subject);
                SendMessage(hEmails, LB_SETITEMDATA, idx, emails[i].id);
            }
        }
    }
}

void SelectEmail(int id) {
    currentEmailId = id;
    if(id == -1) {
        SetWindowTextA(hTitle, "No message selected");
        SetWindowTextA(hBody, "Select an email from the list to read.");
    } else {
        for(int i = 0; i < num_emails; i++) {
            if(emails[i].id == id) {
                char tStr[256];
                if(emails[i].folder == 1)
                    sprintf(tStr, "To: %s\r\n%s", emails[i].sender, emails[i].subject);
                else
                    sprintf(tStr, "From: %s\r\n%s", emails[i].sender, emails[i].subject);
                SetWindowTextA(hTitle, tStr);
                SetWindowTextA(hBody, emails[i].body);
                emails[i].unread = 0;
                break;
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hbgMain = CreateSolidBrush(bgMainCol);
            hbgList = CreateSolidBrush(bgListCol);

            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hBold = CreateFontA(18, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnCompose = CreateWindowEx(0, "BUTTON", "Compose", WS_CHILD | WS_VISIBLE, 10, 10, 100, 30, hwnd, (HMENU)ID_BTN_COMPOSE, NULL, NULL);
            SendMessage(hBtnCompose, WM_SETFONT, (WPARAM)hFont, TRUE);
            
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

            hSearchBox = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 120, 15, 200, 25, hwnd, (HMENU)ID_SEARCH_BOX, NULL, NULL);
            SendMessage(hSearchBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hSearchBox, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search...");

            hEmails = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                120, 50, 200, H - 100, hwnd, (HMENU)ID_EMAIL_LIST, NULL, NULL);
            SendMessage(hEmails, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnDelete = CreateWindowEx(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE, W - 120, 10, 90, 30, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            SendMessage(hBtnDelete, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTitle = CreateWindowEx(0, "STATIC", "No message selected",
                WS_CHILD | WS_VISIBLE,
                330, 50, W - 360, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hBold, TRUE);
            
            hBody = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Select an email from the list to read.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                330, 100, W - 360, H - 150, hwnd, NULL, NULL, NULL);
            SendMessage(hBody, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            RefreshEmailList();
            SelectEmail(-1);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_SEARCH_BOX && HIWORD(wParam) == EN_CHANGE) {
                GetWindowTextA(hSearchBox, searchQuery, sizeof(searchQuery));
                RefreshEmailList();
                SelectEmail(-1);
            }
            else if (LOWORD(wParam) == ID_FOLDER_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                currentFolder = SendMessage(hFolders, LB_GETCURSEL, 0, 0);
                searchQuery[0] = '\0';
                SetWindowTextA(hSearchBox, "");
                RefreshEmailList();
                SelectEmail(-1);
            }
            else if (LOWORD(wParam) == ID_EMAIL_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessage(hEmails, LB_GETCURSEL, 0, 0);
                if (idx != LB_ERR) {
                    int id = SendMessage(hEmails, LB_GETITEMDATA, idx, 0);
                    SelectEmail(id);
                }
            }
            else if (LOWORD(wParam) == ID_BTN_DELETE) {
                if(currentEmailId != -1) {
                    for(int i = 0; i < num_emails; i++) {
                        if(emails[i].id == currentEmailId) {
                            if(emails[i].folder == 2) {
                                // permanently delete
                                emails[i].folder = 99; // hidden
                            } else {
                                emails[i].folder = 2; // to trash
                            }
                            break;
                        }
                    }
                    RefreshEmailList();
                    SelectEmail(-1);
                }
            }
            else if (LOWORD(wParam) == ID_BTN_EMPTY_TRASH) {
                for(int i = 0; i < num_emails; i++) {
                    if(emails[i].folder == 2) {
                        emails[i].folder = 99; // hidden
                    }
                }
                if(currentFolder == 2) {
                    RefreshEmailList();
                    SelectEmail(-1);
                }
                MessageBox(hwnd, "Trash emptied.", "KMail", MB_OK);
            }
            else if (LOWORD(wParam) == ID_BTN_COMPOSE) {
                // simple compose simulation
                if(num_emails < 100) {
                    emails[num_emails].id = num_emails + 1;
                    emails[num_emails].folder = 1; // sent
                    strcpy(emails[num_emails].subject, "New Sent Email");
                    strcpy(emails[num_emails].sender, "someone@kilo.os");
                    strcpy(emails[num_emails].body, "This is a composed message.");
                    emails[num_emails].unread = 0;
                    num_emails++;
                    if(currentFolder == 1) RefreshEmailList();
                    MessageBox(hwnd, "Draft sent! (Simulated compose window)", "KMail", MB_OK);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hFolders, 10, 50, 100, nh - 100, TRUE);
            MoveWindow(hBtnEmptyTrash, 10, nh - 40, 100, 30, TRUE);
            MoveWindow(hSearchBox, 120, 15, 200, 25, TRUE);
            MoveWindow(hEmails, 120, 50, 200, nh - 60, TRUE);
            MoveWindow(hBtnDelete, nw - 110, 10, 90, 30, TRUE);
            MoveWindow(hTitle, 330, 50, nw - 340, 40, TRUE);
            MoveWindow(hBody, 330, 100, nw - 340, nh - 110, TRUE);
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

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMailApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMailApp", "KMail", WS_OVERLAPPEDWINDOW,
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
