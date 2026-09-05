#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#define W 920
#define H 620

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
#define ID_BTN_STAR 113
#define ID_BTN_EXPORT_EML 114
#define ID_BTN_SAVE_DRAFT 115
#define ID_BTN_HELP 116
#define ID_BTN_REPLY 117
#define ID_BTN_EXPORT_MD 118

HWND hFolders, hEmails, hTitle, hBody, hBtnCompose, hBtnDelete, hBtnEmptyTrash, hSearchBox;
HWND hTagFilter, hBtnImport, hBtnExport, hTab, hBtnTag, hBtnDecrypt, hBtnStar, hBtnExportEml, hBtnExportMd, hBtnReply, hBtnSaveDraft, hBtnHelp, hHelpLabel;

WNDPROC oldSearchEditProc = NULL;

typedef struct {
    int id;
    int folder; // 0=inbox, 1=starred (view), 2=sent, 3=drafts, 4=trash, 99=deleted
    char subject[128];
    char sender[128];
    char body[2048];
    int unread;
    int encrypted;
    int starred;
    char tags[128]; // comma separated
} Email;

Email emails[200] = {
    {1, 0, "Welcome to KiloOS", "sysadmin@kilo.os", "Hello User,\r\n\r\nWelcome to KiloOS email suite.\r\nUse folders, star priority emails, save drafts, and export messages.\r\n\r\n- SysAdmin", 1, 0, 1, "Work"},
    {2, 0, "Meeting at 3PM", "boss@kilo.os", "Don't forget our meeting at 3PM in Conference Room B.", 0, 0, 1, "Urgent,Work"},
    {3, 0, "Top Secret Info", "agent@kilo.os", "\x11\x14\x05\x5e\x06\x16\x16\x07\x16\x11\x5e\x1c\x19\x1f\x18", 1, 1, 0, "Personal"}, // Mock XOR encrypted with pass "pass"
    {4, 0, "Newsletter #42", "news@kilo.os", "Weekly digest on minimalist desktop OS development...", 0, 0, 0, "News"},
    {5, 2, "Re: Meeting at 3PM", "me@kilo.os", "I'll be there on time with the slides ready.", 0, 0, 0, ""},
    {6, 3, "Draft: Q4 Roadmap", "team@kilo.os", "Q4 Objectives:\r\n1. KMail feature upgrade\r\n2. Performance optimization", 0, 0, 0, "Work"}
};
int num_emails = 6;
int nextId = 7;

typedef struct {
    int id; // 0 for compose, 1 for read
    int emailId; // valid if id == 1, or draft email ID if id == 0
    char composeTo[128];
    char composeSub[128];
    char composeBody[2048];
    int composeEncrypted;
} TabData;

TabData tabs[20];
int num_tabs = 0;
int currentTabIdx = -1;

int currentFolder = 0; // 0=inbox, 1=starred, 2=sent, 3=drafts, 4=trash
char searchQuery[128] = "";
char tagQuery[64] = "";

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
HFONT hFont, hBold, hSmallFont;
COLORREF textCol = RGB(248, 250, 252);
COLORREF bgMainCol = RGB(15, 23, 42);
COLORREF bgListCol = RGB(30, 41, 59);

void RenderPane();
void ParseComposeFields(const char* text, char* outTo, char* outSub, char* outBody);
void SyncCurrentTabState();
void ReplyToCurrentEmail();
void ExportSingleMarkdown(Email* em);
void ImportJson(HWND hwnd);

void RefreshEmailList() {
    SendMessage(hEmails, LB_RESETCONTENT, 0, 0);
    for(int i = 0; i < num_emails; i++) {
        int matchesFolder = 0;
        if (currentFolder == 0 && emails[i].folder == 0) matchesFolder = 1; // Inbox
        else if (currentFolder == 1 && emails[i].starred && emails[i].folder != 4 && emails[i].folder != 99) matchesFolder = 1; // Starred
        else if (currentFolder == 2 && emails[i].folder == 2) matchesFolder = 1; // Sent
        else if (currentFolder == 3 && emails[i].folder == 3) matchesFolder = 1; // Drafts
        else if (currentFolder == 4 && emails[i].folder == 4) matchesFolder = 1; // Trash

        if(matchesFolder) {
            if (searchQuery[0] == '\0' || 
                contains_nocase(emails[i].subject, searchQuery) || 
                contains_nocase(emails[i].sender, searchQuery)) {
                
                if (tagQuery[0] == '\0' || contains_nocase(emails[i].tags, tagQuery)) {
                    char displayStr[256];
                    wsprintfA(displayStr, "%s%s%s", 
                        emails[i].starred ? "[*] " : "",
                        emails[i].encrypted ? "[ENC] " : "", 
                        emails[i].subject);
                    int idx = SendMessageA(hEmails, LB_ADDSTRING, 0, (LPARAM)displayStr);
                    SendMessage(hEmails, LB_SETITEMDATA, idx, emails[i].id);
                }
            }
        }
    }
}

void SyncCurrentTabState() {
    if (currentTabIdx >= 0 && currentTabIdx < num_tabs) {
        if (tabs[currentTabIdx].id == 0) {
            char rawText[2048] = {0};
            GetWindowTextA(hBody, rawText, sizeof(rawText));
            ParseComposeFields(rawText, tabs[currentTabIdx].composeTo, tabs[currentTabIdx].composeSub, tabs[currentTabIdx].composeBody);
            if (tabs[currentTabIdx].composeSub[0]) {
                TCITEM tie;
                tie.mask = TCIF_TEXT;
                tie.pszText = tabs[currentTabIdx].composeSub;
                SendMessage(hTab, TCM_SETITEM, currentTabIdx, (LPARAM)&tie);
            }
        }
    }
}

