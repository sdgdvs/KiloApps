#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <commdlg.h>
#include <string.h>
#include <ctype.h>
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

HWND hMainWnd;
HWND hList;
HWND hBtnAdd;
HWND hBtnSettings;
HWND hSearchEdit;
HWND hLblTotal, hLblIncome, hLblExpense;

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

void UpdateUI() {
    double total_income = 0;
    double total_expense = 0;
    
    char searchTerm[128] = {0};
    if (hSearchEdit) {
        GetWindowText(hSearchEdit, searchTerm, sizeof(searchTerm));
    }
    for(int i = 0; searchTerm[i]; i++) searchTerm[i] = tolower((unsigned char)searchTerm[i]);
    
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
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
        
        char buf[256];
        snprintf(buf, sizeof(buf), "%s | %s | %s%s%.2f | %s", 
            transactions[i].date, transactions[i].description, 
            transactions[i].is_income ? "+" : "-", currency_symbol, transactions[i].amount, 
            transactions[i].category);
        
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
    }
    
    char sumBuf[128];
    snprintf(sumBuf, sizeof(sumBuf), "Total Balance: %s%.2f", currency_symbol, total_income - total_expense);
    SetWindowText(hLblTotal, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Income: +%s%.2f", currency_symbol, total_income);
    SetWindowText(hLblIncome, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Expenses: -%s%.2f", currency_symbol, total_expense);
    SetWindowText(hLblExpense, sumBuf);
}

INT_PTR CALLBACK AddDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwndDlg, IDC_TYPE_EDIT, "0");
            SetDlgItemText(hwndDlg, IDC_AMOUNT_EDIT, "0.00");
            SetDlgItemText(hwndDlg, IDC_CAT_EDIT, "Food");
            SetDlgItemText(hwndDlg, IDC_DESC_EDIT, "Lunch");
            SetDlgItemText(hwndDlg, IDC_DATE_EDIT, "2026-07-12");
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                if (num_transactions < MAX_TRANSACTIONS) {
                    char typeStr[32], amtStr[32];
                    GetDlgItemText(hwndDlg, IDC_TYPE_EDIT, typeStr, sizeof(typeStr));
                    GetDlgItemText(hwndDlg, IDC_AMOUNT_EDIT, amtStr, sizeof(amtStr));
                    
                    Transaction *t = &transactions[num_transactions];
                    t->is_income = atoi(typeStr);
                    t->amount = atof(amtStr);
                    GetDlgItemText(hwndDlg, IDC_CAT_EDIT, t->category, sizeof(t->category));
                    GetDlgItemText(hwndDlg, IDC_DESC_EDIT, t->description, sizeof(t->description));
                    GetDlgItemText(hwndDlg, IDC_DATE_EDIT, t->date, sizeof(t->date));
                    
                    num_transactions++;
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
            HWND hBtnImp = CreateWindow("BUTTON", "Import CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 160, 150, 30, hwnd, (HMENU)2, NULL, NULL);
            HWND hBtnExp = CreateWindow("BUTTON", "Export CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 200, 150, 30, hwnd, (HMENU)3, NULL, NULL);
            hBtnSettings = CreateWindow("BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 240, 150, 30, hwnd, (HMENU)5, NULL, NULL);
                
            hSearchEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                250, 20, 500, 25, hwnd, (HMENU)4, NULL, NULL);
            hList = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
                250, 50, 500, 270, hwnd, NULL, NULL, NULL);
            
            SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblTotal, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblIncome, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblExpense, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnImp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSettings, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LoadData();
            LoadSettings();
            UpdateUI();
            return 0;
            
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                if (hSearchEdit) {
                    MoveWindow(hSearchEdit, 250, 20, width - 270, 25, TRUE);
                }
                if (hList) {
                    MoveWindow(hList, 250, 50, width - 270, height - 70, TRUE);
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
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 4) {
                UpdateUI();
            } else if (LOWORD(wParam) == 1) {
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_TRANSACTION), hwnd, AddDialogProc);
            } else if (LOWORD(wParam) == 2) {
                ImportCSV(hwnd);
            } else if (LOWORD(wParam) == 3) {
                ExportCSV(hwnd);
            } else if (LOWORD(wParam) == 5) {
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hwnd, SettingsDialogProc);
            }
            break;
            
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
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 400,
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
