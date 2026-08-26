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
    if (!s1 || !s2) return -1;
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
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

#define ID_BTN_SEARCH 1001
#define ID_BTN_TRAVEL_OFFICE 1002
#define ID_BTN_TRAVEL_MANOR 1003
#define ID_BTN_TRAVEL_DOCKS 1004
#define ID_BTN_START 1005
#define ID_LIST_SUSPECTS 1006
#define ID_LIST_CLUES 1007
#define ID_BTN_INTERROGATE 1008
#define ID_BTN_ASK_ALIBI 1009
#define ID_BTN_PRESENT_CLUE 1010
#define ID_BTN_END_INT 1011
#define ID_BTN_LAB 1012
#define ID_BTN_LEAVE_LAB 1013
#define ID_BTN_ANALYZE 1014
#define ID_BTN_SCAN_11 1015
#define ID_BTN_SCAN_7 1016
#define ID_BTN_SCAN_M3 1017
#define ID_LIST_UNANALYZED 1018

#define ID_BTN_ACCUSE 1019
#define ID_BTN_SUBMIT_ACCUSE 1020
#define ID_BTN_CANCEL_ACCUSE 1021
#define ID_CMB_SUSPECT 1022
#define ID_CMB_MOTIVE 1023
#define ID_CMB_WEAPON 1024

HWND hTitle, hLocName, hLocDesc, hBtnSearch, hBtnTravelOffice, hBtnTravelManor, hBtnTravelDocks, hBtnTravelCasino, hBtnTravelStation;
HWND hSuspectTitle, hListSuspects, hClueTitle, hListClues, hUnanalyzedTitle, hListUnanalyzed;
HWND hStartPanel, hBtnStart, hBtnStartMed, hBtnStartHard, hStartDesc;
HWND hBtnInterrogate, hIntDesc, hBtnAskAlibi, hBtnPresentClue, hBtnEndInt;
HWND hBtnLab, hLabTitle, hBtnAnalyze, hBtnLeaveLab;
HWND hScanDesc, hBtnScan11, hBtnScan7, hBtnScanM3;
HWND hBtnAccuse, hAccuseTitle, hAccuseDesc, hBtnSubmitAccuse, hBtnCancelAccuse, hCmbSuspect, hCmbMotive, hCmbWeapon;
HWND hTimeLeft;
HFONT hFont, hFontBold, hFontTitle;

int timeLeft = 12;
int activeItems = 3;

int currentState = 0; // 0 = start, 1 = playing
int currentLocation = 0;

typedef struct {
    char name[32];
    char desc[256];
    char clue[128];
    int searched;
    int clueFound;
    int suspectIdx;
} Location;

Location locations[5] = {
    {"Office", "Your dingy office. Dust motes dance in the light filtering through the blinds.", "", 0, 0, -1},
    {"The Manor", "The sprawling estate where the victim was found. Yellow police tape blocks the study.", "", 0, 0, -1},
    {"The Docks", "Smells like salt and secrets. The fog here is thick enough to cut with a knife.", "", 0, 0, -1},
    {"The Casino", "Flashy lights and the smell of cheap gin. Money changes hands quickly here.", "", 0, 0, -1},
    {"Train Station", "Bustling with travelers, but a secluded corner holds dark secrets.", "", 0, 0, -1}
};

char* suspects[5] = {
    "Mr. Black",
    "Miss Scarlet",
    "Colonel Mustard",
    "Mrs. White",
    "Professor Plum"
};

typedef struct {
    int killerIdx;
    int motiveIdx;
    int weaponIdx;
} Solution;

Solution currentSolution;

int suspectPatience[5];
char suspectAlibis[5][128];

typedef struct {
    int locIdx;
    char clue[128];
} Unanalyzed;
Unanalyzed unanalyzed[5];
int numUnanalyzed = 0;

int scanTarget = 0;
int scanCurrent = 0;
int scanMoves = 0;
int scanItemIdx = 0;

unsigned int my_seed = 0;
int my_rand() {
    my_seed = my_seed * 1103515245 + 12345;
    return (unsigned int)(my_seed / 65536) % 32768;
}

char* killerClues[5] = {
    "A cufflink with an onyx stone.",
    "A faint scent of expensive rose perfume.",
    "A polished brass military button.",
    "A pristine white glove.",
    "A broken pair of spectacles."
};

char* motiveClues[] = {
    "A crumpled letter swearing vengeance.",
    "An altered will leaving everything to the killer.",
    "A torn photograph of a happy couple, with one face scratched out."
};

char* weaponClues[] = {
    "A spent .38 caliber shell casing.",
    "A discarded vial smelling of bitter almonds.",
    "A heavy, blood-stained lead pipe hidden in the corner."
};

