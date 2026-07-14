import sys
import re

with open('c:/KiloApps/KiloApps/KBBS/main.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Definitions to add at top
defs = """#define IDD_DIALDIR 1000
#define IDC_LIST    1001
#define IDC_NAME    1002
#define IDC_HOST    1003
#define IDC_PORT    1004
#define IDC_TYPE    1005
#define IDC_SAVE    1006
#define IDC_NEW     1007
#define IDC_DELETE  1008

#define MAX_BBS 100
struct BBS_ENTRY {
    char name[64];
    char host[128];
    int port;
    char type[32];
};
struct BBS_ENTRY dynBbsList[MAX_BBS];
int numBbs = 0;
int selectedDirIdx = -1;

void LoadBBSList(void) {
    HANDLE hFile = CreateFileA("kbbs_dir.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    numBbs = 0;
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        char buf[8192];
        if (ReadFile(hFile, buf, sizeof(buf)-1, &readBytes, NULL)) {
            buf[readBytes] = 0;
            char* line = buf;
            while (*line && numBbs < MAX_BBS) {
                char* next = line;
                while (*next && *next != '\\n' && *next != '\\r') next++;
                char term = *next;
                *next = 0;
                if (line[0]) {
                    char* p1 = line;
                    char* p2 = p1; while(*p2 && *p2 != '|') p2++; if (*p2) { *p2++ = 0; }
                    char* p3 = p2; while(*p3 && *p3 != '|') p3++; if (*p3) { *p3++ = 0; }
                    char* p4 = p3; while(*p4 && *p4 != '|') p4++; if (*p4) { *p4++ = 0; }
                    
                    if (*p1 && *p2 && *p3) {
                        my_strcpy(dynBbsList[numBbs].name, p1);
                        my_strcpy(dynBbsList[numBbs].host, p2);
                        dynBbsList[numBbs].port = my_atoi(p3);
                        my_strcpy(dynBbsList[numBbs].type, *p4 ? p4 : "Telnet");
                        numBbs++;
                    }
                }
                line = next;
                if (term == '\\r' && *(line+1) == '\\n') line += 2;
                else if (term != 0) line++;
            }
        }
        CloseHandle(hFile);
    } 
    if (numBbs == 0) {
        int i;
        for(i = 0; i < (int)BBS_COUNT && i < MAX_BBS; i++) {
            my_strcpy(dynBbsList[numBbs].name, bbsList[i].name);
            my_strcpy(dynBbsList[numBbs].host, bbsList[i].host);
            dynBbsList[numBbs].port = bbsList[i].port;
            my_strcpy(dynBbsList[numBbs].type, "Telnet");
            numBbs++;
        }
    }
}

void SaveBBSList(void) {
    HANDLE hFile = CreateFileA("kbbs_dir.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        int i;
        char buf[512];
        DWORD written;
        for (i = 0; i < numBbs; i++) {
            char portStr[16];
            my_itoa(dynBbsList[i].port, portStr);
            my_strcpy(buf, dynBbsList[i].name);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), dynBbsList[i].host);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), portStr);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), dynBbsList[i].type);
            my_strcpy(buf + my_strlen(buf), "\\r\\n");
            WriteFile(hFile, buf, (DWORD)my_strlen(buf), &written, NULL);
        }
        CloseHandle(hFile);
    }
}

INT_PTR CALLBACK DialDirProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            HWND hList = GetDlgItem(hdlg, IDC_LIST);
            HWND hType = GetDlgItem(hdlg, IDC_TYPE);
            int i;
            SendMessageA(hType, CB_ADDSTRING, 0, (LPARAM)"Telnet");
            SendMessageA(hType, CB_ADDSTRING, 0, (LPARAM)"WebSocket");
            
            LoadBBSList();
            for (i = 0; i < numBbs; i++) {
                SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[i].name);
            }
            selectedDirIdx = 0;
            if (numBbs > 0) {
                SendMessageA(hList, LB_SETCURSEL, selectedDirIdx, 0);
                SetDlgItemTextA(hdlg, IDC_NAME, dynBbsList[0].name);
                SetDlgItemTextA(hdlg, IDC_HOST, dynBbsList[0].host);
                char pBuf[16]; my_itoa(dynBbsList[0].port, pBuf);
                SetDlgItemTextA(hdlg, IDC_PORT, pBuf);
                SendMessageA(hType, CB_SETCURSEL, dynBbsList[0].type[0]=='W' ? 1 : 0, 0);
            }
            return TRUE;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == IDC_LIST && code == LBN_SELCHANGE) {
                int sel = (int)SendMessageA(GetDlgItem(hdlg, IDC_LIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < numBbs) {
                    selectedDirIdx = sel;
                    SetDlgItemTextA(hdlg, IDC_NAME, dynBbsList[sel].name);
                    SetDlgItemTextA(hdlg, IDC_HOST, dynBbsList[sel].host);
                    char pBuf[16]; my_itoa(dynBbsList[sel].port, pBuf);
                    SetDlgItemTextA(hdlg, IDC_PORT, pBuf);
                    SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_SETCURSEL, dynBbsList[sel].type[0]=='W' ? 1 : 0, 0);
                }
            } else if (id == IDC_NEW) {
                selectedDirIdx = -1;
                SendMessageA(GetDlgItem(hdlg, IDC_LIST), LB_SETCURSEL, (WPARAM)-1, 0);
                SetDlgItemTextA(hdlg, IDC_NAME, "New BBS");
                SetDlgItemTextA(hdlg, IDC_HOST, "");
                SetDlgItemTextA(hdlg, IDC_PORT, "23");
                SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_SETCURSEL, 0, 0);
            } else if (id == IDC_SAVE) {
                char name[64], host[128], port[16];
                GetDlgItemTextA(hdlg, IDC_NAME, name, 64);
                GetDlgItemTextA(hdlg, IDC_HOST, host, 128);
                GetDlgItemTextA(hdlg, IDC_PORT, port, 16);
                int typeSel = (int)SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_GETCURSEL, 0, 0);
                if (name[0] && host[0]) {
                    int target = selectedDirIdx;
                    if (target < 0) {
                        target = numBbs;
                        if (numBbs < MAX_BBS) numBbs++;
                        else target = MAX_BBS - 1;
                    }
                    my_strcpy(dynBbsList[target].name, name);
                    my_strcpy(dynBbsList[target].host, host);
                    dynBbsList[target].port = my_atoi(port);
                    my_strcpy(dynBbsList[target].type, typeSel == 1 ? "WebSocket" : "Telnet");
                    
                    HWND hList = GetDlgItem(hdlg, IDC_LIST);
                    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
                    int i;
                    for (i = 0; i < numBbs; i++) SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[i].name);
                    SendMessageA(hList, LB_SETCURSEL, target, 0);
                    selectedDirIdx = target;
                    SaveBBSList();
                }
            } else if (id == IDC_DELETE) {
                if (selectedDirIdx >= 0 && selectedDirIdx < numBbs) {
                    int i;
                    for (i = selectedDirIdx; i < numBbs - 1; i++) {
                        dynBbsList[i] = dynBbsList[i+1];
                    }
                    numBbs--;
                    selectedDirIdx = -1;
                    HWND hList = GetDlgItem(hdlg, IDC_LIST);
                    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
                    for (i = 0; i < numBbs; i++) SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[i].name);
                    SaveBBSList();
                }
            } else if (id == 1) { // Connect
                if (selectedDirIdx >= 0 && selectedDirIdx < numBbs) {
                    EndDialog(hdlg, selectedDirIdx);
                } else {
                    EndDialog(hdlg, -1);
                }
            } else if (id == 2) { // Cancel
                EndDialog(hdlg, -1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

"""

# Insert right before ClearScreen
code = code.replace("void ClearScreen(void) {", defs + "void ClearScreen(void) {")

# Change hCombo from COMBOBOX to BUTTON "Directory"
# original:
# hCombo = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
#     dpiScale(428), dpiScale(4), dpiScale(150), dpiScale(400), hwnd, (HMENU)101, 0, 0);
btn_str = """hCombo = CreateWindowA("BUTTON", "Directory", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(428), dpiScale(4), dpiScale(100), dpiScale(22), hwnd, (HMENU)101, 0, 0);"""

code = re.sub(
    r'hCombo\s*=\s*CreateWindowA\("COMBOBOX"[^;]+;',
    btn_str,
    code,
    count=1
)

# Remove populate BBS list in WM_CREATE
code = re.sub(
    r'/\*\s*Populate BBS list\s*\*/[\s\S]*?\}',
    '',
    code
)

# Add logic for Dir button
dir_btn_logic = """
            else if (LOWORD(wParam) == 101) {
                /* Directory button */
                int res = DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDD_DIALDIR), hwnd, DialDirProc, 0);
                if (res >= 0 && res < numBbs) {
                    char portBuf[16];
                    SetWindowTextA(hHost, dynBbsList[res].host);
                    my_itoa(dynBbsList[res].port, portBuf);
                    SetWindowTextA(hPort, portBuf);
                    /* Optional: Auto-connect by simulating Connect click */
                    SendMessage(hwnd, WM_COMMAND, 100, 0);
                }
            }
"""

# Replace dropdown logic
code = re.sub(
    r'else if \(LOWORD\(wParam\) == 101 && HIWORD\(wParam\) == CBN_SELCHANGE\) \{[\s\S]*?break;\s*\}',
    dir_btn_logic + "            break;\n        }",
    code
)

with open('c:/KiloApps/KiloApps/KBBS/main.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("done")
