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

float my_sin(float x) {
    while (x > 3.14159265f) x -= 6.2831853f;
    while (x < -3.14159265f) x += 6.2831853f;
    float x2 = x * x;
    return x * (1.0f - x2 / 6.0f * (1.0f - x2 / 20.0f * (1.0f - x2 / 42.0f)));
}

float my_cos(float x) {
    return my_sin(x + 1.5707963f);
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
#define ID_BTN_START_MED 1025
#define ID_BTN_START_HARD 1026
#define ID_BTN_TRAVEL_CASINO 1027
#define ID_BTN_TRAVEL_STATION 1028
#define ID_BTN_HELP 1029
#define ID_BTN_CLOSE_HELP 1030

HWND hMainWnd, hSceneWnd;
HWND hTitle, hLocName, hLocDesc, hBtnSearch, hBtnTravelOffice, hBtnTravelManor, hBtnTravelDocks, hBtnTravelCasino, hBtnTravelStation;
HWND hSuspectTitle, hListSuspects, hClueTitle, hListClues, hUnanalyzedTitle, hListUnanalyzed;
HWND hStartPanel, hBtnStart, hBtnStartMed, hBtnStartHard, hStartDesc, hStatsDesc;
HWND hBtnHelp, hHelpTitle, hHelpDesc, hBtnCloseHelp;
HWND hBtnInterrogate, hIntDesc, hBtnAskAlibi, hBtnPresentClue, hBtnEndInt;
HWND hBtnLab, hLabTitle, hBtnAnalyze, hBtnLeaveLab;
HWND hScanDesc, hBtnScan11, hBtnScan7, hBtnScanM3;
HWND hBtnAccuse, hAccuseTitle, hAccuseDesc, hBtnSubmitAccuse, hBtnCancelAccuse, hCmbSuspect, hCmbMotive, hCmbWeapon;
HWND hTimeLeft;
HFONT hFont, hFontBold, hFontTitle;

int timeLeft = 12;
int activeItems = 3;
int currentState = 0; // 0 = start, 1 = playing, 2 = interrogate, 3 = lab, 4 = scan, 5 = accuse, 6 = help
int prevState = 0;
int currentLocation = 0;
int suspectMood = 0; // 0 = normal, 1 = nervous, 2 = angry, 3 = caught

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

int statsCasesSolved = 0;
int statsFastestSolve = 999;
int statsPerfectSolves = 0;
int initialTime = 0;
int failedCalibrations = 0;
int angrySuspects = 0;

void PlayTypewriter() {
    Beep(150 + (GetTickCount() % 100), 20);
}

// Particle & VFX Engine
#define MAX_PARTICLES 64
#define MAX_SHOCKWAVES 8
#define MAX_RAIN 30
#define MAX_FOG 15

typedef struct {
    float x, y, vx, vy;
    float life, decay;
    int type; // 0: spark, 1: smoke, 2: shard, 3: star
    COLORREF color;
} Particle;

typedef struct {
    float x, y, r, maxR, life;
    COLORREF color;
} Shockwave;

typedef struct {
    float x, y, len, speed;
} RainDrop;

typedef struct {
    float x, y, r, speed;
    int alpha;
} FogMote;

Particle g_particles[MAX_PARTICLES];
Shockwave g_shockwaves[MAX_SHOCKWAVES];
RainDrop g_rain[MAX_RAIN];
FogMote g_fog[MAX_FOG];
int g_animFrame = 0;
float g_shakeIntensity = 0;

void InitVFX() {
    for (int i = 0; i < MAX_PARTICLES; i++) g_particles[i].life = 0;
    for (int i = 0; i < MAX_SHOCKWAVES; i++) g_shockwaves[i].life = 0;
    for (int i = 0; i < MAX_RAIN; i++) {
        g_rain[i].x = (float)(my_rand() % 400);
        g_rain[i].y = (float)(my_rand() % 200);
        g_rain[i].len = 8.0f + (float)(my_rand() % 10);
        g_rain[i].speed = 10.0f + (float)(my_rand() % 8);
    }
    for (int i = 0; i < MAX_FOG; i++) {
        g_fog[i].x = (float)(my_rand() % 400);
        g_fog[i].y = (float)(my_rand() % 200);
        g_fog[i].r = 15.0f + (float)(my_rand() % 20);
        g_fog[i].speed = 0.3f + (float)(my_rand() % 5) * 0.1f;
        g_fog[i].alpha = 20 + (my_rand() % 30);
    }
}

void TriggerShake(float intensity) {
    g_shakeIntensity = intensity;
}

void SpawnShockwave(float x, float y, COLORREF color) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (g_shockwaves[i].life <= 0) {
            g_shockwaves[i].x = x;
            g_shockwaves[i].y = y;
            g_shockwaves[i].r = 4.0f;
            g_shockwaves[i].maxR = 90.0f;
            g_shockwaves[i].life = 1.0f;
            g_shockwaves[i].color = color;
            break;
        }
    }
}

void SpawnBurst(float x, float y, int count, COLORREF color) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (g_particles[p].life <= 0) {
                float angle = (float)(my_rand() % 628) / 100.0f;
                float spd = 1.0f + (float)(my_rand() % 50) / 10.0f;
                g_particles[p].x = x;
                g_particles[p].y = y;
                g_particles[p].vx = my_cos(angle) * spd;
                g_particles[p].vy = my_sin(angle) * spd - 0.5f;
                g_particles[p].life = 1.0f;
                g_particles[p].decay = 0.02f + (float)(my_rand() % 20) * 0.001f;
                g_particles[p].type = (i % 3);
                g_particles[p].color = color;
                break;
            }
        }
    }
}

void PlayDramaticChord() {
    Beep(220, 100);
    Beep(261, 100);
    Beep(329, 250);
    TriggerShake(12.0f);
    SpawnShockwave(200, 90, RGB(212, 175, 55));
}

void LoadStats() {
    HANDLE hFile = CreateFileA("kmystery_stats.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read;
        ReadFile(hFile, &statsCasesSolved, sizeof(int), &read, NULL);
        ReadFile(hFile, &statsFastestSolve, sizeof(int), &read, NULL);
        ReadFile(hFile, &statsPerfectSolves, sizeof(int), &read, NULL);
        CloseHandle(hFile);
    }
}

void SaveStats() {
    HANDLE hFile = CreateFileA("kmystery_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &statsCasesSolved, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsFastestSolve, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsPerfectSolves, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
}

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
    suspectMood = 0;
}