void SelectTab(int tIdx) {
    if (tIdx != currentTabIdx) {
        SyncCurrentTabState();
    }
    currentTabIdx = tIdx;
    SendMessage(hTab, TCM_SETCURSEL, tIdx, 0);
    RenderPane();
}

void OpenEmailTab(int eid) {
    Email* em = NULL;
    for(int i=0; i<num_emails; i++) if(emails[i].id == eid) em = &emails[i];
    if(!em) return;

    SyncCurrentTabState();

    // If opening a draft, open in compose mode
    if(em->folder == 3) {
        for(int i=0; i<num_tabs; i++) {
            if(tabs[i].id == 0 && tabs[i].emailId == eid) {
                SelectTab(i);
                return;
            }
        }
        if(num_tabs >= 20) return;
        tabs[num_tabs].id = 0;
        tabs[num_tabs].emailId = eid;
        lstrcpynA(tabs[num_tabs].composeTo, em->sender, sizeof(tabs[num_tabs].composeTo));
        lstrcpynA(tabs[num_tabs].composeSub, em->subject, sizeof(tabs[num_tabs].composeSub));
        lstrcpynA(tabs[num_tabs].composeBody, em->body, sizeof(tabs[num_tabs].composeBody));
        tabs[num_tabs].composeEncrypted = em->encrypted;

        TCITEM tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = em->subject;
        SendMessage(hTab, TCM_INSERTITEM, num_tabs, (LPARAM)&tie);

        num_tabs++;
        SelectTab(num_tabs - 1);
        return;
    }

    for(int i=0; i<num_tabs; i++) {
        if(tabs[i].id == 1 && tabs[i].emailId == eid) {
            SelectTab(i);
            return;
        }
    }
    if(num_tabs >= 20) return;
    
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
    SyncCurrentTabState();
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
    SetFocus(hBody);
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
    SendMessage(hTab, TCM_SETCURSEL, currentTabIdx, 0);
    RenderPane();
}

