#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DISCS 10
#define MAX_PEGS 5
#define TOTAL_STAGES 15

typedef struct {
    int id;
    const char* name;
    int disks;
    int pegs;
    int timeLimit;
    int moveLimit;
    BOOL adjOnly;
    int lockedDisk;
    int lockDuration;
    int par;
} StageConfig;

StageConfig CAMPAIGN_STAGES[TOTAL_STAGES] = {
    { 1,  "Beginner's Stack", 3, 3, 0,  0,  FALSE, 0, 0, 7 },
    { 2,  "Step Up",          4, 3, 0,  0,  FALSE, 0, 0, 15 },
    { 3,  "Linear Steps",      3, 3, 0,  0,  TRUE,  0, 0, 26 },
    { 4,  "Reve's Intro",      5, 4, 0,  0,  FALSE, 0, 0, 13 },
    { 5,  "Sprint Trial",      4, 3, 40, 0,  FALSE, 0, 0, 15 },
    { 6,  "Move Efficiency",  4, 3, 0,  20, FALSE, 0, 0, 15 },
    { 7,  "Quad Towers",       6, 4, 0,  0,  FALSE, 0, 0, 17 },
    { 8,  "Locked Foundation", 4, 3, 0,  0,  FALSE, 4, 5, 15 },
    { 9,  "Penta Realm",       7, 5, 0,  0,  FALSE, 0, 0, 19 },
    { 10, "Chain Migration",   4, 4, 0,  0,  TRUE,  0, 0, 15 },
    { 11, "Clockwork Tower",   5, 3, 60, 0,  FALSE, 0, 0, 31 },
    { 12, "Reve's Master",     8, 4, 0,  0,  FALSE, 0, 0, 33 },
    { 13, "Precision Stack",   5, 3, 0,  38, FALSE, 0, 0, 31 },
    { 14, "Heavy Chains",      5, 4, 0,  0,  TRUE,  5, 8, 21 },
    { 15, "Grandmaster Summit",10,5, 0,  0,  FALSE, 0, 0, 31 }
};

COLORREF colors[10] = {
    RGB(239, 68, 68),   // #ef4444
    RGB(249, 115, 22),  // #f97316
    RGB(245, 158, 11),  // #f59e0b
    RGB(16, 185, 129),  // #10b981
    RGB(6, 182, 212),   // #06b6d4
    RGB(59, 130, 246),  // #3b82f6
    RGB(99, 102, 241),  // #6366f1
    RGB(139, 92, 246),  // #8b5cf6
    RGB(236, 72, 153),  // #ec4899
    RGB(244, 63, 94)    // #f43f5e
};

typedef struct {
    int stars;
    int moves;
    int time;
} StageSaveData;

StageSaveData stageStats[TOTAL_STAGES + 1];

void LoadStats() {
    FILE* fp = fopen("ktowers_stats.dat", "rb");
    if (fp) {
        fread(stageStats, sizeof(StageSaveData), TOTAL_STAGES + 1, fp);
        fclose(fp);
    } else {
        memset(stageStats, 0, sizeof(stageStats));
    }
}

void SaveStats() {
    FILE* fp = fopen("ktowers_stats.dat", "wb");
    if (fp) {
        fwrite(stageStats, sizeof(StageSaveData), TOTAL_STAGES + 1, fp);
        fclose(fp);
    }
}

// Global Game State
int mode = 0; // 0 = Campaign, 1 = Free Play
int currentStageIdx = 0;
int numDiscs = 4;
int numPegs = 3;

int pegs[MAX_PEGS][MAX_DISCS];
int pegCounts[MAX_PEGS] = {0};
int selectedPeg = -1;
int moves = 0;
BOOL won = FALSE;
BOOL gameOver = FALSE;

int historyFrom[4096];
int historyTo[4096];
int historyCount = 0;

int elapsedSeconds = 0;
int freezeSeconds = 0;
int freezeCharges = 3;
BOOL timerRunning = FALSE;
BOOL autoSolving = FALSE;

int hintFrom = -1;
int hintTo = -1;
char statusMessage[256] = "";

