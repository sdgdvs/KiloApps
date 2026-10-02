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
HWND hBtnSave, hBtnLoad, hBtnHelp;
HWND hStatus;
HWND hSearchEdit;
HWND hSortCombo;
HWND hLblTotal, hLblIncome, hLblExpense;
HWND hBtnPrev, hBtnNext, hLblPage;

HBRUSH hbgBrush;
HFONT hFont;

char currency_symbol[8] = "$";

#define KBUDGET_QUICKSAVE_MAGIC 0x47445542 // "BUDG"

void ShowNativeStatus(const char* msg) {
    if (hStatus && msg) {
        SetWindowTextA(hStatus, msg);
    }
}

void ShowHelp(HWND hwnd) {
    MessageBoxA(hwnd,
        "=== KBudget Studio Guide & Shortcuts ===\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "- F1 or H: Open this Help & Feature Guide\n"
        "- F5: Quicksave ledger snapshot to kbudget_quicksave.dat\n"
        "- F9: Quickload ledger snapshot from kbudget_quicksave.dat\n"
        "- Ctrl+N: Add new transaction (or click '+ New')\n"
        "- Ctrl+F: Focus the search box\n"
        "- Ctrl+S: Export ledger to CSV spreadsheet\n"
        "- Ctrl+O: Import ledger from CSV spreadsheet\n"
        "- Delete: Delete selected transaction\n"
        "- Left / Right Arrow: Previous / Next page\n"
        "- Esc: Close active dialogs\n\n"
        "FEATURES:\n"
        "- Full state snapshot persistence across sessions (F5/F9)\n"
        "- Real-time expenses by category allocation pie chart\n"
        "- Instant multi-criteria search and sort (Date/Amount)\n"
        "- Formatted HTML printable report generation\n\n"
        "KBudget - Lightweight Retro Financial Ledger",
        "KBudget Help & Feature Guide", MB_OK | MB_ICONINFORMATION);
}

void NativeQuickSave(HWND hwnd) {
    HANDLE hFile = CreateFileA("kbudget_quicksave.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        DWORD magic = KBUDGET_QUICKSAVE_MAGIC;
        DWORD version = 1;
        int sortType = hSortCombo ? (int)SendMessage(hSortCombo, CB_GETCURSEL, 0, 0) : 0;
        char searchBuf[128] = {0};
        if (hSearchEdit) {
            GetWindowTextA(hSearchEdit, searchBuf, sizeof(searchBuf));
        }

        WriteFile(hFile, &magic, sizeof(magic), &written, NULL);
        WriteFile(hFile, &version, sizeof(version), &written, NULL);
        WriteFile(hFile, &num_transactions, sizeof(num_transactions), &written, NULL);
        WriteFile(hFile, currency_symbol, sizeof(currency_symbol), &written, NULL);
        WriteFile(hFile, &current_page, sizeof(current_page), &written, NULL);
        WriteFile(hFile, &sortType, sizeof(sortType), &written, NULL);
        WriteFile(hFile, searchBuf, sizeof(searchBuf), &written, NULL);
        if (num_transactions > 0) {
            WriteFile(hFile, transactions, sizeof(Transaction) * num_transactions, &written, NULL);
        }
        CloseHandle(hFile);

        // Also touch tutorial flag so saved states never trigger first-run tutorial
        HANDLE hTut = CreateFileA("kbudget_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            WriteFile(hTut, "1", 1, &written, NULL);
            CloseHandle(hTut);
        }

        char msg[128];
        snprintf(msg, sizeof(msg), " Quicksaved %d transaction(s) to kbudget_quicksave.dat [F5]", num_transactions);
        ShowNativeStatus(msg);
    } else {
        ShowNativeStatus(" Quicksave failed: unable to write kbudget_quicksave.dat");
    }
}

void UpdateUI();
void SaveData();
void SaveSettings();

