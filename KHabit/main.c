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
#define ID_BTN_HELP 112
#define ID_COMBO_SORT 109
#define ID_SEARCH_EDIT 110

typedef struct {
    char name[128];
    int streak;
    int last_check_day;
    char category[32];
    int target_streak;
} Habit;

Habit habits[MAX_HABITS];
int habitCount = 0;

int current_color_index = 0;
int current_sort_index = 0;
COLORREF accentColors[4] = { RGB(139, 92, 246), RGB(249, 115, 22), RGB(59, 130, 246), RGB(16, 185, 129) };
HWND hMainWnd;

void LoadSettings() {
    FILE *f = fopen("khabit_settings.dat", "r");
    if (f) {
        if (fscanf(f, "%d %d", &current_color_index, &current_sort_index) < 2) {
            // default already handled
        }
        if (current_color_index < 0 || current_color_index > 3) current_color_index = 0;
        if (current_sort_index < 0 || current_sort_index > 2) current_sort_index = 0;
        fclose(f);
    }
}

void SaveSettings() {
    FILE *f = fopen("khabit_settings.dat", "w");
    if (f) {
        fprintf(f, "%d %d\n", current_color_index, current_sort_index);
        fclose(f);
    }
}

int GetDaysSinceEpoch() {
    time_t t = time(NULL);
    return (int)(t / (60 * 60 * 24));
}

void LoadHabits() {
    FILE *f = fopen("habits.dat", "r");
    if (!f) return;
    habitCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char name[128] = {0};
        int streak = 0, last = 0, target_streak = 0;
        char cat[32] = "Other";
        int n = sscanf(line, "%127[^|]|%d|%d|%31[^|]|%d", name, &streak, &last, cat, &target_streak);
        if (n >= 4) {
            strcpy(habits[habitCount].name, name);
            habits[habitCount].streak = streak;
            habits[habitCount].last_check_day = last;
            strcpy(habits[habitCount].category, cat);
            habits[habitCount].target_streak = target_streak;
            habitCount++;
            if (habitCount >= MAX_HABITS) break;
        }
    }
    fclose(f);
}

