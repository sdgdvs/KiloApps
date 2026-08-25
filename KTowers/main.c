#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_DISCS 10
#define MAX_PEGS 5
#define TOTAL_STAGES 20
#define MAX_PARTICLES 250

typedef struct {
    int id;
    const char* name;
    int disks;
    int pegs;
    int timeLimit;
    int moveLimit;
    BOOL adjOnly;
    BOOL cyclic;
    int colorRestr; // 0 = None, 1 = Peg 2 accepts even disks only
    int lockedDisk;
    int lockDuration;
    int par;
} StageConfig;

StageConfig CAMPAIGN_STAGES[TOTAL_STAGES] = {
    { 1,  "Beginner's Stack",  3, 3, 0,  0,  FALSE, FALSE, 0, 0, 0, 7 },
    { 2,  "Step Up",           4, 3, 0,  0,  FALSE, FALSE, 0, 0, 0, 15 },
    { 3,  "Linear Steps",       3, 3, 0,  0,  TRUE,  FALSE, 0, 0, 0, 26 },
    { 4,  "Reve's Intro",       5, 4, 0,  0,  FALSE, FALSE, 0, 0, 0, 13 },
    { 5,  "Sprint Trial",       4, 3, 40, 0,  FALSE, FALSE, 0, 0, 0, 15 },
    { 6,  "Move Efficiency",   4, 3, 0,  20, FALSE, FALSE, 0, 0, 0, 15 },
    { 7,  "Quad Towers",        6, 4, 0,  0,  FALSE, FALSE, 0, 0, 0, 17 },
    { 8,  "Locked Foundation",  4, 3, 0,  0,  FALSE, FALSE, 0, 4, 5, 15 },
    { 9,  "Penta Realm",        7, 5, 0,  0,  FALSE, FALSE, 0, 0, 0, 19 },
    { 10, "Chain Migration",    4, 4, 0,  0,  TRUE,  FALSE, 0, 0, 0, 34 },
    { 11, "Clockwork Tower",    5, 3, 60, 0,  FALSE, FALSE, 0, 0, 0, 31 },
    { 12, "Cyclic Orbit",       4, 3, 0,  0,  FALSE, TRUE,  0, 0, 0, 59 },
    { 13, "Reve's Master",      8, 4, 0,  0,  FALSE, FALSE, 0, 0, 0, 33 },
    { 14, "Spectrum Filter",    4, 4, 0,  0,  FALSE, FALSE, 1, 0, 0, 9 },
    { 15, "Precision Stack",    5, 3, 0,  38, FALSE, FALSE, 0, 0, 0, 31 },
    { 16, "Heavy Chains",       5, 4, 0,  0,  TRUE,  FALSE, 0, 5, 8, 57 },
    { 17, "Cyclic Cascade",     5, 4, 0,  0,  FALSE, TRUE,  0, 0, 0, 75 },
    { 18, "Chromatic Citadel",  6, 4, 90, 0,  FALSE, FALSE, 1, 0, 0, 21 },
    { 19, "Pentagonal Matrix",  9, 5, 0,  0,  FALSE, FALSE, 0, 0, 0, 27 },
    { 20, "Tower Grandmaster", 10, 5, 0,  0,  FALSE, FALSE, 0, 0, 0, 31 }
};

typedef struct {
    COLORREF main;
    COLORREF side;
    COLORREF top;
} DiscTheme;

DiscTheme DISC_THEMES[10] = {
    { RGB(239, 68, 68),  RGB(185, 28, 28),  RGB(252, 165, 165) },
    { RGB(249, 115, 22), RGB(194, 65, 12),  RGB(253, 186, 116) },
    { RGB(245, 158, 11), RGB(180, 83, 9),   RGB(253, 224, 71)  },
    { RGB(16, 185, 129), RGB(4, 120, 87),   RGB(110, 231, 183) },
    { RGB(6, 182, 212),  RGB(14, 116, 144), RGB(103, 232, 249) },
    { RGB(59, 130, 246), RGB(29, 78, 216),  RGB(147, 197, 253) },
    { RGB(99, 102, 241), RGB(67, 56, 202),  RGB(165, 180, 252) },
    { RGB(139, 92, 246), RGB(109, 40, 217), RGB(196, 181, 253) },
    { RGB(236, 72, 153), RGB(190, 24, 93),  RGB(249, 168, 212) },
    { RGB(244, 63, 94),  RGB(190, 18, 60),  RGB(253, 164, 175) }
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
int hoverPeg = -1;
int moves = 0;
BOOL won = FALSE;
BOOL gameOver = FALSE;

int historyFrom[4096];
int historyTo[4096];
int historyCount = 0;

int elapsedSeconds = 0;
int freezeSeconds = 0;
int freezeCharges = 3;
int swapCharges = 3;
BOOL timerRunning = FALSE;
BOOL autoSolving = FALSE;

int hintFrom = -1;
int hintTo = -1;
char statusMessage[256] = "";

// Graphics / Animation State
int animTick = 0;
float discAnimY[MAX_PEGS][MAX_DISCS];

typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    int life;
    int maxLife;
} Particle;

Particle particles[MAX_PARTICLES];
int particleCount = 0;

int screenShake = 0;
typedef struct { float x, y, vx, vy; int life; } DustParticle;
#define MAX_DUST 100
DustParticle dusts[MAX_DUST];