// Controls
HWND hModeBtn, hStageMinusBtn, hStagePlusBtn, hStageLabel;
HWND hPegMinusBtn, hPegPlusBtn, hDiscMinusBtn, hDiscPlusBtn;
HWND hUndoBtn, hHintBtn, hFreezeBtn, hAutoBtn, hRestartBtn, hHelpBtn;

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(INT_PTR)lpParam;
    if (type == 1) { // pickup
        Beep(600, 40);
    } else if (type == 2) { // drop
        Beep(450, 40);
    } else if (type == 3) { // error
        Beep(180, 100);
    } else if (type == 4) { // freeze
        Beep(800, 80); Beep(1200, 120);
    } else if (type == 5) { // win
        Beep(523, 80); Beep(659, 80); Beep(784, 80); Beep(1046, 160);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(INT_PTR)type, 0, NULL);
}

// ----------------------------------------------------
// Frame-Stewart & Solver
// ----------------------------------------------------
int FS_DP[12][6];
int FS_SPLIT[12][6];

void InitFrameStewartDP() {
    for (int k = 3; k <= 5; k++) {
        FS_DP[1][k] = 1;
        FS_SPLIT[1][k] = 1;
    }
    for (int n = 1; n <= 10; n++) {
        FS_DP[n][3] = (1 << n) - 1;
        FS_SPLIT[n][3] = 1;
    }
    for (int k = 4; k <= 5; k++) {
        for (int n = 2; n <= 10; n++) {
            int minCost = 999999;
            int bestK = 1;
            for (int r = 1; r < n; r++) {
                int cost = 2 * FS_DP[n - r][k] + FS_DP[r][k - 1];
                if (cost < minCost) {
                    minCost = cost;
                    bestK = r;
                }
            }
            FS_DP[n][k] = minCost;
            FS_SPLIT[n][k] = bestK;
        }
    }
}

StageConfig GetCurrentConfig() {
    if (mode == 0) {
        return CAMPAIGN_STAGES[currentStageIdx];
    } else {
        StageConfig cfg;
        cfg.id = 0;
        cfg.name = "Free Play Mode";
        cfg.disks = numDiscs;
        cfg.pegs = numPegs;
        cfg.timeLimit = 0;
        cfg.moveLimit = 0;
        cfg.adjOnly = FALSE;
        cfg.lockedDisk = 0;
        cfg.lockDuration = 0;
        cfg.par = FS_DP[numDiscs][numPegs];
        return cfg;
    }
}

// BFS Solver for arbitrary state
typedef struct {
    int pegs[MAX_PEGS][MAX_DISCS];
    int pegCounts[MAX_PEGS];
    int firstFrom;
    int firstTo;
    int depth;
} BFSState;

BFSState bfsQueue[5000];

BOOL GetBFSNextMove(int* outFrom, int* outTo) {
    StageConfig cfg = GetCurrentConfig();
    int targetPeg = numPegs - 1;
    if (pegCounts[targetPeg] == numDiscs) return FALSE;

    int head = 0, tail = 0;

    // Initial state
    memcpy(bfsQueue[tail].pegs, pegs, sizeof(pegs));
    memcpy(bfsQueue[tail].pegCounts, pegCounts, sizeof(pegCounts));
    bfsQueue[tail].firstFrom = -1;
    bfsQueue[tail].firstTo = -1;
    bfsQueue[tail].depth = 0;
    tail++;

    while (head < tail && tail < 4800) {
        BFSState curr = bfsQueue[head++];

        if (curr.pegCounts[targetPeg] == numDiscs) {
            *outFrom = curr.firstFrom;
            *outTo = curr.firstTo;
            return TRUE;
        }

        if (curr.depth > 30) continue;

        for (int f = 0; f < numPegs; f++) {
            if (curr.pegCounts[f] == 0) continue;
            int topDisc = curr.pegs[f][curr.pegCounts[f] - 1];

            // Lock check
            if (cfg.lockedDisk > 0 && topDisc == cfg.lockedDisk && (moves + curr.depth) < cfg.lockDuration) {
                continue;
            }

            for (int t = 0; t < numPegs; t++) {
                if (f == t) continue;
                if (cfg.adjOnly && abs(f - t) != 1) continue;

                if (curr.pegCounts[t] == 0 || curr.pegs[t][curr.pegCounts[t] - 1] > topDisc) {
                    BFSState nextState = curr;
                    nextState.pegCounts[f]--;
                    nextState.pegs[t][nextState.pegCounts[t]++] = topDisc;
                    nextState.depth++;

                    if (nextState.firstFrom == -1) {
                        nextState.firstFrom = f;
                        nextState.firstTo = t;
                    }

                    bfsQueue[tail++] = nextState;
                    if (tail >= 4800) break;
                }
            }
        }
    }
    return FALSE;
}

