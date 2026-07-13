#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <commdlg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "resource.h"

#define MAX_TRANSACTIONS 1000

typedef struct {
    int is_income;
    double amount;
    char category[64];
    char description[128];
    char date[32];
} Transaction;

Transaction transactions[MAX_TRANSACTIONS];
int num_transactions = 0;
int editing_index = -1;
int current_page = 1;
const int items_per_page = 20;

HWND hMainWnd;
HWND hList;
HWND hBtnAdd;
HWND hBtnSettings;
HWND hSearchEdit;
HWND hSortCombo;
HWND hLblTotal, hLblIncome, hLblExpense;
HWND hBtnPrev, hBtnNext, hLblPage;

HBRUSH hbgBrush;
HFONT hFont;

char currency_symbol[8] = "$";

void SaveData() {
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        char dirPath[MAX_PATH];
        snprintf(dirPath, sizeof(dirPath), "%s\\KBudget", appDataPath);
        CreateDirectoryA(dirPath, NULL);
        
        char filePath[MAX_PATH];
        snprintf(filePath, sizeof(filePath), "%s\\kbudget.dat", dirPath);
        
        HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bytesWritten;
            WriteFile(hFile, &num_transactions, sizeof(int), &bytesWritten, NULL);
            if (num_transactions > 0) {
                WriteFile(hFile, transactions, sizeof(Transaction) * num_transactions, &bytesWritten, NULL);
            }
            CloseHandle(hFile);
        }
    }
}

void LoadData() {
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        char filePath[MAX_PATH];
        snprintf(filePath, sizeof(filePath), "%s\\KBudget\\kbudget.dat", appDataPath);
        
        HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bytesRead;
            ReadFile(hFile, &num_transactions, sizeof(int), &bytesRead, NULL);
            if (num_transactions < 0 || num_transactions > MAX_TRANSACTIONS) {
                num_transactions = 0;
            } else if (num_transactions > 0) {
                ReadFile(hFile, transactions, sizeof(Transaction) * num_transactions, &bytesRead, NULL);
            }
            CloseHandle(hFile);
        }
    }
}

void SaveSettings() {
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        char dirPath[MAX_PATH];
        snprintf(dirPath, sizeof(dirPath), "%s\\KBudget", appDataPath);
        CreateDirectoryA(dirPath, NULL);
        char filePath[MAX_PATH];
        snprintf(filePath, sizeof(filePath), "%s\\kbudget_settings.dat", dirPath);
        HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bytesWritten;
            WriteFile(hFile, currency_symbol, sizeof(currency_symbol), &bytesWritten, NULL);
            CloseHandle(hFile);
        }
    }
}

void LoadSettings() {
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        char filePath[MAX_PATH];
        snprintf(filePath, sizeof(filePath), "%s\\KBudget\\kbudget_settings.dat", appDataPath);
        HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bytesRead;
            ReadFile(hFile, currency_symbol, sizeof(currency_symbol), &bytesRead, NULL);
            CloseHandle(hFile);
        }
    }
}

int compare_indices(const void *a, const void *b) {
    int idxA = *(int*)a;
    int idxB = *(int*)b;
    int sortType = 0;
    if (hSortCombo) {
        sortType = SendMessage(hSortCombo, CB_GETCURSEL, 0, 0);
    }
    Transaction *tA = &transactions[idxA];
    Transaction *tB = &transactions[idxB];
    
    if (sortType == 0) { // Date Newest
        return strcmp(tB->date, tA->date);
    } else if (sortType == 1) { // Date Oldest
        return strcmp(tA->date, tB->date);
    } else if (sortType == 2) { // Amount Highest
        if (tB->amount > tA->amount) return 1;
        if (tB->amount < tA->amount) return -1;
        return 0;
    } else if (sortType == 3) { // Amount Lowest
        if (tA->amount > tB->amount) return 1;
        if (tA->amount < tB->amount) return -1;
        return 0;
    }
    return 0;
}

