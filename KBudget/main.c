#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
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
HWND hLblTotal, hLblIncome, hLblExpense;

HBRUSH hbgBrush;
HFONT hFont;

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

void UpdateUI() {
    double total_income = 0;
    double total_expense = 0;
    
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
    for (int i = 0; i < num_transactions; i++) {
        if (transactions[i].is_income) {
            total_income += transactions[i].amount;
        } else {
            total_expense += transactions[i].amount;
        }
        
        char buf[256];
        snprintf(buf, sizeof(buf), "%s | %s | %s%.2f | %s", 
            transactions[i].date, transactions[i].description, 
            transactions[i].is_income ? "+" : "-", transactions[i].amount, 
            transactions[i].category);
        
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
    }
    
    char sumBuf[128];
    snprintf(sumBuf, sizeof(sumBuf), "Total Balance: $%.2f", total_income - total_expense);
    SetWindowText(hLblTotal, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Income: +$%.2f", total_income);
    SetWindowText(hLblIncome, sumBuf);
    
    snprintf(sumBuf, sizeof(sumBuf), "Expenses: -$%.2f", total_expense);
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
                
            hList = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
                250, 20, 500, 300, hwnd, NULL, NULL, NULL);
            
            SendMessage(hLblTotal, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblIncome, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblExpense, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LoadData();
            UpdateUI();
            return 0;
            
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                if (hList) {
                    MoveWindow(hList, 250, 20, width - 270, height - 40, TRUE);
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
            if (LOWORD(wParam) == 1) {
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADD_TRANSACTION), hwnd, AddDialogProc);
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
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
