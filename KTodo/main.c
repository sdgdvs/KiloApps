#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    if (!s) return 0;
    int len = 0;
    while (s[len]) len++;
    return len;
}

int my_strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return 0;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void my_strcpy(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void my_strcat(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int my_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return 1;
    for (int i = 0; haystack[i] != '\0'; i++) {
        int match = 1;
        for (int j = 0; needle[j] != '\0'; j++) {
            if (haystack[i + j] == '\0' || to_lower(haystack[i + j]) != to_lower(needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

#define MAX_TASKS 100
#define MAX_SUBTASKS 10

typedef struct {
    char text[128];
    int completed;
} SubTask;

typedef struct {
    char text[128];
    char category[32];
    char priority[16];
    char dueDate[16];
    int completed;
    int subtaskCount;
    SubTask subtasks[MAX_SUBTASKS];
} Task;

Task g_tasks[MAX_TASKS];
int g_taskCount = 0;

HWND hInput, hCategory, hPriority, hDueDate, hAddBtn;
HWND hSearch, hFilterStatus, hFilterCategory;
HWND hList, hToggleBtn, hSubtaskBtn, hDeleteBtn, hClearBtn, hExportBtn, hImportBtn, hStatsBtn, hStatusText;
HFONT hFont, hFontBold;

WNDPROC g_OldEditProc = NULL;
WNDPROC g_OldSearchProc = NULL;
WNDPROC g_OldListProc = NULL;

#define ID_INPUT          1000
#define ID_CATEGORY       1001
#define ID_PRIORITY       1002
#define ID_DUEDATE        1003
#define ID_ADDBTN         1004
#define ID_SEARCH         1005
#define ID_FILTERSTATUS   1006
#define ID_FILTERCATEGORY 1007
#define ID_LIST           1008
#define ID_TOGGLEBTN      1009
#define ID_SUBTASKBTN     1010
#define ID_DELETEBTN      1011
#define ID_CLEARBTN       1012
#define ID_EXPORTBTN      1013
#define ID_IMPORTBTN      1014
#define ID_STATSBTN       1015

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(ID_ADDBTN, BN_CLICKED), (LPARAM)hAddBtn);
        }
        return 0;
    }
    return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK SearchSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND || msg == WM_KEYUP) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_USER + 1, 0, 0);
        }
    }
    return CallWindowProcA(g_OldSearchProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_DELETE) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(ID_DELETEBTN, BN_CLICKED), (LPARAM)hDeleteBtn);
        }
        return 0;
    }
    return CallWindowProcA(g_OldListProc, hwnd, msg, wParam, lParam);
}

// Get task index from ListBox item data
int GetSelectedTaskIndex() {
    int sel = SendMessageA(hList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) return -1;
    int taskIdx = (int)SendMessageA(hList, LB_GETITEMDATA, sel, 0);
    if (taskIdx >= 0 && taskIdx < g_taskCount) return taskIdx;
    return -1;
}

