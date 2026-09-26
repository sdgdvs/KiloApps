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
    char address[96];
    char birthday[32];
    char website[96];
    char tags[64];
    char notes[128];
    int fav;
} Contact;

#define MAX_CONTACTS 150

Contact contacts[MAX_CONTACTS];
int contact_count = 0;
int filtered_indices[MAX_CONTACTS];
int filtered_count = 0;

HWND hList, hEdit, hBtnNew, hBtnDel, hBtnSave, hBtnMerge, hBtnExport, hBtnImport, hBtnCall, hBtnEmail, hSearch, hComboCat, hChkFav, hBtnHelp, hBtnCopy, hBtnDemo, hStatus;
#define TIMER_STATUS 1
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
    contact_count = 5;
    
    my_strncpy(contacts[0].name, "Alice Smith", sizeof(contacts[0].name));
    my_strncpy(contacts[0].phone, "+1 555-0124", sizeof(contacts[0].phone));
    my_strncpy(contacts[0].email, "alice.smith@acme.com", sizeof(contacts[0].email));
    my_strncpy(contacts[0].category, "Work", sizeof(contacts[0].category));
    my_strncpy(contacts[0].company, "Acme Corp", sizeof(contacts[0].company));
    my_strncpy(contacts[0].address, "100 Mission St, San Francisco, CA", sizeof(contacts[0].address));
    my_strncpy(contacts[0].birthday, "1984-04-18", sizeof(contacts[0].birthday));
    my_strncpy(contacts[0].website, "http://www.acme.com/~asmith", sizeof(contacts[0].website));
    my_strncpy(contacts[0].tags, "vip, architect", sizeof(contacts[0].tags));
    my_strncpy(contacts[0].notes, "Lead Architect.", sizeof(contacts[0].notes));
    contacts[0].fav = 1;

    my_strncpy(contacts[1].name, "Bob Jones", sizeof(contacts[1].name));
    my_strncpy(contacts[1].phone, "+1 555-0189", sizeof(contacts[1].phone));
    my_strncpy(contacts[1].email, "bob.jones@gmail.com", sizeof(contacts[1].email));
    my_strncpy(contacts[1].category, "Personal", sizeof(contacts[1].category));
    my_strncpy(contacts[1].company, "", sizeof(contacts[1].company));
    my_strncpy(contacts[1].address, "42 Pine St, Seattle, WA", sizeof(contacts[1].address));
    my_strncpy(contacts[1].birthday, "1988-11-22", sizeof(contacts[1].birthday));
    my_strncpy(contacts[1].website, "", sizeof(contacts[1].website));
    my_strncpy(contacts[1].tags, "tech, meetup", sizeof(contacts[1].tags));
    my_strncpy(contacts[1].notes, "Met at tech conference.", sizeof(contacts[1].notes));
    contacts[1].fav = 0;

    my_strncpy(contacts[2].name, "Carla Rossi", sizeof(contacts[2].name));
    my_strncpy(contacts[2].phone, "+1 555-0199", sizeof(contacts[2].phone));
    my_strncpy(contacts[2].email, "carla@designstudio.io", sizeof(contacts[2].email));
    my_strncpy(contacts[2].category, "Work", sizeof(contacts[2].category));
    my_strncpy(contacts[2].company, "Design Studio", sizeof(contacts[2].company));
    my_strncpy(contacts[2].address, "88 Creative Way, New York, NY", sizeof(contacts[2].address));
    my_strncpy(contacts[2].birthday, "1990-07-09", sizeof(contacts[2].birthday));
    my_strncpy(contacts[2].website, "http://designstudio.io", sizeof(contacts[2].website));
    my_strncpy(contacts[2].tags, "vip, design", sizeof(contacts[2].tags));
    my_strncpy(contacts[2].notes, "UX Consultant.", sizeof(contacts[2].notes));
    contacts[2].fav = 1;

    my_strncpy(contacts[3].name, "David Miller", sizeof(contacts[3].name));
    my_strncpy(contacts[3].phone, "+1 555-0143", sizeof(contacts[3].phone));
    my_strncpy(contacts[3].email, "dmiller@familynet.org", sizeof(contacts[3].email));
    my_strncpy(contacts[3].category, "Family", sizeof(contacts[3].category));
    my_strncpy(contacts[3].company, "", sizeof(contacts[3].company));
    my_strncpy(contacts[3].address, "742 Evergreen Terrace, Springfield, IL", sizeof(contacts[3].address));
    my_strncpy(contacts[3].birthday, "1979-10-14", sizeof(contacts[3].birthday));
    my_strncpy(contacts[3].website, "", sizeof(contacts[3].website));
    my_strncpy(contacts[3].tags, "family", sizeof(contacts[3].tags));
    my_strncpy(contacts[3].notes, "Cousin.", sizeof(contacts[3].notes));
    contacts[3].fav = 0;

    my_strncpy(contacts[4].name, "NOC Routing Admin", sizeof(contacts[4].name));
    my_strncpy(contacts[4].phone, "555-0199", sizeof(contacts[4].phone));
    my_strncpy(contacts[4].email, "sysadmin@10.19.99.4", sizeof(contacts[4].email));
    my_strncpy(contacts[4].category, "Work", sizeof(contacts[4].category));
    my_strncpy(contacts[4].company, "KiloNet Backbone", sizeof(contacts[4].company));
    my_strncpy(contacts[4].address, "Subcarrier Node 0x7F", sizeof(contacts[4].address));
    my_strncpy(contacts[4].birthday, "1999-01-01", sizeof(contacts[4].birthday));
    my_strncpy(contacts[4].website, "http://10.19.99.4/classified", sizeof(contacts[4].website));
    my_strncpy(contacts[4].tags, "sysadmin, network", sizeof(contacts[4].tags));
    my_strncpy(contacts[4].notes, "Server Room B. Subcarrier 1999Hz. Route port 80/classified.", sizeof(contacts[4].notes));
    contacts[4].fav = 1;
}