void SaveHabits() {
    FILE *f = fopen("habits.dat", "w");
    if (!f) return;
    for (int i = 0; i < habitCount; i++) {
        fprintf(f, "%s|%d|%d|%s|%d\n", habits[i].name, habits[i].streak, habits[i].last_check_day, habits[i].category, habits[i].target_streak);
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

int CompareAlpha(const void *a, const void *b) {
    return _stricmp(((Habit*)a)->name, ((Habit*)b)->name);
}

int CompareStreak(const void *a, const void *b) {
    return ((Habit*)b)->streak - ((Habit*)a)->streak;
}

void SortHabits() {
    if (current_sort_index == 1) {
        qsort(habits, habitCount, sizeof(Habit), CompareAlpha);
    } else if (current_sort_index == 2) {
        qsort(habits, habitCount, sizeof(Habit), CompareStreak);
    }
}

HWND hList, hEdit, hSearchEdit;

void UpdateList() {
    char searchStr[128] = {0};
    if (hSearchEdit) {
        GetWindowText(hSearchEdit, searchStr, sizeof(searchStr));
    }
    for (int i=0; i<strlen(searchStr); i++) searchStr[i] = tolower(searchStr[i]);

    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    int today = GetDaysSinceEpoch();
    for (int i = 0; i < habitCount; i++) {
        char lowerName[128];
        strcpy(lowerName, habits[i].name);
        for (int j=0; j<strlen(lowerName); j++) lowerName[j] = tolower(lowerName[j]);
        if (strlen(searchStr) > 0 && strstr(lowerName, searchStr) == NULL) {
            continue;
        }

        char buffer[256];
        char status[16];
        if (habits[i].last_check_day == today) {
            strcpy(status, "[ \xDF ]");
        } else {
            strcpy(status, "[   ]");
        }
        
        if (habits[i].target_streak > 0 && habits[i].streak >= habits[i].target_streak) {
            sprintf(buffer, "%s [\x02Mastered\x02] %s (Streak: %d/%d)", status, habits[i].name, habits[i].streak, habits[i].target_streak);
        } else if (habits[i].target_streak > 0) {
            sprintf(buffer, "%s [%s] %s (Streak: %d/%d)", status, habits[i].category, habits[i].name, habits[i].streak, habits[i].target_streak);
        } else {
            sprintf(buffer, "%s [%s] %s (Streak: %d)", status, habits[i].category, habits[i].name, habits[i].streak);
        }
        
        int lbIdx = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
        SendMessage(hList, LB_SETITEMDATA, lbIdx, i);
    }
    if (hMainWnd) {
        InvalidateRect(hMainWnd, NULL, TRUE);
    }
}

HBRUSH hbgBrush;
HBRUSH hListBrush;
HFONT hFont;

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HWND hLabel = CreateWindowEx(0, "STATIC", "Select Accent Color:", WS_CHILD | WS_VISIBLE, 20, 20, 200, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND hCombo = CreateWindowEx(0, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 20, 45, 150, 150, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Purple");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Orange");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Blue");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Green");
            SendMessage(hCombo, CB_SETCURSEL, current_color_index, 0);
            HWND hSave = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 80, 80, 30, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 2) {
                HWND hCombo = GetDlgItem(hwnd, 1);
                current_color_index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                SaveSettings();
                InvalidateRect(hMainWnd, NULL, TRUE);
                UpdateWindow(hMainWnd);
                DestroyWindow(hwnd);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hbgBrush = CreateSolidBrush(RGB(24, 24, 28));
            hListBrush = CreateSolidBrush(RGB(32, 32, 36));
            hFont = CreateFont(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            hEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                   20, 20, 120, 28, hwnd, (HMENU)ID_EDIT, NULL, NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

#define ID_COMBO_CAT 111
            HWND hCatCombo = CreateWindowEx(0, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                                             145, 20, 80, 150, hwnd, (HMENU)ID_COMBO_CAT, NULL, NULL);
            SendMessage(hCatCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Health");
            SendMessage(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessage(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessage(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessage(hCatCombo, CB_SETCURSEL, 3, 0);

#define ID_COMBO_TARGET 114
            HWND hTargetCombo = CreateWindowEx(0, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                                             230, 20, 60, 150, hwnd, (HMENU)ID_COMBO_TARGET, NULL, NULL);
            SendMessage(hTargetCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hTargetCombo, CB_ADDSTRING, 0, (LPARAM)"0");
            SendMessage(hTargetCombo, CB_ADDSTRING, 0, (LPARAM)"7");
            SendMessage(hTargetCombo, CB_ADDSTRING, 0, (LPARAM)"30");
            SendMessage(hTargetCombo, CB_ADDSTRING, 0, (LPARAM)"90");
            SendMessage(hTargetCombo, CB_SETCURSEL, 0, 0);

            HWND hSortCombo = CreateWindowEx(0, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                                             295, 20, 80, 150, hwnd, (HMENU)ID_COMBO_SORT, NULL, NULL);
            SendMessage(hSortCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Sort...");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"A-Z");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Streak");

            HWND hAdd = CreateWindowEx(0, "BUTTON", "+ New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       380, 20, 55, 28, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
            SendMessage(hAdd, WM_SETFONT, (WPARAM)hFont, TRUE);

#define ID_BTN_STATS 115
            HWND hStats = CreateWindowEx(0, "BUTTON", "Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       440, 20, 50, 28, hwnd, (HMENU)ID_BTN_STATS, NULL, NULL);
            SendMessage(hStats, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hHelp = CreateWindowEx(0, "BUTTON", "?", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        495, 20, 25, 28, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hHelp, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hSearchLabel = CreateWindowEx(0, "STATIC", "Search:", WS_CHILD | WS_VISIBLE,
                                               20, 145, 60, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hSearchLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            hSearchEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                         80, 140, 440, 28, hwnd, (HMENU)ID_SEARCH_EDIT, NULL, NULL);
            SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            hList = CreateWindowEx(0, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
                                   20, 175, 500, 205, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hCheck = CreateWindowEx(0, "BUTTON", "Check Off", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         20, 395, 90, 35, hwnd, (HMENU)ID_BTN_CHECK, NULL, NULL);
            SendMessage(hCheck, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hDel = CreateWindowEx(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       120, 395, 80, 35, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            SendMessage(hDel, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hImport = CreateWindowEx(0, "BUTTON", "Import", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          210, 395, 80, 35, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
            SendMessage(hImport, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hExport = CreateWindowEx(0, "BUTTON", "Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          300, 395, 80, 35, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            SendMessage(hExport, WM_SETFONT, (WPARAM)hFont, TRUE);

#define ID_BTN_SETTINGS 108
            HWND hSettings = CreateWindowEx(0, "BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          390, 395, 80, 35, hwnd, (HMENU)ID_BTN_SETTINGS, NULL, NULL);
            SendMessage(hSettings, WM_SETFONT, (WPARAM)hFont, TRUE);

            hMainWnd = hwnd;
            LoadSettings();
            SendMessage(GetDlgItem(hwnd, ID_COMBO_SORT), CB_SETCURSEL, current_sort_index, 0);
            LoadHabits();
            CheckStreaks();
            SortHabits();
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
        case WM_DRAWITEM: {
            DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
            if (dis->itemID == -1) return 0;
            char text[256];
            SendMessage(dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)text);
            
            HBRUSH hBrush;
            COLORREF textCol;
            int is_mastered = 0;
            if (strstr(text, "[\x02Mastered\x02]") != NULL) {
                is_mastered = 1;
                // remove the markers for clean display
                char *m1 = strstr(text, "\x02");
                if (m1) {
                    char *m2 = strstr(m1 + 1, "\x02");
                    if (m2) {
                        memmove(m1, m1 + 1, m2 - m1 - 1);
                        memmove(m1 + (m2 - m1 - 1), m2 + 1, strlen(m2 + 1) + 1);
                    }
                }
            }

            if (dis->itemState & ODS_SELECTED) {
                hBrush = CreateSolidBrush(accentColors[current_color_index]);
                textCol = RGB(255, 255, 255);
            } else {
                hBrush = CreateSolidBrush(RGB(32, 32, 36));
                if (is_mastered) {
                    textCol = RGB(251, 191, 36); // Golden color
                } else {
                    textCol = RGB(240, 240, 240);
                }
            }
            FillRect(dis->hDC, &dis->rcItem, hBrush);
            DeleteObject(hBrush);
            
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, textCol);
            
            RECT rcText = dis->rcItem;
            rcText.left += 5;
            SelectObject(dis->hDC, hFont);
            DrawText(dis->hDC, text, -1, &rcText, DT_SINGLELINE | DT_VCENTER);
            return 1;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rcDash = {20, 60, 500, 130};
            FillRect(hdc, &rcDash, hListBrush);
            
            int total = habitCount;
            int completedToday = 0;
            int today = GetDaysSinceEpoch();
            for(int i=0; i<total; i++) {
                if(habits[i].last_check_day == today) completedToday++;
            }
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(240, 240, 240));
            SelectObject(hdc, hFont);
            
            char progText[64];
            sprintf(progText, "Daily Progress: %d / %d", completedToday, total);
            RECT rcText = {35, 65, 200, 85};
            DrawText(hdc, progText, -1, &rcText, DT_SINGLELINE | DT_VCENTER);
            
            RECT rcBarBg = {35, 95, 255, 105};
            HBRUSH hBarBg = CreateSolidBrush(RGB(60, 60, 65));
            FillRect(hdc, &rcBarBg, hBarBg);
            DeleteObject(hBarBg);
            
            if (total > 0 && completedToday > 0) {
                RECT rcBarFill = {35, 95, 35 + (220 * completedToday / total), 105};
                HBRUSH hBarFill = CreateSolidBrush(accentColors[current_color_index]);
                FillRect(hdc, &rcBarFill, hBarFill);
                DeleteObject(hBarFill);
            }
            
            for(int d=6; d>=0; d--) {
                int checkDay = today - d;
                int complOnDay = 0;
                for(int i=0; i<total; i++) {
                    if (checkDay <= habits[i].last_check_day && checkDay > habits[i].last_check_day - habits[i].streak) {
                        complOnDay++;
                    }
                }
                
                int startX = 300 + (6 - d) * 22;
                RECT rcHistBg = {startX, 70, startX + 12, 115};
                HBRUSH hHistBg = CreateSolidBrush(RGB(60, 60, 65));
                FillRect(hdc, &rcHistBg, hHistBg);
                DeleteObject(hHistBg);
                
                if (total > 0 && complOnDay > 0) {
                    int h = 45 * complOnDay / total;
                    RECT rcHistFill = {startX, 115 - h, startX + 12, 115};
                    HBRUSH hHistFill = CreateSolidBrush(accentColors[current_color_index]);
                    FillRect(hdc, &rcHistFill, hHistFill);
                    DeleteObject(hHistFill);
                }
            }
            
            EndPaint(hwnd, &ps);
            return 0;
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
            if (wmId == ID_SEARCH_EDIT && wmEvent == EN_CHANGE) {
                UpdateList();
            } else if (wmId == ID_COMBO_SORT && wmEvent == CBN_SELCHANGE) {
                current_sort_index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                SaveSettings();
                SortHabits();
                SaveHabits();
                UpdateList();
            } else if (wmId == ID_BTN_ADD && wmEvent == BN_CLICKED) {
                if (habitCount < MAX_HABITS) {
                    char text[128];
                    GetWindowText(hEdit, text, sizeof(text));
                    if (strlen(text) > 0) {
                        strcpy(habits[habitCount].name, text);
                        habits[habitCount].streak = 0;
                        habits[habitCount].last_check_day = 0;
                        
                        HWND hCatCombo = GetDlgItem(hwnd, ID_COMBO_CAT);
                        int selCat = SendMessage(hCatCombo, CB_GETCURSEL, 0, 0);
                        if(selCat == CB_ERR) selCat = 3;
                        char catText[32];
                        SendMessage(hCatCombo, CB_GETLBTEXT, selCat, (LPARAM)catText);
                        strcpy(habits[habitCount].category, catText);

                        HWND hTargetCombo = GetDlgItem(hwnd, ID_COMBO_TARGET);
                        int selTarget = SendMessage(hTargetCombo, CB_GETCURSEL, 0, 0);
                        int target_streak = 0;
                        if (selTarget == 1) target_streak = 7;
                        else if (selTarget == 2) target_streak = 30;
                        else if (selTarget == 3) target_streak = 90;
                        habits[habitCount].target_streak = target_streak;

                        habitCount++;
                        SortHabits();
                        SaveHabits();
                        UpdateList();
                        SetWindowText(hEdit, "");
                    }
                }
            } else if (wmId == ID_BTN_CHECK && wmEvent == BN_CLICKED) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int original_idx = SendMessage(hList, LB_GETITEMDATA, sel, 0);
                    int today = GetDaysSinceEpoch();
                    if (habits[original_idx].last_check_day != today) {
                        if (today - habits[original_idx].last_check_day == 1) {
                            habits[original_idx].streak++;
                        } else {
                            habits[original_idx].streak = 1;
                        }
                        habits[original_idx].last_check_day = today;
                    } else {
                        // Undo check off
                        habits[original_idx].last_check_day = 0; // rough undo
                        if(habits[original_idx].streak > 0) habits[original_idx].streak--;
                    }
                    SaveHabits();
                    UpdateList();
                    // Try to re-select
                    for (int k=0; k<SendMessage(hList, LB_GETCOUNT, 0, 0); k++) {
                        if (SendMessage(hList, LB_GETITEMDATA, k, 0) == original_idx) {
                            SendMessage(hList, LB_SETCURSEL, k, 0);
                            break;
                        }
                    }
                }
            } else if (wmId == ID_BTN_DELETE && wmEvent == BN_CLICKED) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int original_idx = SendMessage(hList, LB_GETITEMDATA, sel, 0);
                    for (int i = original_idx; i < habitCount - 1; i++) {
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
                            fprintf(f, "%s,%d,%d,%s,%d\n", habits[i].name, habits[i].streak, habits[i].last_check_day, habits[i].category, habits[i].target_streak);
                        }
                        fclose(f);
                        MessageBox(hwnd, "Export successful!", "Success", MB_OK);
                    }
                }
            } else if (wmId == ID_BTN_STATS && wmEvent == BN_CLICKED) {
                int totalActive = habitCount;
                int totalMastered = 0;
                int longestStreak = 0;
                
                struct {
                    char name[32];
                    int total_streak;
                    int count;
                } cats[10] = {0};
                int num_cats = 0;

                for (int i = 0; i < habitCount; i++) {
                    if (habits[i].target_streak > 0 && habits[i].streak >= habits[i].target_streak) {
                        totalMastered++;
                    }
                    if (habits[i].streak > longestStreak) {
                        longestStreak = habits[i].streak;
                    }
                    
                    int found = -1;
                    for (int j = 0; j < num_cats; j++) {
                        if (strcmp(cats[j].name, habits[i].category) == 0) {
                            found = j; break;
                        }
                    }
                    if (found == -1 && num_cats < 10) {
                        strcpy(cats[num_cats].name, habits[i].category);
                        cats[num_cats].total_streak = habits[i].streak;
                        cats[num_cats].count = 1;
                        num_cats++;
                    } else if (found != -1) {
                        cats[found].total_streak += habits[i].streak;
                        cats[found].count++;
                    }
                }
                
                char mostConsistent[32] = "None";
                float highestAvg = -1.0f;
                for (int i = 0; i < num_cats; i++) {
                    float avg = (float)cats[i].total_streak / cats[i].count;
                    if (avg > highestAvg) {
                        highestAvg = avg;
                        strcpy(mostConsistent, cats[i].name);
                    }
                }
                
                char statsStr[512];
                sprintf(statsStr, 
                        "Total Active Habits: %d\n"
                        "Total Mastered Habits: %d\n"
                        "Longest Active Streak: %d\n"
                        "Most Consistent Category: %s (%.1f avg streak)",
                        totalActive, totalMastered, longestStreak, mostConsistent, highestAvg >= 0 ? highestAvg : 0.0f);
                        
                MessageBox(hwnd, statsStr, "Detailed Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (wmId == ID_BTN_SETTINGS && wmEvent == BN_CLICKED) {
                HWND hSettingsWnd = CreateWindowEx(
                    0, "KHabitSettingsClass", "Settings", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                    CW_USEDEFAULT, CW_USEDEFAULT, 220, 180,
                    hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
                );
                ShowWindow(hSettingsWnd, SW_SHOW);
            } else if (wmId == ID_BTN_HELP && wmEvent == BN_CLICKED) {
                MessageBox(hwnd, "KHabit Guide:\n\n"
                                 "Tracking: Select a habit and click 'Check Off' (or press Space) to log daily progress.\n"
                                 "Categories: Group habits by Health, Work, Personal, or Other.\n"
                                 "Shortcuts:\n"
                                 "  Ctrl+N: New Habit\n"
                                 "  Up/Down: Select Habit\n"
                                 "  Space: Toggle Checkmark\n"
                                 "  Delete: Delete Habit\n"
                                 "Data: Use Import/Export to manage your habit lists.\n\n"
                                 "Build your streak by completing habits on consecutive days!", 
                                 "Help / Tutorial", MB_OK | MB_ICONINFORMATION);
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
                        char line[256];
                        while (fgets(line, sizeof(line), f)) {
                            char name[128] = {0};
                            int streak = 0, last = 0, target_streak = 0;
                            char cat[32] = "Other";
                            int n = sscanf(line, "%127[^,],%d,%d,%31[^,],%d", name, &streak, &last, cat, &target_streak);
                            if (n >= 4) {
                                strcpy(habits[habitCount].name, name);
                                habits[habitCount].streak = streak;
                                habits[habitCount].last_check_day = last;
                                strcpy(habits[habitCount].category, cat);
                                habits[habitCount].target_streak = target_streak;
                                habitCount++;
                                if (habitCount >= MAX_HABITS) break;
                            }
                        }
                        fclose(f);
                        SortHabits();
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

    WNDCLASS wcSet = { };
    wcSet.lpfnWndProc = SettingsProc;
    wcSet.hInstance = hInstance;
    wcSet.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcSet.lpszClassName = "KHabitSettingsClass";
    RegisterClass(&wcSet);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KHabit Tracker", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 490,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        BOOL bHandled = FALSE;
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == 'N' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                SetFocus(hEdit);
                bHandled = TRUE;
            } else if (msg.wParam == VK_SPACE) {
                if (GetFocus() == hList || GetFocus() == hwnd) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CHECK, BN_CLICKED), 0);
                    bHandled = TRUE;
                }
            } else if (msg.wParam == VK_DELETE) {
                if (GetFocus() == hList || GetFocus() == hwnd) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DELETE, BN_CLICKED), 0);
                    bHandled = TRUE;
                }
            } else if (msg.wParam == VK_UP || msg.wParam == VK_DOWN) {
                if (GetFocus() != hList && GetFocus() != hEdit && GetFocus() != hSearchEdit) {
                    SetFocus(hList);
                }
            }
        }
        if (!bHandled) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