void NativeQuickLoad(HWND hwnd) {
    HANDLE hFile = CreateFileA("kbudget_quicksave.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ShowNativeStatus(" No quicksave snapshot found (Press F5 to quicksave) [F9]");
        return;
    }

    DWORD magic = 0, version = 0, readBytes = 0;
    ReadFile(hFile, &magic, sizeof(magic), &readBytes, NULL);
    ReadFile(hFile, &version, sizeof(version), &readBytes, NULL);

    if (magic != KBUDGET_QUICKSAVE_MAGIC || version != 1) {
        CloseHandle(hFile);
        ShowNativeStatus(" Invalid or corrupt kbudget_quicksave.dat format!");
        return;
    }

    int count = 0;
    ReadFile(hFile, &count, sizeof(count), &readBytes, NULL);
    if (count < 0 || count > MAX_TRANSACTIONS) {
        CloseHandle(hFile);
        ShowNativeStatus(" Corrupt transaction count in quicksave file!");
        return;
    }

    char cur[8] = {0};
    ReadFile(hFile, cur, sizeof(cur), &readBytes, NULL);
    if (cur[0] != '\0') {
        strncpy(currency_symbol, cur, sizeof(currency_symbol) - 1);
        currency_symbol[sizeof(currency_symbol) - 1] = '\0';
    }

    int page = 1;
    ReadFile(hFile, &page, sizeof(page), &readBytes, NULL);
    current_page = (page > 0) ? page : 1;

    int sortType = 0;
    ReadFile(hFile, &sortType, sizeof(sortType), &readBytes, NULL);
    if (hSortCombo && sortType >= 0 && sortType < 4) {
        SendMessage(hSortCombo, CB_SETCURSEL, sortType, 0);
    }

    char searchBuf[128] = {0};
    ReadFile(hFile, searchBuf, sizeof(searchBuf), &readBytes, NULL);
    if (hSearchEdit) {
        SetWindowTextA(hSearchEdit, searchBuf);
    }

    num_transactions = count;
    if (num_transactions > 0) {
        ReadFile(hFile, transactions, sizeof(Transaction) * num_transactions, &readBytes, NULL);
    }
    CloseHandle(hFile);

    SaveData();
    SaveSettings();
    UpdateUI();

    char msg[128];
    snprintf(msg, sizeof(msg), " Quickloaded %d transaction(s) from kbudget_quicksave.dat [F9]", num_transactions);
    ShowNativeStatus(msg);
}

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
                20, 15, 205, 20, hwnd, NULL, NULL, NULL);
            hLblIncome = CreateWindow("STATIC", "Income: +$0.00", WS_CHILD | WS_VISIBLE,
                20, 38, 205, 20, hwnd, NULL, NULL, NULL);
            hLblExpense = CreateWindow("STATIC", "Expenses: -$0.00", WS_CHILD | WS_VISIBLE,
                20, 61, 205, 20, hwnd, NULL, NULL, NULL);
                
            hBtnAdd = CreateWindow("BUTTON", "+ New (Ctrl+N)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 90, 205, 28, hwnd, (HMENU)1, NULL, NULL);
            HWND hBtnEdit = CreateWindow("BUTTON", "Edit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 122, 100, 28, hwnd, (HMENU)6, NULL, NULL);
            HWND hBtnDel = CreateWindow("BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                125, 122, 100, 28, hwnd, (HMENU)7, NULL, NULL);
            hBtnSave = CreateWindow("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 154, 100, 28, hwnd, (HMENU)12, NULL, NULL);
            hBtnLoad = CreateWindow("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                125, 154, 100, 28, hwnd, (HMENU)13, NULL, NULL);
            HWND hBtnImp = CreateWindow("BUTTON", "Import CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 186, 100, 28, hwnd, (HMENU)2, NULL, NULL);
            HWND hBtnExp = CreateWindow("BUTTON", "Export CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                125, 186, 100, 28, hwnd, (HMENU)3, NULL, NULL);
            hBtnSettings = CreateWindow("BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 218, 100, 28, hwnd, (HMENU)5, NULL, NULL);
            HWND hBtnPrint = CreateWindow("BUTTON", "Print", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                125, 218, 100, 28, hwnd, (HMENU)11, NULL, NULL);
            hBtnHelp = CreateWindow("BUTTON", "Help & Guide [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 250, 205, 28, hwnd, (HMENU)14, NULL, NULL);
                
            hSearchEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                250, 15, 340, 25, hwnd, (HMENU)4, NULL, NULL);
            hSortCombo = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                600, 15, 150, 150, hwnd, (HMENU)8, NULL, NULL);
                
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Date (Newest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Date (Oldest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Amount (Highest)");
            SendMessage(hSortCombo, CB_ADDSTRING, 0, (LPARAM)"Amount (Lowest)");
            SendMessage(hSortCombo, CB_SETCURSEL, 0, 0);

            hList = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
                250, 48, 500, 270, hwnd, NULL, NULL, NULL);
                
            hBtnPrev = CreateWindow("BUTTON", "Prev Page", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                250, 330, 100, 28, hwnd, (HMENU)9, NULL, NULL);
            hLblPage = CreateWindow("STATIC", "Page 1 of 1", WS_CHILD | WS_VISIBLE | SS_CENTER,
                360, 334, 120, 20, hwnd, NULL, NULL, NULL);
            hBtnNext = CreateWindow("BUTTON", "Next Page", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                490, 330, 100, 28, hwnd, (HMENU)10, NULL, NULL);

            hStatus = CreateWindowExA(0, "STATIC",
                " Ready | F1: Help | F5: Save | F9: Load | Ctrl+N: New | Ctrl+F: Search | Ctrl+S: Export",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 545, 760, 20, hwnd, (HMENU)300, NULL, NULL);
            
            SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hSortCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblTotal, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblIncome, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblExpense, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnImp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSettings, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPrint, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblPage, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LoadData();
            LoadSettings();
            UpdateUI();
            return 0;
            
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                if (hSearchEdit) {
                    MoveWindow(hSearchEdit, 250, 15, width - 420, 25, TRUE);
                }
                if (hSortCombo) {
                    MoveWindow(hSortCombo, width - 160, 15, 140, 150, TRUE);
                }
                if (hList) {
                    MoveWindow(hList, 250, 48, width - 270, height - 130, TRUE);
                }
                if (hBtnPrev) {
                    MoveWindow(hBtnPrev, 250, height - 65, 100, 28, TRUE);
                }
                if (hLblPage) {
                    MoveWindow(hLblPage, 360, height - 60, 120, 20, TRUE);
                }
                if (hBtnNext) {
                    MoveWindow(hBtnNext, 490, height - 65, 100, 28, TRUE);
                }
                if (hStatus) {
                    MoveWindow(hStatus, 10, height - 25, width - 20, 20, TRUE);
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
                        ShowNativeStatus(" Transaction deleted.");
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
            } else if (LOWORD(wParam) == 12) {
                NativeQuickSave(hwnd);
            } else if (LOWORD(wParam) == 13) {
                NativeQuickLoad(hwnd);
            } else if (LOWORD(wParam) == 14) {
                ShowHelp(hwnd);
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
                
                int cx = 122;
                int cy = 415;
                int r = 60;
                
                SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(248, 250, 252));
                SetBkMode(hdc, TRANSPARENT);
                TextOut(hdc, cx - 65, cy - r - 25, "Expenses by Category", 20);
                
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
    UpdateWindow(hMainWnd);
    
    // First-run tutorial flag check: never interrupt restored sessions
    HANDLE hTutCheck = CreateFileA("kbudget_tutorial.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hSaveCheck = CreateFileA("kbudget_quicksave.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    BOOL hasSavedState = (hSaveCheck != INVALID_HANDLE_VALUE);
    if (hSaveCheck != INVALID_HANDLE_VALUE) CloseHandle(hSaveCheck);

    if (hTutCheck == INVALID_HANDLE_VALUE && !hasSavedState && num_transactions == 0) {
        MessageBoxA(hMainWnd,
            "=== Welcome to KBudget ===\n\n"
            "KBudget is your personal ledger, expense tracker,\n"
            "and real-time financial visualizer.\n\n"
            "QUICK START GUIDE:\n"
            "- Click '+ New' or press Ctrl+N to record transactions\n"
            "- Press F5 to Quicksave your ledger snapshot at any time\n"
            "- Press F9 to Quickload your saved snapshot\n"
            "- Press F1 or H anytime for the full shortcuts guide\n"
            "- Use the Search box and Sort dropdown to filter records\n"
            "- Import / Export spreadsheets via CSV buttons\n\n"
            "Click OK to begin tracking your finances.",
            "KBudget - First Run Guide", MB_OK | MB_ICONINFORMATION);
        HANDLE hNewTut = CreateFileA("kbudget_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hNewTut != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hNewTut, "1", 1, &written, NULL);
            CloseHandle(hNewTut);
        }
    } else {
        if (hTutCheck != INVALID_HANDLE_VALUE) CloseHandle(hTutCheck);
    }
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        BOOL bHandled = FALSE;
        if (msg.message == WM_KEYDOWN) {
            char className[128] = {0};
            GetClassNameA(msg.hwnd, className, sizeof(className));
            BOOL isEdit = (_stricmp(className, "EDIT") == 0);
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            if (msg.wParam == VK_F5) {
                NativeQuickSave(hMainWnd);
                bHandled = TRUE;
            } else if (msg.wParam == VK_F9) {
                NativeQuickLoad(hMainWnd);
                bHandled = TRUE;
            } else if (msg.wParam == VK_F1 || ((msg.wParam == 'H' || msg.wParam == 'h') && !isEdit)) {
                ShowHelp(hMainWnd);
                bHandled = TRUE;
            } else if (ctrl) {
                if (msg.wParam == 'N' || msg.wParam == 'n') {
                    SendMessage(hMainWnd, WM_COMMAND, 1, 0);
                    bHandled = TRUE;
                } else if (msg.wParam == 'F' || msg.wParam == 'f') {
                    SetFocus(hSearchEdit);
                    bHandled = TRUE;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    SendMessage(hMainWnd, WM_COMMAND, 3, 0);
                    bHandled = TRUE;
                } else if (msg.wParam == 'O' || msg.wParam == 'o') {
                    SendMessage(hMainWnd, WM_COMMAND, 2, 0);
                    bHandled = TRUE;
                }
            } else if (!isEdit) {
                if (msg.wParam == VK_DELETE) {
                    SendMessage(hMainWnd, WM_COMMAND, 7, 0);
                    bHandled = TRUE;
                } else if (msg.wParam == VK_LEFT) {
                    SendMessage(hMainWnd, WM_COMMAND, 9, 0);
                    bHandled = TRUE;
                } else if (msg.wParam == VK_RIGHT) {
                    SendMessage(hMainWnd, WM_COMMAND, 10, 0);
                    bHandled = TRUE;
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