void UpdateUI() {
    double total_income = 0;
    double total_expense = 0;
    
    char searchTerm[128] = {0};
    if (hSearchEdit) {
        GetWindowText(hSearchEdit, searchTerm, sizeof(searchTerm));
    }
    for(int i = 0; searchTerm[i]; i++) searchTerm[i] = tolower((unsigned char)searchTerm[i]);
    
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
    int *filtered = (int*)malloc(num_transactions * sizeof(int));
    int filtered_count = 0;
    
    for (int i = 0; i < num_transactions; i++) {
        char catLower[64], descLower[128];
        strncpy(catLower, transactions[i].category, 64);
        strncpy(descLower, transactions[i].description, 128);
        for(int j = 0; catLower[j]; j++) catLower[j] = tolower((unsigned char)catLower[j]);
        for(int j = 0; descLower[j]; j++) descLower[j] = tolower((unsigned char)descLower[j]);
        
        if (searchTerm[0] != '\0') {
            if (strstr(catLower, searchTerm) == NULL && strstr(descLower, searchTerm) == NULL) {
                continue;
            }
        }
        
        if (transactions[i].is_income) {
            total_income += transactions[i].amount;
        } else {
            total_expense += transactions[i].amount;
        }
        
        filtered[filtered_count++] = i;
    }
    
    qsort(filtered, filtered_count, sizeof(int), compare_indices);
    
    int total_pages = filtered_count / items_per_page;
    if (filtered_count % items_per_page != 0 || total_pages == 0) total_pages++;
    
    if (current_page > total_pages) current_page = total_pages;
    if (current_page < 1) current_page = 1;
    
    char pageBuf[32];
    snprintf(pageBuf, sizeof(pageBuf), "Page %d of %d", current_page, total_pages);
    if (hLblPage) SetWindowText(hLblPage, pageBuf);
    
    if (hBtnPrev) EnableWindow(hBtnPrev, current_page > 1);
    if (hBtnNext) EnableWindow(hBtnNext, current_page < total_pages);

    int start_idx = (current_page - 1) * items_per_page;
    int end_idx = start_idx + items_per_page;
    if (end_idx > filtered_count) end_idx = filtered_count;
    
    for (int k = start_idx; k < end_idx; k++) {
        int i = filtered[k];
        char buf[256];
        snprintf(buf, sizeof(buf), "%s | %s | %s%s%.2f | %s", 
            transactions[i].date, transactions[i].description, 
            transactions[i].is_income ? "+" : "-", currency_symbol, transactions[i].amount, 
            transactions[i].category);
        
        int listIdx = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
        SendMessage(hList, LB_SETITEMDATA, listIdx, (LPARAM)i);
    }
    
    free(filtered);
    
    char sumBuf[128];
    snprintf(sumBuf, sizeof(sumBuf), "Total Balance: %s%.2f", currency_symbol, total_income - total_expense);
    SetWindowText(hLblTotal, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Income: +%s%.2f", currency_symbol, total_income);
    SetWindowText(hLblIncome, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Expenses: -%s%.2f", currency_symbol, total_expense);
    SetWindowText(hLblExpense, sumBuf);
    InvalidateRect(hMainWnd, NULL, TRUE);
}

INT_PTR CALLBACK AddDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            if (editing_index >= 0) {
                Transaction *t = &transactions[editing_index];
                char typeStr[32], amtStr[32];
                snprintf(typeStr, sizeof(typeStr), "%d", t->is_income);
                snprintf(amtStr, sizeof(amtStr), "%.2f", t->amount);
                SetDlgItemText(hwndDlg, IDC_TYPE_EDIT, typeStr);
                SetDlgItemText(hwndDlg, IDC_AMOUNT_EDIT, amtStr);
                SetDlgItemText(hwndDlg, IDC_CAT_EDIT, t->category);
                SetDlgItemText(hwndDlg, IDC_DESC_EDIT, t->description);
                SetDlgItemText(hwndDlg, IDC_DATE_EDIT, t->date);
            } else {
                SetDlgItemText(hwndDlg, IDC_TYPE_EDIT, "0");
                SetDlgItemText(hwndDlg, IDC_AMOUNT_EDIT, "0.00");
                SetDlgItemText(hwndDlg, IDC_CAT_EDIT, "Food");
                SetDlgItemText(hwndDlg, IDC_DESC_EDIT, "Lunch");
                SetDlgItemText(hwndDlg, IDC_DATE_EDIT, "2026-07-12");
            }
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                if (editing_index >= 0 || num_transactions < MAX_TRANSACTIONS) {
                    char typeStr[32], amtStr[32];
                    GetDlgItemText(hwndDlg, IDC_TYPE_EDIT, typeStr, sizeof(typeStr));
                    GetDlgItemText(hwndDlg, IDC_AMOUNT_EDIT, amtStr, sizeof(amtStr));
                    
                    Transaction *t;
                    if (editing_index >= 0) {
                        t = &transactions[editing_index];
                    } else {
                        t = &transactions[num_transactions];
                        num_transactions++;
                    }
                    
                    t->is_income = atoi(typeStr);
                    t->amount = atof(amtStr);
                    GetDlgItemText(hwndDlg, IDC_CAT_EDIT, t->category, sizeof(t->category));
                    GetDlgItemText(hwndDlg, IDC_DESC_EDIT, t->description, sizeof(t->description));
                    GetDlgItemText(hwndDlg, IDC_DATE_EDIT, t->date, sizeof(t->date));
                    
                    SaveData();
                    UpdateUI();
                }
                EndDialog(hwndDlg, IDOK);
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwndDlg, IDC_CURRENCY_EDIT, currency_symbol);
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, IDC_CURRENCY_EDIT, currency_symbol, sizeof(currency_symbol));
                SaveSettings();
                UpdateUI();
                EndDialog(hwndDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void ExportCSV(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = "kbudget_export.csv";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "csv";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        FILE *f = fopen(szFile, "w");
        if (f) {
            fprintf(f, "is_income,amount,category,date,description\n");
            for (int i = 0; i < num_transactions; i++) {
                char cat[64], date[32], desc[128];
                strncpy(cat, transactions[i].category, 64);
                strncpy(date, transactions[i].date, 32);
                strncpy(desc, transactions[i].description, 128);
                for(int j=0; cat[j]; j++) if(cat[j]==',') cat[j]=' ';
                for(int j=0; date[j]; j++) if(date[j]==',') date[j]=' ';
                for(int j=0; desc[j]; j++) if(desc[j]==',') desc[j]=' ';
                
                fprintf(f, "%d,%.2f,%s,%s,%s\n", 
                    transactions[i].is_income, 
                    transactions[i].amount, 
                    cat, 
                    date, 
                    desc);
            }
            fclose(f);
            MessageBoxA(hwnd, "Export successful!", "Success", MB_OK);
        }
    }
}

