#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define ID_BTN_PREV 101
#define ID_BTN_NEXT 102
#define ID_BTN_FLIP 103
#define ID_BTN_ADD  104
#define ID_LBL_CARD 105
#define ID_LBL_INFO 106

// Dialog Add Card IDs
#define IDD_ADDCARD 200
#define IDC_FRONT   201
#define IDC_BACK    202
#define IDC_ADD     203
#define IDC_CANCEL  204

#define ID_BTN_EDIT 107
#define ID_BTN_DELETE 108
#define ID_BTN_IMPORT 109
#define ID_BTN_EXPORT 110
#define ID_BTN_SHUFFLE 111
#define ID_BTN_LOAD_SAMPLE 112
#define ID_EDIT_SEARCH 113
#define ID_BTN_PRINT 114

// Dialog Edit Card IDs
#define IDD_EDITCARD 210
#define IDC_EDIT_FRONT 211
#define IDC_EDIT_BACK 212
#define IDC_EDIT_SAVE 213
#define IDC_EDIT_CANCEL 214

#define ID_BTN_STATS 118
#define IDD_STATS 220
#define IDC_STATS_TOTAL 221
#define IDC_STATS_KNOWN 222
#define IDC_STATS_REVIEW 223
#define IDC_STATS_UNSTUDIED 224
#define IDC_STATS_MASTERY 225
#define IDC_STATS_PROGRESS 226
#define IDC_STATS_CLOSE 227

#define ID_BTN_HELP 119
#define IDD_HELP 230
#define IDC_HELP_TEXT 231
#define IDC_HELP_CLOSE 232

typedef struct {
    char front[256];
    char back[256];
    int status;
} FlashCard;

FlashCard deck[100];
int deckCount = 0;
int currentIndex = 0;
int isFlipped = 0;
int filteredIndices[100];
#define ID_CHK_REVIEW 115
#define ID_BTN_GOTIT  116
#define ID_BTN_REVIEW 117

int filteredCount = 0;

HWND hBtnPrev, hBtnNext, hBtnFlip, hBtnAdd, hBtnEdit, hBtnDelete, hBtnImport, hBtnExport, hBtnShuffle, hEditSearch, hBtnPrint;
HWND hChkReview, hBtnGotIt, hBtnReview;
HWND g_hwnd;
HBRUSH hbgBrush, hCardBrush;
HFONT hFont, hCardFont;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StatsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HelpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LoadDeck() {
    FILE *f = fopen("kflash_data.bin", "rb");
    if (f) {
        fread(&deckCount, sizeof(int), 1, f);
        fread(deck, sizeof(FlashCard), deckCount, f);
        fclose(f);
    } else {
        strcpy(deck[0].front, "Welcome to KFlash!");
        strcpy(deck[0].back, "This is a quick tutorial. Press Space or click the card to flip it.");
        strcpy(deck[1].front, "Navigating Cards");
        strcpy(deck[1].back, "Use the Next and Prev buttons, or the Left/Right arrow keys on your keyboard.");
        strcpy(deck[2].front, "Adding Cards");
        strcpy(deck[2].back, "Click the '+ Add Card' button in the top right to add a new flashcard to your deck.");
        strcpy(deck[3].front, "Editing and Deleting");
        strcpy(deck[3].back, "Use the 'Edit' and 'Delete' buttons to modify the currently displayed card.");
        strcpy(deck[4].front, "Import, Export & Samples");
        strcpy(deck[4].back, "Use 'Export' to save your deck to a file, 'Import' to load one, or 'Load Sample' to try pre-made decks!");
        deckCount = 5;
    }
}

void SaveDeck() {
    FILE *f = fopen("kflash_data.bin", "wb");
    if (f) {
        fwrite(&deckCount, sizeof(int), 1, f);
        fwrite(deck, sizeof(FlashCard), deckCount, f);
        fclose(f);
    }
}