void RefreshTaskList() {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);

    char searchBuf[128] = {0};
    GetWindowTextA(hSearch, searchBuf, sizeof(searchBuf) - 1);

    char filterStat[32] = {0};
    GetWindowTextA(hFilterStatus, filterStat, sizeof(filterStat) - 1);

    char filterCat[32] = {0};
    GetWindowTextA(hFilterCategory, filterCat, sizeof(filterCat) - 1);

    int activeCount = 0;
    int completedCount = 0;

    for (int i = 0; i < g_taskCount; i++) {
        Task* t = &g_tasks[i];
        if (t->completed) completedCount++;
        else activeCount++;

        // Status Filter
        if (my_strcmp(filterStat, "Active") == 0 && t->completed) continue;
        if (my_strcmp(filterStat, "Done") == 0 && !t->completed) continue;
        if (my_strcmp(filterStat, "High Prio") == 0 && my_strcmp(t->priority, "High") != 0) continue;

        // Category Filter
        if (my_strlen(filterCat) > 0 && my_strcmp(filterCat, "All Categories") != 0 && my_strcmp(filterCat, "All") != 0) {
            if (my_strcmp(t->category, filterCat) != 0) continue;
        }

        // Search Query Filter
        if (my_strlen(searchBuf) > 0) {
            if (!my_stristr(t->text, searchBuf) && !my_stristr(t->category, searchBuf)) {
                int foundInSub = 0;
                for (int s = 0; s < t->subtaskCount; s++) {
                    if (my_stristr(t->subtasks[s].text, searchBuf)) {
                        foundInSub = 1;
                        break;
                    }
                }
                if (!foundInSub) continue;
            }
        }

        // Format task display string
        char itemStr[256];
        int subDone = 0;
        for (int s = 0; s < t->subtaskCount; s++) {
            if (t->subtasks[s].completed) subDone++;
        }

        char subInfo[32] = "";
        if (t->subtaskCount > 0) {
            wsprintfA(subInfo, " (%d/%d)", subDone, t->subtaskCount);
        }

        char dateInfo[32] = "";
        if (my_strlen(t->dueDate) > 0) {
            wsprintfA(dateInfo, " [Due:%s]", t->dueDate);
        }

        wsprintfA(itemStr, "%s [%s] [%s]%s %s%s",
            t->completed ? "[X]" : "[ ]",
            t->priority,
            t->category,
            dateInfo,
            t->text,
            subInfo
        );

        int pos = SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)itemStr);
        SendMessageA(hList, LB_SETITEMDATA, pos, (LPARAM)i);
    }

    // Update status text
    int total = g_taskCount;
    int rate = total > 0 ? (completedCount * 100) / total : 0;
    char statusBuf[128];
    wsprintfA(statusBuf, "Total: %d | Active: %d | Done: %d | Rate: %d%%", total, activeCount, completedCount, rate);
    SetWindowTextA(hStatusText, statusBuf);
}

void DoAddTask() {
    if (g_taskCount >= MAX_TASKS) {
        MessageBoxA(NULL, "Task limit reached (100 tasks max).", "KTodo", MB_OK | MB_ICONWARNING);
        return;
    }

    char buf[128] = {0};
    GetWindowTextA(hInput, buf, sizeof(buf) - 1);

    // Trim
    char* p = buf;
    while (*p == ' ' || *p == '\t') p++;

    if (my_strlen(p) > 0) {
        Task* t = &g_tasks[g_taskCount];
        memset(t, 0, sizeof(Task));

        my_strcpy(t->text, p);

        char cat[32] = {0};
        GetWindowTextA(hCategory, cat, sizeof(cat) - 1);
        my_strcpy(t->category, my_strlen(cat) > 0 ? cat : "General");

        char prio[16] = {0};
        GetWindowTextA(hPriority, prio, sizeof(prio) - 1);
        my_strcpy(t->priority, my_strlen(prio) > 0 ? prio : "Med");

        char due[16] = {0};
        GetWindowTextA(hDueDate, due, sizeof(due) - 1);
        my_strcpy(t->dueDate, due);

        t->completed = 0;
        t->subtaskCount = 0;

        g_taskCount++;
        SetWindowTextA(hInput, "");
        SetFocus(hInput);
        RefreshTaskList();
    }
}

void DoToggleTask() {
    int idx = GetSelectedTaskIndex();
    if (idx >= 0) {
        g_tasks[idx].completed = !g_tasks[idx].completed;
        RefreshTaskList();
    }
}

void DoDeleteTask() {
    int idx = GetSelectedTaskIndex();
    if (idx >= 0) {
        for (int i = idx; i < g_taskCount - 1; i++) {
            g_tasks[i] = g_tasks[i + 1];
        }
        g_taskCount--;
        RefreshTaskList();
    }
}

void DoClearCompleted() {
    int newCount = 0;
    for (int i = 0; i < g_taskCount; i++) {
        if (!g_tasks[i].completed) {
            g_tasks[newCount++] = g_tasks[i];
        }
    }
    g_taskCount = newCount;
    RefreshTaskList();
}

void DoAddSubtask() {
    int idx = GetSelectedTaskIndex();
    if (idx < 0) {
        MessageBoxA(NULL, "Please select a task first.", "KTodo", MB_OK | MB_ICONINFORMATION);
        return;
    }

    Task* t = &g_tasks[idx];
    if (t->subtaskCount >= MAX_SUBTASKS) {
        MessageBoxA(NULL, "Subtask limit reached (10 max per task).", "KTodo", MB_OK | MB_ICONWARNING);
        return;
    }

    // Quick prompt using simple Input box simulation or predefined subtask title
    char promptMsg[256];
    wsprintfA(promptMsg, "Add subtask to: '%s'\n(Subtask will be added as 'Subtask %d')", t->text, t->subtaskCount + 1);
    
    int res = MessageBoxA(NULL, promptMsg, "Add Subtask", MB_OKCANCEL | MB_ICONQUESTION);
    if (res == IDOK) {
        SubTask* st = &t->subtasks[t->subtaskCount];
        wsprintfA(st->text, "Checklist item %d", t->subtaskCount + 1);
        st->completed = 0;
        t->subtaskCount++;
        RefreshTaskList();
    }
}