void ImportCSV(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        FILE *f = fopen(szFile, "r");
        if (f) {
            char line[512];
            fgets(line, sizeof(line), f); // skip header
            int imported = 0;
            while (fgets(line, sizeof(line), f) && num_transactions < MAX_TRANSACTIONS) {
                int is_inc = 0;
                double amt = 0;
                char cat[64] = {0}, date[32] = {0}, desc[128] = {0};
                
                int parsed = sscanf(line, "%d,%lf,%63[^,],%31[^,],%127[^\r\n]", &is_inc, &amt, cat, date, desc);
                if (parsed >= 4) {
                    if (parsed == 4) desc[0] = '\0';
                    Transaction t = {0};
                    t.is_income = is_inc;
                    t.amount = amt;
                    strncpy(t.category, cat, sizeof(t.category)-1);
                    strncpy(t.date, date, sizeof(t.date)-1);
                    strncpy(t.description, desc, sizeof(t.description)-1);
                    transactions[num_transactions++] = t;
                    imported++;
                }
            }
            fclose(f);
            if (imported > 0) {
                SaveData();
                UpdateUI();
                char msg[64];
                snprintf(msg, sizeof(msg), "Imported %d transactions!", imported);
                MessageBoxA(hwnd, msg, "Success", MB_OK);
            } else {
                MessageBoxA(hwnd, "No valid transactions found.", "Import Failed", MB_OK | MB_ICONWARNING);
            }
        }
    }
}