void SelectFolder(int fIdx) {
    if (fIdx >= 0 && fIdx <= 4) {
        currentFolder = fIdx;
        SendMessage(hFolders, LB_SETCURSEL, fIdx, 0);
        RefreshEmailList();
    }
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "=== KMail Help & Keyboard Shortcuts ===\n\n"
        "KEYBOARD SHORTCUTS:\n"
        " [1] - [5]     : Switch Folders (1:Inbox, 2:Starred, 3:Sent, 4:Drafts, 5:Trash)\n"
        " [C]           : Compose New Message\n"
        " [R]           : Reply to Active Email (in Read mode)\n"
        " [S]           : Star / Unstar Selected Email\n"
        " [T]           : Add Tag to Selected Email\n"
        " [E]           : Export Single Email (.EML)\n"
        " [M]           : Export Single Email Markdown (.MD)\n"
        " [D]           : Decrypt Encrypted Message\n"
        " [I]           : Import Mailbox (.JSON)\n"
        " [O]           : Export Mailbox (.JSON)\n"
        " [Del]         : Delete Email / Move to Trash\n"
        " [Ctrl+S]      : Save Draft (in Compose)\n"
        " [Ctrl+Enter]  : Send Message (in Compose)\n"
        " [Ctrl+F]      : Focus Search Bar\n"
        " [Ctrl+W]      : Close Current Tab\n"
        " [Ctrl+Tab]    : Cycle Open Tabs\n"
        " [Tab]         : Navigate Controls (WS_TABSTOP)\n"
        " [Esc]         : Close Active Tab / Clear Search\n"
        " [F1] or [H]   : Show this Help Dialog\n\n"
        "FEATURES:\n"
        " * Multi-Tab Email Viewing & Composing with Live Sync\n"
        " * Starred Priority Filtering & Folder Organization\n"
        " * Automatic Draft Saving & Resuming\n"
        " * Quick Reply with Quoting\n"
        " * Search by Subject, Sender, Body & Tags\n"
        " * Full EML, Markdown (.MD) & JSON Export/Import\n",
        "KMail - Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

void RenderPane() {
    if(currentTabIdx == -1) {
        SetWindowTextA(hTitle, "No email selected");
        SetWindowTextA(hBody, "Select an email from the list to read, or click 'Compose [C]' to write a new one.\r\n\r\nFeatures:\r\n- Switch between Inbox [1], Starred [2], Sent [3], Drafts [4], and Trash [5] folders.\r\n- Star priority emails [S] to track important discussions.\r\n- Reply [R] to active emails with quoting.\r\n- Save Drafts [Ctrl+S] to resume composing later.\r\n- Search [Ctrl+F] and filter by tags.\r\n- Open multiple emails in tabs [Ctrl+Tab, Ctrl+W].\r\n- Encrypt messages with passwords.\r\n- Export single emails as .EML [E] or Markdown [M].\r\n- Export full mailbox (.JSON) [O] or Import (.JSON) [I].\r\n\r\nPress F1 or 'H' for help.");
        ShowWindow(hBtnReply, SW_HIDE);
        ShowWindow(hBtnStar, SW_HIDE);
        ShowWindow(hBtnTag, SW_HIDE);
        ShowWindow(hBtnExportEml, SW_HIDE);
        ShowWindow(hBtnExportMd, SW_HIDE);
        ShowWindow(hBtnDecrypt, SW_HIDE);
        ShowWindow(hBtnSaveDraft, SW_HIDE);
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
        wsprintfA(tStr, "%s: %s\r\nTags: %s  |  Status: %s", 
            em->folder == 2 ? "To" : "From", 
            em->sender, 
            em->tags[0] ? em->tags : "(none)",
            em->starred ? "★ Starred" : "Normal");
        SetWindowTextA(hTitle, tStr);
        SetWindowLong(hBody, GWL_STYLE, GetWindowLong(hBody, GWL_STYLE) | ES_READONLY);
        
        ShowWindow(hBtnReply, SW_SHOW);
        ShowWindow(hBtnStar, SW_SHOW);
        SetWindowTextA(hBtnStar, em->starred ? "Unstar [S]" : "★ Star [S]");
        ShowWindow(hBtnTag, SW_SHOW);
        ShowWindow(hBtnExportEml, SW_SHOW);
        ShowWindow(hBtnExportMd, SW_SHOW);
        ShowWindow(hBtnSaveDraft, SW_HIDE);

        if(em->encrypted) {
            SetWindowTextA(hBody, "🔒 This message is encrypted. Click Decrypt [D] to view.");
            ShowWindow(hBtnDecrypt, SW_SHOW);
            SetWindowTextA(hBtnDecrypt, "Decrypt [D]");
            EnableWindow(hBtnDecrypt, TRUE);
            MoveWindow(hBtnDecrypt, 740, 122, 90, 26, TRUE);
        } else {
            SetWindowTextA(hBody, em->body);
            ShowWindow(hBtnDecrypt, SW_HIDE);
        }
        
    } else {
        // Compose mode
        SetWindowTextA(hTitle, "Compose Message (Line 1: To: email, Line 2: Sub: subject)");
        SetWindowLong(hBody, GWL_STYLE, GetWindowLong(hBody, GWL_STYLE) & ~ES_READONLY);
        char compStr[2048];
        if (t->emailId > 0 && t->composeBody[0]) {
            wsprintfA(compStr, "To: %s\r\nSub: %s\r\n\r\n%s", t->composeTo, t->composeSub, t->composeBody);
        } else {
            wsprintfA(compStr, "To: %s\r\nSub: %s\r\n\r\n%s", t->composeTo[0] ? t->composeTo : "", t->composeSub[0] ? t->composeSub : "", t->composeBody[0] ? t->composeBody : "(Write body here...)");
        }
        SetWindowTextA(hBody, compStr);
        
        ShowWindow(hBtnReply, SW_HIDE);
        ShowWindow(hBtnStar, SW_HIDE);
        ShowWindow(hBtnTag, SW_HIDE);
        ShowWindow(hBtnExportEml, SW_HIDE);
        ShowWindow(hBtnExportMd, SW_HIDE);
        ShowWindow(hBtnSaveDraft, SW_SHOW);
        MoveWindow(hBtnSaveDraft, 340, 122, 135, 26, TRUE);

        ShowWindow(hBtnDecrypt, SW_SHOW); // Reuse as 'Send'
        SetWindowTextA(hBtnDecrypt, "Send [Ctrl+↵]");
        MoveWindow(hBtnDecrypt, 485, 122, 135, 26, TRUE);
        EnableWindow(hBtnDecrypt, TRUE);
    }
}

void CryptStr(char* data, const char* pass) {
    int pLen = lstrlenA(pass);
    if(pLen == 0) return;
    for(int i = 0; data[i]; i++) {
        data[i] ^= pass[i % pLen];
    }
}

void ExportSingleEmail(Email* em) {
    char filename[64];
    wsprintfA(filename, "email_%d.eml", em->id);
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return;
    DWORD written;
    char header[512];
    wsprintfA(header, "From: %s\r\nTo: me@kilo.os\r\nSubject: %s\r\nX-Tags: %s\r\nX-Starred: %s\r\nMIME-Version: 1.0\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n",
        em->sender, em->subject, em->tags, em->starred ? "yes" : "no");
    WriteFile(hFile, header, lstrlenA(header), &written, NULL);
    WriteFile(hFile, em->body, lstrlenA(em->body), &written, NULL);
    CloseHandle(hFile);
    char msg[128];
    wsprintfA(msg, "Exported email to %s", filename);
    MessageBoxA(NULL, msg, "KMail Export", MB_OK);
}

void ExportSingleMarkdown(Email* em) {
    char filename[64];
    wsprintfA(filename, "email_%d.md", em->id);
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return;
    DWORD written;
    char header[512];
    wsprintfA(header, "# %s\r\n\r\n- **From:** %s\r\n- **Folder:** %s\r\n- **Tags:** %s\r\n- **Starred:** %s\r\n\r\n---\r\n\r\n",
        em->subject, em->sender,
        em->folder == 0 ? "Inbox" : (em->folder == 2 ? "Sent" : (em->folder == 3 ? "Drafts" : "Trash")),
        em->tags[0] ? em->tags : "none",
        em->starred ? "Yes" : "No");
    WriteFile(hFile, header, lstrlenA(header), &written, NULL);
    WriteFile(hFile, em->body, lstrlenA(em->body), &written, NULL);
    CloseHandle(hFile);
    char msg[128];
    wsprintfA(msg, "Exported markdown to %s", filename);
    MessageBoxA(NULL, msg, "KMail Export", MB_OK);
}

void EscapeJsonString(const char* src, char* dst, int maxDst) {
    int j = 0;
    for (int i = 0; src[i] && j < maxDst - 4; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"') { dst[j++] = '\\'; dst[j++] = '"'; }
        else if (c == '\\') { dst[j++] = '\\'; dst[j++] = '\\'; }
        else if (c == '\r') { dst[j++] = '\\'; dst[j++] = 'r'; }
        else if (c == '\n') { dst[j++] = '\\'; dst[j++] = 'n'; }
        else if (c == '\t') { dst[j++] = '\\'; dst[j++] = 't'; }
        else if (c >= 32) { dst[j++] = c; }
    }
    dst[j] = '\0';
}

// JSON Mailbox Export
void ExportJson() {
    HANDLE hFile = CreateFileA("mailbox.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(hFile, "[\r\n", 3, &written, NULL);

    int activeCount = 0;
    for(int i = 0; i < num_emails; i++) {
        if(emails[i].folder != 99) activeCount++;
    }
    int cur = 0;
    for(int i = 0; i < num_emails; i++) {
        if(emails[i].folder == 99) continue;
        cur++;
        char escSub[256], escSender[256], escTags[256], escBody[3000];
        EscapeJsonString(emails[i].subject, escSub, sizeof(escSub));
        EscapeJsonString(emails[i].sender, escSender, sizeof(escSender));
        EscapeJsonString(emails[i].tags, escTags, sizeof(escTags));
        EscapeJsonString(emails[i].body, escBody, sizeof(escBody));

        char buf[4096];
        wsprintfA(buf, "  {\"id\":%d,\"folder\":%d,\"subject\":\"%s\",\"sender\":\"%s\",\"body\":\"%s\",\"unread\":%d,\"encrypted\":%d,\"starred\":%d,\"tags\":\"%s\"}%s\r\n",
            emails[i].id, emails[i].folder, escSub, escSender, escBody, emails[i].unread, emails[i].encrypted, emails[i].starred, escTags,
            (cur == activeCount) ? "" : ",");
        WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    }
    WriteFile(hFile, "]\r\n", 3, &written, NULL);
    CloseHandle(hFile);
    MessageBoxA(NULL, "Mailbox exported to mailbox.json", "Export", MB_OK);
}

int my_strnicmp(const char* s1, const char* s2, int n) {
    for (int i = 0; i < n; i++) {
        char c1 = my_tolower((unsigned char)s1[i]);
        char c2 = my_tolower((unsigned char)s2[i]);
        if (c1 != c2 || s1[i] == 0 || s2[i] == 0) return (unsigned char)c1 - (unsigned char)c2;
    }
    return 0;
}

void ParseComposeFields(const char* text, char* outTo, char* outSub, char* outBody) {
    outTo[0] = 0;
    outSub[0] = 0;
    outBody[0] = 0;
    const char* p = text;
    // Check line 1: To:
    if (my_strnicmp(p, "To:", 3) == 0) {
        p += 3;
        while (*p == ' ') p++;
        int i = 0;
        while (*p && *p != '\r' && *p != '\n' && i < 127) outTo[i++] = *p++;
        outTo[i] = 0;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }
    // Check line 2: Sub:
    if (my_strnicmp(p, "Sub:", 4) == 0 || my_strnicmp(p, "Subject:", 8) == 0) {
        p += (my_strnicmp(p, "Sub:", 4) == 0) ? 4 : 8;
        while (*p == ' ') p++;
        int i = 0;
        while (*p && *p != '\r' && *p != '\n' && i < 127) outSub[i++] = *p++;
        outSub[i] = 0;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }
    if (*p == '\r') p++;
    if (*p == '\n') p++;
    lstrcpynA(outBody, p, 2047);
}

void ReplyToCurrentEmail() {
    if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
        Email* em = NULL;
        for(int i=0; i<num_emails; i++) if(emails[i].id == tabs[currentTabIdx].emailId) em = &emails[i];
        if(em && num_tabs < 20) {
            SyncCurrentTabState();
            tabs[num_tabs].id = 0;
            tabs[num_tabs].emailId = 0;
            lstrcpynA(tabs[num_tabs].composeTo, em->sender, sizeof(tabs[num_tabs].composeTo));
            if (my_strnicmp(em->subject, "Re:", 3) == 0) {
                lstrcpynA(tabs[num_tabs].composeSub, em->subject, sizeof(tabs[num_tabs].composeSub));
            } else {
                wsprintfA(tabs[num_tabs].composeSub, "Re: %s", em->subject);
            }
            wsprintfA(tabs[num_tabs].composeBody, "\r\n\r\n--- Original Message ---\r\n%s", em->body);
            tabs[num_tabs].composeEncrypted = 0;

            TCITEM tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = tabs[num_tabs].composeSub;
            SendMessage(hTab, TCM_INSERTITEM, num_tabs, (LPARAM)&tie);

            num_tabs++;
            SelectTab(num_tabs - 1);
            SetFocus(hBody);
        }
    }
}

void ImportJson(HWND hwnd) {
    char szFileName[MAX_PATH] = "mailbox.json";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetOpenFileNameA(&ofn)) return;

    HANDLE hFile = CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to open JSON file.", "Import Error", MB_OK | MB_ICONERROR);
        return;
    }
    DWORD dwSize = GetFileSize(hFile, NULL);
    if (dwSize == INVALID_FILE_SIZE || dwSize > 1024 * 500) {
        CloseHandle(hFile);
        MessageBoxA(hwnd, "File too large (max 500KB).", "Import Error", MB_OK | MB_ICONWARNING);
        return;
    }
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + 1);
    if (!buf) {
        CloseHandle(hFile);
        return;
    }
    DWORD dwRead = 0;
    ReadFile(hFile, buf, dwSize, &dwRead, NULL);
    buf[dwRead] = '\0';
    CloseHandle(hFile);

    int count = 0;
    const char* p = buf;
    while (*p && count < 200) {
        const char* objStart = strchr(p, '{');
        if (!objStart) break;
        const char* objEnd = strchr(objStart, '}');
        if (!objEnd) break;

        Email em = {0};
        em.folder = 0;

        const char* fId = strstr(objStart, "\"id\":");
        if (fId && fId < objEnd) {
            em.id = 0;
            fId += 5;
            while (*fId == ' ') fId++;
            while (*fId >= '0' && *fId <= '9') { em.id = em.id * 10 + (*fId - '0'); fId++; }
        } else {
            em.id = count + 1;
        }

        const char* fF = strstr(objStart, "\"folder\":");
        if (fF && fF < objEnd) {
            fF += 9;
            while (*fF == ' ' || *fF == '"') fF++;
            if (*fF == '2' || my_strnicmp(fF, "sent", 4) == 0) em.folder = 2;
            else if (*fF == '3' || my_strnicmp(fF, "draft", 5) == 0) em.folder = 3;
            else if (*fF == '4' || my_strnicmp(fF, "trash", 5) == 0) em.folder = 4;
            else em.folder = 0;
        }

        const char* fSub = strstr(objStart, "\"subject\":\"");
        if (fSub && fSub < objEnd) {
            fSub += 11;
            int si = 0;
            while (fSub < objEnd && *fSub != '"' && si < 127) {
                if (*fSub == '\\' && *(fSub+1) == '"') { em.subject[si++] = '"'; fSub += 2; }
                else if (*fSub == '\\' && *(fSub+1) == '\\') { em.subject[si++] = '\\'; fSub += 2; }
                else { em.subject[si++] = *fSub++; }
            }
            em.subject[si] = '\0';
        }

        const char* fSnd = strstr(objStart, "\"sender\":\"");
        if (fSnd && fSnd < objEnd) {
            fSnd += 10;
            int si = 0;
            while (fSnd < objEnd && *fSnd != '"' && si < 127) {
                if (*fSnd == '\\' && *(fSnd+1) == '"') { em.sender[si++] = '"'; fSnd += 2; }
                else { em.sender[si++] = *fSnd++; }
            }
            em.sender[si] = '\0';
        }

        const char* fBody = strstr(objStart, "\"body\":\"");
        if (fBody && fBody < objEnd) {
            fBody += 8;
            int bi = 0;
            while (fBody < objEnd && *fBody != '"' && bi < 2046) {
                if (*fBody == '\\' && *(fBody+1) == 'n') { em.body[bi++] = '\r'; em.body[bi++] = '\n'; fBody += 2; }
                else if (*fBody == '\\' && *(fBody+1) == 'r') { fBody += 2; }
                else if (*fBody == '\\' && *(fBody+1) == '"') { em.body[bi++] = '"'; fBody += 2; }
                else { em.body[bi++] = *fBody++; }
            }
            em.body[bi] = '\0';
        }

        const char* fTag = strstr(objStart, "\"tags\":");
        if (fTag && fTag < objEnd) {
            fTag += 7;
            while (*fTag == ' ') fTag++;
            if (*fTag == '"') {
                fTag++;
                int ti = 0;
                while (fTag < objEnd && *fTag != '"' && ti < 127) em.tags[ti++] = *fTag++;
                em.tags[ti] = '\0';
            }
        }

        if (strstr(objStart, "\"unread\":true") || strstr(objStart, "\"unread\":1")) em.unread = 1;
        if (strstr(objStart, "\"starred\":true") || strstr(objStart, "\"starred\":1")) em.starred = 1;
        if (strstr(objStart, "\"encrypted\":true") || strstr(objStart, "\"encrypted\":1")) em.encrypted = 1;

        emails[count++] = em;
        p = objEnd + 1;
    }
    HeapFree(GetProcessHeap(), 0, buf);

    if (count > 0) {
        num_emails = count;
        nextId = 1;
        for (int i = 0; i < num_emails; i++) if (emails[i].id >= nextId) nextId = emails[i].id + 1;
        RefreshEmailList();
        char msg[64];
        wsprintfA(msg, "Imported %d emails from JSON.", count);
        MessageBoxA(hwnd, msg, "KMail Import", MB_OK);
    } else {
        MessageBoxA(hwnd, "No valid email objects found in file.", "Import Warning", MB_OK | MB_ICONWARNING);
    }
}