void RefreshList() {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    filtered_count = 0;

    char search_buf[64] = {0};
    GetWindowTextA(hSearch, search_buf, sizeof(search_buf));

    int cat_idx = SendMessageA(hComboCat, CB_GETCURSEL, 0, 0);

    for (int i = 0; i < contact_count; i++) {
        // Search match (including tags, company, address, website)
        int search_match = (search_buf[0] == 0) || 
                           my_stristr(contacts[i].name, search_buf) || 
                           my_stristr(contacts[i].email, search_buf) || 
                           my_stristr(contacts[i].phone, search_buf) ||
                           my_stristr(contacts[i].company, search_buf) ||
                           my_stristr(contacts[i].address, search_buf) ||
                           my_stristr(contacts[i].website, search_buf) ||
                           my_stristr(contacts[i].birthday, search_buf) ||
                           my_stristr(contacts[i].tags, search_buf);

        // Category match
        int cat_match = 1;
        if (cat_idx == 1) {
            cat_match = contacts[i].fav;
        } else if (cat_idx == 2) {
            cat_match = (my_stricmp(contacts[i].category, "Work") == 0);
        } else if (cat_idx == 3) {
            cat_match = (my_stricmp(contacts[i].category, "Personal") == 0);
        } else if (cat_idx == 4) {
            cat_match = (my_stricmp(contacts[i].category, "Family") == 0);
        } else if (cat_idx == 5) {
            cat_match = (my_stricmp(contacts[i].category, "Friends") == 0);
        } else if (cat_idx == 6) {
            cat_match = (my_stricmp(contacts[i].category, "Other") == 0 || contacts[i].category[0] == 0);
        }

        if (search_match && cat_match) {
            filtered_indices[filtered_count] = i;
            filtered_count++;

            char display[256];
            if (contacts[i].tags[0] != 0) {
                wsprintfA(display, "%s%s [%s #%s]", contacts[i].fav ? "* " : "", contacts[i].name, contacts[i].category[0] ? contacts[i].category : "Other", contacts[i].tags);
            } else {
                wsprintfA(display, "%s%s [%s]", contacts[i].fav ? "* " : "", contacts[i].name, contacts[i].category[0] ? contacts[i].category : "Other");
            }
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)display);
        }
    }
}

void ShowNativeStatus(HWND hwnd, const char* text) {
    if (hStatus && text) {
        SetWindowTextA(hStatus, text);
        SetTimer(hwnd, TIMER_STATUS, 3500, NULL);
    }
}

void UpdateAppTitle(HWND hwnd) {
    char title[128];
    int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
    if (list_idx >= 0 && list_idx < filtered_count) {
        int real_idx = filtered_indices[list_idx];
        wsprintfA(title, "KContacts - [%s] (%d/%d) [F1 for Help]", contacts[real_idx].name, filtered_count, contact_count);
    } else {
        wsprintfA(title, "KContacts - %d contacts [F1 for Help]", contact_count);
    }
    SetWindowTextA(hwnd, title);
}

void CopyContactToClipboard(HWND hwnd) {
    int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
    if (list_idx < 0 || list_idx >= filtered_count) {
        ShowNativeStatus(hwnd, " No contact selected to copy.");
        return;
    }
    int real_idx = filtered_indices[list_idx];
    char buf[2048];
    wsprintfA(buf, "Name: %s\r\nPhone: %s\r\nEmail: %s\r\nCategory: %s\r\nCompany: %s\r\nAddress: %s\r\nBirthday: %s\r\nWebsite: %s\r\nTags: %s\r\nNotes: %s",
        contacts[real_idx].name, contacts[real_idx].phone, contacts[real_idx].email,
        contacts[real_idx].category, contacts[real_idx].company, contacts[real_idx].address,
        contacts[real_idx].birthday, contacts[real_idx].website, contacts[real_idx].tags, contacts[real_idx].notes);

    int len = my_strlen(buf);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (hMem) {
        char* pMem = (char*)GlobalLock(hMem);
        if (pMem) {
            my_strncpy(pMem, buf, len + 1);
            GlobalUnlock(hMem);
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hMem);
                CloseClipboard();
                char statusBuf[128];
                wsprintfA(statusBuf, " Copied info for \"%s\" to clipboard!", contacts[real_idx].name);
                ShowNativeStatus(hwnd, statusBuf);
                return;
            }
        }
        GlobalFree(hMem);
    }
    ShowNativeStatus(hwnd, " Failed to copy to clipboard.");
}

