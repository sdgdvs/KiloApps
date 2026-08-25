#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    int len = 0;
    if (!s) return 0;
    while (*s++) len++;
    return len;
}

void my_strncpy(char* d, const char* s, int max_len) {
    if (!d || max_len <= 0) return;
    if (!s) { d[0] = 0; return; }
    int i = 0;
    while (*s && i < max_len - 1) {
        d[i++] = *s++;
    }
    d[i] = 0;
}

void my_strncat(char* d, const char* s, int max_len) {
    if (!d || max_len <= 0) return;
    if (!s) return;
    int len = my_strlen(d);
    int i = len;
    while (*s && i < max_len - 1) {
        d[i++] = *s++;
    }
    d[i] = 0;
}

char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

int my_stricmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2) {
        if (to_lower(*s1) != to_lower(*s2)) return to_lower(*s1) - to_lower(*s2);
        s1++; s2++;
    }
    return to_lower(*s1) - to_lower(*s2);
}

char* my_stristr(const char* s1, const char* s2) {
    if (!s1 || !s2 || !*s2) return (char*)s1;
    for (; *s1; s1++) {
        const char* p1 = s1;
        const char* p2 = s2;
        while (*p1 && *p2 && to_lower(*p1) == to_lower(*p2)) { p1++; p2++; }
        if (!*p2) return (char*)s1;
    }
    return NULL;
}

typedef struct {
    char name[64];
    char phone[64];
    char email[128];
    char category[32];
    char company[64];
    char notes[128];
    int fav;
} Contact;

#define MAX_CONTACTS 150

Contact contacts[MAX_CONTACTS];
int contact_count = 0;
int filtered_indices[MAX_CONTACTS];
int filtered_count = 0;

HWND hList, hEdit, hBtnNew, hBtnDel, hBtnSave, hBtnMerge, hBtnExport, hBtnImport, hBtnCall, hBtnEmail, hSearch, hComboCat, hChkFav, hBtnHelp;
HFONT hFont, hBoldFont;

int g_dpi = 0;
int S(int x) {
    if (g_dpi == 0) {
        HDC hdc = GetDC(NULL);
        g_dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);
        if (g_dpi == 0) g_dpi = 96;
    }
    return MulDiv(x, g_dpi, 96);
}

void LoadDemoData() {
    contact_count = 4;
    
    my_strncpy(contacts[0].name, "Alice Smith", sizeof(contacts[0].name));
    my_strncpy(contacts[0].phone, "+1 555-0124", sizeof(contacts[0].phone));
    my_strncpy(contacts[0].email, "alice.smith@acme.com", sizeof(contacts[0].email));
    my_strncpy(contacts[0].category, "Work", sizeof(contacts[0].category));
    my_strncpy(contacts[0].company, "Acme Corp", sizeof(contacts[0].company));
    my_strncpy(contacts[0].notes, "Lead Architect.", sizeof(contacts[0].notes));
    contacts[0].fav = 1;

    my_strncpy(contacts[1].name, "Bob Jones", sizeof(contacts[1].name));
    my_strncpy(contacts[1].phone, "+1 555-0189", sizeof(contacts[1].phone));
    my_strncpy(contacts[1].email, "bob.jones@gmail.com", sizeof(contacts[1].email));
    my_strncpy(contacts[1].category, "Personal", sizeof(contacts[1].category));
    my_strncpy(contacts[1].company, "", sizeof(contacts[1].company));
    my_strncpy(contacts[1].notes, "Met at tech conference.", sizeof(contacts[1].notes));
    contacts[1].fav = 0;

    my_strncpy(contacts[2].name, "Carla Rossi", sizeof(contacts[2].name));
    my_strncpy(contacts[2].phone, "+1 555-0199", sizeof(contacts[2].phone));
    my_strncpy(contacts[2].email, "carla@designstudio.io", sizeof(contacts[2].email));
    my_strncpy(contacts[2].category, "Work", sizeof(contacts[2].category));
    my_strncpy(contacts[2].company, "Design Studio", sizeof(contacts[2].company));
    my_strncpy(contacts[2].notes, "UX Consultant.", sizeof(contacts[2].notes));
    contacts[2].fav = 1;

    my_strncpy(contacts[3].name, "David Miller", sizeof(contacts[3].name));
    my_strncpy(contacts[3].phone, "+1 555-0143", sizeof(contacts[3].phone));
    my_strncpy(contacts[3].email, "dmiller@familynet.org", sizeof(contacts[3].email));
    my_strncpy(contacts[3].category, "Family", sizeof(contacts[3].category));
    my_strncpy(contacts[3].company, "", sizeof(contacts[3].company));
    my_strncpy(contacts[3].notes, "Cousin.", sizeof(contacts[3].notes));
    contacts[3].fav = 0;
}