void GenerateMystery() {
    my_seed = GetTickCount();
    currentSolution.killerIdx = my_rand() % activeItems;
    currentSolution.motiveIdx = my_rand() % 3;
    currentSolution.weaponIdx = my_rand() % 3;
    
    char* clues[5] = {"", "", "", "", ""};
    clues[0] = killerClues[currentSolution.killerIdx];
    clues[1] = motiveClues[currentSolution.motiveIdx];
    clues[2] = weaponClues[currentSolution.weaponIdx];
    
    int sus[5];
    for (int i=0; i<activeItems; i++) sus[i] = i;
    
    for (int i = activeItems - 1; i > 0; i--) {
        int j = my_rand() % (i + 1);
        char* temp = clues[i];
        clues[i] = clues[j];
        clues[j] = temp;
        
        int ts = sus[i];
        sus[i] = sus[j];
        sus[j] = ts;
    }
    
    for (int i = 0; i < activeItems; i++) {
        my_strcpy(locations[i].clue, clues[i]);
        locations[i].searched = 0;
        locations[i].clueFound = 0;
        locations[i].suspectIdx = sus[i];
    }
    
    for (int i = 0; i < activeItems; i++) {
        suspectPatience[i] = 3;
    }

    int killerIdx = currentSolution.killerIdx;
    int innocent[4];
    int numInnocent = 0;
    for (int i=0; i<activeItems; i++) {
        if (i != killerIdx) innocent[numInnocent++] = i;
    }

    for (int i=0; i<numInnocent; i++) {
        wsprintfA(suspectAlibis[innocent[i]], "\"I was with %s the whole time.\"", suspects[innocent[(i+1)%numInnocent]]);
    }
    wsprintfA(suspectAlibis[killerIdx], "\"I was with %s.\"", suspects[innocent[0]]);
}