void UpdateUI() {
    if (currentState == 0) {
        ShowWindow(hStartPanel, SW_SHOW);
        ShowWindow(hStartDesc, SW_SHOW);
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnStartMed, SW_SHOW);
        ShowWindow(hBtnStartHard, SW_SHOW);
        
        char* rank = "Rookie";
        if (statsCasesSolved >= 1) rank = "Gumshoe";
        if (statsCasesSolved >= 3) rank = "Detective";
        if (statsCasesSolved >= 5) rank = "Master Sleuth";
        if (statsCasesSolved >= 10) rank = "Sherlock";
        
        char statsBuf[256];
        if (statsFastestSolve == 999) {
            wsprintfA(statsBuf, "Rank: %s (Cases: %d)\nFastest Solve: N/A\nZero-Penalty Solves: %d", rank, statsCasesSolved, statsPerfectSolves);
        } else {
            wsprintfA(statsBuf, "Rank: %s (Cases: %d)\nFastest Solve: %dh\nZero-Penalty Solves: %d", rank, statsCasesSolved, statsFastestSolve, statsPerfectSolves);
        }
        SetWindowTextA(hStatsDesc, statsBuf);
        ShowWindow(hStatsDesc, SW_SHOW);
        
        ShowWindow(hBtnHelp, SW_SHOW);
        ShowWindow(hHelpTitle, SW_HIDE);
        ShowWindow(hHelpDesc, SW_HIDE);
        ShowWindow(hBtnCloseHelp, SW_HIDE);
        
        ShowWindow(hSceneWnd, SW_HIDE);
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
        ShowWindow(hStatsDesc, SW_HIDE);
        
        ShowWindow(hBtnHelp, SW_SHOW);
        ShowWindow(hHelpTitle, SW_HIDE);
        ShowWindow(hHelpDesc, SW_HIDE);
        ShowWindow(hBtnCloseHelp, SW_HIDE);
        
        ShowWindow(hSceneWnd, SW_SHOW);
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
    } else if (currentState == 2) {
        ShowWindow(hSceneWnd, SW_SHOW);
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
        ShowWindow(hSceneWnd, SW_SHOW);
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnTravelCasino, SW_HIDE);
        ShowWindow(hBtnTravelStation, SW_HIDE);
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
        ShowWindow(hSceneWnd, SW_SHOW);
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnTravelCasino, SW_HIDE);
        ShowWindow(hBtnTravelStation, SW_HIDE);
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
        wsprintfA(scanBuf, "Calibrate Spectral Scanner to match frequency:\nTarget: %d Hz | Tuned: %d Hz | Stability Moves: %d", scanTarget, scanCurrent, scanMoves);
        SetWindowTextA(hScanDesc, scanBuf);
    } else if (currentState == 5) {
        ShowWindow(hSceneWnd, SW_SHOW);
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnInterrogate, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hBtnTravelCasino, SW_HIDE);
        ShowWindow(hBtnTravelStation, SW_HIDE);
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
        
        ShowWindow(hBtnHelp, SW_SHOW);
        ShowWindow(hHelpTitle, SW_HIDE);
        ShowWindow(hHelpDesc, SW_HIDE);
        ShowWindow(hBtnCloseHelp, SW_HIDE);
    } else if (currentState == 6) {
        ShowWindow(hSceneWnd, SW_HIDE);
        ShowWindow(hStartPanel, SW_HIDE);
        ShowWindow(hStartDesc, SW_HIDE);
        ShowWindow(hBtnStart, SW_HIDE);
        ShowWindow(hBtnStartMed, SW_HIDE);
        ShowWindow(hBtnStartHard, SW_HIDE);
        ShowWindow(hStatsDesc, SW_HIDE);
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
        
        ShowWindow(hBtnHelp, SW_HIDE);
        ShowWindow(hHelpTitle, SW_SHOW);
        ShowWindow(hHelpDesc, SW_SHOW);
        ShowWindow(hBtnCloseHelp, SW_SHOW);
    }
}

void StartGame(int items, int time) {
    activeItems = items;
    GenerateMystery();
    currentState = 1;
    timeLeft = time;
    initialTime = time;
    failedCalibrations = 0;
    angrySuspects = 0;
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
        PlayDramaticChord();
        MessageBoxA(hwnd, "Time's up! The killer has escaped into the night. GAME OVER. (Restart to play again)", "Game Over", MB_OK | MB_ICONERROR);
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
            
            PlayDramaticChord();
            SpawnBurst(200, 90, 25, RGB(212, 175, 55));
            MessageBoxA(hwnd, "You found a mysterious object! Take it to the Evidence Lab to analyze.", "Object Found", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, "You didn't find anything useful here.", "Nothing Found", MB_OK | MB_ICONINFORMATION);
        }
        UpdateUI();
    }
}

void Travel(int loc) {
    currentLocation = loc;
    suspectMood = 0;
    TriggerShake(3.0f);
    UpdateUI();
}