void SaveCurrentDraft(HWND hwnd) {
    if(currentTabIdx != -1 && tabs[currentTabIdx].id == 0) {
        char rawText[2048] = {0};
        GetWindowTextA(hBody, rawText, 2048);
        char to[128] = {0}, sub[128] = {0}, body[2048] = {0};
        ParseComposeFields(rawText, to, sub, body);
        if (sub[0] == 0) lstrcpyA(sub, "Untitled Draft");
        if (to[0] == 0) lstrcpyA(to, "draft@kilo.os");

        int did = tabs[currentTabIdx].emailId;
        if (did > 0) {
            for(int i=0; i<num_emails; i++) {
                if (emails[i].id == did) {
                    lstrcpynA(emails[i].sender, to, 128);
                    lstrcpynA(emails[i].subject, sub, 128);
                    lstrcpynA(emails[i].body, body, 2048);
                    break;
                }
            }
        } else {
            if (num_emails < 200) {
                emails[num_emails].id = nextId++;
                emails[num_emails].folder = 3; // drafts
                lstrcpynA(emails[num_emails].sender, to, 128);
                lstrcpynA(emails[num_emails].subject, sub, 128);
                lstrcpynA(emails[num_emails].body, body, 2048);
                emails[num_emails].unread = 0;
                emails[num_emails].encrypted = 0;
                emails[num_emails].starred = 0;
                emails[num_emails].tags[0] = 0;
                tabs[currentTabIdx].emailId = emails[num_emails].id;
                num_emails++;
            }
        }
        RefreshEmailList();
        MessageBoxA(hwnd, "Draft saved to Drafts folder.", "KMail Draft", MB_OK);
    }
}