void UpdateControlsVisibility() {
    if (mode == 0) {
        ShowWindow(hStageMinusBtn, SW_SHOW);
        ShowWindow(hStagePlusBtn, SW_SHOW);
        ShowWindow(hStageLabel, SW_SHOW);
        ShowWindow(hPegMinusBtn, SW_HIDE);
        ShowWindow(hPegPlusBtn, SW_HIDE);
        ShowWindow(hDiscMinusBtn, SW_HIDE);
        ShowWindow(hDiscPlusBtn, SW_HIDE);

        char buf[64];
        StageSaveData st = stageStats[CAMPAIGN_STAGES[currentStageIdx].id];
        char starStr[8] = "";
        for (int i = 0; i < st.stars; i++) strcat(starStr, "*");
        sprintf(buf, "Stage %d / 15 %s", currentStageIdx + 1, starStr);
        SetWindowText(hStageLabel, buf);
        SetWindowText(hModeBtn, "Mode: Campaign");
    } else {
        ShowWindow(hStageMinusBtn, SW_HIDE);
        ShowWindow(hStagePlusBtn, SW_HIDE);
        ShowWindow(hStageLabel, SW_SHOW);
        ShowWindow(hPegMinusBtn, SW_SHOW);
        ShowWindow(hPegPlusBtn, SW_SHOW);
        ShowWindow(hDiscMinusBtn, SW_SHOW);
        ShowWindow(hDiscPlusBtn, SW_SHOW);

        char buf[64];
        sprintf(buf, "%d Discs | %d Pegs", numDiscs, numPegs);
        SetWindowText(hStageLabel, buf);
        SetWindowText(hModeBtn, "Mode: Free Play");
    }
}

void InitGame(HWND hwnd) {
    StageConfig cfg = GetCurrentConfig();
    numPegs = cfg.pegs;
    numDiscs = cfg.disks;

    memset(pegCounts, 0, sizeof(pegCounts));
    for (int i = numDiscs; i >= 1; i--) {
        pegs[0][pegCounts[0]++] = i;
    }

    selectedPeg = -1;
    moves = 0;
    won = FALSE;
    gameOver = FALSE;
    historyCount = 0;
    elapsedSeconds = 0;
    freezeSeconds = 0;
    freezeCharges = 3;
    hintFrom = -1;
    hintTo = -1;
    strcpy(statusMessage, "");

    if (timerRunning) {
        KillTimer(hwnd, 1);
        timerRunning = FALSE;
    }
    if (autoSolving) {
        autoSolving = FALSE;
        KillTimer(hwnd, 2);
        SetWindowText(hAutoBtn, "Auto-Solve");
    }

    UpdateControlsVisibility();
    InvalidateRect(hwnd, NULL, TRUE);
}

void CheckWinOrLoss(HWND hwnd) {
    StageConfig cfg = GetCurrentConfig();
    int targetPeg = numPegs - 1;

    if (pegCounts[targetPeg] == numDiscs) {
        won = TRUE;
        if (timerRunning) {
            KillTimer(hwnd, 1);
            timerRunning = FALSE;
        }

        int stars = 1;
        if (moves <= cfg.par * 1.2) stars = 3;
        else if (moves <= cfg.par * 1.6) stars = 2;

        if (mode == 0) {
            int sid = cfg.id;
            if (stars > stageStats[sid].stars || (stars == stageStats[sid].stars && moves < stageStats[sid].moves)) {
                stageStats[sid].stars = stars;
                stageStats[sid].moves = moves;
                stageStats[sid].time = elapsedSeconds;
                SaveStats();
            }
        }

        PlaySoundEffect(5);
        sprintf(statusMessage, "STAGE CLEARED! Stars: %d | Moves: %d | Time: %02d:%02d", stars, moves, elapsedSeconds/60, elapsedSeconds%60);
    } else if (cfg.moveLimit > 0 && moves > cfg.moveLimit) {
        gameOver = TRUE;
        if (timerRunning) {
            KillTimer(hwnd, 1);
            timerRunning = FALSE;
        }
        PlaySoundEffect(3);
        sprintf(statusMessage, "STAGE FAILED! Exceeded move limit of %d!", cfg.moveLimit);
    }
}