// GDI Viewport Drawing Helpers
void DrawSuspectPortraitGDI(HDC hdc, int suspectIdx, int mood, int px, int py, int w, int h) {
    // Polaroid card frame
    HBRUSH hCardBrush = CreateSolidBrush(RGB(28, 27, 24));
    HPEN hGoldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    HGDIOBJ oldB = SelectObject(hdc, hCardBrush);
    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
    Rectangle(hdc, px, py, px + w, py + h);
    DeleteObject(hCardBrush);
    DeleteObject(hGoldPen);

    // Inner photo square
    HBRUSH hDarkBrush = CreateSolidBrush(RGB(9, 8, 11));
    SelectObject(hdc, hDarkBrush);
    Rectangle(hdc, px + 5, py + 5, px + w - 5, py + h - 22);
    DeleteObject(hDarkBrush);

    int cx = px + w / 2;
    int cy = py + (h - 22) / 2 + 5;

    if (suspectIdx == 0) {
        // Mr. Black: Dark trenchcoat, fedora
        HBRUSH hCoat = CreateSolidBrush(RGB(20, 20, 22));
        SelectObject(hdc, hCoat);
        Ellipse(hdc, cx - 28, cy + 12, cx + 28, cy + 50);
        DeleteObject(hCoat);

        HBRUSH hSkin = CreateSolidBrush(RGB(158, 133, 113));
        SelectObject(hdc, hSkin);
        Ellipse(hdc, cx - 14, cy - 8, cx + 14, cy + 18);
        DeleteObject(hSkin);

        // Fedora brim and crown
        HBRUSH hHat = CreateSolidBrush(RGB(10, 10, 12));
        SelectObject(hdc, hHat);
        Ellipse(hdc, cx - 26, cy - 14, cx + 26, cy - 2);
        Rectangle(hdc, cx - 16, cy - 32, cx + 16, cy - 8);
        DeleteObject(hHat);

        // Shadow over eyes
        HBRUSH hEyeShadow = CreateSolidBrush(RGB(0, 0, 0));
        SelectObject(hdc, hEyeShadow);
        Rectangle(hdc, cx - 12, cy - 4, cx + 12, cy + 4);
        DeleteObject(hEyeShadow);

        // Eyes
        COLORREF eyeCol = (mood == 3) ? RGB(255, 50, 50) : RGB(240, 235, 216);
        SetPixel(hdc, cx - 7, cy, eyeCol);
        SetPixel(hdc, cx - 6, cy, eyeCol);
        SetPixel(hdc, cx + 6, cy, eyeCol);
        SetPixel(hdc, cx + 7, cy, eyeCol);
    } else if (suspectIdx == 1) {
        // Miss Scarlet: Red dress, cloche hat
        HBRUSH hDress = CreateSolidBrush(RGB(85, 14, 24));
        SelectObject(hdc, hDress);
        Ellipse(hdc, cx - 26, cy + 12, cx + 26, cy + 50);
        DeleteObject(hDress);

        HBRUSH hSkin = CreateSolidBrush(RGB(212, 173, 155));
        SelectObject(hdc, hSkin);
        Ellipse(hdc, cx - 13, cy - 8, cx + 13, cy + 17);
        DeleteObject(hSkin);

        HBRUSH hHat = CreateSolidBrush(RGB(128, 19, 34));
        SelectObject(hdc, hHat);
        Pie(hdc, cx - 18, cy - 28, cx + 18, cy + 6, cx + 18, cy - 8, cx - 18, cy - 8);
        Rectangle(hdc, cx - 20, cy - 10, cx + 20, cy - 4);
        DeleteObject(hHat);

        // Red lips
        HBRUSH hLips = CreateSolidBrush(RGB(187, 17, 34));
        SelectObject(hdc, hLips);
        Rectangle(hdc, cx - 4, cy + 10, cx + 4, cy + 13);
        DeleteObject(hLips);
    } else if (suspectIdx == 2) {
        // Colonel Mustard: Khaki uniform, cap, mustache
        HBRUSH hUniform = CreateSolidBrush(RGB(74, 66, 32));
        SelectObject(hdc, hUniform);
        Ellipse(hdc, cx - 28, cy + 12, cx + 28, cy + 50);
        DeleteObject(hUniform);

        HBRUSH hSkin = CreateSolidBrush(RGB(199, 157, 128));
        SelectObject(hdc, hSkin);
        Ellipse(hdc, cx - 14, cy - 8, cx + 14, cy + 18);
        DeleteObject(hSkin);

        // Military cap
        HBRUSH hCap = CreateSolidBrush(RGB(58, 52, 24));
        SelectObject(hdc, hCap);
        Rectangle(hdc, cx - 16, cy - 26, cx + 16, cy - 8);
        DeleteObject(hCap);
        HBRUSH hVisor = CreateSolidBrush(RGB(10, 10, 10));
        SelectObject(hdc, hVisor);
        Ellipse(hdc, cx - 20, cy - 12, cx + 20, cy - 4);
        DeleteObject(hVisor);

        // White Handlebar Mustache
        HBRUSH hMustache = CreateSolidBrush(RGB(240, 235, 216));
        SelectObject(hdc, hMustache);
        Ellipse(hdc, cx - 12, cy + 6, cx + 12, cy + 14);
        DeleteObject(hMustache);
    } else if (suspectIdx == 3) {
        // Mrs. White: Black mourning veil & pearls
        HBRUSH hDress = CreateSolidBrush(RGB(26, 24, 28));
        SelectObject(hdc, hDress);
        Ellipse(hdc, cx - 26, cy + 12, cx + 26, cy + 50);
        DeleteObject(hDress);

        HBRUSH hSkin = CreateSolidBrush(RGB(210, 186, 168));
        SelectObject(hdc, hSkin);
        Ellipse(hdc, cx - 13, cy - 8, cx + 13, cy + 17);
        DeleteObject(hSkin);

        // Lace veil
        HBRUSH hVeil = CreateSolidBrush(RGB(40, 36, 44));
        SelectObject(hdc, hVeil);
        Rectangle(hdc, cx - 18, cy - 24, cx + 18, cy - 4);
        DeleteObject(hVeil);

        // Pearl necklace pips
        for (int p = -8; p <= 8; p += 4) {
            SetPixel(hdc, cx + p, cy + 18, RGB(240, 235, 220));
        }
    } else if (suspectIdx == 4) {
        // Professor Plum: Purple tweed, bowtie, spectacles
        HBRUSH hTweed = CreateSolidBrush(RGB(43, 27, 54));
        SelectObject(hdc, hTweed);
        Ellipse(hdc, cx - 26, cy + 12, cx + 26, cy + 50);
        DeleteObject(hTweed);

        HBRUSH hSkin = CreateSolidBrush(RGB(201, 168, 144));
        SelectObject(hdc, hSkin);
        Ellipse(hdc, cx - 14, cy - 8, cx + 14, cy + 18);
        DeleteObject(hSkin);

        // Hair
        HBRUSH hHair = CreateSolidBrush(RGB(74, 56, 46));
        SelectObject(hdc, hHair);
        Pie(hdc, cx - 16, cy - 18, cx + 16, cy + 6, cx + 16, cy - 8, cx - 16, cy - 8);
        DeleteObject(hHair);

        // Spectacles
        HPEN hGoldP = CreatePen(PS_SOLID, 1, RGB(212, 175, 55));
        HGDIOBJ oldSpecP = SelectObject(hdc, hGoldP);
        HBRUSH hGlass = (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ oldGlassB = SelectObject(hdc, hGlass);
        Ellipse(hdc, cx - 11, cy - 3, cx - 2, cy + 6);
        Ellipse(hdc, cx + 2, cy - 3, cx + 11, cy + 6);
        MoveToEx(hdc, cx - 2, cy + 1, NULL);
        LineTo(hdc, cx + 2, cy + 1);
        SelectObject(hdc, oldSpecP);
        SelectObject(hdc, oldGlassB);
        DeleteObject(hGoldP);
    }

    // Mood badge text or indicator
    if (mood == 3) {
        SetTextColor(hdc, RGB(255, 60, 60));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, cx - 18, py + 8, "LIAR!", 5);
    } else if (mood == 2) {
        SetTextColor(hdc, RGB(255, 120, 120));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, cx - 22, py + 8, "ANGRY", 5);
    } else if (mood == 1) {
        SetTextColor(hdc, RGB(100, 200, 255));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, cx - 26, py + 8, "SWEATING", 8);
    }

    // Suspect Name Banner
    SetTextColor(hdc, RGB(212, 175, 55));
    SetBkMode(hdc, TRANSPARENT);
    int sLen = my_strlen(suspects[suspectIdx]);
    TextOutA(hdc, px + 8, py + h - 18, suspects[suspectIdx], sLen);

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
}