// Controls
HWND hModeBtn, hStageMinusBtn, hStagePlusBtn, hStageLabel;
HWND hPegMinusBtn, hPegPlusBtn, hDiscMinusBtn, hDiscPlusBtn;
HWND hUndoBtn, hHintBtn, hFreezeBtn, hSwapBtn, hAutoBtn, hRestartBtn, hHelpBtn;

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(INT_PTR)lpParam;
    if (type == 1) {
        Beep(600, 40);
    } else if (type == 2) {
        Beep(450, 40);
    } else if (type == 3) {
        Beep(180, 100);
    } else if (type == 4) {
        Beep(800, 80); Beep(1200, 120);
    } else if (type == 5) {
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
        cfg.cyclic = FALSE;
        cfg.colorRestr = 0;
        cfg.lockedDisk = 0;
        cfg.lockDuration = 0;
        cfg.par = FS_DP[numDiscs][numPegs];
        return cfg;
    }
}

BOOL IsValidMove(StageConfig cfg, int f, int t, int topDisc, int currentMoves) {
    if (f == t) return FALSE;
    if (cfg.adjOnly && abs(f - t) != 1) return FALSE;
    if (cfg.cyclic && ((f + 1) % cfg.pegs != t)) return FALSE;
    if (cfg.lockedDisk > 0 && topDisc == cfg.lockedDisk && currentMoves < cfg.lockDuration) return FALSE;
    if (cfg.colorRestr == 1 && t == 1 && (topDisc % 2 != 0)) return FALSE;
    return TRUE;
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

    bfsQueue[tail].firstFrom = -1;
    bfsQueue[tail].firstTo = -1;
    bfsQueue[tail].depth = 0;
    memcpy(bfsQueue[tail].pegs, pegs, sizeof(pegs));
    memcpy(bfsQueue[tail].pegCounts, pegCounts, sizeof(pegCounts));
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

            if (cfg.lockedDisk > 0 && topDisc == cfg.lockedDisk && (moves + curr.depth) < cfg.lockDuration) {
                continue;
            }

            for (int t = 0; t < numPegs; t++) {
                if (f == t) continue;
                if (cfg.adjOnly && abs(f - t) != 1) continue;
                if (cfg.cyclic && ((f + 1) % numPegs != t)) continue;
                if (cfg.colorRestr == 1 && t == 1 && (topDisc % 2 != 0)) continue;

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
    char buf[64];
    sprintf(buf, "Freeze [F] (%d)", freezeCharges);
    SetWindowText(hFreezeBtn, buf);
    sprintf(buf, "Swap [S] (%d)", swapCharges);
    SetWindowText(hSwapBtn, buf);

    if (mode == 0) {
        ShowWindow(hStageMinusBtn, SW_SHOW);
        ShowWindow(hStagePlusBtn, SW_SHOW);
        ShowWindow(hStageLabel, SW_SHOW);
        ShowWindow(hPegMinusBtn, SW_HIDE);
        ShowWindow(hPegPlusBtn, SW_HIDE);
        ShowWindow(hDiscMinusBtn, SW_HIDE);
        ShowWindow(hDiscPlusBtn, SW_HIDE);

        StageSaveData st = stageStats[CAMPAIGN_STAGES[currentStageIdx].id];
        char starStr[8] = "";
        for (int i = 0; i < st.stars; i++) strcat(starStr, "*");
        sprintf(buf, "Stage %d / 20 %s", currentStageIdx + 1, starStr);
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

        sprintf(buf, "%d Discs | %d Pegs", numDiscs, numPegs);
        SetWindowText(hStageLabel, buf);
        SetWindowText(hModeBtn, "Mode: Free Play");
    }
}

void SpawnFireworks() {
    particleCount = 0;
    COLORREF colors[] = { RGB(244, 63, 94), RGB(56, 189, 248), RGB(245, 158, 11), RGB(16, 185, 129), RGB(168, 85, 247) };
    int cx = 150 + rand() % 550;
    int cy = 100 + rand() % 150;
    COLORREF c = colors[rand() % 5];

    particles[particleCount].x = cx; particles[particleCount].y = cy;
    particles[particleCount].vx = 0; particles[particleCount].vy = 0;
    particles[particleCount].color = RGB(255, 255, 255);
    particles[particleCount].life = 20; particles[particleCount].maxLife = 20;
    particleCount++;

    for (int i = 0; i < 150; i++) {
        particles[particleCount].x = cx; particles[particleCount].y = cy;
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 3.0f + (float)(rand() % 80) / 10.0f;
        particles[particleCount].vx = cosf(angle) * speed; particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].color = c;
        particles[particleCount].life = 30 + rand() % 30; particles[particleCount].maxLife = particles[particleCount].life;
        particleCount++;
    }
    for (int i = 0; i < 50; i++) {
        particles[particleCount].x = cx; particles[particleCount].y = cy;
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (float)(rand() % 30) / 10.0f;
        particles[particleCount].vx = cosf(angle) * speed; particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].color = RGB(255, 255, 255);
        particles[particleCount].life = 40 + rand() % 40; particles[particleCount].maxLife = particles[particleCount].life;
        particleCount++;
    }
}