void RefreshList() {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    filtered_count = 0;

    char search_buf[64] = {0};
    GetWindowTextA(hSearch, search_buf, sizeof(search_buf));

    int cat_idx = SendMessageA(hComboCat, CB_GETCURSEL, 0, 0);
    char cat_filter[32] = {0};
    if (cat_idx > 0) {
        SendMessageA(hComboCat, CB_GETLBTEXT, cat_idx, (LPARAM)cat_filter);
    }

    for (int i = 0; i < contact_count; i++) {
        // Search match
        int search_match = (search_buf[0] == 0) || 
                           my_stristr(contacts[i].name, search_buf) || 
                           my_stristr(contacts[i].email, search_buf) || 
                           my_stristr(contacts[i].phone, search_buf) ||
                           my_stristr(contacts[i].company, search_buf);

        // Category match
        int cat_match = 1;
        if (my_stricmp(cat_filter, "Favorites") == 0) {
            cat_match = contacts[i].fav;
        } else if (cat_filter[0] != 0 && my_stricmp(cat_filter, "All") != 0) {
            cat_match = (my_stricmp(contacts[i].category, cat_filter) == 0);
        }

        if (search_match && cat_match) {
            filtered_indices[filtered_count] = i;
            filtered_count++;

            char display[256];
            wsprintfA(display, "%s%s [%s]", contacts[i].fav ? "* " : "", contacts[i].name, contacts[i].category[0] ? contacts[i].category : "Other");
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)display);
        }
    }
}

void extract_field(const char* text, const char* prefix, char* out, int out_len) {
    if (!text || !prefix || !out || out_len <= 0) return;
    char* p = my_stristr(text, prefix);
    if (p) {
        p += my_strlen(prefix);
        int i = 0;
        while (*p && *p != '\r' && *p != '\n' && i < out_len - 1) {
            out[i++] = *p++;
        }
        out[i] = 0;
    } else {
        out[0] = 0;
    }
}