void DrawLocationSceneGDI(HDC hdc, int locIdx, int w, int h) {
    if (locIdx == 0) {
        // Office: Venetian blinds and banker's lamp
        HBRUSH hWall = CreateSolidBrush(RGB(16, 14, 10));
        RECT r = {0, 0, w, h};
        FillRect(hdc, &r, hWall);
        DeleteObject(hWall);

        // Window frame
        HBRUSH hWin = CreateSolidBrush(RGB(9, 10, 16));
        RECT rw = {w / 6, 15, w * 5 / 6, h * 7 / 10};
        FillRect(hdc, &rw, hWin);
        DeleteObject(hWin);

        // Venetian blinds slats
        HPEN hBlindPen = CreatePen(PS_SOLID, 2, RGB(45, 38, 22));
        HGDIOBJ oldP = SelectObject(hdc, hBlindPen);
        for (int y = 20; y < h * 7 / 10; y += 10) {
            MoveToEx(hdc, w / 6, y, NULL);
            LineTo(hdc, w * 5 / 6, y);
        }
        SelectObject(hdc, oldP);
        DeleteObject(hBlindPen);

        // Desk
        HBRUSH hDesk = CreateSolidBrush(RGB(20, 18, 14));
        RECT rd = {0, h * 7 / 10, w, h};
        FillRect(hdc, &rd, hDesk);
        DeleteObject(hDesk);

        // Green Banker's lamp
        int lx = w / 4, ly = h * 7 / 10;
        HBRUSH hLamp = CreateSolidBrush(RGB(26, 94, 42));
        SelectObject(hdc, hLamp);
        Ellipse(hdc, lx - 16, ly - 35, lx + 16, ly - 20);
        DeleteObject(hLamp);
    } else if (locIdx == 1) {
        // Manor: Gothic roofline and yellow police tape
        HBRUSH hSky = CreateSolidBrush(RGB(9, 10, 18));
        RECT r = {0, 0, w, h};
        FillRect(hdc, &r, hSky);
        DeleteObject(hSky);

        // Mansion silhouette
        HBRUSH hManor = CreateSolidBrush(RGB(8, 7, 11));
        SelectObject(hdc, hManor);
        POINT pts[7] = {
            {w / 8, h}, {w / 8, h / 2}, {w * 3 / 10, h / 4},
            {w / 2, h / 6}, {w * 7 / 10, h / 4}, {w * 7 / 8, h / 2}, {w * 7 / 8, h}
        };
        Polygon(hdc, pts, 7);
        DeleteObject(hManor);

        // Lit yellow windows
        HBRUSH hWin = CreateSolidBrush(RGB(243, 207, 88));
        RECT rw1 = {w / 2 - 8, h / 2 - 15, w / 2 + 8, h / 2 + 5};
        FillRect(hdc, &rw1, hWin);
        DeleteObject(hWin);

        // Police tape
        HBRUSH hTape = CreateSolidBrush(RGB(230, 200, 32));
        RECT rt = {0, h * 4 / 5, w, h * 4 / 5 + 10};
        FillRect(hdc, &rt, hTape);
        DeleteObject(hTape);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 20, h * 4 / 5 - 1, "POLICE LINE DO NOT CROSS - CRIME SCENE", 38);
    } else if (locIdx == 2) {
        // Docks: Water, pier, lantern
        HBRUSH hSea = CreateSolidBrush(RGB(6, 10, 16));
        RECT r = {0, 0, w, h};
        FillRect(hdc, &r, hSea);
        DeleteObject(hSea);

        // Pier
        HBRUSH hPier = CreateSolidBrush(RGB(13, 12, 10));
        RECT rp = {0, h * 3 / 5, w / 2, h};
        FillRect(hdc, &rp, hPier);
        DeleteObject(hPier);

        // Streetlamp
        int lx = w / 3, ly = h * 3 / 5;
        HPEN hPole = CreatePen(PS_SOLID, 4, RGB(34, 34, 34));
        HGDIOBJ oldP = SelectObject(hdc, hPole);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, lx, ly - 70);
        SelectObject(hdc, oldP);
        DeleteObject(hPole);

        HBRUSH hLight = CreateSolidBrush(RGB(255, 170, 51));
        SelectObject(hdc, hLight);
        Ellipse(hdc, lx - 10, ly - 80, lx + 10, ly - 60);
        DeleteObject(hLight);
    } else if (locIdx == 3) {
        // Casino: Marquee, green felt roulette
        HBRUSH hCas = CreateSolidBrush(RGB(18, 8, 8));
        RECT r = {0, 0, w, h};
        FillRect(hdc, &r, hCas);
        DeleteObject(hCas);

        SetTextColor(hdc, RGB(255, 51, 102));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, w / 2 - 80, 20, "* GOLDEN PALACE CASINO *", 24);

        // Green roulette table
        HBRUSH hFelt = CreateSolidBrush(RGB(15, 56, 30));
        SelectObject(hdc, hFelt);
        Ellipse(hdc, w / 2 - 120, h * 7 / 10, w / 2 + 120, h + 30);
        DeleteObject(hFelt);
    } else if (locIdx == 4) {
        // Train Station: Concourse clock & locomotive
        HBRUSH hStn = CreateSolidBrush(RGB(10, 13, 20));
        RECT r = {0, 0, w, h};
        FillRect(hdc, &r, hStn);
        DeleteObject(hStn);

        // Station Clock
        HBRUSH hClock = CreateSolidBrush(RGB(240, 235, 216));
        SelectObject(hdc, hClock);
        Ellipse(hdc, w / 2 - 20, 20, w / 2 + 20, 60);
        DeleteObject(hClock);

        // Locomotive engine
        HBRUSH hTrain = CreateSolidBrush(RGB(8, 8, 10));
        RECT rt = {w / 6, h / 2, w * 5 / 6, h};
        FillRect(hdc, &rt, hTrain);
        DeleteObject(hTrain);
    }
}