void ResetDemoData(HWND hwnd) {
    LoadDemoData();
    RefreshList();
    if (filtered_count > 0) {
        SendMessageA(hList, LB_SETCURSEL, 0, 0);
        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
    }
    ShowNativeStatus(hwnd, " Restored 5 standard sample contacts.");
    UpdateAppTitle(hwnd);
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

void extract_multiline_field(const char* text, const char* prefix, char* out, int out_len) {
    if (!text || !prefix || !out || out_len <= 0) return;
    char* p = my_stristr(text, prefix);
    if (p) {
        p += my_strlen(prefix);
        int i = 0;
        while (*p && i < out_len - 1) {
            out[i++] = *p++;
        }
        while (i > 0 && (out[i - 1] == '\r' || out[i - 1] == '\n' || out[i - 1] == ' ')) {
            i--;
        }
        out[i] = 0;
    } else {
        out[0] = 0;
    }
}

int parse_csv_field(const char** pp, char* out, int out_len) {
    if (!pp || !*pp || !out || out_len <= 0) return 0;
    const char* p = *pp;
    int i = 0;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') {
        p++;
        while (*p && i < out_len - 1) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    out[i++] = '"';
                    p += 2;
                } else {
                    p++;
                    break;
                }
            } else {
                out[i++] = *p++;
            }
        }
        while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    } else {
        while (*p && *p != ',' && *p != '\r' && *p != '\n' && i < out_len - 1) {
            out[i++] = *p++;
        }
    }
    out[i] = 0;
    if (*p == ',') {
        p++;
        *pp = p;
        return 1;
    }
    if (*p == '\r') p++;
    if (*p == '\n') p++;
    *pp = p;
    return 0;
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

void extract_json_field(const char* obj, const char* key, char* out, int out_len) {
    if (!out || out_len <= 0) return;
    out[0] = 0;
    if (!obj || !key) return;
    char pattern[64];
    wsprintfA(pattern, "\"%s\"", key);
    char* p = my_stristr(obj, pattern);
    if (!p) return;
    p += my_strlen(pattern);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
    if (*p == '\"') {
        p++;
        int i = 0;
        while (*p && *p != '\"' && i < out_len - 1) {
            if (*p == '\\' && *(p + 1)) p++;
            out[i++] = *p++;
        }
        out[i] = 0;
    }
}