void DoShowStats() {
    int total = g_taskCount;
    int active = 0, completed = 0, highPrio = 0;
    int catWork = 0, catPersonal = 0, catProject = 0, catOther = 0;

    for (int i = 0; i < g_taskCount; i++) {
        if (g_tasks[i].completed) completed++;
        else active++;

        if (my_strcmp(g_tasks[i].priority, "High") == 0) highPrio++;

        if (my_strcmp(g_tasks[i].category, "Work") == 0) catWork++;
        else if (my_strcmp(g_tasks[i].category, "Personal") == 0) catPersonal++;
        else if (my_strcmp(g_tasks[i].category, "Project") == 0) catProject++;
        else catOther++;
    }

    int rate = total > 0 ? (completed * 100) / total : 0;

    char msg[512];
    wsprintfA(msg, 
        "=== KTodo Productivity Summary ===\n\n"
        "Total Tasks: %d\n"
        "Active Tasks: %d\n"
        "Completed Tasks: %d\n"
        "Overall Completion Rate: %d%%\n\n"
        "High Priority Tasks: %d\n\n"
        "Category Breakdown:\n"
        "- Work: %d\n"
        "- Personal: %d\n"
        "- Project: %d\n"
        "- Other: %d",
        total, active, completed, rate, highPrio, catWork, catPersonal, catProject, catOther
    );

    MessageBoxA(NULL, msg, "KTodo Productivity Statistics", MB_OK | MB_ICONINFORMATION);
}