void DrawEvidenceLabGDI(HDC hdc, int w, int h) {
    // Oscilloscope CRT Screen
    HBRUSH hCRT = CreateSolidBrush(RGB(6, 16, 6));
    RECT r = {0, 0, w, h};
    FillRect(hdc, &r, hCRT);
    DeleteObject(hCRT);

    // Green raster grid
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(18, 60, 18));
    HGDIOBJ oldP = SelectObject(hdc, hGridPen);
    for (int x = 0; x < w; x += 25) {
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, h);
    }
    for (int y = 0; y < h; y += 25) {
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, w, y);
    }
    SelectObject(hdc, oldP);
    DeleteObject(hGridPen);

    // Center reticle
    HPEN hRetPen = CreatePen(PS_SOLID, 1, RGB(40, 140, 40));
    SelectObject(hdc, hRetPen);
    MoveToEx(hdc, w / 2, 0, NULL); LineTo(hdc, w / 2, h);
    MoveToEx(hdc, 0, h / 2, NULL); LineTo(hdc, w, h / 2);
    SelectObject(hdc, oldP);
    DeleteObject(hRetPen);

    if (currentState == 4) {
        // Target Waveform (Golden)
        HPEN hTargetPen = CreatePen(PS_SOLID, 2, RGB(230, 200, 32));
        SelectObject(hdc, hTargetPen);
        float tFreq = (float)scanTarget * 0.05f;
        for (int x = 0; x < w; x += 2) {
            float y = (float)(h / 2) + my_sin((float)x * 0.04f * tFreq + (float)g_animFrame * 0.08f) * 30.0f;
            if (x == 0) MoveToEx(hdc, x, (int)y, NULL);
            else LineTo(hdc, x, (int)y);
        }
        SelectObject(hdc, oldP);
        DeleteObject(hTargetPen);

        // Tuned Waveform (Bright Green / Cyan)
        COLORREF cCol = (scanCurrent == scanTarget) ? RGB(68, 255, 68) : RGB(0, 220, 255);
        HPEN hCurrPen = CreatePen(PS_SOLID, 2, cCol);
        SelectObject(hdc, hCurrPen);
        float cFreq = (float)scanCurrent * 0.05f;
        for (int x = 0; x < w; x += 2) {
            float amp = (scanCurrent == 0) ? 4.0f : 30.0f;
            float y = (float)(h / 2) + my_sin((float)x * 0.04f * cFreq + (float)g_animFrame * 0.08f) * amp;
            if (x == 0) MoveToEx(hdc, x, (int)y, NULL);
            else LineTo(hdc, x, (int)y);
        }
        SelectObject(hdc, oldP);
        DeleteObject(hCurrPen);

        SetTextColor(hdc, RGB(68, 255, 68));
        SetBkMode(hdc, TRANSPARENT);
        char buf[64];
        wsprintfA(buf, "SPECTRAL FREQ: %d / %d Hz", scanCurrent, scanTarget);
        TextOutA(hdc, 15, 10, buf, my_strlen(buf));
    } else {
        SetTextColor(hdc, RGB(68, 255, 68));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, w / 2 - 120, h / 2 - 8, "EVIDENCE LAB - SELECT OBJECT TO SCAN", 36);
    }
}

void DrawArtDecoFiligreeGDI(HDC hdc, int w, int h) {
    HPEN hGoldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);

    int b = 12;
    // Top-Left
    MoveToEx(hdc, 4, 4 + b, NULL); LineTo(hdc, 4, 4); LineTo(hdc, 4 + b, 4);
    // Top-Right
    MoveToEx(hdc, w - 4, 4 + b, NULL); LineTo(hdc, w - 4, 4); LineTo(hdc, w - 4 - b, 4);
    // Bottom-Left
    MoveToEx(hdc, 4, h - 4 - b, NULL); LineTo(hdc, 4, h - 4); LineTo(hdc, 4 + b, h - 4);
    // Bottom-Right
    MoveToEx(hdc, w - 4, h - 4 - b, NULL); LineTo(hdc, w - 4, h - 4); LineTo(hdc, w - 4 - b, h - 4);

    SelectObject(hdc, oldP);
    DeleteObject(hGoldPen);

    // Traveling specular glint pip
    int perim = (g_animFrame * 4) % (w * 2 + h * 2);
    int gx = 0, gy = 0;
    if (perim < w) { gx = perim; gy = 2; }
    else if (perim < w + h) { gx = w - 2; gy = perim - w; }
    else if (perim < w * 2 + h) { gx = w - (perim - (w + h)); gy = h - 2; }
    else { gx = 2; gy = h - (perim - (w * 2 + h)); }

    SetPixel(hdc, gx, gy, RGB(255, 255, 255));
    SetPixel(hdc, gx + 1, gy, RGB(212, 175, 55));
    SetPixel(hdc, gx, gy + 1, RGB(212, 175, 55));
}