void UpdateUI() {
    if (currentState == 0) {
        ShowWindow(hStartPanel, SW_SHOW);
        ShowWindow(hStartDesc, SW_SHOW);
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnStartMed, SW_SHOW);
        ShowWindow(hBtnStartHard, SW_SHOW);
        
        ShowWindow(hTimeLeft, SW_HIDE);
        ShowWindow(hTitle, SW_HIDE);
        ShowWindow(hLocName, SW_HIDE);
        ShowWindow(hLocDesc, SW_HIDE);
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnTravelCasino, SW_HIDE);
        ShowWindow(hBtnTravelStation, SW_HIDE);
        ShowWindow(hSuspectTitle, SW_HIDE);
        ShowWindow(hListSuspects, SW_HIDE);
        ShowWindow(hClueTitle, SW_HIDE);
        ShowWindow(hListClues, SW_HIDE);
        ShowWindow(hUnanalyzedTitle, SW_HIDE);
        ShowWindow(hListUnanalyzed, SW_HIDE);
        
        ShowWindow(hIntDesc, SW_HIDE);
        ShowWindow(hBtnAskAlibi, SW_HIDE);
        ShowWindow(hBtnPresentClue, SW_HIDE);
        ShowWindow(hBtnEndInt, SW_HIDE);

        ShowWindow(hBtnLab, SW_HIDE);
        ShowWindow(hLabTitle, SW_HIDE);
        ShowWindow(hBtnAnalyze, SW_HIDE);
        ShowWindow(hBtnLeaveLab, SW_HIDE);
        
        ShowWindow(hScanDesc, SW_HIDE);
        ShowWindow(hBtnScan11, SW_HIDE);
        ShowWindow(hBtnScan7, SW_HIDE);
        ShowWindow(hBtnScanM3, SW_HIDE);
        
        ShowWindow(hBtnAccuse, SW_HIDE);
        ShowWindow(hAccuseTitle, SW_HIDE);
        ShowWindow(hAccuseDesc, SW_HIDE);
        ShowWindow(hBtnSubmitAccuse, SW_HIDE);
        ShowWindow(hBtnCancelAccuse, SW_HIDE);
        ShowWindow(hCmbSuspect, SW_HIDE);
        ShowWindow(hCmbMotive, SW_HIDE);
        ShowWindow(hCmbWeapon, SW_HIDE);
    } else if (currentState == 1) {
        ShowWindow(hStartPanel, SW_HIDE);
        ShowWindow(hStartDesc, SW_HIDE);
        ShowWindow(hBtnStart, SW_HIDE);
        ShowWindow(hBtnStartMed, SW_HIDE);
        ShowWindow(hBtnStartHard, SW_HIDE);
        
        ShowWindow(hTimeLeft, SW_SHOW);
        ShowWindow(hTitle, SW_SHOW);
        ShowWindow(hLocName, SW_SHOW);
        ShowWindow(hLocDesc, SW_SHOW);
        ShowWindow(hBtnSearch, SW_SHOW);
        ShowWindow(hBtnInterrogate, SW_SHOW);
        ShowWindow(hBtnTravelOffice, SW_SHOW);
        ShowWindow(hBtnTravelManor, SW_SHOW);
        ShowWindow(hBtnTravelDocks, SW_SHOW);
        ShowWindow(hBtnTravelCasino, activeItems > 3 ? SW_SHOW : SW_HIDE);
        ShowWindow(hBtnTravelStation, activeItems > 4 ? SW_SHOW : SW_HIDE);
        ShowWindow(hSuspectTitle, SW_SHOW);
        ShowWindow(hListSuspects, SW_SHOW);
        ShowWindow(hClueTitle, SW_SHOW);
        ShowWindow(hListClues, SW_SHOW);
        ShowWindow(hUnanalyzedTitle, SW_SHOW);
        ShowWindow(hListUnanalyzed, SW_SHOW);
        
        ShowWindow(hIntDesc, SW_HIDE);
        ShowWindow(hBtnAskAlibi, SW_HIDE);
        ShowWindow(hBtnPresentClue, SW_HIDE);
        ShowWindow(hBtnEndInt, SW_HIDE);
        
        ShowWindow(hBtnLab, SW_SHOW);
        ShowWindow(hLabTitle, SW_HIDE);
        ShowWindow(hBtnAnalyze, SW_HIDE);
        ShowWindow(hBtnLeaveLab, SW_HIDE);
        
        ShowWindow(hScanDesc, SW_HIDE);
        ShowWindow(hBtnScan11, SW_HIDE);
        ShowWindow(hBtnScan7, SW_HIDE);
        ShowWindow(hBtnScanM3, SW_HIDE);

        ShowWindow(hBtnAccuse, SW_SHOW);
        ShowWindow(hAccuseTitle, SW_HIDE);
        ShowWindow(hAccuseDesc, SW_HIDE);
        ShowWindow(hBtnSubmitAccuse, SW_HIDE);
        ShowWindow(hBtnCancelAccuse, SW_HIDE);
        ShowWindow(hCmbSuspect, SW_HIDE);
        ShowWindow(hCmbMotive, SW_HIDE);
        ShowWindow(hCmbWeapon, SW_HIDE);
        
        char locNameBuf[64];
        wsprintfA(locNameBuf, "Location: %s", locations[currentLocation].name);
        SetWindowTextA(hLocName, locNameBuf);
        SetWindowTextA(hLocDesc, locations[currentLocation].desc);
        
        if (locations[currentLocation].searched) {
            EnableWindow(hBtnSearch, FALSE);
            SetWindowTextA(hBtnSearch, "Already searched");
        } else {
            EnableWindow(hBtnSearch, TRUE);
            SetWindowTextA(hBtnSearch, "Search for Clues");
        }
        
        char intBuf[64];
        wsprintfA(intBuf, "Interrogate %s", suspects[locations[currentLocation].suspectIdx]);
        SetWindowTextA(hBtnInterrogate, intBuf);
        
        EnableWindow(hBtnTravelOffice, currentLocation != 0);
        EnableWindow(hBtnTravelManor, currentLocation != 1);
        EnableWindow(hBtnTravelDocks, currentLocation != 2);
        EnableWindow(hBtnTravelCasino, currentLocation != 3);
        EnableWindow(hBtnTravelStation, currentLocation != 4);
    } else if (currentState >= 2) {
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnTravelCasino, SW_HIDE);
        ShowWindow(hBtnTravelStation, SW_HIDE);
        ShowWindow(hBtnLab, SW_HIDE);
        ShowWindow(hBtnAccuse, SW_HIDE);
        
        ShowWindow(hIntDesc, SW_SHOW);
        ShowWindow(hBtnAskAlibi, SW_SHOW);
        ShowWindow(hBtnPresentClue, SW_SHOW);
        ShowWindow(hBtnEndInt, SW_SHOW);
    } else if (currentState == 3) {
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnLab, SW_HIDE);
        ShowWindow(hBtnAccuse, SW_HIDE);
        
        ShowWindow(hIntDesc, SW_HIDE);
        ShowWindow(hBtnAskAlibi, SW_HIDE);
        ShowWindow(hBtnPresentClue, SW_HIDE);
        ShowWindow(hBtnEndInt, SW_HIDE);
        
        ShowWindow(hLabTitle, SW_SHOW);
        ShowWindow(hBtnAnalyze, SW_SHOW);
        ShowWindow(hBtnLeaveLab, SW_SHOW);
        
        ShowWindow(hScanDesc, SW_HIDE);
        ShowWindow(hBtnScan11, SW_HIDE);
        ShowWindow(hBtnScan7, SW_HIDE);
        ShowWindow(hBtnScanM3, SW_HIDE);
    } else if (currentState == 4) {
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnLab, SW_HIDE);
        ShowWindow(hBtnAccuse, SW_HIDE);
        
        ShowWindow(hIntDesc, SW_HIDE);
        ShowWindow(hBtnAskAlibi, SW_HIDE);
        ShowWindow(hBtnPresentClue, SW_HIDE);
        ShowWindow(hBtnEndInt, SW_HIDE);
        
        ShowWindow(hLabTitle, SW_HIDE);
        ShowWindow(hBtnAnalyze, SW_HIDE);
        ShowWindow(hBtnLeaveLab, SW_SHOW);
        
        ShowWindow(hScanDesc, SW_SHOW);
        ShowWindow(hBtnScan11, SW_SHOW);
        ShowWindow(hBtnScan7, SW_SHOW);
        ShowWindow(hBtnScanM3, SW_SHOW);
        
        char scanBuf[256];
        wsprintfA(scanBuf, "Calibrate Scanner to reveal clue.\nTarget: %d | Current: %d | Moves: %d", scanTarget, scanCurrent, scanMoves);
        SetWindowTextA(hScanDesc, scanBuf);
    } else if (currentState == 5) {
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnLab, SW_HIDE);
        ShowWindow(hBtnAccuse, SW_HIDE);
        
        ShowWindow(hIntDesc, SW_HIDE);
        ShowWindow(hBtnAskAlibi, SW_HIDE);
        ShowWindow(hBtnPresentClue, SW_HIDE);
        ShowWindow(hBtnEndInt, SW_HIDE);
        
        ShowWindow(hLabTitle, SW_HIDE);
        ShowWindow(hBtnAnalyze, SW_HIDE);
        ShowWindow(hBtnLeaveLab, SW_HIDE);
        
        ShowWindow(hScanDesc, SW_HIDE);
        ShowWindow(hBtnScan11, SW_HIDE);
        ShowWindow(hBtnScan7, SW_HIDE);
        ShowWindow(hBtnScanM3, SW_HIDE);
        
        ShowWindow(hAccuseTitle, SW_SHOW);
        ShowWindow(hAccuseDesc, SW_SHOW);
        ShowWindow(hBtnSubmitAccuse, SW_SHOW);
        ShowWindow(hBtnCancelAccuse, SW_SHOW);
        ShowWindow(hCmbSuspect, SW_SHOW);
        ShowWindow(hCmbMotive, SW_SHOW);
        ShowWindow(hCmbWeapon, SW_SHOW);
    }
}