void UpdateParticles() {
    for (int i = 0; i < particleCount; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            if (particles[i].maxLife != 20) {
                particles[i].vy += 0.15f; // gravity
            }
            particles[i].life--;
        }
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

    for (int p = 0; p < MAX_PEGS; p++) {
        for (int d = 0; d < MAX_DISCS; d++) {
            discAnimY[p][d] = -1.0f;
        }
    }

    selectedPeg = -1;
    moves = 0;
    won = FALSE;
    gameOver = FALSE;
    historyCount = 0;
    elapsedSeconds = 0;
    freezeSeconds = 0;
    freezeCharges = 3;
    swapCharges = 3;
    hintFrom = -1;
    hintTo = -1;
    particleCount = 0;
    screenShake = 0;
    for(int i=0; i<MAX_DUST; i++) dusts[i].life = 0;
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
    InvalidateRect(hwnd, NULL, FALSE);
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

        SpawnFireworks();
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
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
            selectedPeg = clickedPeg;
            hintFrom = -1; hintTo = -1;
            strcpy(statusMessage, "");
            PlaySoundEffect(1);
            screenShake = 6;
        } else {
            PlaySoundEffect(3);
            screenShake = 4;
        }
    } else if (selectedPeg == clickedPeg) {
        selectedPeg = -1;
        PlaySoundEffect(2);
    } else {
        if (cfg.adjOnly && abs(selectedPeg - clickedPeg) != 1) {
            strcpy(statusMessage, "Adjacent Pegs Only!");
            selectedPeg = -1;
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
        if (cfg.cyclic && ((selectedPeg + 1) % numPegs != clickedPeg)) {
            strcpy(statusMessage, "Cyclic Move Only (Clockwise)!");
            selectedPeg = -1;
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }

        int movingDisc = pegs[selectedPeg][pegCounts[selectedPeg] - 1];

        if (cfg.colorRestr == 1 && clickedPeg == 1 && (movingDisc % 2 != 0)) {
            strcpy(statusMessage, "Peg 2 accepts Even Disks only!");
            selectedPeg = -1;
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }

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
            screenShake = 4;
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
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
    InvalidateRect(hwnd, NULL, FALSE);
}

void ApplyHint(HWND hwnd) {
    if (won || gameOver) return;
    int f, t;
    if (GetBFSNextMove(&f, &t)) {
        hintFrom = f;
        hintTo = t;
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        strcpy(statusMessage, "No hint available!");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void UseTimeFreeze(HWND hwnd) {
    if (won || gameOver) return;
    if (freezeCharges <= 0) {
        strcpy(statusMessage, "No Freeze charges remaining!");
        PlaySoundEffect(3);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    freezeCharges--;
    freezeSeconds += 15;
    PlaySoundEffect(4);
    if (!timerRunning) {
        timerRunning = TRUE;
        SetTimer(hwnd, 1, 1000, NULL);
    }
    UpdateControlsVisibility();
    InvalidateRect(hwnd, NULL, FALSE);
}

void UseDiskSwap(HWND hwnd) {
    if (won || gameOver) return;
    if (swapCharges <= 0) {
        strcpy(statusMessage, "No Swap charges remaining!");
        PlaySoundEffect(3);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }

    StageConfig cfg = GetCurrentConfig();
    int srcPeg = -1, targetPeg = -1;

    if (selectedPeg != -1 && pegCounts[selectedPeg] > 0) {
        srcPeg = selectedPeg;
        int topDisc = pegs[srcPeg][pegCounts[srcPeg] - 1];
        int defaultTarget = numPegs - 1;
        if (defaultTarget != srcPeg && IsValidMove(cfg, srcPeg, defaultTarget, topDisc, moves)) {
            if (pegCounts[defaultTarget] == 0 || pegs[defaultTarget][pegCounts[defaultTarget] - 1] > topDisc) {
                targetPeg = defaultTarget;
            }
        }
        if (targetPeg == -1) {
            for (int p = 0; p < numPegs; p++) {
                if (p == srcPeg) continue;
                if (IsValidMove(cfg, srcPeg, p, topDisc, moves)) {
                    if (pegCounts[p] == 0 || pegs[p][pegCounts[p] - 1] > topDisc) {
                        targetPeg = p;
                        break;
                    }
                }
            }
        }
    } else {
        if (!GetBFSNextMove(&srcPeg, &targetPeg)) {
            strcpy(statusMessage, "No valid teleport available!");
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }

    if (srcPeg != -1 && targetPeg != -1) {
        int movingDisc = pegs[srcPeg][pegCounts[srcPeg] - 1];
        historyFrom[historyCount] = srcPeg;
        historyTo[historyCount] = targetPeg;
        historyCount++;
        pegCounts[srcPeg]--;
        pegs[targetPeg][pegCounts[targetPeg]++] = movingDisc;
        moves++;
        swapCharges--;
        selectedPeg = -1;
        hintFrom = -1; hintTo = -1;
        strcpy(statusMessage, "DISK TELEPORTED!");
        PlaySoundEffect(4);
        screenShake = 15;

        if (!timerRunning) {
            timerRunning = TRUE;
            SetTimer(hwnd, 1, 1000, NULL);
        }

        UpdateControlsVisibility();
        CheckWinOrLoss(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        strcpy(statusMessage, "No valid target peg for Teleport!");
        PlaySoundEffect(3);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

// ----------------------------------------------------
// GDI 3D Skyscraper Block Renderer
// ----------------------------------------------------
void Draw3DSkyscraperBlockGDI(HDC hdc, int x, int y, int width, int height, DiscTheme theme, int discSize, BOOL isLocked, BOOL isTopDisc, int targetY, BOOL isWarning) {
    int slant = 8;
    int depth = 10;

    int dropDist = targetY > y ? (targetY - y) : 0;
    int shadowW = width + 8 + (dropDist / 10);
    int shadowH = 12 + (dropDist / 20);
    int sy = targetY + height + 2;

    HBRUSH shadowBrush = CreateSolidBrush(RGB(15, 20, 30));
    HGDIOBJ oldBrush = SelectObject(hdc, shadowBrush);
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HGDIOBJ oldPen = SelectObject(hdc, nullPen);
    Ellipse(hdc, x - shadowW/2, sy - shadowH/2, x + shadowW/2, sy + shadowH/2);
    DeleteObject(shadowBrush);

    BOOL isGlass = (discSize % 2 == 0);

    // 1. Right Side Facade (3D Wall Polygon)
    POINT sidePts[4] = {
        { x + width/2, y },
        { x + width/2 + slant, y - depth },
        { x + width/2 + slant, y + height - depth },
        { x + width/2, y + height }
    };
    HBRUSH sideBrush = CreateSolidBrush(theme.side);
    HGDIOBJ oldBrush = SelectObject(hdc, sideBrush);
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HGDIOBJ oldPen = SelectObject(hdc, nullPen);
    Polygon(hdc, sidePts, 4);

    // 2. Top Roof Facade Polygon
    POINT topPts[4] = {
        { x - width/2, y },
        { x - width/2 + slant, y - depth },
        { x + width/2 + slant, y - depth },
        { x + width/2, y }
    };
    HBRUSH topBrush = CreateSolidBrush(theme.top);
    SelectObject(hdc, topBrush);
    Polygon(hdc, topPts, 4);
    DeleteObject(topBrush);
    DeleteObject(sideBrush);

    // Antenna Spire on top block
    if (isTopDisc || discSize == 1) {
        HPEN spirePen = CreatePen(PS_SOLID, 2, RGB(203, 213, 225));
        SelectObject(hdc, spirePen);
        MoveToEx(hdc, x, y - depth, NULL);
        LineTo(hdc, x, y - depth - 14);
        DeleteObject(spirePen);

        // Blinking Beacon
        BOOL blink = ((animTick / 10) % 2 == 0);
        HBRUSH bBrush = CreateSolidBrush(blink ? RGB(239, 68, 68) : RGB(127, 29, 29));
        SelectObject(hdc, bBrush);
        Ellipse(hdc, x - 3, y - depth - 17, x + 3, y - depth - 11);
        DeleteObject(bBrush);
    }

    // 3. Front Facade Main Rect
    RECT fRect = { x - width/2, y, x + width/2, y + height };
    HBRUSH mainBrush = CreateSolidBrush(isGlass ? theme.main : theme.side);
    FillRect(hdc, &fRect, mainBrush);
    DeleteObject(mainBrush);

    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
    SelectObject(hdc, borderPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, fRect.left, fRect.top, fRect.right, fRect.bottom);
    DeleteObject(borderPen);

    if (!isGlass) {
        HPEN concPen = CreatePen(PS_SOLID, 1, theme.main);
        SelectObject(hdc, concPen);
        for (int ty = y + 4; ty < y + height; ty += 4) {
            MoveToEx(hdc, x - width/2, ty, NULL);
            LineTo(hdc, x + width/2, ty);
        }
        DeleteObject(concPen);
    }

    // 4. Window Grid Animation
    int cols = (width / 14);
    if (cols < 2) cols = 2;
    int padX = (width - cols * 4) / (cols + 1);
    int pattern = discSize % 3;

    for (int c = 0; c < cols; c++) {
        if (pattern == 2 && c % 2 == 0) continue; // Checkered window pattern

        int wx = (x - width/2) + padX + c * (4 + padX);
        int wy1 = y + 4;
        int wy2 = y + 13;

        int seed1 = (discSize * 13 + c * 7 + animTick / 4) % 10;
        COLORREF litCol = isGlass ? (seed1 > 6 ? RGB(165, 243, 252) : RGB(254, 240, 138)) : RGB(254, 240, 138);
        COLORREF offCol = isGlass ? RGB(30, 41, 59) : RGB(15, 23, 42);
        COLORREF wColor1 = (seed1 > 2) ? litCol : offCol;
        HBRUSH wBrush1 = CreateSolidBrush(wColor1);
        
        if (pattern == 1) {
            RECT wR1 = { wx, wy1, wx + 4, wy2 + 5 };
            FillRect(hdc, &wR1, wBrush1);
        } else {
            RECT wR1 = { wx, wy1, wx + 4, wy1 + 5 };
            FillRect(hdc, &wR1, wBrush1);
            RECT wR2 = { wx, wy2, wx + 4, wy2 + 5 };
            FillRect(hdc, &wR2, wBrush1);
        }
        DeleteObject(wBrush1);
    }

    // 5. Text Label / Locked Overlay
    if (isLocked) {
        HPEN cagePen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
        SelectObject(hdc, cagePen);
        MoveToEx(hdc, fRect.left, fRect.top, NULL); LineTo(hdc, fRect.right, fRect.bottom);
        MoveToEx(hdc, fRect.left, fRect.bottom, NULL); LineTo(hdc, fRect.right, fRect.top);
        DeleteObject(cagePen);

        SetTextColor(hdc, RGB(239, 68, 68));
        TextOut(hdc, x - 14, y + 3, "LOCK", 4);
    } else {
        char buf[8];
        sprintf(buf, "%d", discSize);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOut(hdc, x - 4, y + 3, buf, strlen(buf));
    }

    if (isWarning) {
        BOOL blinkWarn = ((animTick / 5) % 2 == 0);
        HBRUSH wBrush = CreateSolidBrush(blinkWarn ? RGB(239, 68, 68) : RGB(250, 204, 21));
        RECT wlR1 = { x - width/2 - 4, y + height/2 - 4, x - width/2, y + height/2 + 4 };
        FillRect(hdc, &wlR1, wBrush);
        RECT wlR2 = { x + width/2, y + height/2 - 4, x + width/2 + 4, y + height/2 + 4 };
        FillRect(hdc, &wlR2, wBrush);
        DeleteObject(wBrush);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(nullPen);
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
            hStageLabel = CreateWindow("STATIC", "Stage 1 / 20", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                                       170, 10, 140, 28, hwnd, (HMENU)103, NULL, NULL);
            hStagePlusBtn = CreateWindow("BUTTON", ">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        315, 10, 25, 28, hwnd, (HMENU)104, NULL, NULL);

            hPegMinusBtn = CreateWindow("BUTTON", "-P", WS_CHILD | BS_PUSHBUTTON, 345, 10, 30, 28, hwnd, (HMENU)105, NULL, NULL);
            hPegPlusBtn = CreateWindow("BUTTON", "+P", WS_CHILD | BS_PUSHBUTTON, 380, 10, 30, 28, hwnd, (HMENU)106, NULL, NULL);
            hDiscMinusBtn = CreateWindow("BUTTON", "-D", WS_CHILD | BS_PUSHBUTTON, 415, 10, 30, 28, hwnd, (HMENU)107, NULL, NULL);
            hDiscPlusBtn = CreateWindow("BUTTON", "+D", WS_CHILD | BS_PUSHBUTTON, 450, 10, 30, 28, hwnd, (HMENU)108, NULL, NULL);

            hUndoBtn = CreateWindow("BUTTON", "Undo [U]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 45, 75, 28, hwnd, (HMENU)1, NULL, NULL);
            hHintBtn = CreateWindow("BUTTON", "Hint [H]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 45, 75, 28, hwnd, (HMENU)2, NULL, NULL);
            hFreezeBtn = CreateWindow("BUTTON", "Freeze [F]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 45, 85, 28, hwnd, (HMENU)3, NULL, NULL);
            hSwapBtn = CreateWindow("BUTTON", "Swap [S]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 260, 45, 80, 28, hwnd, (HMENU)7, NULL, NULL);
            hAutoBtn = CreateWindow("BUTTON", "Auto-Solve", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 345, 45, 90, 28, hwnd, (HMENU)4, NULL, NULL);
            hRestartBtn = CreateWindow("BUTTON", "Restart [R]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 440, 45, 85, 28, hwnd, (HMENU)5, NULL, NULL);
            hHelpBtn = CreateWindow("BUTTON", "Help [?]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 530, 45, 70, 28, hwnd, (HMENU)6, NULL, NULL);

            SetTimer(hwnd, 3, 30, NULL); // 30fps animation timer
            InitGame(hwnd);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) { // 1 sec clock
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
            } else if (wParam == 3) { // 30fps Animation Tick
                animTick++;
                if (won) UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 101) {
                mode = (mode == 0) ? 1 : 0;
                InitGame(hwnd);
            } else if (id == 102) {
                if (currentStageIdx > 0) {
                    currentStageIdx--;
                    InitGame(hwnd);
                }
            } else if (id == 104) {
                if (currentStageIdx < TOTAL_STAGES - 1) {
                    BOOL isUnlocked = (stageStats[CAMPAIGN_STAGES[currentStageIdx].id].stars > 0);
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
            else if (id == 7) UseDiskSwap(hwnd);
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
                    "How to Play KTowers (Loop 7 Expanded)\n\n"
                    "Goal: Move all skyscraper blocks to the target (last) peg.\n\n"
                    "Rules & Modifiers:\n"
                    "- Only top skyscraper blocks can be moved.\n"
                    "- Larger blocks cannot be placed on smaller blocks.\n"
                    "- Adjacent Only: Blocks can only move to neighboring pegs.\n"
                    "- Cyclic Move: Blocks can only move clockwise (Peg 1->2->3->1).\n"
                    "- Color Restriction: Peg 2 accepts even-numbered disks only.\n"
                    "- Locked Disks: Cannot move until turn requirement is met.\n\n"
                    "Controls & Skills:\n"
                    "- Click peg to pick/drop block or use keys 1 to 5.\n"
                    "- [U] Undo last move.\n"
                    "- [H] Optimal Frame-Stewart Hint.\n"
                    "- [F] Time Freeze (pauses timer for 15s).\n"
                    "- [S] Disk Swap / Instant Teleport to valid peg.",
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
            else if (wParam == 'S' || wParam == 's') UseDiskSwap(hwnd);
            else if (wParam == 'R' || wParam == 'r') InitGame(hwnd);
            else if (wParam == 'A' || wParam == 'a') {
                SendMessage(hwnd, WM_COMMAND, 4, 0);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (y > 140 && numPegs > 0) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                int w = rc.right - rc.left;
                hoverPeg = x / (w / numPegs);
            } else {
                hoverPeg = -1;
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

            // Double Buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, width, height);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);

            // 1. Background City Sky & Stars
            HBRUSH bgBrush = CreateSolidBrush(RGB(9, 13, 22));
            FillRect(memDC, &rc, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(memDC, TRANSPARENT);

            // Stars
            for (int i = 0; i < 30; i++) {
                int sx = (i * 37) % width;
                int sy = (i * 19) % (int)(height * 0.45);
                SetPixel(memDC, sx, sy, RGB(255, 255, 255));
                SetPixel(memDC, sx + 1, sy, RGB(200, 220, 255));
            }

            // Cyber-grid backdrop
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(20, 40, 60));
            HGDIOBJ oldGrid = SelectObject(memDC, gridPen);
            for (int i = 0; i < width; i += 40) {
                MoveToEx(memDC, i, 0, NULL); LineTo(memDC, i, height);
            }
            for (int i = 0; i < height; i += 40) {
                MoveToEx(memDC, 0, i, NULL); LineTo(memDC, width, i);
            }
            SelectObject(memDC, oldGrid);
            DeleteObject(gridPen);

            // Lightning Flash
            if ((animTick % 180) == 0 || (animTick % 180) == 4) {
                HBRUSH flashBrush = CreateSolidBrush(RGB(200, 200, 255));
                FillRect(memDC, &rc, flashBrush);
                DeleteObject(flashBrush);
            }

            // Cloud Shadows / Smog Layers
            HBRUSH smogBrush = CreateSolidBrush(RGB(15, 23, 42)); // Dark blue-grey smog
            HGDIOBJ oldSBrush = SelectObject(memDC, smogBrush);
            HPEN nullPen = CreatePen(PS_NULL, 0, 0);
            HGDIOBJ oldSPen = SelectObject(memDC, nullPen);
            for(int i=0; i<3; i++) {
                int speed = 2 + i;
                int cloudX = (animTick * speed + i*300) % (width + 400) - 200;
                int cloudY = height/5 + i*50;
                Ellipse(memDC, cloudX - 100 - i*20, cloudY - 20 - i*10, cloudX + 100 + i*20, cloudY + 20 + i*10);
            }
            SelectObject(memDC, oldSBrush);
            DeleteObject(smogBrush);
            
            // Animated Flying Traffic (Hover Cars)
            for(int i=0; i<5; i++) {
                int dir = (i % 2 == 0) ? 1 : -1;
                int speed = 3 + i;
                int trafficX;
                if (dir == 1) {
                    trafficX = (animTick * speed + i*250) % (width + 200) - 100;
                } else {
                    trafficX = width + 100 - ((animTick * speed + i*250) % (width + 200));
                }
                int trafficY = height - 200 - (i * 35);
                
                COLORREF carCol = (i%3==0) ? RGB(56, 189, 248) : ((i%3==1) ? RGB(239, 68, 68) : RGB(245, 158, 11));
                HBRUSH carBrush = CreateSolidBrush(carCol);
                RECT carRect = { trafficX, trafficY, trafficX + 16, trafficY + 5 };
                FillRect(memDC, &carRect, carBrush);
                DeleteObject(carBrush);
                
                HBRUSH engineBrush = CreateSolidBrush(RGB(165, 243, 252));
                if (dir == 1) {
                    RECT eRect = { trafficX - 4, trafficY + 1, trafficX, trafficY + 4 };
                    FillRect(memDC, &eRect, engineBrush);
                    RECT tRect = { trafficX - 18, trafficY + 2, trafficX - 4, trafficY + 3 };
                    FillRect(memDC, &tRect, engineBrush);
                } else {
                    RECT eRect = { trafficX + 16, trafficY + 1, trafficX + 20, trafficY + 4 };
                    FillRect(memDC, &eRect, engineBrush);
                    RECT tRect = { trafficX + 20, trafficY + 2, trafficX + 34, trafficY + 3 };
                    FillRect(memDC, &tRect, engineBrush);
                }
                DeleteObject(engineBrush);
            }
            SelectObject(memDC, oldSPen);
            DeleteObject(nullPen);

            // Neon Advertising Holograms
            if ((animTick / 60) % 4 != 0) {
                SetTextColor(memDC, RGB(236, 72, 153)); // Neon Pink
                TextOut(memDC, width - 200, 150, ">> KILO_CORP <<", 15);
            }
            if ((animTick / 40) % 3 != 0) {
                SetTextColor(memDC, RGB(56, 189, 248)); // Neon Cyan
                TextOut(memDC, 80, 120, "SYS.ONLINE", 10);
            }
            if ((animTick / 50) % 5 != 0) {
                SetTextColor(memDC, RGB(250, 204, 21)); // Neon Yellow
                TextOut(memDC, width/2 - 40, 90, "O R B I T A L", 13);
            }

            StageConfig cfg = GetCurrentConfig();

            // Status Banner Line 1
            char line1[256];
            char rulesStr[128] = "";
            if (cfg.adjOnly) strcat(rulesStr, " [Adjacent Only]");
            if (cfg.cyclic) strcat(rulesStr, " [Cyclic Moves]");
            if (cfg.colorRestr == 1) strcat(rulesStr, " [Peg 2 Even Only]");
            if (cfg.moveLimit > 0) sprintf(rulesStr + strlen(rulesStr), " [Max %d Moves]", cfg.moveLimit);
            if (cfg.timeLimit > 0) sprintf(rulesStr + strlen(rulesStr), " [Time Limit %ds]", cfg.timeLimit);
            if (cfg.lockedDisk > 0) sprintf(rulesStr + strlen(rulesStr), " [Disk %d Locked %dm]", cfg.lockedDisk, cfg.lockDuration);

            sprintf(line1, "%s %s", cfg.name, rulesStr);
            HFONT fontBold = CreateFont(18, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, fontBold);
            SetTextColor(memDC, RGB(56, 189, 248));
            TextOut(memDC, 15, 80, line1, strlen(line1));

            // Line 2: Stats
            char line2[256];
            char freezeStr[32] = "";
            if (freezeSeconds > 0) sprintf(freezeStr, " | FROZEN (%ds)", freezeSeconds);
            sprintf(line2, "Time: %02d:%02d%s | Moves: %d / Par: %d | Freeze: %d | Swap: %d",
                    elapsedSeconds/60, elapsedSeconds%60, freezeStr, moves, cfg.par, freezeCharges, swapCharges);
            SetTextColor(memDC, freezeSeconds > 0 ? RGB(6, 182, 212) : RGB(148, 163, 184));
            TextOut(memDC, 15, 105, line2, strlen(line2));

            if (strlen(statusMessage) > 0) {
                SetTextColor(memDC, won ? RGB(34, 197, 94) : RGB(239, 68, 68));
                TextOut(memDC, 15, 130, statusMessage, strlen(statusMessage));
            }

            SelectObject(memDC, oldFont);
            DeleteObject(fontBold);

            // 2. 3D Asphalt Street Grid & Sidewalk Base
            int pegAreaWidth = width / numPegs;
            int groundY = height - 65;

            RECT roadRect = { 0, groundY, width, height };
            HBRUSH roadBrush = CreateSolidBrush(RGB(30, 41, 59));
            FillRect(memDC, &roadRect, roadBrush);
            DeleteObject(roadBrush);

            RECT kerbRect = { 0, groundY - 6, width, groundY };
            HBRUSH kerbBrush = CreateSolidBrush(RGB(71, 85, 105));
            FillRect(memDC, &kerbRect, kerbBrush);
            DeleteObject(kerbBrush);

            // Yellow Lane Markings
            HPEN yellowPen = CreatePen(PS_DASH, 2, RGB(245, 158, 11));
            HGDIOBJ oPen = SelectObject(memDC, yellowPen);
            MoveToEx(memDC, 0, groundY + 30, NULL);
            LineTo(memDC, width, groundY + 30);
            SelectObject(memDC, oPen);
            DeleteObject(yellowPen);

            // Street Grid Compass Clues
            SetTextColor(memDC, RGB(148, 163, 184));
            TextOut(memDC, 20, groundY + 45, "STREET GRID: [WEST] <--", 23);
            TextOut(memDC, width - 210, groundY + 45, "--> [EAST] | EYE VIEW", 21);

            // 3. Draw Peg Foundation Pedestals & Lattice Pylons
            HBRUSH poleBrush = CreateSolidBrush(RGB(100, 116, 139));
            HBRUSH baseBrush = CreateSolidBrush(RGB(51, 65, 85));

            for (int i = 0; i < numPegs; i++) {
                int baseX = pegAreaWidth * i + pegAreaWidth / 2;

                // Selection / Hint Light Shaft Beam
                if (selectedPeg == i || hintFrom == i || hintTo == i) {
                    COLORREF hlColor = (selectedPeg == i) ? RGB(56, 189, 248) :
                                       (hintFrom == i) ? RGB(245, 158, 11) : RGB(34, 197, 94);
                    HPEN hlPen = CreatePen(PS_DOT, 1, hlColor);
                    HGDIOBJ oldP = SelectObject(memDC, hlPen);
                    RECT hlRect = { baseX - pegAreaWidth/2 + 8, groundY - 220, baseX + pegAreaWidth/2 - 8, groundY };
                    FrameRect(memDC, &hlRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                    SelectObject(memDC, oldP);
                    DeleteObject(hlPen);
                }

                // Steel Lattice Pole
                RECT poleRect = { baseX - 5, groundY - 200, baseX + 5, groundY };
                FillRect(memDC, &poleRect, poleBrush);

                // Concrete Pedestal Base
                RECT bRect = { baseX - pegAreaWidth/2 + 12, groundY - 12, baseX + pegAreaWidth/2 - 12, groundY };
                FillRect(memDC, &bRect, baseBrush);

                // Label
                char pLabel[48];
                if (cfg.colorRestr == 1 && i == 1) {
                    sprintf(pLabel, "Peg %d (EVEN ONLY)", i + 1);
                } else {
                    sprintf(pLabel, "Peg %d%s", i + 1, (i == numPegs - 1) ? " (Target)" : "");
                }
                SetTextColor(memDC, (cfg.colorRestr == 1 && i == 1) ? RGB(236, 72, 153) : RGB(226, 232, 240));
                TextOut(memDC, baseX - 40, groundY + 12, pLabel, strlen(pLabel));
            }

            DeleteObject(poleBrush);
            DeleteObject(baseBrush);

            // 4. Draw 3D Skyscraper Blocks (Discs)
            int blockH = 24;
            int spacing = 4;

            for (int p = 0; p < numPegs; p++) {
                int baseX = pegAreaWidth * p + pegAreaWidth / 2;
                for (int j = 0; j < pegCounts[p]; j++) {
                    int discSize = pegs[p][j];
                    int discW = (int)(35 + ((double)discSize / numDiscs) * (pegAreaWidth - 45));

                    BOOL isTopDisc = (j == pegCounts[p] - 1);
                    BOOL isLocked = (cfg.lockedDisk > 0 && discSize == cfg.lockedDisk && moves < cfg.lockDuration);

                    int targetY = groundY - 12 - (j + 1) * (blockH + spacing);
                    if (selectedPeg == p && isTopDisc) {
                        targetY = groundY - 215;
                    }

                    // Lerp position
                    if (discAnimY[p][j] < 0) discAnimY[p][j] = (float)targetY - 30;
                    float distBefore = fabs(discAnimY[p][j] - targetY);
                    discAnimY[p][j] += ((float)targetY - discAnimY[p][j]) * 0.35f;
                    float distAfter = fabs(discAnimY[p][j] - targetY);
                    int rectY = (int)discAnimY[p][j];

                    if (distBefore > 5.0f && distAfter <= 5.0f && selectedPeg != p) {
                        screenShake = 12;
                        discAnimY[p][j] = (float)targetY + 4.0f; // bounce squash
                        for(int d=0; d<15; d++) {
                            int idx = -1;
                            for(int i=0; i<MAX_DUST; i++) if(dusts[i].life <= 0) { idx = i; break; }
                            if (idx != -1) {
                                dusts[idx].x = baseX + (rand() % (discW+1)) - discW/2.0f;
                                dusts[idx].y = targetY + blockH;
                                dusts[idx].vx = ((rand()%100)/50.0f - 1.0f) * 3.0f;
                                dusts[idx].vy = -((rand()%100)/100.0f) * 2.0f;
                                dusts[idx].life = 15 + rand()%15;
                            }
                        }
                    }

                    if (distAfter > 2.0f && selectedPeg != p) {
                        int pulseRadius = discW/2 + 15 + (int)(5 * sin(animTick * 0.5));
                        HPEN auraPen = CreatePen(PS_SOLID, 2, RGB(250, 204, 21));
                        HGDIOBJ oldAura = SelectObject(memDC, auraPen);
                        SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        Ellipse(memDC, baseX - pulseRadius, rectY + blockH/2 - pulseRadius/2, baseX + pulseRadius, rectY + blockH/2 + pulseRadius/2);
                        SelectObject(memDC, oldAura);
                        DeleteObject(auraPen);
                    }

                    if (selectedPeg == p && isTopDisc) {
                        int pulseRadius = discW/2 + 10 + (int)(5 * sin(animTick * 0.3));
                        HPEN auraPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                        HGDIOBJ oldAura = SelectObject(memDC, auraPen);
                        SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        Ellipse(memDC, baseX - pulseRadius, rectY + blockH/2 - pulseRadius/2, baseX + pulseRadius, rectY + blockH/2 + pulseRadius/2);
                        SelectObject(memDC, oldAura);
                        DeleteObject(auraPen);
                    }

                    DiscTheme theme = DISC_THEMES[(discSize - 1) % 10];

                    BOOL isWarning = FALSE;
                    if (selectedPeg == p && isTopDisc && hoverPeg != -1 && hoverPeg != selectedPeg) {
                        BOOL valid = IsValidMove(cfg, selectedPeg, hoverPeg, discSize, moves);
                        if (valid) {
                            if (pegCounts[hoverPeg] > 0 && pegs[hoverPeg][pegCounts[hoverPeg] - 1] < discSize) {
                                valid = FALSE;
                            }
                        }
                        isWarning = !valid;
                    }

                    Draw3DSkyscraperBlockGDI(memDC, baseX, rectY, discW, blockH, theme, discSize, isLocked, isTopDisc, targetY, isWarning);
                }
            }

            // Rain Particles
            HPEN rainPen = CreatePen(PS_SOLID, 1, RGB(100, 150, 255));
            HGDIOBJ oldRain = SelectObject(memDC, rainPen);
            for (int i = 0; i < 80; i++) {
                int seed = i * 214013 + 2531011;
                int rx = (seed % width - (animTick * 6) % width + width) % width;
                int ry = ((seed >> 16) % height + (animTick * 18) % height) % height;
                MoveToEx(memDC, rx, ry, NULL);
                LineTo(memDC, rx - 3, ry + 12);
            }
            SelectObject(memDC, oldRain);
            DeleteObject(rainPen);

            // 5. Draw Victory Celebration Fireworks
            if (won) {
                for (int i = 0; i < particleCount; i++) {
                    if (particles[i].life > 0) {
                        if (particles[i].maxLife == 20) {
                            int radius = (20 - particles[i].life) * 4;
                            HPEN swPen = CreatePen(PS_SOLID, particles[i].life / 4 + 1, particles[i].color);
                            HGDIOBJ oldSW = SelectObject(memDC, swPen);
                            SelectObject(memDC, GetStockObject(NULL_BRUSH));
                            Ellipse(memDC, (int)particles[i].x - radius, (int)particles[i].y - radius, (int)particles[i].x + radius, (int)particles[i].y + radius);
                            SelectObject(memDC, oldSW);
                            DeleteObject(swPen);
                        } else {
                            HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                            RECT pRect = { (int)particles[i].x - 2, (int)particles[i].y - 2, (int)particles[i].x + 3, (int)particles[i].y + 3 };
                            FillRect(memDC, &pRect, pBrush);
                            DeleteObject(pBrush);
                        }
                    }
                }
                if (rand() % 30 == 0) SpawnFireworks();
            }

            // Draw Dust Particles
            for(int d=0; d<MAX_DUST; d++) {
                if(dusts[d].life > 0) {
                    dusts[d].x += dusts[d].vx;
                    dusts[d].y += dusts[d].vy;
                    dusts[d].life--;
                    HBRUSH dBrush = CreateSolidBrush(RGB(148, 163, 184));
                    RECT dRect = { (int)dusts[d].x - 2, (int)dusts[d].y - 2, (int)dusts[d].x + 3, (int)dusts[d].y + 3 };
                    FillRect(memDC, &dRect, dBrush);
                    DeleteObject(dBrush);
                }
            }

            // Copy double buffer to screen
            int shakeOffsetX = 0, shakeOffsetY = 0;
            if (screenShake > 0) {
                shakeOffsetX = (int)(sin(animTick * 1.5f) * screenShake);
                shakeOffsetY = (int)(cos(animTick * 1.8f) * screenShake);
                screenShake--;
            }
            BitBlt(hdc, shakeOffsetX, shakeOffsetY, width, height, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

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