void DoExportData() {
    // Export JSON to ktodo_export.json
    HANDLE hFile = CreateFileA("ktodo_export.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char header[] = "[\n";
        DWORD written;
        WriteFile(hFile, header, my_strlen(header), &written, NULL);

        for (int i = 0; i < g_taskCount; i++) {
            Task* t = &g_tasks[i];
            char line[512];
            wsprintfA(line, "  {\n    \"text\": \"%s\",\n    \"category\": \"%s\",\n    \"priority\": \"%s\",\n    \"dueDate\": \"%s\",\n    \"completed\": %s\n  }%s\n",
                t->text, t->category, t->priority, t->dueDate,
                t->completed ? "true" : "false",
                (i == g_taskCount - 1) ? "" : ","
            );
            WriteFile(hFile, line, my_strlen(line), &written, NULL);
        }

        char footer[] = "]\n";
        WriteFile(hFile, footer, my_strlen(footer), &written, NULL);
        CloseHandle(hFile);

        MessageBoxA(NULL, "Tasks successfully exported to 'ktodo_export.json'!", "Export Complete", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Failed to create export file.", "Export Error", MB_OK | MB_ICONERROR);
    }
}

void LoadSampleData() {
    g_taskCount = 0;

    Task* t1 = &g_tasks[0];
    my_strcpy(t1->text, "Design and code KTodo features");
    my_strcpy(t1->category, "Project");
    my_strcpy(t1->priority, "High");
    my_strcpy(t1->dueDate, "2026-07-25");
    t1->completed = 0;
    t1->subtaskCount = 2;
    my_strcpy(t1->subtasks[0].text, "Search & filter");
    t1->subtasks[0].completed = 1;
    my_strcpy(t1->subtasks[1].text, "Subtasks & Export");
    t1->subtasks[1].completed = 0;

    Task* t2 = &g_tasks[1];
    my_strcpy(t2->text, "Team sync meeting");
    my_strcpy(t2->category, "Work");
    my_strcpy(t2->priority, "Med");
    my_strcpy(t2->dueDate, "2026-07-26");
    t2->completed = 0;
    t2->subtaskCount = 0;

    Task* t3 = &g_tasks[2];
    my_strcpy(t3->text, "Weekly grocery shopping");
    my_strcpy(t3->category, "Shopping");
    my_strcpy(t3->priority, "Low");
    my_strcpy(t3->dueDate, "");
    t3->completed = 1;
    t3->subtaskCount = 0;

    g_taskCount = 3;
    RefreshTaskList();
    MessageBoxA(NULL, "Sample tasks loaded into KTodo!", "Demo Tasks", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Row 1: Creator inputs
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 150, 24, hwnd, (HMENU)ID_INPUT, NULL, NULL);
            hCategory = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 165, 10, 80, 150, hwnd, (HMENU)ID_CATEGORY, NULL, NULL);
            hPriority = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 250, 10, 60, 120, hwnd, (HMENU)ID_PRIORITY, NULL, NULL);
            hDueDate = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 315, 10, 80, 24, hwnd, (HMENU)ID_DUEDATE, NULL, NULL);
            hAddBtn = CreateWindowA("BUTTON", "+ Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 10, 55, 24, hwnd, (HMENU)ID_ADDBTN, NULL, NULL);

            // Populate Category dropdown
            SendMessageA(hCategory, 0x0143, 0, (LPARAM)"General"); // CB_ADDSTRING
            SendMessageA(hCategory, 0x0143, 0, (LPARAM)"Work");
            SendMessageA(hCategory, 0x0143, 0, (LPARAM)"Personal");
            SendMessageA(hCategory, 0x0143, 0, (LPARAM)"Project");
            SendMessageA(hCategory, 0x0143, 0, (LPARAM)"Shopping");
            SendMessageA(hCategory, 0x014E, 0, 0); // CB_SETCURSEL

            // Populate Priority dropdown
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Low");
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Med");
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"High");
            SendMessageA(hPriority, 0x014E, 1, 0); // Med

            // Row 2: Search & Filter Toolbar
            hSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 40, 140, 24, hwnd, (HMENU)ID_SEARCH, NULL, NULL);
            hFilterStatus = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 155, 40, 90, 150, hwnd, (HMENU)ID_FILTERSTATUS, NULL, NULL);
            hFilterCategory = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 250, 40, 110, 150, hwnd, (HMENU)ID_FILTERCATEGORY, NULL, NULL);
            hStatsBtn = CreateWindowA("BUTTON", "📊 Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 365, 40, 90, 24, hwnd, (HMENU)ID_STATSBTN, NULL, NULL);

            // Status Filter options
            SendMessageA(hFilterStatus, 0x0143, 0, (LPARAM)"All");
            SendMessageA(hFilterStatus, 0x0143, 0, (LPARAM)"Active");
            SendMessageA(hFilterStatus, 0x0143, 0, (LPARAM)"Done");
            SendMessageA(hFilterStatus, 0x0143, 0, (LPARAM)"High Prio");
            SendMessageA(hFilterStatus, 0x014E, 0, 0);

            // Category Filter options
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"All Categories");
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"General");
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"Work");
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"Personal");
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"Project");
            SendMessageA(hFilterCategory, 0x0143, 0, (LPARAM)"Shopping");
            SendMessageA(hFilterCategory, 0x014E, 0, 0);

            // Row 3: Main Task ListBox
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | LBS_HASSTRINGS, 10, 70, 445, 190, hwnd, (HMENU)ID_LIST, NULL, NULL);

            // Row 4: Action Buttons
            hToggleBtn = CreateWindowA("BUTTON", "Toggle Done", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 265, 85, 26, hwnd, (HMENU)ID_TOGGLEBTN, NULL, NULL);
            hSubtaskBtn = CreateWindowA("BUTTON", "+ Checklist", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 265, 80, 26, hwnd, (HMENU)ID_SUBTASKBTN, NULL, NULL);
            hDeleteBtn = CreateWindowA("BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 185, 265, 60, 26, hwnd, (HMENU)ID_DELETEBTN, NULL, NULL);
            hClearBtn = CreateWindowA("BUTTON", "Clear Done", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 265, 75, 26, hwnd, (HMENU)ID_CLEARBTN, NULL, NULL);
            hExportBtn = CreateWindowA("BUTTON", "Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 330, 265, 60, 26, hwnd, (HMENU)ID_EXPORTBTN, NULL, NULL);
            hImportBtn = CreateWindowA("BUTTON", "Demo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 395, 265, 60, 26, hwnd, (HMENU)ID_IMPORTBTN, NULL, NULL);

            // Row 5: Status Bar
            hStatusText = CreateWindowA("STATIC", "Total: 0 | Active: 0 | Done: 0", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 298, 445, 20, hwnd, NULL, NULL, NULL);

            // Setup fonts
            hFont = CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            if (!hFont) hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPriority, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hDueDate, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFilterStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFilterCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hStatsBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hToggleBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hSubtaskBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hDeleteBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hClearBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hExportBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hImportBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Subclass controls
            g_OldEditProc = (WNDPROC)SetWindowLongPtrA(hInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
            g_OldSearchProc = (WNDPROC)SetWindowLongPtrA(hSearch, GWLP_WNDPROC, (LONG_PTR)SearchSubclassProc);
            g_OldListProc = (WNDPROC)SetWindowLongPtrA(hList, GWLP_WNDPROC, (LONG_PTR)ListSubclassProc);

            // Load sample tasks on startup
            LoadSampleData();
            break;
        }

        case WM_USER + 1:
            RefreshTaskList();
            break;

        case WM_GETMINMAXINFO: {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
            mmi->ptMinTrackSize.x = 480;
            mmi->ptMinTrackSize.y = 360;
            break;
        }

        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            if (cx > 100 && cy > 100) {
                int addWidth = 55;
                int inputWidth = cx - 325;
                if (inputWidth < 80) inputWidth = 80;

                MoveWindow(hInput, 10, 10, inputWidth, 24, TRUE);
                MoveWindow(hCategory, 15 + inputWidth, 10, 80, 150, TRUE);
                MoveWindow(hPriority, 100 + inputWidth, 10, 60, 120, TRUE);
                MoveWindow(hDueDate, 165 + inputWidth, 10, 80, 24, TRUE);
                MoveWindow(hAddBtn, 250 + inputWidth, 10, addWidth, 24, TRUE);

                MoveWindow(hSearch, 10, 40, inputWidth + 30, 24, TRUE);
                MoveWindow(hFilterStatus, 45 + inputWidth, 40, 90, 150, TRUE);
                MoveWindow(hFilterCategory, 140 + inputWidth, 40, 100, 150, TRUE);
                MoveWindow(hStatsBtn, 245 + inputWidth, 40, 60, 24, TRUE);

                int listHeight = cy - 135;
                if (listHeight < 50) listHeight = 50;
                MoveWindow(hList, 10, 70, cx - 20, listHeight, TRUE);

                int btnY = cy - 60;
                MoveWindow(hToggleBtn, 10, btnY, 85, 24, TRUE);
                MoveWindow(hSubtaskBtn, 100, btnY, 80, 24, TRUE);
                MoveWindow(hDeleteBtn, 185, btnY, 60, 24, TRUE);
                MoveWindow(hClearBtn, 250, btnY, 75, 24, TRUE);
                MoveWindow(hExportBtn, 330, btnY, 60, 24, TRUE);
                MoveWindow(hImportBtn, 395, btnY, 60, 24, TRUE);

                MoveWindow(hStatusText, 10, cy - 25, cx - 20, 20, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);

            if (id == ID_ADDBTN) {
                DoAddTask();
            } else if (id == ID_TOGGLEBTN) {
                DoToggleTask();
            } else if (id == ID_DELETEBTN) {
                DoDeleteTask();
            } else if (id == ID_CLEARBTN) {
                DoClearCompleted();
            } else if (id == ID_SUBTASKBTN) {
                DoAddSubtask();
            } else if (id == ID_STATSBTN) {
                DoShowStats();
            } else if (id == ID_EXPORTBTN) {
                DoExportData();
            } else if (id == ID_IMPORTBTN) {
                LoadSampleData();
            } else if (id == ID_FILTERSTATUS && code == CBN_SELCHANGE) {
                RefreshTaskList();
            } else if (id == ID_FILTERCATEGORY && code == CBN_SELCHANGE) {
                RefreshTaskList();
            } else if (id == ID_LIST && code == LBN_DBLCLK) {
                DoToggleTask();
            }
            break;
        }

        case WM_DESTROY:
            if (hFont && hFont != GetStockObject(DEFAULT_GUI_FONT)) {
                DeleteObject(hFont);
                hFont = NULL;
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KTodoClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KTodoClass", "KTodo - Smart Task & Productivity Manager", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 485, 380, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