void UpdateFilteredIndices() {
    char query[256] = "";
    if (hEditSearch) {
        GetWindowText(hEditSearch, query, 255);
    }
    for (int i = 0; i < strlen(query); i++) query[i] = tolower(query[i]);
    
    int reviewOnly = 0;
    if (hChkReview && SendMessage(hChkReview, BM_GETCHECK, 0, 0) == BST_CHECKED) {
        reviewOnly = 1;
    }
    
    filteredCount = 0;
    for (int i = 0; i < deckCount; i++) {
        if (reviewOnly && deck[i].status != 2) continue;

        if (strlen(query) == 0) {
            filteredIndices[filteredCount++] = i;
        } else {
            char f[256], b[256];
            strcpy(f, deck[i].front);
            strcpy(b, deck[i].back);
            for(int j=0; j<strlen(f); j++) f[j] = tolower(f[j]);
            for(int j=0; j<strlen(b); j++) b[j] = tolower(b[j]);
            if (strstr(f, query) || strstr(b, query)) {
                filteredIndices[filteredCount++] = i;
            }
        }
    }
}

void UpdateCardDisplay() {
    UpdateFilteredIndices();
    if (currentIndex >= filteredCount && filteredCount > 0) currentIndex = filteredCount - 1;
    if (currentIndex < 0 && filteredCount > 0) currentIndex = 0;

    if (filteredCount == 0) {
        EnableWindow(hBtnPrev, FALSE);
        EnableWindow(hBtnNext, FALSE);
        EnableWindow(hBtnFlip, FALSE);
        EnableWindow(hBtnEdit, FALSE);
        EnableWindow(hBtnDelete, FALSE);
        EnableWindow(hBtnExport, FALSE);
        EnableWindow(hBtnShuffle, FALSE);
        if (hBtnGotIt) ShowWindow(hBtnGotIt, SW_HIDE);
        if (hBtnReview) ShowWindow(hBtnReview, SW_HIDE);
    } else {
        EnableWindow(hBtnFlip, TRUE);
        EnableWindow(hBtnEdit, TRUE);
        EnableWindow(hBtnDelete, TRUE);
        EnableWindow(hBtnExport, TRUE);
        EnableWindow(hBtnShuffle, filteredCount > 1);
        EnableWindow(hBtnPrev, currentIndex > 0);
        EnableWindow(hBtnNext, currentIndex < filteredCount - 1);
        if (isFlipped) {
            if (hBtnGotIt) ShowWindow(hBtnGotIt, SW_SHOW);
            if (hBtnReview) ShowWindow(hBtnReview, SW_SHOW);
        } else {
            if (hBtnGotIt) ShowWindow(hBtnGotIt, SW_HIDE);
            if (hBtnReview) ShowWindow(hBtnReview, SW_HIDE);
        }
    }
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KFlashWindowClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(13, 17, 23));

    RegisterClass(&wc);

    hbgBrush = CreateSolidBrush(RGB(13, 17, 23));
    hCardBrush = CreateSolidBrush(RGB(22, 27, 34));

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    // Load deck
    srand((unsigned int)time(NULL));
    LoadDeck();

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KFlash",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 450,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    g_hwnd = hwnd;
    ShowWindow(hwnd, nCmdShow);
    UpdateCardDisplay();

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        int handled = 0;
        if (msg.message == WM_KEYDOWN && GetActiveWindow() == g_hwnd) {
            if (msg.wParam == VK_SPACE || msg.wParam == VK_RETURN) {
                SendMessage(g_hwnd, WM_COMMAND, ID_BTN_FLIP, 0);
                handled = 1;
            } else if (msg.wParam == VK_LEFT) {
                SendMessage(g_hwnd, WM_COMMAND, ID_BTN_PREV, 0);
                handled = 1;
            } else if (msg.wParam == VK_RIGHT) {
                SendMessage(g_hwnd, WM_COMMAND, ID_BTN_NEXT, 0);
                handled = 1;
            }
        }
        if (!handled) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DeleteObject(hbgBrush);
    DeleteObject(hCardBrush);
    if(hFont) DeleteObject(hFont);
    if(hCardFont) DeleteObject(hCardFont);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hCardFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            hEditSearch = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                   10, 10, 110, 30, hwnd, (HMENU)ID_EDIT_SEARCH, NULL, NULL);
            SendMessage(hEditSearch, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnImport = CreateWindow("BUTTON", "Imp", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   125, 10, 35, 30, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
                                   
            hBtnExport = CreateWindow("BUTTON", "Exp", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   165, 10, 35, 30, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);

            HWND hBtnLoadSample = CreateWindow("BUTTON", "Smpls", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   205, 10, 45, 30, hwnd, (HMENU)ID_BTN_LOAD_SAMPLE, NULL, NULL);
            SendMessage(hBtnLoadSample, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnPrint = CreateWindow("BUTTON", "Prt", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   255, 10, 35, 30, hwnd, (HMENU)ID_BTN_PRINT, NULL, NULL);
            SendMessage(hBtnPrint, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnShuffle = CreateWindow("BUTTON", "Shuf", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   295, 10, 40, 30, hwnd, (HMENU)ID_BTN_SHUFFLE, NULL, NULL);

            hBtnEdit = CreateWindow("BUTTON", "Edit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   340, 10, 40, 30, hwnd, (HMENU)ID_BTN_EDIT, NULL, NULL);
                                   
            hBtnDelete = CreateWindow("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   385, 10, 35, 30, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);

            hBtnAdd = CreateWindow("BUTTON", "+Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   425, 10, 45, 30, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);

            HWND hBtnStats = CreateWindow("BUTTON", "Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   475, 10, 45, 30, hwnd, (HMENU)ID_BTN_STATS, NULL, NULL);
            SendMessage(hBtnStats, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnHelp = CreateWindow("BUTTON", "?", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   525, 10, 25, 30, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnPrev = CreateWindow("BUTTON", "<- Prev", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    150, 320, 80, 30, hwnd, (HMENU)ID_BTN_PREV, NULL, NULL);

            hBtnFlip = CreateWindow("BUTTON", "Flip", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    250, 320, 80, 30, hwnd, (HMENU)ID_BTN_FLIP, NULL, NULL);

            hBtnNext = CreateWindow("BUTTON", "Next ->", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    350, 320, 80, 30, hwnd, (HMENU)ID_BTN_NEXT, NULL, NULL);

            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDelete, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFlip, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnShuffle, WM_SETFONT, (WPARAM)hFont, TRUE);

            hChkReview = CreateWindow("BUTTON", "Review Only", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                   10, 45, 100, 20, hwnd, (HMENU)ID_CHK_REVIEW, NULL, NULL);
            SendMessage(hChkReview, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnGotIt = CreateWindow("BUTTON", "Got It", WS_CHILD | BS_PUSHBUTTON,
                                   150, 360, 100, 30, hwnd, (HMENU)ID_BTN_GOTIT, NULL, NULL);
            SendMessage(hBtnGotIt, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnReview = CreateWindow("BUTTON", "Needs Review", WS_CHILD | BS_PUSHBUTTON,
                                   330, 360, 110, 30, hwnd, (HMENU)ID_BTN_REVIEW, NULL, NULL);
            SendMessage(hBtnReview, WM_SETFONT, (WPARAM)hFont, TRUE);

            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            FillRect(hdcMem, &rcClient, hbgBrush);

            RECT rcCard = { 50, 60, rcClient.right - 50, 300 };
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
            HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hCardBrush);
            RoundRect(hdcMem, rcCard.left, rcCard.top, rcCard.right, rcCard.bottom, 20, 20);

            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(139, 148, 158));
            SelectObject(hdcMem, hFont);
            char info[64];
            if (filteredCount == 0) {
                strcpy(info, "0/0");
            } else {
                int st = deck[filteredIndices[currentIndex]].status;
                sprintf(info, "Card %d/%d %s", currentIndex + 1, filteredCount, st == 1 ? "[Known]" : (st == 2 ? "[Review]" : ""));
            }
            RECT rcInfo = { rcCard.left + 20, rcCard.top + 20, rcCard.right - 20, rcCard.top + 40 };
            DrawText(hdcMem, info, -1, &rcInfo, DT_LEFT | DT_TOP | DT_SINGLELINE);

            SetTextColor(hdcMem, RGB(201, 209, 217));
            SelectObject(hdcMem, hCardFont);
            RECT rcText = { rcCard.left + 20, rcCard.top + 60, rcCard.right - 20, rcCard.bottom - 20 };
            
            const char* textToDraw = "";
            if (filteredCount == 0) {
                char query[256] = "";
                if (hEditSearch) GetWindowText(hEditSearch, query, 255);
                if (strlen(query) > 0) {
                    textToDraw = "No matches found\n\nTry a different search.";
                } else {
                    textToDraw = "Deck is empty\n\nClick 'Add Card' to begin.";
                }
            } else {
                int realIndex = filteredIndices[currentIndex];
                textToDraw = isFlipped ? deck[realIndex].back : deck[realIndex].front;
            }
            
            DrawText(hdcMem, textToDraw, -1, &rcText, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

            SelectObject(hdcMem, hOldBrush);
            SelectObject(hdcMem, hOldPen);
            DeleteObject(hPen);

            BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            if (HIWORD(wParam) == EN_CHANGE && wmId == ID_EDIT_SEARCH) {
                currentIndex = 0;
                isFlipped = 0;
                UpdateCardDisplay();
                break;
            }
            
            if (HIWORD(wParam) == BN_CLICKED && wmId == ID_CHK_REVIEW) {
                currentIndex = 0;
                isFlipped = 0;
                UpdateCardDisplay();
                break;
            }
            
            switch (wmId) {
                case ID_BTN_PREV:
                    if (currentIndex > 0) {
                        currentIndex--;
                        isFlipped = 0;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_NEXT:
                    if (currentIndex < filteredCount - 1) {
                        currentIndex++;
                        isFlipped = 0;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_FLIP:
                    if (filteredCount > 0) {
                        isFlipped = !isFlipped;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_GOTIT:
                    if (filteredCount > 0) {
                        deck[filteredIndices[currentIndex]].status = 1;
                        SaveDeck();
                        if (currentIndex < filteredCount - 1) {
                            currentIndex++;
                            isFlipped = 0;
                        } else {
                            isFlipped = 0;
                        }
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_REVIEW:
                    if (filteredCount > 0) {
                        deck[filteredIndices[currentIndex]].status = 2;
                        SaveDeck();
                        if (currentIndex < filteredCount - 1) {
                            currentIndex++;
                            isFlipped = 0;
                        } else {
                            isFlipped = 0;
                        }
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_STATS:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STATS), hwnd, StatsProc);
                    break;
                case ID_BTN_HELP:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HELP), hwnd, HelpProc);
                    break;
                case ID_BTN_ADD:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADDCARD), hwnd, AddCardProc);
                    UpdateCardDisplay();
                    break;
                case ID_BTN_EDIT:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EDITCARD), hwnd, EditCardProc);
                    UpdateCardDisplay();
                    break;
                case ID_BTN_DELETE:
                    if (filteredCount > 0) {
                        if (MessageBox(hwnd, "Are you sure you want to delete this card?", "Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            int realIndex = filteredIndices[currentIndex];
                            for (int i = realIndex; i < deckCount - 1; i++) {
                                deck[i] = deck[i + 1];
                            }
                            deckCount--;
                            isFlipped = 0;
                            SaveDeck();
                            UpdateCardDisplay();
                        }
                    }
                    break;
                case ID_BTN_IMPORT: {
                    OPENFILENAME ofn;
                    char szFile[260];
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if (GetOpenFileName(&ofn) == TRUE) {
                        FILE* f = fopen(szFile, "r");
                        if (f) {
                            int replace = 1;
                            if (deckCount > 0) {
                                replace = (MessageBox(hwnd, "Replace current deck? (No to append)", "Import", MB_YESNO | MB_ICONQUESTION) == IDYES);
                            }
                            if (replace) deckCount = 0;
                            char line[1024];
                            while (fgets(line, sizeof(line), f) && deckCount < 100) {
                                char* comma = strchr(line, ',');
                                if (comma) {
                                    *comma = '\0';
                                    char* front = line;
                                    char* back = comma + 1;
                                    char* nl = strchr(back, '\n');
                                    if (nl) *nl = '\0';
                                    if(front[0]=='\"') { front++; front[strlen(front)-1]='\0'; }
                                    if(back[0]=='\"') { back++; back[strlen(back)-1]='\0'; }
                                    strncpy(deck[deckCount].front, front, 255);
                                    deck[deckCount].front[255] = '\0';
                                    strncpy(deck[deckCount].back, back, 255);
                                    deck[deckCount].back[255] = '\0';
                                    deckCount++;
                                }
                            }
                            fclose(f);
                            SaveDeck();
                            currentIndex = 0;
                            isFlipped = 0;
                            UpdateCardDisplay();
                        } else {
                            MessageBox(hwnd, "Failed to open file", "Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                }
                case ID_BTN_LOAD_SAMPLE: {
                    OPENFILENAME ofn;
                    char szFile[260];
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "CSV Files\0*.csv\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrInitialDir = ".\\SamplePacks";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if (GetOpenFileName(&ofn) == TRUE) {
                        FILE* f = fopen(szFile, "r");
                        if (f) {
                            int replace = 1;
                            if (deckCount > 0) {
                                replace = (MessageBox(hwnd, "Replace current deck? (No to append)", "Load Sample", MB_YESNO | MB_ICONQUESTION) == IDYES);
                            }
                            if (replace) deckCount = 0;
                            char line[1024];
                            while (fgets(line, sizeof(line), f) && deckCount < 100) {
                                char* comma = strchr(line, ',');
                                if (comma) {
                                    *comma = '\0';
                                    char* front = line;
                                    char* back = comma + 1;
                                    char* nl = strchr(back, '\n');
                                    if (nl) *nl = '\0';
                                    if(front[0]=='\"') { front++; front[strlen(front)-1]='\0'; }
                                    if(back[0]=='\"') { back++; back[strlen(back)-1]='\0'; }
                                    strncpy(deck[deckCount].front, front, 255);
                                    deck[deckCount].front[255] = '\0';
                                    strncpy(deck[deckCount].back, back, 255);
                                    deck[deckCount].back[255] = '\0';
                                    deckCount++;
                                }
                            }
                            fclose(f);
                            SaveDeck();
                            currentIndex = 0;
                            isFlipped = 0;
                            UpdateCardDisplay();
                        } else {
                            MessageBox(hwnd, "Failed to open file", "Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                }
                case ID_BTN_EXPORT: {
                    OPENFILENAME ofn;
                    char szFile[260] = "deck.csv";
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                    if (GetSaveFileName(&ofn) == TRUE) {
                        FILE* f = fopen(szFile, "w");
                        if (f) {
                            for (int i = 0; i < deckCount; i++) {
                                fprintf(f, "\"%s\",\"%s\"\n", deck[i].front, deck[i].back);
                            }
                            fclose(f);
                            MessageBox(hwnd, "Export successful", "Export", MB_OK | MB_ICONINFORMATION);
                        } else {
                            MessageBox(hwnd, "Failed to save file", "Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                }
                case ID_BTN_PRINT:
                    if (deckCount > 0) {
                        FILE *f = fopen("kflash_print_temp.html", "w");
                        if (f) {
                            fprintf(f, "<html><head><title>KFlash Deck</title><style>body{font-family:sans-serif;} .card{border:1px solid #ccc; padding:10px; margin-bottom:10px; border-radius:8px; page-break-inside:avoid;} .front{font-weight:bold;} </style></head><body>");
                            fprintf(f, "<h2>Flashcard Deck</h2>");
                            for(int i=0; i<deckCount; i++) {
                                fprintf(f, "<div class='card'><div class='front'>Q: %s</div><div class='back'>A: %s</div></div>", deck[i].front, deck[i].back);
                            }
                            fprintf(f, "<script>window.print();</script></body></html>");
                            fclose(f);
                            ShellExecute(NULL, "open", "kflash_print_temp.html", NULL, NULL, SW_SHOWNORMAL);
                        } else {
                            MessageBox(hwnd, "Failed to create print file.", "Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                case ID_BTN_SHUFFLE:
                    if (deckCount > 1) {
                        for (int i = deckCount - 1; i > 0; i--) {
                            int j = rand() % (i + 1);
                            FlashCard temp = deck[i];
                            deck[i] = deck[j];
                            deck[j] = temp;
                        }
                        currentIndex = 0;
                        isFlipped = 0;
                        SaveDeck();
                        UpdateCardDisplay();
                    }
                    break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK AddCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_ADD) {
                if (deckCount < 100) {
                    GetDlgItemText(hwndDlg, IDC_FRONT, deck[deckCount].front, 255);
                    GetDlgItemText(hwndDlg, IDC_BACK, deck[deckCount].back, 255);
                    deck[deckCount].status = 0;
                    if (strlen(deck[deckCount].front) > 0 && strlen(deck[deckCount].back) > 0) {
                        deckCount++;
                        SaveDeck();
                    }
                }
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else if (LOWORD(wParam) == IDC_CANCEL || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK EditCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int realIndex = filteredIndices[currentIndex];
    switch (uMsg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwndDlg, IDC_EDIT_FRONT, deck[realIndex].front);
            SetDlgItemText(hwndDlg, IDC_EDIT_BACK, deck[realIndex].back);
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_EDIT_SAVE) {
                GetDlgItemText(hwndDlg, IDC_EDIT_FRONT, deck[realIndex].front, 255);
                GetDlgItemText(hwndDlg, IDC_EDIT_BACK, deck[realIndex].back, 255);
                if (strlen(deck[realIndex].front) > 0 && strlen(deck[realIndex].back) > 0) {
                    SaveDeck();
                }
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else if (LOWORD(wParam) == IDC_EDIT_CANCEL || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK StatsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG: {
            int total = deckCount;
            int known = 0, review = 0, unstudied = 0;
            for (int i = 0; i < total; i++) {
                if (deck[i].status == 1) known++;
                else if (deck[i].status == 2) review++;
                else unstudied++;
            }
            float mastery = total > 0 ? ((float)known / total) * 100.0f : 0.0f;
            
            char buf[32];
            sprintf(buf, "%d", total);
            SetDlgItemText(hwndDlg, IDC_STATS_TOTAL, buf);
            sprintf(buf, "%d", known);
            SetDlgItemText(hwndDlg, IDC_STATS_KNOWN, buf);
            sprintf(buf, "%d", review);
            SetDlgItemText(hwndDlg, IDC_STATS_REVIEW, buf);
            sprintf(buf, "%d", unstudied);
            SetDlgItemText(hwndDlg, IDC_STATS_UNSTUDIED, buf);
            sprintf(buf, "%.1f%%", mastery);
            SetDlgItemText(hwndDlg, IDC_STATS_MASTERY, buf);
            
            SendDlgItemMessage(hwndDlg, IDC_STATS_PROGRESS, PBM_SETPOS, (WPARAM)(int)mastery, 0);
            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_STATS_CLOSE || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HelpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG: {
            SetDlgItemText(hwndDlg, IDC_HELP_TEXT, "KFlash Help & Shortcuts\r\n\r\n"
                           "Card Basics\r\nUse '+Add' to create, 'Edit' to modify, 'Del' to remove.\r\n\r\n"
                           "Import & Export\r\nExport to CSV. Import CSV/JSON decks.\r\n\r\n"
                           "Review Mode\r\nUse Search bar to filter. Check 'Review Only' for cards needing review. Click 'Shuf' to shuffle.\r\n\r\n"
                           "Spaced Repetition\r\nFlip a card and use 'Got It' or 'Needs Review' to track progress.\r\n\r\n"
                           "Keyboard Shortcuts\r\n- Space/Enter: Flip card\r\n- Left/Right Arrows: Prev/Next card");
            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_HELP_CLOSE || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