void PrintReport(HWND hwnd) {
    char tempPath[MAX_PATH];
    char filePath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    snprintf(filePath, MAX_PATH, "%skbudget_print.html", tempPath);
    FILE *f = fopen(filePath, "w");
    if (f) {
        fprintf(f, "<html><head><title>KBudget Report</title><style>body{font-family:sans-serif;} table{width:100%%;border-collapse:collapse;} th,td{border:1px solid #ccc;padding:8px;text-align:left;}</style></head><body>");
        fprintf(f, "<h2>KBudget Transaction Report</h2>");
        fprintf(f, "<table><tr><th>Date</th><th>Description</th><th>Category</th><th>Amount</th></tr>");
        for (int i = 0; i < num_transactions; i++) {
            fprintf(f, "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s%s%.2f</td></tr>",
                transactions[i].date, transactions[i].description, transactions[i].category,
                transactions[i].is_income ? "+" : "-", currency_symbol, transactions[i].amount);
        }
        fprintf(f, "</table><script>window.print();</script></body></html>");
        fclose(f);
        ShellExecuteA(hwnd, "open", filePath, NULL, NULL, SW_SHOWNORMAL);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CREATE:
            hbgBrush = CreateSolidBrush(RGB(15, 23, 42)); // dark blue background #0f172a
            hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            
            hLblTotal = CreateWindow("STATIC", "Total Balance: $0.00", WS_CHILD | WS_VISIBLE,
                20, 20, 200, 20, hwnd, NULL, NULL, NULL);
            hLblIncome = CreateWindow("STATIC", "Income: +$0.00", WS_CHILD | WS_VISIBLE,
                20, 50, 200, 20, hwnd, NULL, NULL, NULL);
            hLblExpense = CreateWindow("STATIC", "Expenses: -$0.00", WS_CHILD | WS_VISIBLE,
                20, 80, 200, 20, hwnd, NULL, NULL, NULL);
                
            hBtnAdd = CreateWindow("BUTTON", "+ New Transaction", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 120, 150, 30, hwnd, (HMENU)1, NULL, NULL);
            HWND hBtnEdit = CreateWindow("BUTTON", "Edit Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 160, 150, 30, hwnd, (HMENU)6, NULL, NULL);
            HWND hBtnDel = CreateWindow("BUTTON", "Delete Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 200, 150, 30, hwnd, (HMENU)7, NULL, NULL);
            HWND hBtnImp = CreateWindow("BUTTON", "Import CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 240, 150, 30, hwnd, (HMENU)2, NULL, NULL);
            HWND hBtnExp = CreateWindow("BUTTON", "Export CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 280, 150, 30, hwnd, (HMENU)3, NULL, NULL);
            hBtnSettings = CreateWindow("BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 320, 150, 30, hwnd, (HMENU)5, NULL, NULL);
            HWND hBtnPrint = CreateWindow("BUTTON", "Print", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 360, 150, 30, hwnd, (HMENU)11, NULL, NULL);
                
            hSearchEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                250, 20, 340, 25, hwnd, (HMENU)4, NULL, NULL);
            hSortCombo = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                600, 20, 150, 150, hwnd, (HMENU)8, NULL, NULL);
                
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Date (Newest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Date (Oldest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Amount (Highest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Amount (Lowest)");
            SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);

            hList = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
                250, 50, 500, 270, hwnd, NULL, NULL, NULL);
                
            hBtnPrev = CreateWindow("BUTTON", "Prev Page", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                250, 330, 100, 30, hwnd, (HMENU)9, NULL, NULL);
            hLblPage = CreateWindow("STATIC", "Page 1 of 1", WS_CHILD | WS_VISIBLE | SS_CENTER,
                360, 335, 120, 20, hwnd, NULL, NULL, NULL);
            hBtnNext = CreateWindow("BUTTON", "Next Page", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                490, 330, 100, 30, hwnd, (HMENU)10, NULL, NULL);
            
            SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSortCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblTotal, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblIncome, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblExpense, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnImp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSettings, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPrint, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblPage, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LoadData();
            LoadSettings();
            UpdateUI();
            return 0;
            
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                if (hSearchEdit) {
                    MoveWindow(hSearchEdit, 250, 20, width - 430, 25, TRUE);
                }
                if (hSortCombo) {
                    MoveWindow(hSortCombo, width - 170, 20, 150, 150, TRUE);
                }
                if (hList) {
                    MoveWindow(hList, 250, 50, width - 270, height - 110, TRUE);
                }
                if (hBtnPrev) {
                    MoveWindow(hBtnPrev, 250, height - 50, 100, 30, TRUE);
                }
                if (hLblPage) {
                    MoveWindow(hLblPage, 360, height - 45, 120, 20, TRUE);
                }
                if (hBtnNext) {
                    MoveWindow(hBtnNext, 490, height - 50, 100, 30, TRUE);
                }
            }
            return 0;
            
        case WM_CTLCOLORSTATIC:
            {
                HDC hdcStatic = (HDC)wParam;
                SetTextColor(hdcStatic, RGB(248, 250, 252));
                SetBkColor(hdcStatic, RGB(15, 23, 42));
                return (INT_PTR)hbgBrush;
            }
            
        case WM_COMMAND:
            if ((HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 4) || (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 8)) {
                current_page = 1;
                UpdateUI();
            } else if (LOWORD(wParam) == 1) {
                editing_index = -1;
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_TRANSACTION), hwnd, AddDialogProc);
            } else if (LOWORD(wParam) == 6) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    editing_index = SendMessage(hList, LB_GETITEMDATA, sel, 0);
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_TRANSACTION), hwnd, AddDialogProc);
                } else {
                    MessageBoxA(hwnd, "Please select a transaction to edit.", "Notice", MB_OK);
                }
            } else if (LOWORD(wParam) == 7) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int idx = SendMessage(hList, LB_GETITEMDATA, sel, 0);
                    if (MessageBoxA(hwnd, "Are you sure you want to delete this transaction?", "Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        for (int i = idx; i < num_transactions - 1; i++) {
                            transactions[i] = transactions[i + 1];
                        }
                        num_transactions--;
                        SaveData();
                        UpdateUI();
                    }
                } else {
                    MessageBoxA(hwnd, "Please select a transaction to delete.", "Notice", MB_OK);
                }
            } else if (LOWORD(wParam) == 2) {
                ImportCSV(hwnd);
            } else if (LOWORD(wParam) == 3) {
                ExportCSV(hwnd);
            } else if (LOWORD(wParam) == 5) {
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hwnd, SettingsDialogProc);
            } else if (LOWORD(wParam) == 9) {
                if (current_page > 1) {
                    current_page--;
                    UpdateUI();
                }
            } else if (LOWORD(wParam) == 10) {
                current_page++;
                UpdateUI();
            } else if (LOWORD(wParam) == 11) {
                PrintReport(hwnd);
            }
            break;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                double total_expense = 0;
                double categoryTotals[10] = {0};
                const char* cats[] = {"Food", "Transport", "Utilities", "Entertainment", "Shopping", "Health", "Other"};
                int num_cats = 7;
                
                for (int i = 0; i < num_transactions; i++) {
                    if (!transactions[i].is_income) {
                        total_expense += transactions[i].amount;
                        int found = 0;
                        for (int c = 0; c < num_cats; c++) {
                            if (strcmp(transactions[i].category, cats[c]) == 0) {
                                categoryTotals[c] += transactions[i].amount;
                                found = 1;
                                break;
                            }
                        }
                        if (!found) categoryTotals[6] += transactions[i].amount;
                    }
                }
                
                int cx = 110;
                int cy = 450;
                int r = 60;
                
                SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(248, 250, 252));
                SetBkMode(hdc, TRANSPARENT);
                TextOut(hdc, cx - 70, cy - r - 25, "Expenses by Category", 20);
                
                if (total_expense > 0) {
                    COLORREF colors[] = {RGB(239, 68, 68), RGB(249, 115, 22), RGB(245, 158, 11), RGB(234, 179, 8), RGB(132, 204, 22), RGB(34, 197, 94), RGB(59, 130, 246)};
                    
                    double start_angle = -1.57079632679; // -90 degrees
                    for (int c = 0; c < num_cats; c++) {
                        if (categoryTotals[c] > 0) {
                            if (categoryTotals[c] >= total_expense) {
                                HBRUSH hBrush = CreateSolidBrush(colors[c % 7]);
                                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
                                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                                Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
                                SelectObject(hdc, hOldBrush);
                                SelectObject(hdc, hOldPen);
                                DeleteObject(hBrush);
                                DeleteObject(hPen);
                                break;
                            }
                            double slice_angle = (categoryTotals[c] / total_expense) * 2.0 * 3.1415926535;
                            double end_angle = start_angle - slice_angle;
                            
                            int x1 = cx + (int)(r * cos(start_angle));
                            int y1 = cy + (int)(r * sin(start_angle));
                            int x2 = cx + (int)(r * cos(end_angle));
                            int y2 = cy + (int)(r * sin(end_angle));
                            
                            HBRUSH hBrush = CreateSolidBrush(colors[c % 7]);
                            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
                            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                            
                            Pie(hdc, cx - r, cy - r, cx + r, cy + r, x1, y1, x2, y2);
                            
                            SelectObject(hdc, hOldBrush);
                            SelectObject(hdc, hOldPen);
                            DeleteObject(hBrush);
                            DeleteObject(hPen);
                            
                            start_angle = end_angle;
                        }
                    }
                } else {
                    SetTextColor(hdc, RGB(100, 116, 139));
                    TextOut(hdc, cx - 40, cy - 10, "No expenses", 11);
                }
                
                EndPaint(hwnd, &ps);
            }
            return 0;
            
        case WM_DESTROY:
            SaveData();
            DeleteObject(hbgBrush);
            DeleteObject(hFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KBudgetClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);
    
    hMainWnd = CreateWindowEx(
        0, CLASS_NAME, "KBudget",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );
    
    if (hMainWnd == NULL) return 0;
    
    SetClassLongPtr(hMainWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(15, 23, 42)));
    
    ShowWindow(hMainWnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        BOOL bHandled = FALSE;
        if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
            if (msg.wParam == 'N') {
                SendMessage(hMainWnd, WM_COMMAND, 1, 0);
                bHandled = TRUE;
            } else if (msg.wParam == 'F') {
                SetFocus(hSearchEdit);
                bHandled = TRUE;
            } else if (msg.wParam == 'S') {
                SendMessage(hMainWnd, WM_COMMAND, 3, 0);
                bHandled = TRUE;
            } else if (msg.wParam == 'O') {
                SendMessage(hMainWnd, WM_COMMAND, 2, 0);
                bHandled = TRUE;
            }
        }
        if (!bHandled) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return 0;
}
