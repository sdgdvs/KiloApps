#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_HABITS 100
#define ID_LISTBOX 101
#define ID_EDIT 102
#define ID_BTN_ADD 103
#define ID_BTN_CHECK 104
#define ID_BTN_DELETE 105
#define ID_BTN_IMPORT 106
#define ID_BTN_EXPORT 107

typedef struct {
    char name[128];
    int streak;
    int last_check_day;
} Habit;

Habit habits[MAX_HABITS];
int habitCount = 0;

int GetDaysSinceEpoch() {
    time_t t = time(NULL);
    return (int)(t / (60 * 60 * 24));
}

void LoadHabits() {
    FILE *f = fopen("habits.dat", "r");
    if (!f) return;
    habitCount = 0;
    while (fscanf(f, "%127[^|]|%d|%d\n", habits[habitCount].name, &habits[habitCount].streak, &habits[habitCount].last_check_day) == 3) {
        habitCount++;
        if (habitCount >= MAX_HABITS) break;
    }
    fclose(f);
}

void SaveHabits() {
    FILE *f = fopen("habits.dat", "w");
    if (!f) return;
    for (int i = 0; i < habitCount; i++) {
        fprintf(f, "%s|%d|%d\n", habits[i].name, habits[i].streak, habits[i].last_check_day);
    }
    fclose(f);
}

void CheckStreaks() {
    int today = GetDaysSinceEpoch();
    for (int i=0; i<habitCount; i++) {
        if (today - habits[i].last_check_day > 1 && habits[i].streak > 0) {
            habits[i].streak = 0;
        }
    }
}

HWND hList, hEdit;

void UpdateList() {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    int today = GetDaysSinceEpoch();
    for (int i = 0; i < habitCount; i++) {
        char buffer[256];
        char status[16];
        if (habits[i].last_check_day == today) {
            strcpy(status, "[ \xDF ]"); // some kind of check symbol
        } else {
            strcpy(status, "[   ]");
        }
        sprintf(buffer, "%s  %s (Streak: %d)", status, habits[i].name, habits[i].streak);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

HBRUSH hbgBrush;
HBRUSH hListBrush;
HFONT hFont;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hbgBrush = CreateSolidBrush(RGB(24, 24, 28));
            hListBrush = CreateSolidBrush(RGB(32, 32, 36));
            hFont = CreateFont(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            hEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                   20, 20, 340, 28, hwnd, (HMENU)ID_EDIT, NULL, NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hAdd = CreateWindowEx(0, "BUTTON", "+ New Habit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       370, 20, 100, 28, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
            SendMessage(hAdd, WM_SETFONT, (WPARAM)hFont, TRUE);

            hList = CreateWindowEx(0, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
                                   20, 60, 450, 320, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hCheck = CreateWindowEx(0, "BUTTON", "Check Off", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         20, 395, 120, 35, hwnd, (HMENU)ID_BTN_CHECK, NULL, NULL);
            SendMessage(hCheck, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hDel = CreateWindowEx(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       150, 395, 120, 35, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            SendMessage(hDel, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hImport = CreateWindowEx(0, "BUTTON", "Import", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          280, 395, 80, 35, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
            SendMessage(hImport, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hExport = CreateWindowEx(0, "BUTTON", "Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          370, 395, 80, 35, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            SendMessage(hExport, WM_SETFONT, (WPARAM)hFont, TRUE);

            LoadHabits();
            CheckStreaks();
            UpdateList();
            return 0;
        }
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(32, 32, 36));
            SetTextColor(hdc, RGB(240, 240, 240));
            return (LRESULT)hListBrush;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(24, 24, 28));
            SetTextColor(hdc, RGB(240, 240, 240));
            return (LRESULT)hbgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(45, 45, 50));
            SetTextColor(hdc, RGB(255, 255, 255));
            return (LRESULT)CreateSolidBrush(RGB(45, 45, 50));
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbgBrush);
            return 1;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            if (wmId == ID_BTN_ADD && wmEvent == BN_CLICKED) {
                if (habitCount < MAX_HABITS) {
                    char text[128];
                    GetWindowText(hEdit, text, sizeof(text));
                    if (strlen(text) > 0) {
                        strcpy(habits[habitCount].name, text);
                        habits[habitCount].streak = 0;
                        habits[habitCount].last_check_day = 0;
                        habitCount++;
                        SaveHabits();
                        UpdateList();
                        SetWindowText(hEdit, "");
                    }
                }
            } else if (wmId == ID_BTN_CHECK && wmEvent == BN_CLICKED) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int today = GetDaysSinceEpoch();
                    if (habits[sel].last_check_day != today) {
                        if (today - habits[sel].last_check_day == 1) {
                            habits[sel].streak++;
                        } else {
                            habits[sel].streak = 1;
                        }
                        habits[sel].last_check_day = today;
                    } else {
                        // Undo check off
                        habits[sel].last_check_day = 0; // rough undo
                        if(habits[sel].streak > 0) habits[sel].streak--;
                    }
                    SaveHabits();
                    UpdateList();
                    SendMessage(hList, LB_SETCURSEL, sel, 0);
                }
            } else if (wmId == ID_BTN_DELETE && wmEvent == BN_CLICKED) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    for (int i = sel; i < habitCount - 1; i++) {
                        habits[i] = habits[i + 1];
                    }
                    habitCount--;
                    SaveHabits();
                    UpdateList();
                }
            } else if (wmId == ID_BTN_EXPORT && wmEvent == BN_CLICKED) {
                OPENFILENAME ofn;
                char szFile[260] = "khabit_export.csv";
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                if (GetSaveFileName(&ofn) == TRUE) {
                    FILE *f = fopen(ofn.lpstrFile, "w");
                    if (f) {
                        for (int i = 0; i < habitCount; i++) {
                            fprintf(f, "%s,%d,%d\n", habits[i].name, habits[i].streak, habits[i].last_check_day);
                        }
                        fclose(f);
                        MessageBox(hwnd, "Export successful!", "Success", MB_OK);
                    }
                }
            } else if (wmId == ID_BTN_IMPORT && wmEvent == BN_CLICKED) {
                OPENFILENAME ofn;
                char szFile[260] = "";
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileName(&ofn) == TRUE) {
                    FILE *f = fopen(ofn.lpstrFile, "r");
                    if (f) {
                        habitCount = 0;
                        while (fscanf(f, "%127[^,],%d,%d\n", habits[habitCount].name, &habits[habitCount].streak, &habits[habitCount].last_check_day) == 3) {
                            habitCount++;
                            if (habitCount >= MAX_HABITS) break;
                        }
                        fclose(f);
                        SaveHabits();
                        UpdateList();
                        MessageBox(hwnd, "Import successful!", "Success", MB_OK);
                    }
                }
            }
            return 0;
        }
        case WM_DESTROY:
            DeleteObject(hbgBrush);
            DeleteObject(hListBrush);
            DeleteObject(hFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KHabitWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KHabit Tracker", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 506, 490,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