void extract_vcard_field(const char* card, const char* key, char* out, int out_len) {
    if (!out || out_len <= 0) return;
    out[0] = 0;
    if (!card || !key) return;

    const char* p = card;
    int key_len = my_strlen(key);

    while (*p) {
        if (my_stristr(p, key) == p) {
            const char* field = p + key_len;
            while (*field && *field != ':' && *field != '\r' && *field != '\n') {
                field++;
            }
            if (*field == ':') field++;
            int i = 0;
            while (*field && *field != '\r' && *field != '\n' && i < out_len - 1) {
                out[i++] = *field++;
            }
            out[i] = 0;
            return;
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
}

void MergeDuplicates(HWND hwnd) {
    if (contact_count < 2) {
        MessageBoxA(hwnd, "Not enough contacts to merge.", "KContacts", MB_OK | MB_ICONINFORMATION);
        return;
    }

    int merged = 0;
    for (int i = 0; i < contact_count; i++) {
        for (int j = i + 1; j < contact_count; j++) {
            if (my_stricmp(contacts[i].name, contacts[j].name) == 0 && contacts[i].name[0] != 0) {
                if (contacts[i].phone[0] == 0 && contacts[j].phone[0] != 0) my_strncpy(contacts[i].phone, contacts[j].phone, sizeof(contacts[i].phone));
                if (contacts[i].email[0] == 0 && contacts[j].email[0] != 0) my_strncpy(contacts[i].email, contacts[j].email, sizeof(contacts[i].email));
                if (contacts[i].company[0] == 0 && contacts[j].company[0] != 0) my_strncpy(contacts[i].company, contacts[j].company, sizeof(contacts[i].company));
                if (contacts[i].notes[0] == 0 && contacts[j].notes[0] != 0) my_strncpy(contacts[i].notes, contacts[j].notes, sizeof(contacts[i].notes));
                if (contacts[j].fav) contacts[i].fav = 1;

                for (int k = j; k < contact_count - 1; k++) {
                    contacts[k] = contacts[k + 1];
                }
                memset(&contacts[contact_count - 1], 0, sizeof(Contact));
                contact_count--;
                j--;
                merged++;
            }
        }
    }

    RefreshList();
    SetWindowTextA(hEdit, "");
    SendMessageA(hChkFav, BM_SETCHECK, BST_UNCHECKED, 0);

    char msg[128];
    wsprintfA(msg, "Duplicate scan completed.\nMerged %d contact(s).", merged);
    MessageBoxA(hwnd, msg, "KContacts Merge", MB_OK | MB_ICONINFORMATION);
}

void ExportVCard(HWND hwnd) {
    if (contact_count == 0) {
        MessageBoxA(hwnd, "No contacts to export.", "Export Error", MB_OK | MB_ICONWARNING);
        return;
    }

    char filepath[MAX_PATH] = "contacts.vcf";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "vCard Files (*.vcf)\0*.vcf\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "vcf";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            for (int i = 0; i < contact_count; i++) {
                char buf[1024];
                wsprintfA(buf, "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:%s\r\nTEL;TYPE=CELL:%s\r\nEMAIL;TYPE=INTERNET:%s\r\nORG:%s\r\nCATEGORIES:%s\r\nNOTE:%s\r\nEND:VCARD\r\n",
                    contacts[i].name, contacts[i].phone, contacts[i].email, contacts[i].company, contacts[i].category, contacts[i].notes);
                DWORD written = 0;
                WriteFile(hFile, buf, my_strlen(buf), &written, NULL);
            }
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Contacts exported to vCard file successfully!", "KContacts Export", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void ImportVCard(HWND hwnd) {
    if (contact_count >= MAX_CONTACTS) {
        MessageBoxA(hwnd, "Contact capacity reached (150 max). Cannot import.", "KContacts Import", MB_OK | MB_ICONWARNING);
        return;
    }

    char filepath[MAX_PATH] = "";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "vCard Files (*.vcf)\0*.vcf\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(hFile, NULL);
            if (size > 0 && size < 2000000) {
                char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 1);
                if (buf) {
                    DWORD read_bytes = 0;
                    ReadFile(hFile, buf, size, &read_bytes, NULL);
                    buf[read_bytes] = 0;

                    int imported = 0;
                    char* p = buf;
                    while (p && *p) {
                        char* card_start = my_stristr(p, "BEGIN:VCARD");
                        if (!card_start) break;
                        char* card_end = my_stristr(card_start, "END:VCARD");
                        if (!card_end) break;

                        if (contact_count < MAX_CONTACTS) {
                            Contact* c = &contacts[contact_count];
                            memset(c, 0, sizeof(Contact));

                            extract_vcard_field(card_start, "FN", c->name, sizeof(c->name));
                            if (c->name[0] == 0) extract_vcard_field(card_start, "N", c->name, sizeof(c->name));
                            extract_vcard_field(card_start, "TEL", c->phone, sizeof(c->phone));
                            extract_vcard_field(card_start, "EMAIL", c->email, sizeof(c->email));
                            extract_vcard_field(card_start, "ORG", c->company, sizeof(c->company));
                            extract_vcard_field(card_start, "CATEGORIES", c->category, sizeof(c->category));
                            if (c->category[0] == 0) my_strncpy(c->category, "Personal", sizeof(c->category));
                            extract_vcard_field(card_start, "NOTE", c->notes, sizeof(c->notes));

                            if (c->name[0] != 0) {
                                contact_count++;
                                imported++;
                            }
                        }
                        p = card_end + 9;
                    }
                    HeapFree(GetProcessHeap(), 0, buf);
                    RefreshList();
                    char msg[128];
                    wsprintfA(msg, "Successfully imported %d contact(s) from vCard!", imported);
                    MessageBoxA(hwnd, msg, "KContacts Import", MB_OK | MB_ICONINFORMATION);
                }
            }
            CloseHandle(hFile);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            LoadDemoData();

            // Fonts
            int fontHeight = -MulDiv(12, g_dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hBoldFont = CreateFontA(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Search bar & Filter
            hSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(10), S(10), S(180), S(24), hwnd, (HMENU)1011, NULL, NULL);
            SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search...");

            hComboCat = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, S(200), S(10), S(140), S(150), hwnd, (HMENU)1012, NULL, NULL);
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"All");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Favorites");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Family");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Friends");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessageA(hComboCat, CB_SETCURSEL, 0, 0);

            // ListBox
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, S(10), S(42), S(330), S(475), hwnd, (HMENU)1001, NULL, NULL);

            // Action Buttons Sidebar
            hBtnNew = CreateWindowExA(0, "BUTTON", "+ New", WS_CHILD | WS_VISIBLE, S(10), S(525), S(55), S(28), hwnd, (HMENU)1002, NULL, NULL);
            hBtnDel = CreateWindowExA(0, "BUTTON", "Del", WS_CHILD | WS_VISIBLE, S(70), S(525), S(40), S(28), hwnd, (HMENU)1003, NULL, NULL);
            hBtnMerge = CreateWindowExA(0, "BUTTON", "Merge", WS_CHILD | WS_VISIBLE, S(115), S(525), S(50), S(28), hwnd, (HMENU)1005, NULL, NULL);
            hBtnImport = CreateWindowExA(0, "BUTTON", "Imp", WS_CHILD | WS_VISIBLE, S(170), S(525), S(45), S(28), hwnd, (HMENU)1007, NULL, NULL);
            hBtnExport = CreateWindowExA(0, "BUTTON", "Exp", WS_CHILD | WS_VISIBLE, S(220), S(525), S(45), S(28), hwnd, (HMENU)1006, NULL, NULL);
            hBtnHelp = CreateWindowExA(0, "BUTTON", "Help(H)", WS_CHILD | WS_VISIBLE, S(270), S(525), S(70), S(28), hwnd, (HMENU)1013, NULL, NULL);

            // Details / Form View
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN, S(350), S(10), S(470), S(507), hwnd, NULL, NULL, NULL);
            SendMessageA(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(8), S(8)));
            
            hChkFav = CreateWindowExA(0, "BUTTON", "Favorite *", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, S(350), S(525), S(90), S(24), hwnd, (HMENU)1010, NULL, NULL);
            hBtnCall = CreateWindowExA(0, "BUTTON", "Call", WS_CHILD | WS_VISIBLE, S(450), S(525), S(50), S(28), hwnd, (HMENU)1008, NULL, NULL);
            hBtnEmail = CreateWindowExA(0, "BUTTON", "Email", WS_CHILD | WS_VISIBLE, S(505), S(525), S(55), S(28), hwnd, (HMENU)1009, NULL, NULL);
            hBtnSave = CreateWindowExA(0, "BUTTON", "Save Details", WS_CHILD | WS_VISIBLE, S(565), S(525), S(255), S(28), hwnd, (HMENU)1004, NULL, NULL);

            // Apply Fonts
            SendMessageA(hSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hComboCat, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnNew, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
            SendMessageA(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnMerge, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnExport, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnCall, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnEmail, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hChkFav, WM_SETFONT, (WPARAM)hFont, TRUE);

            RefreshList();
            break;
        }
        case WM_COMMAND: {
            int control_id = LOWORD(wParam);
            int notify_code = HIWORD(wParam);

            if (control_id == 1001 && notify_code == LBN_SELCHANGE) {
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    char buf[1024];
                    wsprintfA(buf, "Name: %s\r\nPhone: %s\r\nEmail: %s\r\nCategory: %s\r\nCompany: %s\r\nNotes: %s", 
                        contacts[real_idx].name, contacts[real_idx].phone, contacts[real_idx].email,
                        contacts[real_idx].category, contacts[real_idx].company, contacts[real_idx].notes);
                    SetWindowTextA(hEdit, buf);
                    SendMessageA(hChkFav, BM_SETCHECK, contacts[real_idx].fav ? BST_CHECKED : BST_UNCHECKED, 0);
                }
            }
            else if ((control_id == 1011 && notify_code == EN_CHANGE) || (control_id == 1012 && notify_code == CBN_SELCHANGE)) {
                RefreshList();
            }
            else if (control_id == 1002) { // New
                if (contact_count < MAX_CONTACTS) {
                    my_strncpy(contacts[contact_count].name, "New Contact", sizeof(contacts[contact_count].name));
                    my_strncpy(contacts[contact_count].phone, "", sizeof(contacts[contact_count].phone));
                    my_strncpy(contacts[contact_count].email, "", sizeof(contacts[contact_count].email));
                    my_strncpy(contacts[contact_count].category, "Personal", sizeof(contacts[contact_count].category));
                    my_strncpy(contacts[contact_count].company, "", sizeof(contacts[contact_count].company));
                    my_strncpy(contacts[contact_count].notes, "", sizeof(contacts[contact_count].notes));
                    contacts[contact_count].fav = 0;
                    contact_count++;
                    RefreshList();
                    SendMessageA(hList, LB_SETCURSEL, filtered_count - 1, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                } else {
                    MessageBoxA(hwnd, "Maximum contact limit (150) reached.", "KContacts", MB_OK | MB_ICONWARNING);
                }
            }
            else if (control_id == 1003) { // Del
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    for (int i = real_idx; i < contact_count - 1; i++) {
                        contacts[i] = contacts[i+1];
                    }
                    memset(&contacts[contact_count - 1], 0, sizeof(Contact));
                    contact_count--;
                    RefreshList();
                    SetWindowTextA(hEdit, "");
                    SendMessageA(hChkFav, BM_SETCHECK, BST_UNCHECKED, 0);
                }
            }
            else if (control_id == 1004) { // Save Details
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    char buf[1024];
                    GetWindowTextA(hEdit, buf, sizeof(buf));
                    extract_field(buf, "Name: ", contacts[real_idx].name, sizeof(contacts[real_idx].name));
                    if (contacts[real_idx].name[0] == 0) {
                        my_strncpy(contacts[real_idx].name, "Unnamed Contact", sizeof(contacts[real_idx].name));
                    }
                    extract_field(buf, "Phone: ", contacts[real_idx].phone, sizeof(contacts[real_idx].phone));
                    extract_field(buf, "Email: ", contacts[real_idx].email, sizeof(contacts[real_idx].email));
                    extract_field(buf, "Category: ", contacts[real_idx].category, sizeof(contacts[real_idx].category));
                    extract_field(buf, "Company: ", contacts[real_idx].company, sizeof(contacts[real_idx].company));
                    extract_field(buf, "Notes: ", contacts[real_idx].notes, sizeof(contacts[real_idx].notes));
                    contacts[real_idx].fav = (SendMessageA(hChkFav, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    RefreshList();
                }
            }
            else if (control_id == 1005) { // Merge Dups
                MergeDuplicates(hwnd);
            }
            else if (control_id == 1006) { // Export vCard
                ExportVCard(hwnd);
            }
            else if (control_id == 1007) { // Import vCard
                ImportVCard(hwnd);
            }
            else if (control_id == 1008) { // Call
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    if (contacts[real_idx].phone[0] != 0) {
                        char url[256];
                        wsprintfA(url, "tel:%s", contacts[real_idx].phone);
                        ShellExecuteA(hwnd, "open", url, NULL, NULL, SW_SHOWNORMAL);
                    } else {
                        MessageBoxA(hwnd, "No phone number available for this contact.", "KContacts Call", MB_OK | MB_ICONINFORMATION);
                    }
                }
            }
            else if (control_id == 1009) { // Email
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    if (contacts[real_idx].email[0] != 0) {
                        char url[256];
                        wsprintfA(url, "mailto:%s", contacts[real_idx].email);
                        ShellExecuteA(hwnd, "open", url, NULL, NULL, SW_SHOWNORMAL);
                    } else {
                        MessageBoxA(hwnd, "No email address available for this contact.", "KContacts Email", MB_OK | MB_ICONINFORMATION);
                    }
                }
            }
            else if (control_id == 1013) { // Help
                MessageBoxA(hwnd, "KContacts Help:\n\n- Add new contacts using '+ New'.\n- Select a contact to view/edit details on the right.\n- Use the search bar to find contacts by name, email, phone, or company.\n- Click 'Save Details' to apply changes to the selected contact.\n- Export and import your contacts using the 'Exp' and 'Imp' buttons.\n- 'Merge' resolves exact duplicate names.", "KContacts Help", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hBoldFont) DeleteObject(hBoldFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KContactsClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);
    
    RECT rect = {0, 0, S(830), S(565)};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE, 0);

    HWND hwnd = CreateWindowExA(0, "KContactsClass", "KContacts - Contact Manager (Press H for Help)", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            char className[256];
            GetClassNameA(msg.hwnd, className, sizeof(className));
            if (msg.wParam == VK_F1 || (msg.wParam == 'H' && my_stricmp(className, "EDIT") != 0)) {
                SendMessageA(hwnd, WM_COMMAND, 1013, 0);
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