LRESULT CALLBACK SearchEditSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_ESCAPE) {
            SetWindowTextA(hwnd, "");
            SetFocus(hEmails);
            return 0;
        }
    }
    return CallWindowProc(oldSearchEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitCommonControls();
            hbgMain = CreateSolidBrush(bgMainCol);
            hbgList = CreateSolidBrush(bgListCol);

            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            if (dpi == 0) dpi = 96;
            ReleaseDC(hwnd, hdc);
            int fontHeight = -MulDiv(12, dpi, 72);
            int boldHeight = -MulDiv(14, dpi, 72);
            int smallHeight = -MulDiv(10, dpi, 72);

            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hBold = CreateFontA(boldHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hSmallFont = CreateFontA(smallHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnCompose = CreateWindowEx(0, "BUTTON", "+ Compose [C]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 10, 110, 30, hwnd, (HMENU)ID_BTN_COMPOSE, NULL, NULL);
            SendMessage(hBtnCompose, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnImport = CreateWindowEx(0, "BUTTON", "Import [I]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 125, 10, 75, 30, hwnd, (HMENU)ID_BTN_IMPORT, NULL, NULL);
            hBtnExport = CreateWindowEx(0, "BUTTON", "Export [O]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 205, 10, 75, 30, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 285, 10, 75, 30, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            hHelpLabel = CreateWindowEx(0, "STATIC", "Press F1 or H for Help", WS_CHILD | WS_VISIBLE, 370, 16, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hHelpLabel, WM_SETFONT, (WPARAM)hSmallFont, TRUE);

            hFolders = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_TABSTOP,
                10, 50, 110, H - 100, hwnd, (HMENU)ID_FOLDER_LIST, NULL, NULL);
            SendMessage(hFolders, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"1. Inbox");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"2. Starred");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"3. Sent");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"4. Drafts");
            SendMessageA(hFolders, LB_ADDSTRING, 0, (LPARAM)"5. Trash");
            SendMessage(hFolders, LB_SETCURSEL, 0, 0);

            hBtnEmptyTrash = CreateWindowEx(0, "BUTTON", "Empty Trash", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, H - 40, 110, 30, hwnd, (HMENU)ID_BTN_EMPTY_TRASH, NULL, NULL);
            SendMessage(hBtnEmptyTrash, WM_SETFONT, (WPARAM)hFont, TRUE);

            hSearchBox = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 130, 50, 200, 25, hwnd, (HMENU)ID_SEARCH_BOX, NULL, NULL);
            SendMessage(hSearchBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hSearchBox, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search... (Ctrl+F)");
            oldSearchEditProc = (WNDPROC)SetWindowLongPtrA(hSearchBox, GWLP_WNDPROC, (LONG_PTR)SearchEditSubclass);

            hTagFilter = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 130, 80, 200, 25, hwnd, (HMENU)ID_TAG_FILTER, NULL, NULL);
            SendMessage(hTagFilter, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hTagFilter, EM_SETCUEBANNER, FALSE, (LPARAM)L"Tag filter...");

            hEmails = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_TABSTOP,
                130, 110, 200, H - 160, hwnd, (HMENU)ID_EMAIL_LIST, NULL, NULL);
            SendMessage(hEmails, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnDelete = CreateWindowEx(0, "BUTTON", "Delete [Del]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 120, 10, 95, 30, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
            SendMessage(hBtnDelete, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTab = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | TCS_TABS | WS_TABSTOP, 340, 50, W-370, 25, hwnd, (HMENU)ID_TAB, NULL, NULL);
            SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTitle = CreateWindowEx(0, "STATIC", "No email selected",
                WS_CHILD | WS_VISIBLE,
                340, 85, W - 370, 40, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hBold, TRUE);
            
            hBtnReply = CreateWindowEx(0, "BUTTON", "Reply [R]", WS_CHILD | WS_TABSTOP, 340, 122, 75, 26, hwnd, (HMENU)ID_BTN_REPLY, NULL, NULL);
            hBtnStar = CreateWindowEx(0, "BUTTON", "★ Star [S]", WS_CHILD | WS_TABSTOP, 420, 122, 80, 26, hwnd, (HMENU)ID_BTN_STAR, NULL, NULL);
            hBtnTag = CreateWindowEx(0, "BUTTON", "Tag [T]", WS_CHILD | WS_TABSTOP, 505, 122, 65, 26, hwnd, (HMENU)ID_BTN_TAG, NULL, NULL);
            hBtnExportEml = CreateWindowEx(0, "BUTTON", "Export .EML [E]", WS_CHILD | WS_TABSTOP, 575, 122, 80, 26, hwnd, (HMENU)ID_BTN_EXPORT_EML, NULL, NULL);
            hBtnExportMd = CreateWindowEx(0, "BUTTON", "Export .MD [M]", WS_CHILD | WS_TABSTOP, 660, 122, 75, 26, hwnd, (HMENU)ID_BTN_EXPORT_MD, NULL, NULL);
            hBtnDecrypt = CreateWindowEx(0, "BUTTON", "Decrypt [D]", WS_CHILD | WS_TABSTOP, 740, 122, 90, 26, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            hBtnSaveDraft = CreateWindowEx(0, "BUTTON", "Save Draft [Ctrl+S]", WS_CHILD | WS_TABSTOP, 340, 122, 135, 26, hwnd, (HMENU)ID_BTN_SAVE_DRAFT, NULL, NULL);
            SendMessage(hBtnReply, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnStar, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnTag, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExportEml, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnExportMd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDecrypt, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSaveDraft, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBody = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP,
                340, 155, W - 370, H - 200, hwnd, NULL, NULL, NULL);
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
            else if (LOWORD(wParam) == ID_BTN_HELP) {
                ShowHelpDialog(hwnd);
            }
            else if (LOWORD(wParam) == ID_BTN_REPLY) {
                ReplyToCurrentEmail();
            }
            else if (LOWORD(wParam) == ID_BTN_STAR) {
                if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
                    int eid = tabs[currentTabIdx].emailId;
                    for(int i = 0; i < num_emails; i++) {
                        if(emails[i].id == eid) {
                            emails[i].starred = !emails[i].starred;
                            break;
                        }
                    }
                    RenderPane();
                    RefreshEmailList();
                }
            }
            else if (LOWORD(wParam) == ID_BTN_EXPORT_EML) {
                if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
                    int eid = tabs[currentTabIdx].emailId;
                    for(int i = 0; i < num_emails; i++) {
                        if(emails[i].id == eid) {
                            ExportSingleEmail(&emails[i]);
                            break;
                        }
                    }
                }
            }
            else if (LOWORD(wParam) == ID_BTN_EXPORT_MD) {
                if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
                    int eid = tabs[currentTabIdx].emailId;
                    for(int i = 0; i < num_emails; i++) {
                        if(emails[i].id == eid) {
                            ExportSingleMarkdown(&emails[i]);
                            break;
                        }
                    }
                }
            }
            else if (LOWORD(wParam) == ID_BTN_SAVE_DRAFT) {
                SaveCurrentDraft(hwnd);
            }
            else if (LOWORD(wParam) == ID_BTN_DELETE) {
                if(currentTabIdx != -1) {
                    if(tabs[currentTabIdx].id == 1) { // read tab
                        int eid = tabs[currentTabIdx].emailId;
                        for(int i = 0; i < num_emails; i++) {
                            if(emails[i].id == eid) {
                                if(emails[i].folder == 4) emails[i].folder = 99; // permanent delete
                                else emails[i].folder = 4; // to trash
                                break;
                            }
                        }
                    }
                    CloseCurrentTab();
                    RefreshEmailList();
                }
            }
            else if (LOWORD(wParam) == ID_BTN_EMPTY_TRASH) {
                if (MessageBoxA(hwnd, "Are you sure you want to empty the Trash folder?", "Confirm Empty Trash", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    for(int i = 0; i < num_emails; i++) if(emails[i].folder == 4) emails[i].folder = 99;
                    if(currentFolder == 4) RefreshEmailList();
                    MessageBoxA(hwnd, "Trash folder emptied.", "KMail", MB_OK);
                }
            }
            else if (LOWORD(wParam) == ID_BTN_COMPOSE) {
                NewComposeTab();
            }
            else if (LOWORD(wParam) == ID_BTN_EXPORT) {
                ExportJson();
            }
            else if (LOWORD(wParam) == ID_BTN_IMPORT) {
                ImportJson(hwnd);
            }
            else if (LOWORD(wParam) == ID_BTN_TAG) {
                if(currentTabIdx != -1 && tabs[currentTabIdx].id == 1) {
                    Email* em = NULL;
                    for(int i=0; i<num_emails; i++) if(emails[i].id == tabs[currentTabIdx].emailId) em = &emails[i];
                    if(em) {
                        if (contains_nocase(em->tags, "Important")) {
                            MessageBoxA(hwnd, "Tag 'Important' is already applied to this email.", "Tag", MB_OK);
                        } else {
                            if (lstrlenA(em->tags) + 11 < sizeof(em->tags)) {
                                if (em->tags[0]) lstrcatA(em->tags, ",Important");
                                else lstrcpyA(em->tags, "Important");
                            }
                            MessageBoxA(hwnd, "Appended 'Important' tag.", "Tag", MB_OK);
                            RenderPane();
                            RefreshEmailList();
                        }
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
                            char pwd[32] = "pass";
                            char decStr[2048];
                            lstrcpyA(decStr, em->body);
                            CryptStr(decStr, pwd);
                            SetWindowTextA(hBody, decStr);
                            SetWindowTextA(hBtnDecrypt, "Decrypted!");
                            EnableWindow(hBtnDecrypt, FALSE);
                        }
                    } else if (tabs[currentTabIdx].id == 0) {
                        // Send
                        char rawText[2048] = {0};
                        GetWindowTextA(hBody, rawText, sizeof(rawText));
                        char to[128] = {0}, sub[128] = {0}, body[2048] = {0};
                        ParseComposeFields(rawText, to, sub, body);
                        if (sub[0] == 0) lstrcpyA(sub, "New Message");
                        if (to[0] == 0) lstrcpyA(to, "recipient@kilo.os");

                        int did = tabs[currentTabIdx].emailId;
                        if (did > 0) {
                            // Update draft into sent email
                            for(int i=0; i<num_emails; i++) {
                                if (emails[i].id == did) {
                                    emails[i].folder = 2; // Sent
                                    lstrcpynA(emails[i].sender, to, 128);
                                    lstrcpynA(emails[i].subject, sub, 128);
                                    lstrcpynA(emails[i].body, body, 2048);
                                    break;
                                }
                            }
                        } else {
                            if(num_emails < 200) {
                                emails[num_emails].id = nextId++;
                                emails[num_emails].folder = 2; // sent
                                lstrcpynA(emails[num_emails].subject, sub, 128);
                                lstrcpynA(emails[num_emails].sender, to, 128);
                                lstrcpynA(emails[num_emails].body, body, 2048);
                                emails[num_emails].unread = 0;
                                emails[num_emails].encrypted = 0;
                                emails[num_emails].starred = 0;
                                emails[num_emails].tags[0] = 0;
                                num_emails++;
                            }
                        }
                        CloseCurrentTab();
                        SelectFolder(2); // View Sent folder
                        MessageBoxA(hwnd, "Message Sent!", "KMail", MB_OK);
                    }
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hFolders, 10, 50, 110, nh - 100, TRUE);
            MoveWindow(hBtnEmptyTrash, 10, nh - 40, 110, 30, TRUE);
            
            MoveWindow(hSearchBox, 130, 50, 200, 25, TRUE);
            MoveWindow(hTagFilter, 130, 80, 200, 25, TRUE);
            MoveWindow(hEmails, 130, 110, 200, nh - 120, TRUE);
            
            MoveWindow(hBtnDelete, nw - 110, 10, 95, 30, TRUE);
            
            MoveWindow(hTab, 340, 50, nw - 350, 25, TRUE);
            MoveWindow(hTitle, 340, 85, nw - 350, 35, TRUE);
            
            MoveWindow(hBtnReply, 340, 122, 75, 26, TRUE);
            MoveWindow(hBtnStar, 420, 122, 80, 26, TRUE);
            MoveWindow(hBtnTag, 505, 122, 65, 26, TRUE);
            MoveWindow(hBtnExportEml, 575, 122, 80, 26, TRUE);
            MoveWindow(hBtnExportMd, 660, 122, 75, 26, TRUE);
            MoveWindow(hBtnDecrypt, 740, 122, 90, 26, TRUE);
            MoveWindow(hBtnSaveDraft, 340, 122, 135, 26, TRUE);
            
            MoveWindow(hBody, 340, 155, nw - 350, nh - 165, TRUE);
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
            DeleteObject(hSmallFont);
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
    HWND hwnd = CreateWindowEx(0, "KMailApp", "KMail - Press F1 or H for Help [C: Compose | R: Reply | 1-5: Folders | Esc: Close Tab]", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            char cls[64] = {0};
            GetClassNameA(hFocus, cls, 64);
            int isEdit = (lstrcmpiA(cls, "EDIT") == 0);
            int isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            if (msg.wParam == VK_F1 || (!isEdit && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (isCtrl && (msg.wParam == 'S' || msg.wParam == 's')) {
                SaveCurrentDraft(hwnd);
                continue;
            }
            if (isCtrl && msg.wParam == VK_RETURN) {
                if (currentTabIdx != -1 && tabs[currentTabIdx].id == 0) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DECRYPT, 0), 0);
                    continue;
                }
            }
            if (isCtrl && (msg.wParam == 'F' || msg.wParam == 'f')) {
                SetFocus(hSearchBox);
                SendMessage(hSearchBox, EM_SETSEL, 0, -1);
                continue;
            }
            if (isCtrl && (msg.wParam == 'W' || msg.wParam == 'w')) {
                CloseCurrentTab();
                continue;
            }
            if (isCtrl && msg.wParam == VK_TAB) {
                if (num_tabs > 1) {
                    int nextIdx = (currentTabIdx + 1) % num_tabs;
                    SelectTab(nextIdx);
                }
                continue;
            }
            if (!isEdit) {
                if (msg.wParam == '1') { SelectFolder(0); continue; }
                if (msg.wParam == '2') { SelectFolder(1); continue; }
                if (msg.wParam == '3') { SelectFolder(2); continue; }
                if (msg.wParam == '4') { SelectFolder(3); continue; }
                if (msg.wParam == '5') { SelectFolder(4); continue; }
                if (msg.wParam == 'C' || msg.wParam == 'c') { NewComposeTab(); continue; }
                if (msg.wParam == 'R' || msg.wParam == 'r') { ReplyToCurrentEmail(); continue; }
                if (msg.wParam == 'S' || msg.wParam == 's') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_STAR, 0), 0); continue; }
                if (msg.wParam == 'T' || msg.wParam == 't') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_TAG, 0), 0); continue; }
                if (msg.wParam == 'E' || msg.wParam == 'e') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_EXPORT_EML, 0), 0); continue; }
                if (msg.wParam == 'M' || msg.wParam == 'm') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_EXPORT_MD, 0), 0); continue; }
                if (msg.wParam == 'D' || msg.wParam == 'd') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DECRYPT, 0), 0); continue; }
                if (msg.wParam == 'I' || msg.wParam == 'i') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_IMPORT, 0), 0); continue; }
                if (msg.wParam == 'O' || msg.wParam == 'o') { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_EXPORT, 0), 0); continue; }
                if (msg.wParam == VK_DELETE) { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_DELETE, 0), 0); continue; }
                if (msg.wParam == VK_ESCAPE) { CloseCurrentTab(); continue; }
            }
        }
        if (IsDialogMessage(hwnd, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