void StartGame(int items, int time) {
    activeItems = items;
    GenerateMystery();
    currentState = 1;
    timeLeft = time;
    char timeBuf[64];
    wsprintfA(timeBuf, "Time Left: %dh", timeLeft);
    SetWindowTextA(hTimeLeft, timeBuf);
    numUnanalyzed = 0;
    currentLocation = 0;
    SendMessageA(hListSuspects, LB_RESETCONTENT, 0, 0);
    SendMessageA(hListClues, LB_RESETCONTENT, 0, 0);
    SendMessageA(hListUnanalyzed, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < activeItems; i++) {
        SendMessageA(hListSuspects, LB_ADDSTRING, 0, (LPARAM)suspects[i]);
    }
    SendMessageA(hCmbSuspect, CB_RESETCONTENT, 0, 0);
    for (int i = 0; i < activeItems; i++) {
        SendMessageA(hCmbSuspect, CB_ADDSTRING, 0, (LPARAM)suspects[i]);
    }
    SendMessageA(hCmbSuspect, CB_SETCURSEL, 0, 0);

    UpdateUI();
}

int AdvanceTime(HWND hwnd, int hours) {
    timeLeft -= hours;
    if (timeLeft < 0) timeLeft = 0;
    
    char timeBuf[64];
    wsprintfA(timeBuf, "Time Left: %dh", timeLeft);
    SetWindowTextA(hTimeLeft, timeBuf);
    
    if (timeLeft <= 0) {
        MessageBoxA(hwnd, "Time's up! The killer has escaped. GAME OVER. (Restart to play again)", "Game Over", MB_OK | MB_ICONERROR);
        currentState = 0;
        UpdateUI();
        return 1;
    }
    return 0;
}

void SearchLocation(HWND hwnd) {
    if (!locations[currentLocation].searched) {
        if (AdvanceTime(hwnd, 2)) return;
        locations[currentLocation].searched = 1;
        if (my_strlen(locations[currentLocation].clue) > 0) {
            locations[currentLocation].clueFound = 1;
            
            unanalyzed[numUnanalyzed].locIdx = currentLocation;
            my_strcpy(unanalyzed[numUnanalyzed].clue, locations[currentLocation].clue);
            numUnanalyzed++;
            
            char itemBuf[256];
            wsprintfA(itemBuf, "Object from %s", locations[currentLocation].name);
            SendMessageA(hListUnanalyzed, LB_ADDSTRING, 0, (LPARAM)itemBuf);
            
            MessageBoxA(hwnd, "You found a mysterious object! Take it to the Evidence Lab to analyze.", "Object Found", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, "You didn't find anything useful here.", "Nothing Found", MB_OK | MB_ICONINFORMATION);
        }
        UpdateUI();
    }
}

void Travel(int loc) {
    currentLocation = loc;
    UpdateUI();
}