void MergeDuplicates(HWND hwnd) {
    if (contact_count < 2) {
        ShowNativeStatus(hwnd, " Not enough contacts to merge.");
        return;
    }

    int merged = 0;
    for (int i = 0; i < contact_count; i++) {
        for (int j = i + 1; j < contact_count; j++) {
            if (my_stricmp(contacts[i].name, contacts[j].name) == 0 && contacts[i].name[0] != 0) {
                if (contacts[i].phone[0] == 0 && contacts[j].phone[0] != 0) my_strncpy(contacts[i].phone, contacts[j].phone, sizeof(contacts[i].phone));
                if (contacts[i].email[0] == 0 && contacts[j].email[0] != 0) my_strncpy(contacts[i].email, contacts[j].email, sizeof(contacts[i].email));
                if (contacts[i].company[0] == 0 && contacts[j].company[0] != 0) my_strncpy(contacts[i].company, contacts[j].company, sizeof(contacts[i].company));
                if (contacts[i].address[0] == 0 && contacts[j].address[0] != 0) my_strncpy(contacts[i].address, contacts[j].address, sizeof(contacts[i].address));
                if (contacts[i].birthday[0] == 0 && contacts[j].birthday[0] != 0) my_strncpy(contacts[i].birthday, contacts[j].birthday, sizeof(contacts[i].birthday));
                if (contacts[i].website[0] == 0 && contacts[j].website[0] != 0) my_strncpy(contacts[i].website, contacts[j].website, sizeof(contacts[i].website));
                if (contacts[i].tags[0] == 0 && contacts[j].tags[0] != 0) my_strncpy(contacts[i].tags, contacts[j].tags, sizeof(contacts[i].tags));
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
    if (filtered_count > 0) {
        SendMessageA(hList, LB_SETCURSEL, 0, 0);
        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
    } else {
        SetWindowTextA(hEdit, "");
        SendMessageA(hChkFav, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    char msg[128];
    wsprintfA(msg, " Duplicate scan completed: merged %d contact(s).", merged);
    ShowNativeStatus(hwnd, msg);
    UpdateAppTitle(hwnd);
}

void ExportContacts(HWND hwnd) {
    if (contact_count == 0) {
        ShowNativeStatus(hwnd, " No contacts to export.");
        return;
    }

    char filepath[MAX_PATH] = "contacts_directory.md";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Markdown Directory (*.md)\0*.md\0JSON Database (*.json)\0*.json\0vCard Files (*.vcf)\0*.vcf\0CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "md";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            if (my_stristr(filepath, ".json")) {
                const char* header = "[\r\n";
                WriteFile(hFile, header, my_strlen(header), &written, NULL);
                for (int i = 0; i < contact_count; i++) {
                    char buf[2048];
                    wsprintfA(buf, "  {\r\n    \"name\": \"%s\",\r\n    \"phone\": \"%s\",\r\n    \"email\": \"%s\",\r\n    \"category\": \"%s\",\r\n    \"company\": \"%s\",\r\n    \"address\": \"%s\",\r\n    \"birthday\": \"%s\",\r\n    \"website\": \"%s\",\r\n    \"tags\": \"%s\",\r\n    \"fav\": %d,\r\n    \"notes\": \"%s\"\r\n  }%s\r\n",
                        contacts[i].name, contacts[i].phone, contacts[i].email, contacts[i].category, contacts[i].company,
                        contacts[i].address, contacts[i].birthday, contacts[i].website,
                        contacts[i].tags, contacts[i].fav, contacts[i].notes, (i < contact_count - 1) ? "," : "");
                    WriteFile(hFile, buf, my_strlen(buf), &written, NULL);
                }
                const char* footer = "]\r\n";
                WriteFile(hFile, footer, my_strlen(footer), &written, NULL);
            } else if (my_stristr(filepath, ".csv")) {
                const char* header = "Name,Phone,Email,Category,Company,Address,Birthday,Website,Tags,Favorite,Notes\r\n";
                WriteFile(hFile, header, my_strlen(header), &written, NULL);
                for (int i = 0; i < contact_count; i++) {
                    char buf[2048];
                    wsprintfA(buf, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%s,\"%s\"\r\n",
                        contacts[i].name, contacts[i].phone, contacts[i].email, contacts[i].category, contacts[i].company,
                        contacts[i].address, contacts[i].birthday, contacts[i].website,
                        contacts[i].tags, contacts[i].fav ? "TRUE" : "FALSE", contacts[i].notes);
                    WriteFile(hFile, buf, my_strlen(buf), &written, NULL);
                }
            } else if (my_stristr(filepath, ".vcf")) {
                for (int i = 0; i < contact_count; i++) {
                    char esc_notes[256] = {0};
                    int ni = 0;
                    for (int si = 0; contacts[i].notes[si] && ni < (int)sizeof(esc_notes) - 3; si++) {
                        if (contacts[i].notes[si] == '\r') continue;
                        if (contacts[i].notes[si] == '\n') {
                            esc_notes[ni++] = '\\';
                            esc_notes[ni++] = 'n';
                        } else {
                            esc_notes[ni++] = contacts[i].notes[si];
                        }
                    }
                    esc_notes[ni] = 0;

                    char buf[2048];
                    wsprintfA(buf, "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:%s\r\nTEL;TYPE=CELL:%s\r\nEMAIL;TYPE=INTERNET:%s\r\nORG:%s\r\nADR;TYPE=WORK:;;%s;;;;\r\nBDAY:%s\r\nURL:%s\r\nCATEGORIES:%s\r\nX-TAGS:%s\r\nNOTE:%s\r\nEND:VCARD\r\n",
                        contacts[i].name, contacts[i].phone, contacts[i].email, contacts[i].company,
                        contacts[i].address, contacts[i].birthday, contacts[i].website,
                        contacts[i].category, contacts[i].tags, esc_notes);
                    WriteFile(hFile, buf, my_strlen(buf), &written, NULL);
                }
            } else { // Markdown default
                const char* title = "# \xF0\x9F\x93\x87 KContacts Directory\r\n\r\n## Contacts Summary\r\n\r\n";
                WriteFile(hFile, title, my_strlen(title), &written, NULL);
                for (int i = 0; i < contact_count; i++) {
                    char buf[2048];
                    wsprintfA(buf, "### %s%s\r\n- **Category:** %s\r\n- **Tags:** #%s\r\n- **Phone:** [%s](tel:%s)\r\n- **Email:** [%s](mailto:%s)\r\n- **Company:** %s\r\n- **Address:** %s\r\n- **Birthday:** %s\r\n- **Website:** %s\r\n- **Notes:** %s\r\n\r\n---\r\n\r\n",
                        contacts[i].name, contacts[i].fav ? " [Favorite]" : "", contacts[i].category, contacts[i].tags[0] ? contacts[i].tags : "none",
                        contacts[i].phone, contacts[i].phone, contacts[i].email, contacts[i].email, contacts[i].company,
                        contacts[i].address[0] ? contacts[i].address : "none", contacts[i].birthday[0] ? contacts[i].birthday : "none",
                        contacts[i].website[0] ? contacts[i].website : "none", contacts[i].notes);
                    WriteFile(hFile, buf, my_strlen(buf), &written, NULL);
                }
            }
            CloseHandle(hFile);
            char statusMsg[128];
            wsprintfA(statusMsg, " Exported %d contacts successfully!", contact_count);
            ShowNativeStatus(hwnd, statusMsg);
        }
    }
}

void ImportContacts(HWND hwnd) {
    if (contact_count >= MAX_CONTACTS) {
        ShowNativeStatus(hwnd, " Contact capacity reached (150 max). Cannot import.");
        return;
    }

    char filepath[MAX_PATH] = "";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "All Supported (*.vcf;*.json;*.csv)\0*.vcf;*.json;*.csv\0vCard (*.vcf)\0*.vcf\0JSON (*.json)\0*.json\0CSV (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
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
                    if (my_stristr(filepath, ".json") || buf[0] == '[') {
                        char* p = buf;
                        while (p && *p) {
                            char* obj_start = my_stristr(p, "{");
                            if (!obj_start) break;
                            char* obj_end = my_stristr(obj_start, "}");
                            if (!obj_end) break;

                            if (contact_count < MAX_CONTACTS) {
                                Contact* c = &contacts[contact_count];
                                memset(c, 0, sizeof(Contact));

                                extract_json_field(obj_start, "name", c->name, sizeof(c->name));
                                extract_json_field(obj_start, "phone", c->phone, sizeof(c->phone));
                                extract_json_field(obj_start, "email", c->email, sizeof(c->email));
                                extract_json_field(obj_start, "category", c->category, sizeof(c->category));
                                if (c->category[0] == 0) my_strncpy(c->category, "Personal", sizeof(c->category));
                                extract_json_field(obj_start, "company", c->company, sizeof(c->company));
                                extract_json_field(obj_start, "address", c->address, sizeof(c->address));
                                extract_json_field(obj_start, "birthday", c->birthday, sizeof(c->birthday));
                                extract_json_field(obj_start, "website", c->website, sizeof(c->website));
                                extract_json_field(obj_start, "tags", c->tags, sizeof(c->tags));
                                extract_json_field(obj_start, "notes", c->notes, sizeof(c->notes));

                                if (c->name[0] != 0) {
                                    contact_count++;
                                    imported++;
                                }
                            }
                            p = obj_end + 1;
                        }
                    } else if (my_stristr(filepath, ".csv") || (!my_stristr(buf, "BEGIN:VCARD") && my_stristr(buf, ","))) {
                        const char* p = buf;
                        int has_extended_cols = (my_stristr(buf, "Address") != NULL || my_stristr(buf, "Street") != NULL);
                        if (my_stristr(p, "Name") == p || my_stristr(p, "\"Name\"") == p) {
                            while (*p && *p != '\n') p++;
                            if (*p == '\n') p++;
                        }
                        while (*p && contact_count < MAX_CONTACTS) {
                            while (*p == '\r' || *p == '\n') p++;
                            if (!*p) break;

                            char f_name[64] = {0}, f_phone[64] = {0}, f_email[128] = {0};
                            char f_cat[32] = {0}, f_comp[64] = {0}, f_addr[96] = {0};
                            char f_bday[32] = {0}, f_web[96] = {0}, f_tags[64] = {0}, f_fav[16] = {0}, f_notes[128] = {0};

                            parse_csv_field(&p, f_name, sizeof(f_name));
                            parse_csv_field(&p, f_phone, sizeof(f_phone));
                            parse_csv_field(&p, f_email, sizeof(f_email));
                            parse_csv_field(&p, f_cat, sizeof(f_cat));
                            parse_csv_field(&p, f_comp, sizeof(f_comp));
                            if (has_extended_cols) {
                                parse_csv_field(&p, f_addr, sizeof(f_addr));
                                parse_csv_field(&p, f_bday, sizeof(f_bday));
                                parse_csv_field(&p, f_web, sizeof(f_web));
                                parse_csv_field(&p, f_tags, sizeof(f_tags));
                                parse_csv_field(&p, f_fav, sizeof(f_fav));
                                parse_csv_field(&p, f_notes, sizeof(f_notes));
                            } else {
                                parse_csv_field(&p, f_tags, sizeof(f_tags));
                                parse_csv_field(&p, f_fav, sizeof(f_fav));
                                parse_csv_field(&p, f_notes, sizeof(f_notes));
                            }

                            if (f_name[0] != 0) {
                                Contact* c = &contacts[contact_count];
                                memset(c, 0, sizeof(Contact));
                                my_strncpy(c->name, f_name, sizeof(c->name));
                                my_strncpy(c->phone, f_phone, sizeof(c->phone));
                                my_strncpy(c->email, f_email, sizeof(c->email));
                                my_strncpy(c->category, f_cat[0] ? f_cat : "Personal", sizeof(c->category));
                                my_strncpy(c->company, f_comp, sizeof(c->company));
                                my_strncpy(c->address, f_addr, sizeof(c->address));
                                my_strncpy(c->birthday, f_bday, sizeof(c->birthday));
                                my_strncpy(c->website, f_web, sizeof(c->website));
                                my_strncpy(c->tags, f_tags, sizeof(c->tags));
                                c->fav = (my_stricmp(f_fav, "TRUE") == 0 || my_stricmp(f_fav, "1") == 0);
                                my_strncpy(c->notes, f_notes, sizeof(c->notes));

                                contact_count++;
                                imported++;
                            }
                        }
                    } else {
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
                                extract_vcard_field(card_start, "ADR", c->address, sizeof(c->address));
                                extract_vcard_field(card_start, "BDAY", c->birthday, sizeof(c->birthday));
                                extract_vcard_field(card_start, "URL", c->website, sizeof(c->website));
                                extract_vcard_field(card_start, "CATEGORIES", c->category, sizeof(c->category));
                                if (c->category[0] == 0) my_strncpy(c->category, "Personal", sizeof(c->category));
                                extract_vcard_field(card_start, "X-TAGS", c->tags, sizeof(c->tags));
                                extract_vcard_field(card_start, "NOTE", c->notes, sizeof(c->notes));

                                if (c->name[0] != 0) {
                                    contact_count++;
                                    imported++;
                                }
                            }
                            p = card_end + 9;
                        }
                    }

                    HeapFree(GetProcessHeap(), 0, buf);
                    RefreshList();
                    if (filtered_count > 0) {
                        SendMessageA(hList, LB_SETCURSEL, 0, 0);
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                    }
                    char msg[128];
                    wsprintfA(msg, " Successfully imported %d contact(s)!", imported);
                    ShowNativeStatus(hwnd, msg);
                    UpdateAppTitle(hwnd);
                }
            }
            CloseHandle(hFile);
        }
    }
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd, 
        "KContacts - Contact Manager & Address Book\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  [F1] or [H]       - Show this Help Guide\n"
        "  [Ctrl+S]          - Save contact details\n"
        "  [N] or [Ctrl+N]   - Create new contact draft\n"
        "  [Ctrl+C] or [C]   - Copy selected contact details to clipboard\n"
        "  [Alt+C]           - Call contact phone\n"
        "  [Alt+M]           - Email contact\n"
        "  [1] - [7]         - Quick category filter (1=All, 2=Favs, 3=Work, 4=Personal, 5=Family, 6=Friends, 7=Other)\n"
        "  [Ctrl+D]          - Restore 5 standard sample contacts\n"
        "  [Esc]             - Clear search query and reset filter\n"
        "  [Delete]          - Delete currently selected contact\n"
        "  [Ctrl+M]          - Merge duplicate contacts\n"
        "  [Ctrl+E]          - Export directory (.md, .json, .vcf, .csv)\n"
        "  [Ctrl+I]          - Import contacts (.json, .vcf, .csv)\n\n"
        "FEATURES:\n"
        "  * Select a contact from the list to view/edit details on the right.\n"
        "  * Edit Name, Phone, Email, Category, Company, Address, Birthday, Website, Tags, and Notes.\n"
        "  * Search contacts by typing in search box (matches name, email, phone, company, address, tags).\n"
        "  * Click 'Save Details' to persist edits to memory.\n"
        "  * 'Exp' exports to Markdown Directory, JSON Database, vCard, or CSV.\n"
        "  * 'Imp' auto-detects and imports vCards, JSON, or CSV.\n"
        "  * 'Merge' deduplicates identical contact names and merges tags/fields.\n"
        "  * 'Demo' restores standard sample contacts.",
        "KContacts User Guide & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            LoadDemoData();

            S(0);
            int fontHeight = -MulDiv(12, g_dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hBoldFont = CreateFontA(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Search bar & Filter
            hSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, S(10), S(10), S(180), S(24), hwnd, (HMENU)1011, NULL, NULL);
            SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search contacts...");

            hComboCat = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, S(195), S(10), S(145), S(160), hwnd, (HMENU)1012, NULL, NULL);
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[1] All");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[2] Favorites");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[3] Work");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[4] Personal");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[5] Family");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[6] Friends");
            SendMessageA(hComboCat, CB_ADDSTRING, 0, (LPARAM)"[7] Other");
            SendMessageA(hComboCat, CB_SETCURSEL, 0, 0);

            // ListBox
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY, S(10), S(42), S(330), S(475), hwnd, (HMENU)1001, NULL, NULL);

            // Action Buttons Sidebar (Total width 330px)
            hBtnNew = CreateWindowExA(0, "BUTTON", "+ New [N]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(10), S(525), S(60), S(28), hwnd, (HMENU)1002, NULL, NULL);
            hBtnDel = CreateWindowExA(0, "BUTTON", "Del", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(72), S(525), S(36), S(28), hwnd, (HMENU)1003, NULL, NULL);
            hBtnMerge = CreateWindowExA(0, "BUTTON", "Merge", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(110), S(525), S(50), S(28), hwnd, (HMENU)1005, NULL, NULL);
            hBtnImport = CreateWindowExA(0, "BUTTON", "Imp", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(162), S(525), S(40), S(28), hwnd, (HMENU)1007, NULL, NULL);
            hBtnExport = CreateWindowExA(0, "BUTTON", "Exp", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(204), S(525), S(40), S(28), hwnd, (HMENU)1006, NULL, NULL);
            hBtnDemo = CreateWindowExA(0, "BUTTON", "Demo", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(246), S(525), S(46), S(28), hwnd, (HMENU)1015, NULL, NULL);
            hBtnHelp = CreateWindowExA(0, "BUTTON", "Help", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(294), S(525), S(46), S(28), hwnd, (HMENU)1013, NULL, NULL);

            // Details / Form View (Total width 470px)
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN, S(350), S(10), S(470), S(507), hwnd, NULL, NULL, NULL);
            SendMessageA(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(8), S(8)));
            
            hChkFav = CreateWindowExA(0, "BUTTON", "Fav *", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, S(350), S(525), S(78), S(26), hwnd, (HMENU)1010, NULL, NULL);
            hBtnCall = CreateWindowExA(0, "BUTTON", "Call", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(432), S(525), S(58), S(28), hwnd, (HMENU)1008, NULL, NULL);
            hBtnEmail = CreateWindowExA(0, "BUTTON", "Email", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(493), S(525), S(60), S(28), hwnd, (HMENU)1009, NULL, NULL);
            hBtnCopy = CreateWindowExA(0, "BUTTON", "Copy [^C]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(556), S(525), S(82), S(28), hwnd, (HMENU)1014, NULL, NULL);
            hBtnSave = CreateWindowExA(0, "BUTTON", "Save Details [Ctrl+S]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, S(642), S(525), S(178), S(28), hwnd, (HMENU)1004, NULL, NULL);

            // Bottom Status Bar
            hStatus = CreateWindowExA(0, "STATIC", " Ready - 5 contacts | Press F1 for Help", WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, S(10), S(558), S(810), S(20), hwnd, (HMENU)1020, NULL, NULL);

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
            SendMessageA(hBtnDemo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnCall, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnEmail, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnCopy, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hChkFav, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            RefreshList();
            if (filtered_count > 0) {
                SendMessageA(hList, LB_SETCURSEL, 0, 0);
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
            }
            UpdateAppTitle(hwnd);
            break;
        }
        case WM_TIMER: {
            if (wParam == TIMER_STATUS) {
                KillTimer(hwnd, TIMER_STATUS);
                char readyMsg[128];
                wsprintfA(readyMsg, " Ready - %d contact%s | Press F1 for Help", contact_count, contact_count == 1 ? "" : "s");
                SetWindowTextA(hStatus, readyMsg);
            }
            break;
        }
        case WM_COMMAND: {
            int control_id = LOWORD(wParam);
            int notify_code = HIWORD(wParam);

            if (control_id == 1001 && notify_code == LBN_SELCHANGE) {
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    char buf[2048];
                    wsprintfA(buf, "Name: %s\r\nPhone: %s\r\nEmail: %s\r\nCategory: %s\r\nCompany: %s\r\nAddress: %s\r\nBirthday: %s\r\nWebsite: %s\r\nTags: %s\r\nNotes: %s", 
                        contacts[real_idx].name, contacts[real_idx].phone, contacts[real_idx].email,
                        contacts[real_idx].category, contacts[real_idx].company,
                        contacts[real_idx].address, contacts[real_idx].birthday, contacts[real_idx].website,
                        contacts[real_idx].tags, contacts[real_idx].notes);
                    SetWindowTextA(hEdit, buf);
                    SendMessageA(hChkFav, BM_SETCHECK, contacts[real_idx].fav ? BST_CHECKED : BST_UNCHECKED, 0);
                    UpdateAppTitle(hwnd);
                }
            }
            else if ((control_id == 1011 && notify_code == EN_CHANGE) || (control_id == 1012 && notify_code == CBN_SELCHANGE)) {
                RefreshList();
                if (filtered_count > 0) {
                    SendMessageA(hList, LB_SETCURSEL, 0, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                } else {
                    SetWindowTextA(hEdit, "");
                    SendMessageA(hChkFav, BM_SETCHECK, BST_UNCHECKED, 0);
                }
                UpdateAppTitle(hwnd);
            }
            else if (control_id == 1002) { // New
                if (contact_count < MAX_CONTACTS) {
                    my_strncpy(contacts[contact_count].name, "New Contact", sizeof(contacts[contact_count].name));
                    my_strncpy(contacts[contact_count].phone, "", sizeof(contacts[contact_count].phone));
                    my_strncpy(contacts[contact_count].email, "", sizeof(contacts[contact_count].email));
                    my_strncpy(contacts[contact_count].category, "Personal", sizeof(contacts[contact_count].category));
                    my_strncpy(contacts[contact_count].company, "", sizeof(contacts[contact_count].company));
                    contacts[contact_count].address[0] = 0;
                    contacts[contact_count].birthday[0] = 0;
                    contacts[contact_count].website[0] = 0;
                    contacts[contact_count].tags[0] = 0;
                    my_strncpy(contacts[contact_count].notes, "", sizeof(contacts[contact_count].notes));
                    contacts[contact_count].fav = 0;
                    contact_count++;
                    RefreshList();
                    SendMessageA(hList, LB_SETCURSEL, filtered_count - 1, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                    ShowNativeStatus(hwnd, " Created new contact draft.");
                    UpdateAppTitle(hwnd);
                    SetFocus(hEdit);
                } else {
                    ShowNativeStatus(hwnd, " Maximum contact limit (150) reached.");
                }
            }
            else if (control_id == 1003) { // Del
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    char deletedName[64];
                    my_strncpy(deletedName, contacts[real_idx].name, sizeof(deletedName));
                    for (int i = real_idx; i < contact_count - 1; i++) {
                        contacts[i] = contacts[i+1];
                    }
                    memset(&contacts[contact_count - 1], 0, sizeof(Contact));
                    contact_count--;
                    RefreshList();
                    if (filtered_count > 0) {
                        if (list_idx >= filtered_count) list_idx = filtered_count - 1;
                        SendMessageA(hList, LB_SETCURSEL, list_idx, 0);
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                    } else {
                        SetWindowTextA(hEdit, "");
                        SendMessageA(hChkFav, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                    char msg[128];
                    wsprintfA(msg, " Deleted \"%s\".", deletedName);
                    ShowNativeStatus(hwnd, msg);
                    UpdateAppTitle(hwnd);
                }
            }
            else if (control_id == 1004) { // Save Details
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    char buf[2048];
                    GetWindowTextA(hEdit, buf, sizeof(buf));
                    extract_field(buf, "Name: ", contacts[real_idx].name, sizeof(contacts[real_idx].name));
                    if (contacts[real_idx].name[0] == 0) {
                        my_strncpy(contacts[real_idx].name, "Unnamed Contact", sizeof(contacts[real_idx].name));
                    }
                    extract_field(buf, "Phone: ", contacts[real_idx].phone, sizeof(contacts[real_idx].phone));
                    extract_field(buf, "Email: ", contacts[real_idx].email, sizeof(contacts[real_idx].email));
                    extract_field(buf, "Category: ", contacts[real_idx].category, sizeof(contacts[real_idx].category));
                    extract_field(buf, "Company: ", contacts[real_idx].company, sizeof(contacts[real_idx].company));
                    extract_field(buf, "Address: ", contacts[real_idx].address, sizeof(contacts[real_idx].address));
                    extract_field(buf, "Birthday: ", contacts[real_idx].birthday, sizeof(contacts[real_idx].birthday));
                    extract_field(buf, "Website: ", contacts[real_idx].website, sizeof(contacts[real_idx].website));
                    extract_field(buf, "Tags: ", contacts[real_idx].tags, sizeof(contacts[real_idx].tags));
                    extract_multiline_field(buf, "Notes: ", contacts[real_idx].notes, sizeof(contacts[real_idx].notes));
                    contacts[real_idx].fav = (SendMessageA(hChkFav, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    RefreshList();
                    SendMessageA(hList, LB_SETCURSEL, list_idx, 0);
                    char msg[128];
                    wsprintfA(msg, " Saved details for \"%s\".", contacts[real_idx].name);
                    ShowNativeStatus(hwnd, msg);
                    UpdateAppTitle(hwnd);
                }
            }
            else if (control_id == 1005) { // Merge Dups
                MergeDuplicates(hwnd);
            }
            else if (control_id == 1006) { // Export
                ExportContacts(hwnd);
            }
            else if (control_id == 1007) { // Import
                ImportContacts(hwnd);
            }
            else if (control_id == 1008) { // Call
                int list_idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (list_idx >= 0 && list_idx < filtered_count) {
                    int real_idx = filtered_indices[list_idx];
                    if (contacts[real_idx].phone[0] != 0) {
                        char url[256];
                        wsprintfA(url, "tel:%s", contacts[real_idx].phone);
                        ShellExecuteA(hwnd, "open", url, NULL, NULL, SW_SHOWNORMAL);
                        ShowNativeStatus(hwnd, " Dialing phone number...");
                    } else {
                        ShowNativeStatus(hwnd, " No phone number available for this contact.");
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
                        ShowNativeStatus(hwnd, " Opening email client...");
                    } else {
                        ShowNativeStatus(hwnd, " No email address available for this contact.");
                    }
                }
            }
            else if (control_id == 1013) { // Help
                ShowHelpDialog(hwnd);
            }
            else if (control_id == 1014) { // Copy
                CopyContactToClipboard(hwnd);
            }
            else if (control_id == 1015) { // Demo
                ResetDemoData(hwnd);
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
    
    RECT rect = {0, 0, S(830), S(588)};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE, 0);

    HWND hwnd = CreateWindowExA(0, "KContactsClass", "KContacts - Contact Manager [Press F1 for Help]", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            char className[256] = {0};
            GetClassNameA(msg.hwnd, className, sizeof(className));
            int isEdit = (my_stricmp(className, "EDIT") == 0);
            int ctrl = (GetKeyState(VK_CONTROL) & 0x8000);
            int alt = (GetKeyState(VK_MENU) & 0x8000);

            if (msg.wParam == VK_F1 || ((msg.wParam == 'H' || msg.wParam == 'h') && !isEdit)) {
                SendMessageA(hwnd, WM_COMMAND, 1013, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'S' || msg.wParam == 's')) {
                SendMessageA(hwnd, WM_COMMAND, 1004, 0);
                continue;
            }
            if ((ctrl && (msg.wParam == 'N' || msg.wParam == 'n')) || (!isEdit && (msg.wParam == 'N' || msg.wParam == 'n'))) {
                SendMessageA(hwnd, WM_COMMAND, 1002, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'M' || msg.wParam == 'm')) {
                SendMessageA(hwnd, WM_COMMAND, 1005, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'E' || msg.wParam == 'e')) {
                SendMessageA(hwnd, WM_COMMAND, 1006, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'I' || msg.wParam == 'i')) {
                SendMessageA(hwnd, WM_COMMAND, 1007, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'D' || msg.wParam == 'd')) {
                SendMessageA(hwnd, WM_COMMAND, 1015, 0);
                continue;
            }
            if (alt && (msg.wParam == 'C' || msg.wParam == 'c')) {
                SendMessageA(hwnd, WM_COMMAND, 1008, 0);
                continue;
            }
            if (alt && (msg.wParam == 'M' || msg.wParam == 'm')) {
                SendMessageA(hwnd, WM_COMMAND, 1009, 0);
                continue;
            }
            if (!isEdit && (msg.wParam == 'C' || msg.wParam == 'c')) {
                SendMessageA(hwnd, WM_COMMAND, 1014, 0);
                continue;
            }
            if (!isEdit && msg.wParam >= '1' && msg.wParam <= '7') {
                int catIndex = msg.wParam - '1';
                SendMessageA(hComboCat, CB_SETCURSEL, catIndex, 0);
                RefreshList();
                if (filtered_count > 0) {
                    SendMessageA(hList, LB_SETCURSEL, 0, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                }
                UpdateAppTitle(hwnd);
                continue;
            }
            if (msg.wParam == VK_ESCAPE) {
                SetWindowTextA(hSearch, "");
                SendMessageA(hComboCat, CB_SETCURSEL, 0, 0);
                RefreshList();
                if (filtered_count > 0) {
                    SendMessageA(hList, LB_SETCURSEL, 0, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                }
                SetFocus(hList);
                UpdateAppTitle(hwnd);
                continue;
            }
            if (!isEdit && msg.wParam == VK_DELETE) {
                SendMessageA(hwnd, WM_COMMAND, 1003, 0);
                continue;
            }
        }
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