// Scene Viewport Window Procedure
LRESULT CALLBACK SceneWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);

            // Screen shake offset
            int ox = 0, oy = 0;
            if (g_shakeIntensity > 0.1f) {
                ox = (my_rand() % (int)(g_shakeIntensity * 2 + 1)) - (int)g_shakeIntensity;
                oy = (my_rand() % (int)(g_shakeIntensity * 2 + 1)) - (int)g_shakeIntensity;
                SetViewportOrgEx(memDC, ox, oy, NULL);
            }

            if (currentState == 3 || currentState == 4) {
                DrawEvidenceLabGDI(memDC, w, h);
            } else {
                DrawLocationSceneGDI(memDC, currentLocation, w, h);
                if (currentState == 2) {
                    int sIdx = locations[currentLocation].suspectIdx;
                    DrawSuspectPortraitGDI(memDC, sIdx, suspectMood, w - 120, 15, 105, 140);
                }
            }

            // Rain streaks
            HPEN hRainPen = CreatePen(PS_SOLID, 1, RGB(180, 200, 230));
            HGDIOBJ oldP = SelectObject(memDC, hRainPen);
            for (int i = 0; i < MAX_RAIN; i++) {
                MoveToEx(memDC, (int)g_rain[i].x, (int)g_rain[i].y, NULL);
                LineTo(memDC, (int)g_rain[i].x - 1, (int)(g_rain[i].y + g_rain[i].len));
            }
            SelectObject(memDC, oldP);
            DeleteObject(hRainPen);

            // Shockwave rings
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (g_shockwaves[i].life > 0) {
                    HPEN hSwPen = CreatePen(PS_SOLID, 2, g_shockwaves[i].color);
                    HGDIOBJ oldSw = SelectObject(memDC, hSwPen);
                    HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HGDIOBJ oldB = SelectObject(memDC, hNull);
                    int sr = (int)g_shockwaves[i].r;
                    Ellipse(memDC, (int)g_shockwaves[i].x - sr, (int)g_shockwaves[i].y - sr,
                                  (int)g_shockwaves[i].x + sr, (int)g_shockwaves[i].y + sr);
                    SelectObject(memDC, oldSw);
                    SelectObject(memDC, oldB);
                    DeleteObject(hSwPen);
                }
            }

            // Particles
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (g_particles[i].life > 0) {
                    HBRUSH hPb = CreateSolidBrush(g_particles[i].color);
                    RECT pr = {(int)g_particles[i].x - 1, (int)g_particles[i].y - 1, (int)g_particles[i].x + 2, (int)g_particles[i].y + 2};
                    FillRect(memDC, &pr, hPb);
                    DeleteObject(hPb);
                }
            }

            DrawArtDecoFiligreeGDI(memDC, w, h);

            if (g_shakeIntensity > 0.1f) {
                SetViewportOrgEx(memDC, 0, 0, NULL);
            }

            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontBold = CreateFontA(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontTitle = CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");

            InitVFX();
            SetTimer(hwnd, 999, 33, NULL);

            // Register Viewport Class
            WNDCLASSA sc = {0};
            sc.lpfnWndProc = SceneWndProc;
            sc.hInstance = GetModuleHandleA(NULL);
            sc.lpszClassName = "KMysterySceneClass";
            sc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
            sc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RegisterClassA(&sc);

            hSceneWnd = CreateWindowExA(0, "KMysterySceneClass", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 0, 0, hwnd, NULL, GetModuleHandleA(NULL), NULL);

            // Start Screen
            hStartPanel = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hStartDesc = CreateWindowA("STATIC", "Homicide Investigation: A murder occurred at the Manor.", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnStart = CreateWindowA("BUTTON", "Start Easy (3 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START, NULL, NULL);
            hBtnStartMed = CreateWindowA("BUTTON", "Start Medium (4 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START_MED, NULL, NULL);
            hBtnStartHard = CreateWindowA("BUTTON", "Start Hard (5 Suspects)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START_HARD, NULL, NULL);
            hStatsDesc = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            SendMessageA(hStatsDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            LoadStats();

            // Game Screen
            hTimeLeft = CreateWindowA("STATIC", "Time Left: 12h", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hTitle = CreateWindowA("STATIC", "KMystery", WS_CHILD | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocName = CreateWindowA("STATIC", "Location:", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnSearch = CreateWindowA("BUTTON", "Search for Clues", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SEARCH, NULL, NULL);
            hBtnInterrogate = CreateWindowA("BUTTON", "Interrogate", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_INTERROGATE, NULL, NULL);
            
            hBtnTravelOffice = CreateWindowA("BUTTON", "Travel: Office", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_OFFICE, NULL, NULL);
            hBtnTravelManor = CreateWindowA("BUTTON", "Travel: The Manor", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_MANOR, NULL, NULL);
            hBtnTravelDocks = CreateWindowA("BUTTON", "Travel: The Docks", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_DOCKS, NULL, NULL);
            hBtnTravelCasino = CreateWindowA("BUTTON", "Travel: The Casino", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_CASINO, NULL, NULL);
            hBtnTravelStation = CreateWindowA("BUTTON", "Travel: Station", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_STATION, NULL, NULL);
            
            hBtnLab = CreateWindowA("BUTTON", "Evidence Lab", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LAB, NULL, NULL);
            hLabTitle = CreateWindowA("STATIC", "Forensic Lab", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnAnalyze = CreateWindowA("BUTTON", "Analyze Selected Object", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_ANALYZE, NULL, NULL);
            hBtnLeaveLab = CreateWindowA("BUTTON", "Leave Lab", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LEAVE_LAB, NULL, NULL);
            
            hScanDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnScan11 = CreateWindowA("BUTTON", "+11", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_11, NULL, NULL);
            hBtnScan7 = CreateWindowA("BUTTON", "+7", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_7, NULL, NULL);
            hBtnScanM3 = CreateWindowA("BUTTON", "-3", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN_M3, NULL, NULL);

            hBtnAccuse = CreateWindowA("BUTTON", "Accuse Someone", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_ACCUSE, NULL, NULL);
            hAccuseTitle = CreateWindowA("STATIC", "Grand Jury Indictment", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
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

            hSuspectTitle = CreateWindowA("STATIC", "Suspects in Case", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hListSuspects = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | LBS_NOTIFY | WS_VSCROLL, 0, 0, 0, 0, hwnd, (HMENU)ID_LIST_SUSPECTS, NULL, NULL);
            
            hUnanalyzedTitle = CreateWindowA("STATIC", "Unanalyzed Evidence", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hListUnanalyzed = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | LBS_NOTIFY | WS_VSCROLL, 0, 0, 0, 0, hwnd, (HMENU)ID_LIST_UNANALYZED, NULL, NULL);
            
            hClueTitle = CreateWindowA("STATIC", "Verified Clues", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hListClues = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | LBS_NOTIFY | WS_VSCROLL, 0, 0, 0, 0, hwnd, (HMENU)ID_LIST_CLUES, NULL, NULL);

            hIntDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnAskAlibi = CreateWindowA("BUTTON", "Ask for Alibi", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_ASK_ALIBI, NULL, NULL);
            hBtnPresentClue = CreateWindowA("BUTTON", "Present Clue", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_PRESENT_CLUE, NULL, NULL);
            hBtnEndInt = CreateWindowA("BUTTON", "End Interrogation", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_END_INT, NULL, NULL);

            SendMessageA(hTimeLeft, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessageA(hLocName, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hLocDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnInterrogate, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hSuspectTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hClueTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hUnanalyzedTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            
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

            hBtnHelp = CreateWindowA("BUTTON", "Manual", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            hHelpTitle = CreateWindowA("STATIC", "Detective's Manual", WS_CHILD | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hHelpDesc = CreateWindowA("STATIC", "HOW TO PLAY:\n1. Search locations for clues (2h).\n2. Analyze objects in the lab (1h).\n3. Interrogate suspects to catch them in lies (1h).\n4. Accuse the killer with correct motive & weapon!\n\nTIPS:\n- Suspects have patience. Unrelated clues make them angry.\n- The killer's specific clue will catch them immediately!\n- Cross-reference alibis to spot liars.\n- In the lab, calibrate scanner to exactly match the target.", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnCloseHelp = CreateWindowA("BUTTON", "Close Manual", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_CLOSE_HELP, NULL, NULL);
            
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hHelpTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessageA(hHelpDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnCloseHelp, WM_SETFONT, (WPARAM)hFontBold, TRUE);

            UpdateUI();
            break;
        }

        case WM_TIMER: {
            g_animFrame++;
            if (g_shakeIntensity > 0) {
                g_shakeIntensity -= 0.5f;
                if (g_shakeIntensity < 0) g_shakeIntensity = 0;
            }

            // Rain update
            for (int i = 0; i < MAX_RAIN; i++) {
                g_rain[i].y += g_rain[i].speed;
                g_rain[i].x -= 1.2f;
                if (g_rain[i].y > 200) {
                    g_rain[i].y = -15;
                    g_rain[i].x = (float)(my_rand() % 400);
                }
                if (g_rain[i].x < 0) g_rain[i].x = 410;
            }

            // Shockwaves update
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (g_shockwaves[i].life > 0) {
                    g_shockwaves[i].r += 3.5f;
                    g_shockwaves[i].life -= 0.04f;
                }
            }

            // Particles update
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (g_particles[i].life > 0) {
                    g_particles[i].x += g_particles[i].vx;
                    g_particles[i].y += g_particles[i].vy;
                    g_particles[i].life -= g_particles[i].decay;
                    if (g_particles[i].type == 2) {
                        g_particles[i].vy += 0.15f; // Gravity
                    }
                }
            }

            if (hSceneWnd && IsWindowVisible(hSceneWnd)) {
                InvalidateRect(hSceneWnd, NULL, FALSE);
            }
            break;
        }

        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            
            if (currentState == 0) {
                MoveWindow(hStartPanel, 0, 0, cx, cy, TRUE);
                MoveWindow(hStartDesc, cx/2 - 200, cy/2 - 60, 400, 30, TRUE);
                MoveWindow(hBtnStart, cx/2 - 120, cy/2 - 10, 240, 36, TRUE);
                MoveWindow(hBtnStartMed, cx/2 - 120, cy/2 + 35, 240, 36, TRUE);
                MoveWindow(hBtnStartHard, cx/2 - 120, cy/2 + 80, 240, 36, TRUE);
                MoveWindow(hStatsDesc, cx/2 - 160, cy/2 + 130, 320, 100, TRUE);
            } else {
                int headerH = 35;
                int pad = 12;
                int leftW = (cx - pad*3) / 2;
                int rightW = leftW;
                
                MoveWindow(hTitle, 0, 4, cx, headerH, TRUE);
                MoveWindow(hTimeLeft, cx - 160, pad, 140, 22, TRUE);
                
                int top = headerH + pad;
                int sceneH = 160;
                MoveWindow(hSceneWnd, pad, top, leftW, sceneH, TRUE);
                
                int afterSceneY = top + sceneH + 8;
                MoveWindow(hLocName, pad, afterSceneY, leftW, 20, TRUE);
                MoveWindow(hLocDesc, pad, afterSceneY + 22, leftW, 45, TRUE);
                
                int btnY = afterSceneY + 70;
                MoveWindow(hBtnSearch, pad, btnY, 150, 28, TRUE);
                MoveWindow(hBtnInterrogate, pad + 160, btnY, 150, 28, TRUE);
                MoveWindow(hBtnLab, pad, btnY + 34, 150, 28, TRUE);
                MoveWindow(hBtnAccuse, pad + 160, btnY + 34, 150, 28, TRUE);
                
                int travelY = btnY + 72;
                MoveWindow(hBtnTravelOffice, pad, travelY, 150, 26, TRUE);
                MoveWindow(hBtnTravelManor, pad + 160, travelY, 150, 26, TRUE);
                MoveWindow(hBtnTravelDocks, pad, travelY + 30, 150, 26, TRUE);
                MoveWindow(hBtnTravelCasino, pad + 160, travelY + 30, 150, 26, TRUE);
                MoveWindow(hBtnTravelStation, pad, travelY + 60, 150, 26, TRUE);
                
                MoveWindow(hIntDesc, pad, travelY, leftW - pad, 45, TRUE);
                MoveWindow(hBtnAskAlibi, pad, travelY + 50, 140, 28, TRUE);
                MoveWindow(hBtnPresentClue, pad + 150, travelY + 50, 150, 28, TRUE);
                MoveWindow(hBtnEndInt, pad, travelY + 84, 140, 28, TRUE);
                
                MoveWindow(hLabTitle, pad, travelY, leftW - pad, 25, TRUE);
                MoveWindow(hBtnAnalyze, pad, travelY + 30, 220, 28, TRUE);
                MoveWindow(hBtnLeaveLab, pad, travelY + 65, 140, 28, TRUE);
                
                MoveWindow(hScanDesc, pad, travelY, leftW - pad, 45, TRUE);
                MoveWindow(hBtnScan11, pad, travelY + 50, 70, 28, TRUE);
                MoveWindow(hBtnScan7, pad + 76, travelY + 50, 70, 28, TRUE);
                MoveWindow(hBtnScanM3, pad + 152, travelY + 50, 70, 28, TRUE);

                MoveWindow(hAccuseTitle, pad, travelY, leftW - pad, 25, TRUE);
                MoveWindow(hAccuseDesc, pad, travelY + 25, leftW - pad, 25, TRUE);
                MoveWindow(hCmbSuspect, pad, travelY + 55, 130, 180, TRUE);
                MoveWindow(hCmbMotive, pad + 136, travelY + 55, 130, 180, TRUE);
                MoveWindow(hCmbWeapon, pad + 272, travelY + 55, 130, 180, TRUE);
                MoveWindow(hBtnSubmitAccuse, pad, travelY + 90, 180, 28, TRUE);
                MoveWindow(hBtnCancelAccuse, pad + 190, travelY + 90, 120, 28, TRUE);
                
                int rightX = pad*2 + leftW;
                int listH = (cy - top - pad*2 - 80) / 3;
                if (listH < 45) listH = 45;
                
                MoveWindow(hSuspectTitle, rightX, top, rightW, 20, TRUE);
                MoveWindow(hListSuspects, rightX, top + 24, rightW, listH, TRUE);
                
                int unY = top + 24 + listH + 8;
                MoveWindow(hUnanalyzedTitle, rightX, unY, rightW, 20, TRUE);
                MoveWindow(hListUnanalyzed, rightX, unY + 24, rightW, listH, TRUE);
                
                int clueY = unY + 24 + listH + 8;
                MoveWindow(hClueTitle, rightX, clueY, rightW, 20, TRUE);
                MoveWindow(hListClues, rightX, clueY + 24, rightW, cy - (clueY + 24) - pad, TRUE);
                
                MoveWindow(hBtnHelp, cx - 90, 8, 75, 24, TRUE);
            }
            if (currentState == 6) {
                MoveWindow(hHelpTitle, 0, 20, cx, 35, TRUE);
                MoveWindow(hHelpDesc, cx/2 - 280, 70, 560, 240, TRUE);
                MoveWindow(hBtnCloseHelp, cx/2 - 90, 320, 180, 36, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);
            if (code == 0) PlayTypewriter();

            if (id == ID_BTN_START) {
                StartGame(3, 16);
                RECT r; GetClientRect(hwnd, &r); SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_START_MED) {
                StartGame(4, 12);
                RECT r; GetClientRect(hwnd, &r); SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_START_HARD) {
                StartGame(5, 8);
                RECT r; GetClientRect(hwnd, &r); SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
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
                suspectMood = 0;
                SetWindowTextA(hIntDesc, "\"What do you want, Detective?\"");
                UpdateUI();
            } else if (id == ID_BTN_ASK_ALIBI) {
                int sIdx = locations[currentLocation].suspectIdx;
                if (suspectPatience[sIdx] <= 0) {
                    suspectMood = 2;
                    SetWindowTextA(hIntDesc, "\"I told you, I want my lawyer! No more questions!\"");
                } else {
                    suspectMood = 0;
                    SetWindowTextA(hIntDesc, suspectAlibis[sIdx]);
                }
            } else if (id == ID_BTN_END_INT) {
                currentState = 1;
                suspectMood = 0;
                UpdateUI();
            } else if (id == ID_BTN_PRESENT_CLUE) {
                int sIdx = locations[currentLocation].suspectIdx;
                int sel = SendMessageA(hListClues, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR) {
                    SetWindowTextA(hIntDesc, "\"You need to select a clue from your notebook first!\"");
                } else if (suspectPatience[sIdx] <= 0) {
                    suspectMood = 2;
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
                            PlayDramaticChord();
                            suspectMood = 3;
                            SpawnBurst(200, 90, 35, RGB(255, 60, 60));
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
                                    suspectMood = 1;
                                    my_strcpy(response, "\"That proves nothing! Anyone could have that motive!\" (They look nervous)");
                                } else {
                                    suspectMood = 2;
                                    my_strcpy(response, "\"Shocking, but not my problem.\"");
                                    intimidated = 1;
                                }
                            } else if (isWeapon) {
                                if (sIdx == currentSolution.killerIdx) {
                                    suspectMood = 1;
                                    my_strcpy(response, "\"I've never seen that weapon in my life!\" (They are sweating)");
                                } else {
                                    suspectMood = 2;
                                    my_strcpy(response, "\"A gruesome weapon, but I didn't use it.\"");
                                    intimidated = 1;
                                }
                            } else {
                                suspectMood = 2;
                                my_strcpy(response, "\"That doesn't belong to me.\"");
                                intimidated = 1;
                            }
                        }
                        
                        if (intimidated) {
                            suspectPatience[sIdx]--;
                            if (suspectPatience[sIdx] <= 0) {
                                suspectMood = 2;
                                my_strcpy(response, "\"That's it! You're just guessing. I want my lawyer!\" (They refuse to talk anymore)");
                                angrySuspects++;
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
                TriggerShake(3.0f);
                
                if (scanCurrent == scanTarget) {
                    char foundText[256];
                    wsprintfA(foundText, "[%s] %s", locations[unanalyzed[scanItemIdx].locIdx].name, unanalyzed[scanItemIdx].clue);
                    SendMessageA(hListClues, LB_ADDSTRING, 0, (LPARAM)foundText);
                    
                    PlayDramaticChord();
                    SpawnBurst(200, 90, 45, RGB(68, 255, 68));
                    
                    char msgBuf[256];
                    wsprintfA(msgBuf, "Spectral Analysis complete! Revealed:\n%s", unanalyzed[scanItemIdx].clue);
                    MessageBoxA(hwnd, msgBuf, "Success", MB_OK | MB_ICONINFORMATION);
                    
                    SendMessageA(hListUnanalyzed, LB_DELETESTRING, scanItemIdx, 0);
                    for (int i = scanItemIdx; i < numUnanalyzed - 1; i++) {
                        unanalyzed[i] = unanalyzed[i+1];
                    }
                    numUnanalyzed--;
                    
                    currentState = 3;
                    UpdateUI();
                } else if (scanMoves <= 0) {
                    TriggerShake(8.0f);
                    MessageBoxA(hwnd, "Calibration failed: Sensor overload. Try again.", "Failed", MB_OK | MB_ICONERROR);
                    failedCalibrations++;
                    currentState = 3;
                    UpdateUI();
                } else {
                    char scanBuf[256];
                    wsprintfA(scanBuf, "Calibrate Spectral Scanner to match frequency:\nTarget: %d Hz | Tuned: %d Hz | Stability Moves: %d", scanTarget, scanCurrent, scanMoves);
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
                    int hoursTaken = initialTime - timeLeft;
                    int penaltyFree = (failedCalibrations == 0 && angrySuspects == 0);
                    
                    statsCasesSolved++;
                    if (hoursTaken < statsFastestSolve) statsFastestSolve = hoursTaken;
                    if (penaltyFree) statsPerfectSolves++;
                    SaveStats();
                    
                    PlayDramaticChord();
                    SpawnBurst(200, 90, 80, RGB(212, 175, 55));
                    
                    char msgBuf[512];
                    if (penaltyFree) {
                        wsprintfA(msgBuf, "Case Closed, Detective! %s broke down and confessed everything.\n\nTime taken: %dh\nPERFECT INVESTIGATION! (No failed labs, no angry suspects)\nGAME OVER - YOU WIN! (Restart to play again)", suspects[sIdx], hoursTaken);
                    } else {
                        wsprintfA(msgBuf, "Case Closed, Detective! %s broke down and confessed everything.\n\nTime taken: %dh\nGAME OVER - YOU WIN! (Restart to play again)", suspects[sIdx], hoursTaken);
                    }
                    MessageBoxA(hwnd, msgBuf, "You Win!", MB_OK | MB_ICONINFORMATION);
                } else {
                    PlayDramaticChord();
                    TriggerShake(25.0f);
                    char msgBuf[256];
                    wsprintfA(msgBuf, "Disastrous mistake! The real culprit was %s. The commissioner has revoked your badge. GAME OVER.", suspects[currentSolution.killerIdx]);
                    MessageBoxA(hwnd, msgBuf, "Game Over", MB_OK | MB_ICONERROR);
                }
                currentState = 0;
                UpdateUI();
            } else if (id == ID_BTN_HELP) {
                prevState = currentState;
                currentState = 6;
                UpdateUI();
                RECT r; GetClientRect(hwnd, &r); SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_CLOSE_HELP) {
                currentState = prevState;
                UpdateUI();
                RECT r; GetClientRect(hwnd, &r); SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(7, 7, 9));
            SetTextColor(hdc, RGB(216, 216, 216));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(14, 14, 18));
            SetTextColor(hdc, RGB(212, 175, 55));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            
            HPEN hPen1 = CreatePen(PS_SOLID, 2, RGB(90, 72, 40));
            HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(50, 40, 20));
            
            HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldBrush = SelectObject(hdc, hNullBrush);
            
            HGDIOBJ oldPen = SelectObject(hdc, hPen1);
            Rectangle(hdc, 6, 6, rc.right - 6, rc.bottom - 6);
            
            SelectObject(hdc, hPen2);
            Rectangle(hdc, 10, 10, rc.right - 10, rc.bottom - 10);
            
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

    hMainWnd = CreateWindowExA(
        0, "KMysteryClass", "KMystery",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 840, 640,
        NULL, NULL, wc.hInstance, NULL
    );

    if (hMainWnd) {
        ShowWindow(hMainWnd, SW_SHOWDEFAULT);
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}