#define ID_BTN_START_MED 1025
#define ID_BTN_START_HARD 1026
#define ID_BTN_TRAVEL_CASINO 1027
#define ID_BTN_TRAVEL_STATION 1028

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontBold = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontTitle = CreateFontA(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");

            // Start Screen
            hStartPanel = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hStartDesc = CreateWindowA("STATIC", "Detective, a murder has occurred.", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnStart = CreateWindowA("BUTTON", "Start Easy (3 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START, NULL, NULL);
            hBtnStartMed = CreateWindowA("BUTTON", "Start Medium (4 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START_MED, NULL, NULL);
            hBtnStartHard = CreateWindowA("BUTTON", "Start Hard (5 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START_HARD, NULL, NULL);

            // Game Screen
            hTimeLeft = CreateWindowA("STATIC", "Time Left: 12h", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hTitle = CreateWindowA("STATIC", "KMystery", WS_CHILD | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocName = CreateWindowA("STATIC", "Location:", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnSearch = CreateWindowA("BUTTON", "Search for Clues", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SEARCH, NULL, NULL);
            hBtnInterrogate = CreateWindowA("BUTTON", "Interrogate", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_INTERROGATE, NULL, NULL);
            
            hBtnTravelOffice = CreateWindowA("BUTTON", "Travel to Office", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_OFFICE, NULL, NULL);
            hBtnTravelManor = CreateWindowA("BUTTON", "Travel to The Manor", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_MANOR, NULL, NULL);
            hBtnTravelDocks = CreateWindowA("BUTTON", "Travel to The Docks", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_DOCKS, NULL, NULL);
            hBtnTravelCasino = CreateWindowA("BUTTON", "Travel to The Casino", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_CASINO, NULL, NULL);
            hBtnTravelStation = CreateWindowA("BUTTON", "Travel to Train Station", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_STATION, NULL, NULL);
            
            hBtnLab = CreateWindowA("BUTTON", "Evidence Lab", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LAB, NULL, NULL);
            hLabTitle = CreateWindowA("STATIC", "Evidence Lab", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnAnalyze = CreateWindowA("BUTTON", "Analyze Selected Object", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_ANALYZE, NULL, NULL);
            hBtnLeaveLab = CreateWindowA("BUTTON", "Leave Lab", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LEAVE_LAB, NULL, NULL);
            
            hScanDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnScan11 = CreateWindowA("BUTTON", "+11", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_11, NULL, NULL);
            hBtnScan7 = CreateWindowA("BUTTON", "+7", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_7, NULL, NULL);
            hBtnScanM3 = CreateWindowA("BUTTON", "-3", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_M3, NULL, NULL);

            hBtnAccuse = CreateWindowA("BUTTON", "Accuse Someone", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_ACCUSE, NULL, NULL);
            hAccuseTitle = CreateWindowA("STATIC", "Final Accusation", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hAccuseDesc = CreateWindowA("STATIC", "Present your findings. If you are wrong, you will be fired.", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnSubmitAccuse = CreateWindowA("BUTTON", "Submit Accusation", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SUBMIT_ACCUSE, NULL, NULL);
            hBtnCancelAccuse = CreateWindowA("BUTTON", "Cancel", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_CANCEL_ACCUSE, NULL, NULL);
            hCmbSuspect = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | CBS_DROPDOWNLIST, 0, 0, 0, 0, hwnd, (HMENU)ID_CMB_SUSPECT, NULL, NULL);
            hCmbMotive = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | CBS_DROPDOWNLIST, 0, 0, 0, 0, hwnd, (HMENU)ID_CMB_MOTIVE, NULL, NULL);
            hCmbWeapon = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | CBS_DROPDOWNLIST, 0, 0, 0, 0, hwnd, (HMENU)ID_CMB_WEAPON, NULL, NULL);
            
            char* m[3] = {"Revenge", "Greed", "Jealousy"};
            char* w[3] = {"Revolver", "Poison", "Lead Pipe"};
            for (int i = 0; i < 3; i++) {
                SendMessageA(hCmbSuspect, CB_ADDSTRING, 0, (LPARAM)suspects[i]);
                SendMessageA(hCmbMotive, CB_ADDSTRING, 0, (LPARAM)m[i]);
                SendMessageA(hCmbWeapon, CB_ADDSTRING, 0, (LPARAM)w[i]);
            }
            SendMessageA(hCmbSuspect, CB_SETCURSEL, 0, 0);
            SendMessageA(hCmbMotive, CB_SETCURSEL, 0, 0);
            SendMessageA(hCmbWeapon, CB_SETCURSEL, 0, 0);
            
            SendMessageA(hIntDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnAskAlibi, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnPresentClue, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnEndInt, WM_SETFONT, (WPARAM)hFont, TRUE);

            SendMessageA(hBtnLab, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hLabTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessageA(hBtnAnalyze, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnLeaveLab, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hScanDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnScan11, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hBtnScan7, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hBtnScanM3, WM_SETFONT, (WPARAM)hFontBold, TRUE);

            SendMessageA(hBtnAccuse, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hAccuseTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessageA(hAccuseDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSubmitAccuse, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hBtnCancelAccuse, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCmbSuspect, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCmbMotive, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCmbWeapon, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateUI();
            break;
        }

        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            
            if (currentState == 0) {
                MoveWindow(hStartPanel, 0, 0, cx, cy, TRUE);
                MoveWindow(hStartDesc, cx/2 - 150, cy/2 - 50, 300, 30, TRUE);
                MoveWindow(hBtnStart, cx/2 - 100, cy/2 - 10, 200, 40, TRUE);
                MoveWindow(hBtnStartMed, cx/2 - 100, cy/2 + 40, 200, 40, TRUE);
                MoveWindow(hBtnStartHard, cx/2 - 100, cy/2 + 90, 200, 40, TRUE);
            } else {
                int headerH = 40;
                int pad = 20;
                int leftW = (cx - pad*3) / 2;
                int rightW = leftW;
                
                MoveWindow(hTitle, 0, 0, cx, headerH, TRUE);
                MoveWindow(hTimeLeft, cx - 170, pad, 150, 24, TRUE);
                
                int top = headerH + pad;
                MoveWindow(hLocName, pad, top, leftW, 24, TRUE);
                MoveWindow(hLocDesc, pad, top + 30, leftW, 60, TRUE);
                MoveWindow(hBtnSearch, pad, top + 100, 160, 30, TRUE);
                MoveWindow(hBtnInterrogate, pad + 170, top + 100, 160, 30, TRUE);
                MoveWindow(hBtnLab, pad, top + 140, 160, 30, TRUE);
                MoveWindow(hBtnAccuse, pad + 170, top + 140, 160, 30, TRUE);
                
                int travelY = top + 180;
                MoveWindow(hBtnTravelOffice, pad, travelY, 200, 30, TRUE);
                MoveWindow(hBtnTravelManor, pad, travelY + 40, 200, 30, TRUE);
                MoveWindow(hBtnTravelDocks, pad, travelY + 80, 200, 30, TRUE);
                MoveWindow(hBtnTravelCasino, pad, travelY + 120, 200, 30, TRUE);
                MoveWindow(hBtnTravelStation, pad, travelY + 160, 200, 30, TRUE);
                
                MoveWindow(hIntDesc, pad, travelY, leftW - pad, 60, TRUE);
                MoveWindow(hBtnAskAlibi, pad, travelY + 70, 150, 30, TRUE);
                MoveWindow(hBtnPresentClue, pad + 160, travelY + 70, 200, 30, TRUE);
                MoveWindow(hBtnEndInt, pad, travelY + 110, 150, 30, TRUE);
                
                MoveWindow(hLabTitle, pad, travelY, leftW - pad, 30, TRUE);
                MoveWindow(hBtnAnalyze, pad, travelY + 40, 250, 30, TRUE);
                MoveWindow(hBtnLeaveLab, pad, travelY + 80, 150, 30, TRUE);
                
                MoveWindow(hScanDesc, pad, travelY, leftW - pad, 60, TRUE);
                MoveWindow(hBtnScan11, pad, travelY + 70, 80, 30, TRUE);
                MoveWindow(hBtnScan7, pad + 90, travelY + 70, 80, 30, TRUE);
                MoveWindow(hBtnScanM3, pad + 180, travelY + 70, 80, 30, TRUE);

                MoveWindow(hAccuseTitle, pad, travelY, leftW - pad, 30, TRUE);
                MoveWindow(hAccuseDesc, pad, travelY + 30, leftW - pad, 30, TRUE);
                MoveWindow(hCmbSuspect, pad, travelY + 70, 150, 200, TRUE);
                MoveWindow(hCmbMotive, pad + 160, travelY + 70, 150, 200, TRUE);
                MoveWindow(hCmbWeapon, pad + 320, travelY + 70, 150, 200, TRUE);
                MoveWindow(hBtnSubmitAccuse, pad, travelY + 110, 200, 30, TRUE);
                MoveWindow(hBtnCancelAccuse, pad + 210, travelY + 110, 150, 30, TRUE);
                
                int rightX = pad*2 + leftW;
                int listH = (cy - top - pad*2 - 90) / 3;
                if (listH < 50) listH = 50; // min height
                
                MoveWindow(hSuspectTitle, rightX, top, rightW, 24, TRUE);
                MoveWindow(hListSuspects, rightX, top + 30, rightW, listH, TRUE);
                
                int unY = top + 30 + listH + 10;
                MoveWindow(hUnanalyzedTitle, rightX, unY, rightW, 24, TRUE);
                MoveWindow(hListUnanalyzed, rightX, unY + 30, rightW, listH, TRUE);
                
                int clueY = unY + 30 + listH + 10;
                MoveWindow(hClueTitle, rightX, clueY, rightW, 24, TRUE);
                MoveWindow(hListClues, rightX, clueY + 30, rightW, cy - (clueY + 30) - pad, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_START) {
                StartGame(3, 16);
                RECT r;
                GetClientRect(hwnd, &r);
                SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_START_MED) {
                StartGame(4, 12);
                RECT r;
                GetClientRect(hwnd, &r);
                SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_START_HARD) {
                StartGame(5, 8);
                RECT r;
                GetClientRect(hwnd, &r);
                SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_SEARCH) {
                SearchLocation(hwnd);
            } else if (id == ID_BTN_TRAVEL_OFFICE) {
                if (currentLocation != 0 && AdvanceTime(hwnd, 1)) break;
                Travel(0);
            } else if (id == ID_BTN_TRAVEL_MANOR) {
                if (currentLocation != 1 && AdvanceTime(hwnd, 1)) break;
                Travel(1);
            } else if (id == ID_BTN_TRAVEL_DOCKS) {
                if (currentLocation != 2 && AdvanceTime(hwnd, 1)) break;
                Travel(2);
            } else if (id == ID_BTN_TRAVEL_CASINO) {
                if (currentLocation != 3 && AdvanceTime(hwnd, 1)) break;
                Travel(3);
            } else if (id == ID_BTN_TRAVEL_STATION) {
                if (currentLocation != 4 && AdvanceTime(hwnd, 1)) break;
                Travel(4);
            } else if (id == ID_BTN_INTERROGATE) {
                if (AdvanceTime(hwnd, 1)) break;
                currentState = 2;
                SetWindowTextA(hIntDesc, "\"What do you want, Detective?\"");
                UpdateUI();
            } else if (id == ID_BTN_ASK_ALIBI) {
                int sIdx = locations[currentLocation].suspectIdx;
                if (suspectPatience[sIdx] <= 0) {
                    SetWindowTextA(hIntDesc, "\"I told you, I want my lawyer! No more questions!\"");
                } else {
                    SetWindowTextA(hIntDesc, suspectAlibis[sIdx]);
                }
            } else if (id == ID_BTN_END_INT) {
                currentState = 1;
                UpdateUI();
            } else if (id == ID_BTN_PRESENT_CLUE) {
                int sIdx = locations[currentLocation].suspectIdx;
                int sel = SendMessageA(hListClues, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR) {
                    SetWindowTextA(hIntDesc, "\"You need to select a clue from your notebook first!\"");
                } else if (suspectPatience[sIdx] <= 0) {
                    SetWindowTextA(hIntDesc, "\"I told you, I want my lawyer! No more questions!\"");
                } else {
                    char clueText[256];
                    SendMessageA(hListClues, LB_GETTEXT, sel, (LPARAM)clueText);
                    
                    char* actualClue = clueText;
                    while (*actualClue && *actualClue != ']') actualClue++;
                    if (*actualClue == ']') actualClue += 2;
                    
                    if (my_strlen(actualClue) == 0) {
                        SetWindowTextA(hIntDesc, "\"That's not a valid clue.\"");
                    } else {
                        char response[256];
                        int intimidated = 0;
                        if (my_strcmp(actualClue, killerClues[sIdx]) == 0) {
                            my_strcpy(response, "\"Wait, where did you find that?! I... I lost it weeks ago! You can't prove anything!\" (Caught in a lie!)");
                        } else {
                            int isMotive = 0, isWeapon = 0;
                            for (int m=0; m<3; m++) {
                                if (my_strcmp(actualClue, motiveClues[m]) == 0) isMotive = 1;
                            }
                            for (int w=0; w<3; w++) {
                                if (my_strcmp(actualClue, weaponClues[w]) == 0) isWeapon = 1;
                            }
                            
                            if (isMotive) {
                                if (sIdx == currentSolution.killerIdx) {
                                    my_strcpy(response, "\"That proves nothing! Anyone could have that motive!\" (They look nervous)");
                                } else {
                                    my_strcpy(response, "\"Shocking, but not my problem.\"");
                                    intimidated = 1;
                                }
                            } else if (isWeapon) {
                                if (sIdx == currentSolution.killerIdx) {
                                    my_strcpy(response, "\"I've never seen that weapon in my life!\" (They are sweating)");
                                } else {
                                    my_strcpy(response, "\"A gruesome weapon, but I didn't use it.\"");
                                    intimidated = 1;
                                }
                            } else {
                                my_strcpy(response, "\"That doesn't belong to me.\"");
                                intimidated = 1;
                            }
                        }
                        
                        if (intimidated) {
                            suspectPatience[sIdx]--;
                            if (suspectPatience[sIdx] <= 0) {
                                my_strcpy(response, "\"That's it! You're just guessing. I want my lawyer!\" (They refuse to talk anymore)");
                            } else {
                                char temp[256];
                                wsprintfA(temp, "%s (Patience: %d/3)", response, suspectPatience[sIdx]);
                                my_strcpy(response, temp);
                            }
                        }
                        SetWindowTextA(hIntDesc, response);
                    }
                }
            } else if (id == ID_BTN_LAB) {
                currentState = 3;
                UpdateUI();
            } else if (id == ID_BTN_LEAVE_LAB) {
                currentState = 1;
                UpdateUI();
            } else if (id == ID_BTN_ANALYZE) {
                int sel = SendMessageA(hListUnanalyzed, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR) {
                    MessageBoxA(hwnd, "Select an object from the Unanalyzed Evidence list first.", "No Selection", MB_OK);
                } else {
                    if (AdvanceTime(hwnd, 1)) break;
                    scanItemIdx = sel;
                    scanTarget = (my_rand() % 30) + 50;
                    scanCurrent = 0;
                    scanMoves = 12;
                    currentState = 4;
                    UpdateUI();
                }
            } else if (id >= ID_BTN_SCAN_11 && id <= ID_BTN_SCAN_M3) {
                if (id == ID_BTN_SCAN_11) scanCurrent += 11;
                else if (id == ID_BTN_SCAN_7) scanCurrent += 7;
                else if (id == ID_BTN_SCAN_M3) scanCurrent -= 3;
                
                scanMoves--;
                
                if (scanCurrent == scanTarget) {
                    char foundText[256];
                    wsprintfA(foundText, "[%s] %s", locations[unanalyzed[scanItemIdx].locIdx].name, unanalyzed[scanItemIdx].clue);
                    SendMessageA(hListClues, LB_ADDSTRING, 0, (LPARAM)foundText);
                    
                    char msgBuf[256];
                    wsprintfA(msgBuf, "Analysis complete! Revealed:\n%s", unanalyzed[scanItemIdx].clue);
                    MessageBoxA(hwnd, msgBuf, "Success", MB_OK | MB_ICONINFORMATION);
                    
                    SendMessageA(hListUnanalyzed, LB_DELETESTRING, scanItemIdx, 0);
                    for (int i = scanItemIdx; i < numUnanalyzed - 1; i++) {
                        unanalyzed[i] = unanalyzed[i+1];
                    }
                    numUnanalyzed--;
                    
                    currentState = 3;
                    UpdateUI();
                } else if (scanMoves <= 0) {
                    MessageBoxA(hwnd, "Calibration failed. Try again.", "Failed", MB_OK | MB_ICONERROR);
                    currentState = 3;
                    UpdateUI();
                } else {
                    char scanBuf[256];
                    wsprintfA(scanBuf, "Calibrate Scanner to reveal clue.\nTarget: %d | Current: %d | Moves: %d", scanTarget, scanCurrent, scanMoves);
                    SetWindowTextA(hScanDesc, scanBuf);
                }
            } else if (id == ID_BTN_ACCUSE) {
                currentState = 5;
                UpdateUI();
            } else if (id == ID_BTN_CANCEL_ACCUSE) {
                currentState = 1;
                UpdateUI();
            } else if (id == ID_BTN_SUBMIT_ACCUSE) {
                int sIdx = SendMessageA(hCmbSuspect, CB_GETCURSEL, 0, 0);
                int mIdx = SendMessageA(hCmbMotive, CB_GETCURSEL, 0, 0);
                int wIdx = SendMessageA(hCmbWeapon, CB_GETCURSEL, 0, 0);
                
                if (sIdx == currentSolution.killerIdx && mIdx == currentSolution.motiveIdx && wIdx == currentSolution.weaponIdx) {
                    MessageBoxA(hwnd, "You did it, Detective! You caught the killer! They confessed everything. GAME OVER - YOU WIN! (Restart to play again)", "You Win!", MB_OK | MB_ICONINFORMATION);
                } else {
                    char msgBuf[256];
                    wsprintfA(msgBuf, "You were wrong! The real killer was %s. The commissioner is furious. You're fired. GAME OVER.", suspects[currentSolution.killerIdx]);
                    MessageBoxA(hwnd, msgBuf, "Game Over", MB_OK | MB_ICONERROR);
                }
                currentState = 0;
                UpdateUI();
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(220, 220, 220));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(220, 220, 220));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            
            HPEN hPen1 = CreatePen(PS_SOLID, 2, RGB(80, 80, 80));
            HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(40, 40, 40));
            
            HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldBrush = SelectObject(hdc, hNullBrush);
            
            HGDIOBJ oldPen = SelectObject(hdc, hPen1);
            Rectangle(hdc, 10, 10, rc.right - 10, rc.bottom - 10);
            
            SelectObject(hdc, hPen2);
            Rectangle(hdc, 14, 14, rc.right - 14, rc.bottom - 14);
            
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(hPen1);
            DeleteObject(hPen2);
            
            return 1;
        }

        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hFontBold) DeleteObject(hFontBold);
            if (hFontTitle) DeleteObject(hFontTitle);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SETPROCESSDPIAWARE)();
        SETPROCESSDPIAWARE pSetProcessDPIAware = (SETPROCESSDPIAWARE)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) pSetProcessDPIAware();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMysteryClass";
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, "KMysteryClass", "KMystery",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, wc.hInstance, NULL
    );

    if (hwnd) {
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