void PerformPegClick(HWND hwnd, int clickedPeg) {
    if (won || gameOver) return;
    if (clickedPeg < 0 || clickedPeg >= numPegs) return;

    StageConfig cfg = GetCurrentConfig();

    if (selectedPeg == -1) {
        if (pegCounts[clickedPeg] > 0) {
            int topDisc = pegs[clickedPeg][pegCounts[clickedPeg] - 1];
            if (cfg.lockedDisk > 0 && topDisc == cfg.lockedDisk && moves < cfg.lockDuration) {
                sprintf(statusMessage, "Disk %d is locked for %d more moves!", cfg.lockedDisk, cfg.lockDuration - moves);
                PlaySoundEffect(3);
                InvalidateRect(hwnd, NULL, TRUE);
                return;
            }
            selectedPeg = clickedPeg;
            hintFrom = -1; hintTo = -1;
            strcpy(statusMessage, "");
            PlaySoundEffect(1);
        } else {
            PlaySoundEffect(3);
        }
    } else if (selectedPeg == clickedPeg) {
        selectedPeg = -1;
        PlaySoundEffect(2);
    } else {
        if (cfg.adjOnly && abs(selectedPeg - clickedPeg) != 1) {
            strcpy(statusMessage, "Adjacent Pegs Only!");
            selectedPeg = -1;
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, TRUE);
            return;
        }

        int movingDisc = pegs[selectedPeg][pegCounts[selectedPeg] - 1];
        BOOL canMove = FALSE;
        if (pegCounts[clickedPeg] == 0) {
            canMove = TRUE;
        } else {
            int targetTop = pegs[clickedPeg][pegCounts[clickedPeg] - 1];
            if (movingDisc < targetTop) {
                canMove = TRUE;
            }
        }

        if (canMove) {
            historyFrom[historyCount] = selectedPeg;
            historyTo[historyCount] = clickedPeg;
            historyCount++;
            pegCounts[selectedPeg]--;
            pegs[clickedPeg][pegCounts[clickedPeg]++] = movingDisc;
            moves++;
            selectedPeg = -1;
            hintFrom = -1; hintTo = -1;
            strcpy(statusMessage, "");

            if (!timerRunning) {
                timerRunning = TRUE;
                SetTimer(hwnd, 1, 1000, NULL);
            }

            CheckWinOrLoss(hwnd);
            if (!won && !gameOver) {
                PlaySoundEffect(2);
            }
        } else {
            strcpy(statusMessage, "Cannot place larger disk on smaller disk!");
            selectedPeg = -1;
            PlaySoundEffect(3);
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void UndoMove(HWND hwnd) {
    if (historyCount == 0 || won || gameOver) return;
    historyCount--;
    int f = historyFrom[historyCount];
    int t = historyTo[historyCount];

    int disc = pegs[t][pegCounts[t] - 1];
    pegCounts[t]--;
    pegs[f][pegCounts[f]++] = disc;
    moves--;
    selectedPeg = -1;
    hintFrom = -1; hintTo = -1;
    strcpy(statusMessage, "");
    PlaySoundEffect(2);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyHint(HWND hwnd) {
    if (won || gameOver) return;
    int f, t;
    if (GetBFSNextMove(&f, &t)) {
        hintFrom = f;
        hintTo = t;
        InvalidateRect(hwnd, NULL, TRUE);
    } else {
        strcpy(statusMessage, "No hint available!");
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void UseTimeFreeze(HWND hwnd) {
    if (won || gameOver) return;
    if (freezeCharges <= 0) {
        strcpy(statusMessage, "No Freeze charges remaining!");
        PlaySoundEffect(3);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    freezeCharges--;
    freezeSeconds += 15;
    PlaySoundEffect(4);
    if (!timerRunning) {
        timerRunning = TRUE;
        SetTimer(hwnd, 1, 1000, NULL);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitFrameStewartDP();
            LoadStats();

            hModeBtn = CreateWindow("BUTTON", "Mode: Campaign", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  10, 10, 120, 28, hwnd, (HMENU)101, NULL, NULL);
            hStageMinusBtn = CreateWindow("BUTTON", "<", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         140, 10, 25, 28, hwnd, (HMENU)102, NULL, NULL);
            hStageLabel = CreateWindow("STATIC", "Stage 1 / 15", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                                       170, 10, 140, 28, hwnd, (HMENU)103, NULL, NULL);
            hStagePlusBtn = CreateWindow("BUTTON", ">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        315, 10, 25, 28, hwnd, (HMENU)104, NULL, NULL);

            hPegMinusBtn = CreateWindow("BUTTON", "-P", WS_CHILD | BS_PUSHBUTTON, 345, 10, 30, 28, hwnd, (HMENU)105, NULL, NULL);
            hPegPlusBtn = CreateWindow("BUTTON", "+P", WS_CHILD | BS_PUSHBUTTON, 380, 10, 30, 28, hwnd, (HMENU)106, NULL, NULL);
            hDiscMinusBtn = CreateWindow("BUTTON", "-D", WS_CHILD | BS_PUSHBUTTON, 415, 10, 30, 28, hwnd, (HMENU)107, NULL, NULL);
            hDiscPlusBtn = CreateWindow("BUTTON", "+D", WS_CHILD | BS_PUSHBUTTON, 450, 10, 30, 28, hwnd, (HMENU)108, NULL, NULL);

            hUndoBtn = CreateWindow("BUTTON", "Undo [U]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 45, 80, 28, hwnd, (HMENU)1, NULL, NULL);
            hHintBtn = CreateWindow("BUTTON", "Hint [H]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 95, 45, 80, 28, hwnd, (HMENU)2, NULL, NULL);
            hFreezeBtn = CreateWindow("BUTTON", "Freeze [F]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 45, 85, 28, hwnd, (HMENU)3, NULL, NULL);
            hAutoBtn = CreateWindow("BUTTON", "Auto-Solve", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 270, 45, 95, 28, hwnd, (HMENU)4, NULL, NULL);
            hRestartBtn = CreateWindow("BUTTON", "Restart [R]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 45, 85, 28, hwnd, (HMENU)5, NULL, NULL);
            hHelpBtn = CreateWindow("BUTTON", "Help [?]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 460, 45, 75, 28, hwnd, (HMENU)6, NULL, NULL);

            InitGame(hwnd);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) { // 1 sec tick
                if (!won && !gameOver) {
                    if (freezeSeconds > 0) {
                        freezeSeconds--;
                    } else {
                        elapsedSeconds++;
                    }
                    StageConfig cfg = GetCurrentConfig();
                    if (cfg.timeLimit > 0 && elapsedSeconds >= cfg.timeLimit) {
                        gameOver = TRUE;
                        KillTimer(hwnd, 1);
                        timerRunning = FALSE;
                        PlaySoundEffect(3);
                        sprintf(statusMessage, "STAGE FAILED! Time limit expired!");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 2) { // Auto solve step
                if (won || gameOver) {
                    autoSolving = FALSE;
                    KillTimer(hwnd, 2);
                    SetWindowText(hAutoBtn, "Auto-Solve");
                } else {
                    int f, t;
                    if (GetBFSNextMove(&f, &t)) {
                        PerformPegClick(hwnd, f);
                        PerformPegClick(hwnd, t);
                    } else {
                        autoSolving = FALSE;
                        KillTimer(hwnd, 2);
                        SetWindowText(hAutoBtn, "Auto-Solve");
                    }
                }
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 101) { // Mode Toggle
                mode = (mode == 0) ? 1 : 0;
                InitGame(hwnd);
            } else if (id == 102) { // Stage -
                if (currentStageIdx > 0) {
                    currentStageIdx--;
                    InitGame(hwnd);
                }
            } else if (id == 104) { // Stage +
                if (currentStageIdx < TOTAL_STAGES - 1) {
                    BOOL isUnlocked = (currentStageIdx == 0) || (stageStats[CAMPAIGN_STAGES[currentStageIdx].id].stars > 0);
                    if (isUnlocked) {
                        currentStageIdx++;
                        InitGame(hwnd);
                    }
                }
            } else if (id == 105) { if (numPegs > 3) { numPegs--; InitGame(hwnd); } }
            else if (id == 106) { if (numPegs < 5) { numPegs++; InitGame(hwnd); } }
            else if (id == 107) { if (numDiscs > 3) { numDiscs--; InitGame(hwnd); } }
            else if (id == 108) { if (numDiscs < 10) { numDiscs++; InitGame(hwnd); } }
            else if (id == 1) UndoMove(hwnd);
            else if (id == 2) ApplyHint(hwnd);
            else if (id == 3) UseTimeFreeze(hwnd);
            else if (id == 4) {
                if (autoSolving) {
                    autoSolving = FALSE;
                    KillTimer(hwnd, 2);
                    SetWindowText(hAutoBtn, "Auto-Solve");
                } else {
                    if (!won && !gameOver) {
                        autoSolving = TRUE;
                        SetTimer(hwnd, 2, 400, NULL);
                        SetWindowText(hAutoBtn, "Stop Auto");
                    }
                }
            } else if (id == 5) InitGame(hwnd);
            else if (id == 6) {
                MessageBox(hwnd,
                    "How to Play KTowers\n\n"
                    "Goal: Move all discs to the target (last) peg.\n\n"
                    "Rules:\n"
                    "- Only top discs can be moved.\n"
                    "- Larger discs cannot be placed on smaller discs.\n"
                    "- Adjacent Only: Discs can only move to neighboring pegs.\n"
                    "- Locked Disks: Cannot move until turn requirement is met.\n\n"
                    "Controls:\n"
                    "- Click peg to pick/drop disk or use keys 1 to 5.\n"
                    "- [U] Undo last move.\n"
                    "- [H] Optimal Hint (Frame-Stewart algorithm).\n"
                    "- [F] Time Freeze (pauses timer for 15s).",
                    "Help / Instructions", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam >= '1' && wParam <= '5') {
                int p = (int)(wParam - '1');
                if (p < numPegs) PerformPegClick(hwnd, p);
            } else if (wParam == 'U' || wParam == 'u') UndoMove(hwnd);
            else if (wParam == 'H' || wParam == 'h') ApplyHint(hwnd);
            else if (wParam == 'F' || wParam == 'f') UseTimeFreeze(hwnd);
            else if (wParam == 'R' || wParam == 'r') InitGame(hwnd);
            else if (wParam == 'A' || wParam == 'a') {
                SendMessage(hwnd, WM_COMMAND, 4, 0);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (y > 140) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                int w = rc.right - rc.left;
                int pegAreaW = w / numPegs;
                int clickedPeg = x / pegAreaW;
                PerformPegClick(hwnd, clickedPeg);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;

            // Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rc, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(248, 250, 252));

            StageConfig cfg = GetCurrentConfig();

            // Status Banner Line 1: Stage Info & Rules
            char line1[256];
            char rulesStr[128] = "";
            if (cfg.adjOnly) strcat(rulesStr, " [Adjacent Only]");
            if (cfg.moveLimit > 0) sprintf(rulesStr + strlen(rulesStr), " [Max %d Moves]", cfg.moveLimit);
            if (cfg.timeLimit > 0) sprintf(rulesStr + strlen(rulesStr), " [Time Limit %ds]", cfg.timeLimit);
            if (cfg.lockedDisk > 0) sprintf(rulesStr + strlen(rulesStr), " [Disk %d Locked %dm]", cfg.lockedDisk, cfg.lockDuration);

            sprintf(line1, "%s %s", cfg.name, rulesStr);
            HFONT fontBold = CreateFont(18, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(hdc, fontBold);
            TextOut(hdc, 15, 80, line1, strlen(line1));

            // Line 2: Stats & Freeze badge
            char line2[256];
            char freezeStr[32] = "";
            if (freezeSeconds > 0) sprintf(freezeStr, " | FROZEN (%ds)", freezeSeconds);
            sprintf(line2, "Time: %02d:%02d%s | Moves: %d / Par: %d | Freeze Charges: %d",
                    elapsedSeconds/60, elapsedSeconds%60, freezeStr, moves, cfg.par, freezeCharges);
            SetTextColor(hdc, freezeSeconds > 0 ? RGB(6, 182, 212) : RGB(148, 163, 184));
            TextOut(hdc, 15, 105, line2, strlen(line2));

            if (strlen(statusMessage) > 0) {
                SetTextColor(hdc, won ? RGB(34, 197, 94) : RGB(239, 68, 68));
                TextOut(hdc, 15, 130, statusMessage, strlen(statusMessage));
            }

            SelectObject(hdc, oldFont);
            DeleteObject(fontBold);

            // Draw Pegs & Disks
            int pegAreaWidth = width / numPegs;
            int groundY = height - 60;

            HBRUSH poleBrush = CreateSolidBrush(RGB(100, 116, 139));
            HBRUSH baseBrush = CreateSolidBrush(RGB(71, 85, 105));

            for (int i = 0; i < numPegs; i++) {
                int baseX = pegAreaWidth * i + pegAreaWidth / 2;

                // Peg Selection / Hint highlight
                if (selectedPeg == i || hintFrom == i || hintTo == i) {
                    COLORREF hlColor = (selectedPeg == i) ? RGB(56, 189, 248) :
                                       (hintFrom == i) ? RGB(245, 158, 11) : RGB(34, 197, 94);
                    HBRUSH hlBrush = CreateSolidBrush(hlColor);
                    RECT hlRect = { baseX - pegAreaWidth/2 + 8, groundY - 220, baseX + pegAreaWidth/2 - 8, groundY + 12 };
                    FrameRect(hdc, &hlRect, hlBrush);
                    DeleteObject(hlBrush);
                }

                // Pole
                RECT poleRect = { baseX - 5, groundY - 200, baseX + 5, groundY };
                FillRect(hdc, &poleRect, poleBrush);

                // Base
                RECT bRect = { baseX - pegAreaWidth/2 + 10, groundY, baseX + pegAreaWidth/2 - 10, groundY + 8 };
                FillRect(hdc, &bRect, baseBrush);

                // Label
                char pLabel[32];
                sprintf(pLabel, "Peg %d%s", i + 1, (i == numPegs - 1) ? " (Target)" : "");
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOut(hdc, baseX - 30, groundY + 12, pLabel, strlen(pLabel));
            }

            DeleteObject(poleBrush);
            DeleteObject(baseBrush);

            // Draw Discs
            for (int p = 0; p < numPegs; p++) {
                int baseX = pegAreaWidth * p + pegAreaWidth / 2;
                for (int j = 0; j < pegCounts[p]; j++) {
                    int discSize = pegs[p][j];
                    int discW = (int)((30.0 + ((double)discSize / numDiscs) * (pegAreaWidth - 40)));
                    int discH = 20;
                    int rectY = groundY - (j + 1) * (discH + 2);

                    RECT dRect = { baseX - discW / 2, rectY, baseX + discW / 2, rectY + discH };

                    BOOL isLocked = (cfg.lockedDisk > 0 && discSize == cfg.lockedDisk && moves < cfg.lockDuration);
                    COLORREF dColor = isLocked ? RGB(71, 85, 105) : colors[(discSize - 1) % 10];

                    HBRUSH dBrush = CreateSolidBrush(dColor);
                    FillRect(hdc, &dRect, dBrush);

                    HPEN dPen = CreatePen(PS_SOLID, isLocked ? 2 : 1, isLocked ? RGB(239, 68, 68) : RGB(0, 0, 0));
                    HGDIOBJ oPen = SelectObject(hdc, dPen);
                    HGDIOBJ oBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

                    RoundRect(hdc, dRect.left, dRect.top, dRect.right, dRect.bottom, 8, 8);

                    SelectObject(hdc, oBrush);
                    SelectObject(hdc, oPen);
                    DeleteObject(dPen);
                    DeleteObject(dBrush);

                    // Disc text
                    char dText[16];
                    if (isLocked) sprintf(dText, "%d LOCK", discSize);
                    else sprintf(dText, "%d", discSize);

                    SetTextColor(hdc, RGB(255, 255, 255));
                    TextOut(hdc, baseX - (isLocked ? 18 : 4), rectY + 2, dText, strlen(dText));
                }
            }

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));
    wc.lpszClassName = "KTowersClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("KTowersClass", "KTowers", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 900, 650,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
