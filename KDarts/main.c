#pragma comment(lib, "msvcrt.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define PI 3.14159265358979323846
#define TIMER_ID 1
#define CX 360
#define CY 460
#define BOARD_R 260
#define R 190

// Game Modes
#define MODE_501 0
#define MODE_301 1
#define MODE_CRICKET 2
#define MODE_ATC 3
#define MODE_BLITZ 4

typedef struct {
    const char* name;
    const char* opponent;
    int mode;
    int aiErr;
    int wobble;
    int wind;
} CampaignStage;

static const CampaignStage CAMPAIGN_STAGES[15] = {
    {"Local Pub", "Rookie Rick", MODE_301, 120, 10, 0},
    {"Amateur Open", "Amateur Andy", MODE_ATC, 105, 12, 2},
    {"League Qualifier", "Pub Regular Pete", MODE_301, 90, 14, 3},
    {"Town Championship", "Local Hero Luke", MODE_CRICKET, 80, 15, 4},
    {"County Clash", "District Champ Dave", MODE_BLITZ, 70, 16, 5},
    {"Regional Cup", "Regional Ace Ray", MODE_501, 62, 18, 6},
    {"State Invitational", "State Master Sam", MODE_ATC, 54, 20, 7},
    {"National Qualifier", "Pro Qualifier Phil", MODE_BLITZ, 46, 22, 8},
    {"National Semifinal", "National Pro Ned", MODE_CRICKET, 38, 24, 9},
    {"National Final", "National Champ Nick", MODE_501, 32, 26, 10},
    {"International Masters", "Euro Champ Eric", MODE_301, 26, 28, 11},
    {"Grand Prix", "Grand Prix Finalist Gary", MODE_CRICKET, 20, 30, 12},
    {"Premier League", "Premier Star Paul", MODE_ATC, 15, 32, 13},
    {"World Semis", "World Contender Will", MODE_BLITZ, 10, 34, 14},
    {"World Final", "World Champion Vic", MODE_501, 5, 36, 15}
};

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // whoosh
        Beep(800, 30);
        Beep(400, 30);
        Beep(200, 30);
    } else if (type == 2) { // thud
        Beep(120, 50);
    } else if (type == 3) { // cheer
        Beep(440, 80);
        Beep(554, 80);
        Beep(659, 160);
    } else if (type == 4) { // powerup
        Beep(523, 60);
        Beep(659, 60);
        Beep(784, 100);
    }
    return 0;
}

void PlayGameSound(int type) {
    extern int soundEnabled;
    if (!soundEnabled) return;
    CreateThread(NULL, 0, PlaySoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

typedef struct {
    int targetX, targetY;
    float x, y;
    int pts;
    int number;
    int mult;
    float progress;
    int animating;
} Dart;

int isCampaign = 0;
int campaignStage = 0; // 0..14
int gameMode = MODE_501;
int aiDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard, 3=Legend, 4=Human
int currentPlayer = 0; // 0=Player 1, 1=AI/Player 2
int soundEnabled = 1;
int dartStyle = 0; // 0=Cyan, 1=Red, 2=Gold

// Scoring state
int scores[2] = {501, 501};
int prevScores[2] = {501, 501};
int cricketHits[2][7] = {{0}}; // 20, 19, 18, 17, 16, 15, 25
int atcTarget[2] = {1, 1}; // 1..20
int blitzHits[2] = {0, 0}; // 0..10

// Stats
int totalDarts[2] = {0, 0};
int highestCheckout[2] = {0, 0};

// Power-ups
int focusActive[2] = {0, 0};
int focusUses[2] = {2, 2};
int magnetActive[2] = {0, 0};
int magnetUses[2] = {2, 2};
int undoUses[2] = {1, 1};

// Wind & Wobble
float windX = 0.0f, windY = 0.0f;
int windSpeed = 0;
float windAngle = 0.0f;
int currentWobbleAmp = 15;

int dartsLeft = 3;
Dart darts[3];
int dartsCount = 0;
int gameState = 0; // 0=PLAYING, 1=TURN_END, 2=WON, 3=CAMPAIGN_OVER
int mouseX = CX, mouseY = CY;
int wobbleX = 0, wobbleY = 0;
float t = 0.0f;
int aiTimer = 0;
char statusMsg[128] = "Game On! Player 1 Turn - Throw 3 Darts";

typedef struct {
    int isCampaign;
    int campaignStage;
    int gameMode;
    int aiDifficulty;
    int currentPlayer;
    int cricketHits[2][7];
    int atcTarget[2];
    int blitzHits[2];
    int totalDarts[2];
    int highestCheckout[2];
    int scores[2];
    int prevScores[2];
    int focusActive[2];
    int focusUses[2];
    int magnetActive[2];
    int magnetUses[2];
    int undoUses[2];
    int dartsLeft;
    int gameState;
    int dartsCount;
    Dart darts[3];
} GameStateSnapshot;

#define MAX_HISTORY 50
GameStateSnapshot history[MAX_HISTORY];
int historyCount = 0;
GameStateSnapshot redoHistory[MAX_HISTORY];
int redoCount = 0;

void GetSnapshot(GameStateSnapshot* st) {
    st->isCampaign = isCampaign;
    st->campaignStage = campaignStage;
    st->gameMode = gameMode;
    st->aiDifficulty = aiDifficulty;
    st->currentPlayer = currentPlayer;
    memcpy(st->cricketHits, cricketHits, sizeof(cricketHits));
    memcpy(st->atcTarget, atcTarget, sizeof(atcTarget));
    memcpy(st->blitzHits, blitzHits, sizeof(blitzHits));
    memcpy(st->totalDarts, totalDarts, sizeof(totalDarts));
    memcpy(st->highestCheckout, highestCheckout, sizeof(highestCheckout));
    memcpy(st->scores, scores, sizeof(scores));
    memcpy(st->prevScores, prevScores, sizeof(prevScores));
    memcpy(st->focusActive, focusActive, sizeof(focusActive));
    memcpy(st->focusUses, focusUses, sizeof(focusUses));
    memcpy(st->magnetActive, magnetActive, sizeof(magnetActive));
    memcpy(st->magnetUses, magnetUses, sizeof(magnetUses));
    memcpy(st->undoUses, undoUses, sizeof(undoUses));
    st->dartsLeft = dartsLeft;
    st->gameState = gameState;
    st->dartsCount = dartsCount;
    memcpy(st->darts, darts, sizeof(darts));
}

void RestoreSnapshot(GameStateSnapshot* st) {
    isCampaign = st->isCampaign;
    campaignStage = st->campaignStage;
    gameMode = st->gameMode;
    aiDifficulty = st->aiDifficulty;
    currentPlayer = st->currentPlayer;
    memcpy(cricketHits, st->cricketHits, sizeof(cricketHits));
    memcpy(atcTarget, st->atcTarget, sizeof(atcTarget));
    memcpy(blitzHits, st->blitzHits, sizeof(blitzHits));
    memcpy(totalDarts, st->totalDarts, sizeof(totalDarts));
    memcpy(highestCheckout, st->highestCheckout, sizeof(highestCheckout));
    memcpy(scores, st->scores, sizeof(scores));
    memcpy(prevScores, st->prevScores, sizeof(prevScores));
    memcpy(focusActive, st->focusActive, sizeof(focusActive));
    memcpy(focusUses, st->focusUses, sizeof(focusUses));
    memcpy(magnetActive, st->magnetActive, sizeof(magnetActive));
    memcpy(magnetUses, st->magnetUses, sizeof(magnetUses));
    memcpy(undoUses, st->undoUses, sizeof(undoUses));
    dartsLeft = st->dartsLeft;
    gameState = st->gameState;
    dartsCount = st->dartsCount;
    memcpy(darts, st->darts, sizeof(darts));
    aiTimer = 0;
}

void PushHistory() {
    if (historyCount < MAX_HISTORY) {
        GetSnapshot(&history[historyCount]);
        historyCount++;
    } else {
        memmove(&history[0], &history[1], sizeof(GameStateSnapshot) * (MAX_HISTORY - 1));
        GetSnapshot(&history[MAX_HISTORY - 1]);
    }
    redoCount = 0;
}

void Undo(HWND hwnd) {
    if (historyCount > 0) {
        do {
            if (redoCount < MAX_HISTORY) {
                GetSnapshot(&redoHistory[redoCount]);
                redoCount++;
            } else {
                memmove(&redoHistory[0], &redoHistory[1], sizeof(GameStateSnapshot) * (MAX_HISTORY - 1));
                GetSnapshot(&redoHistory[MAX_HISTORY - 1]);
            }
            historyCount--;
            RestoreSnapshot(&history[historyCount]);
        } while (historyCount > 0 && currentPlayer == 1 && aiDifficulty != 4);
        
        const char* turnName = currentPlayer == 0 ? "Player 1" : (isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI"));
        sprintf(statusMsg, "%s's Turn - Darts left: %d", turnName, dartsLeft);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void Redo(HWND hwnd) {
    if (redoCount > 0) {
        if (historyCount < MAX_HISTORY) {
            GetSnapshot(&history[historyCount]);
            historyCount++;
        } else {
            memmove(&history[0], &history[1], sizeof(GameStateSnapshot) * (MAX_HISTORY - 1));
            GetSnapshot(&history[MAX_HISTORY - 1]);
        }
        redoCount--;
        RestoreSnapshot(&redoHistory[redoCount]);
        
        const char* turnName = currentPlayer == 0 ? "Player 1" : (isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI"));
        sprintf(statusMsg, "%s's Turn - Darts left: %d", turnName, dartsLeft);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void UpdateWind() {
    int maxWind = 0;
    if (isCampaign) {
        maxWind = CAMPAIGN_STAGES[campaignStage].wind;
        currentWobbleAmp = CAMPAIGN_STAGES[campaignStage].wobble;
    } else {
        maxWind = (aiDifficulty + 1) * 2;
        currentWobbleAmp = 15;
    }
    
    if (maxWind == 0) {
        windSpeed = 0;
        windX = 0.0f;
        windY = 0.0f;
    } else {
        windSpeed = (rand() % (maxWind + 1));
        windAngle = ((float)rand() / (float)RAND_MAX) * 2.0f * (float)PI;
        windX = cosf(windAngle) * (float)windSpeed * 2.2f;
        windY = sinf(windAngle) * (float)windSpeed * 2.2f;
    }
}

void SetMode(int mode) {
    isCampaign = 0;
    gameMode = mode;
    gameState = 0;
    dartsCount = 0;
    dartsLeft = 3;
    historyCount = 0;
    redoCount = 0;
    currentPlayer = 0;
    totalDarts[0] = 0; totalDarts[1] = 0;
    highestCheckout[0] = 0; highestCheckout[1] = 0;
    focusUses[0] = 2; focusUses[1] = 2;
    magnetUses[0] = 2; magnetUses[1] = 2;
    undoUses[0] = 1; undoUses[1] = 1;
    focusActive[0] = 0; focusActive[1] = 0;
    magnetActive[0] = 0; magnetActive[1] = 0;
    
    if (mode == MODE_501) {
        scores[0] = 501; scores[1] = 501;
        prevScores[0] = 501; prevScores[1] = 501;
    } else if (mode == MODE_301) {
        scores[0] = 301; scores[1] = 301;
        prevScores[0] = 301; prevScores[1] = 301;
    } else if (mode == MODE_CRICKET) {
        for(int i=0; i<7; i++) { cricketHits[0][i] = 0; cricketHits[1][i] = 0; }
    } else if (mode == MODE_ATC) {
        atcTarget[0] = 1; atcTarget[1] = 1;
    } else if (mode == MODE_BLITZ) {
        blitzHits[0] = 0; blitzHits[1] = 0;
    }
    
    UpdateWind();
    sprintf(statusMsg, "Game On! Player 1 Turn - Throw 3 Darts");
}

void StartCampaign(int stage) {
    isCampaign = 1;
    campaignStage = stage;
    gameMode = CAMPAIGN_STAGES[stage].mode;
    gameState = 0;
    dartsCount = 0;
    dartsLeft = 3;
    historyCount = 0;
    redoCount = 0;
    currentPlayer = 0;
    totalDarts[0] = 0; totalDarts[1] = 0;
    highestCheckout[0] = 0; highestCheckout[1] = 0;
    focusUses[0] = 2; focusUses[1] = 2;
    magnetUses[0] = 2; magnetUses[1] = 2;
    undoUses[0] = 1; undoUses[1] = 1;
    focusActive[0] = 0; focusActive[1] = 0;
    magnetActive[0] = 0; magnetActive[1] = 0;
    
    if (gameMode == MODE_501) {
        scores[0] = 501; scores[1] = 501;
        prevScores[0] = 501; prevScores[1] = 501;
    } else if (gameMode == MODE_301) {
        scores[0] = 301; scores[1] = 301;
        prevScores[0] = 301; prevScores[1] = 301;
    } else if (gameMode == MODE_CRICKET) {
        for(int i=0; i<7; i++) { cricketHits[0][i] = 0; cricketHits[1][i] = 0; }
    } else if (gameMode == MODE_ATC) {
        atcTarget[0] = 1; atcTarget[1] = 1;
    } else if (gameMode == MODE_BLITZ) {
        blitzHits[0] = 0; blitzHits[1] = 0;
    }
    
    UpdateWind();
    sprintf(statusMsg, "Stage %d/15: %s vs %s", stage + 1, CAMPAIGN_STAGES[stage].name, CAMPAIGN_STAGES[stage].opponent);
}

void ActivateFocus(HWND hwnd) {
    if (focusUses[currentPlayer] > 0 && !focusActive[currentPlayer]) {
        focusActive[currentPlayer] = 1;
        focusUses[currentPlayer]--;
        PlayGameSound(4);
        sprintf(statusMsg, "Precision Focus Active! Aim wobble slowed by 50%%");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void ActivateMagnet(HWND hwnd) {
    if (magnetUses[currentPlayer] > 0 && !magnetActive[currentPlayer]) {
        magnetActive[currentPlayer] = 1;
        magnetUses[currentPlayer]--;
        PlayGameSound(4);
        sprintf(statusMsg, "Ring Magnet Active! Double & Triple zones expanded");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void ActivateUndoDart(HWND hwnd) {
    if (undoUses[currentPlayer] > 0 && dartsCount > 0) {
        undoUses[currentPlayer]--;
        PlayGameSound(4);
        
        dartsCount--;
        dartsLeft++;
        totalDarts[currentPlayer]--;
        
        int pts = darts[dartsCount].pts;
        int number = darts[dartsCount].number;
        int mult = darts[dartsCount].mult;
        
        if (gameMode == MODE_501 || gameMode == MODE_301) {
            scores[currentPlayer] += pts;
        } else if (gameMode == MODE_CRICKET) {
            if (mult > 0 && number > 0) {
                int tIdx = -1;
                if (number == 20) tIdx = 0; else if (number == 19) tIdx = 1;
                else if (number == 18) tIdx = 2; else if (number == 17) tIdx = 3;
                else if (number == 16) tIdx = 4; else if (number == 15) tIdx = 5;
                else if (number == 25) tIdx = 6;
                if (tIdx != -1) {
                    cricketHits[currentPlayer][tIdx] -= mult;
                    if (cricketHits[currentPlayer][tIdx] < 0) cricketHits[currentPlayer][tIdx] = 0;
                }
            }
        } else if (gameMode == MODE_ATC) {
            if (number == atcTarget[currentPlayer] - mult || number == atcTarget[currentPlayer] - 1) {
                atcTarget[currentPlayer] -= mult;
                if (atcTarget[currentPlayer] < 1) atcTarget[currentPlayer] = 1;
            }
        } else if (gameMode == MODE_BLITZ) {
            if (number == 25) {
                blitzHits[currentPlayer] -= mult;
                if (blitzHits[currentPlayer] < 0) blitzHits[currentPlayer] = 0;
            }
        }
        
        if (gameState == 1) gameState = 0;
        sprintf(statusMsg, "Dart Undone! Re-throw last dart.");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void GetHitDetails(int x, int y, int* number, int* mult) {
    float dx = (float)(x - CX);
    float dy = (float)(y - CY);
    float d = sqrtf(dx*dx + dy*dy) / (float)R;
    
    *number = 0;
    *mult = 0;
    
    if (d > 1.05f) return;
    if (d <= 0.037f) { *number = 25; *mult = 2; return; }
    if (d <= 0.093f) { *number = 25; *mult = 1; return; }
    
    *mult = 1;
    float tripMin = 0.582f, tripMax = 0.629f;
    float dblMin = 0.952f, dblMax = 1.0f;
    
    if (magnetActive[currentPlayer]) {
        tripMin = 0.52f; tripMax = 0.69f;
        dblMin = 0.91f; dblMax = 1.04f;
    }
    
    if (d >= tripMin && d <= tripMax) *mult = 3;
    else if (d >= dblMin && d <= dblMax) *mult = 2;
    
    float angle = atan2f(dy, dx) + (float)PI / 2.0f;
    if (angle < 0) angle += 2.0f * (float)PI;
    
    int idx = (int)floorf((angle + (float)PI / 20.0f) / ((float)PI / 10.0f)) % 20;
    int scores[] = {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
    *number = scores[idx];
}

void SaveState(HWND hwnd) {
    FILE *f = fopen("kdarts_save.dat", "wb");
    if (f) {
        fwrite(&isCampaign, sizeof(int), 1, f);
        fwrite(&campaignStage, sizeof(int), 1, f);
        fwrite(&gameMode, sizeof(int), 1, f);
        fwrite(&aiDifficulty, sizeof(int), 1, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(cricketHits, sizeof(int), 14, f);
        fwrite(atcTarget, sizeof(int), 2, f);
        fwrite(blitzHits, sizeof(int), 2, f);
        fwrite(totalDarts, sizeof(int), 2, f);
        fwrite(scores, sizeof(int), 2, f);
        fwrite(prevScores, sizeof(int), 2, f);
        fwrite(&dartsLeft, sizeof(int), 1, f);
        fwrite(&gameState, sizeof(int), 1, f);
        fwrite(highestCheckout, sizeof(int), 2, f);
        fwrite(focusUses, sizeof(int), 2, f);
        fwrite(magnetUses, sizeof(int), 2, f);
        fwrite(undoUses, sizeof(int), 2, f);
        fwrite(&soundEnabled, sizeof(int), 1, f);
        fwrite(&dartStyle, sizeof(int), 1, f);
        fclose(f);
        sprintf(statusMsg, "Game Saved Successfully!");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void LoadState(HWND hwnd) {
    FILE *f = fopen("kdarts_save.dat", "rb");
    if (f) {
        fread(&isCampaign, sizeof(int), 1, f);
        fread(&campaignStage, sizeof(int), 1, f);
        fread(&gameMode, sizeof(int), 1, f);
        fread(&aiDifficulty, sizeof(int), 1, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(cricketHits, sizeof(int), 14, f);
        fread(atcTarget, sizeof(int), 2, f);
        fread(blitzHits, sizeof(int), 2, f);
        fread(totalDarts, sizeof(int), 2, f);
        fread(scores, sizeof(int), 2, f);
        fread(prevScores, sizeof(int), 2, f);
        fread(&dartsLeft, sizeof(int), 1, f);
        fread(&gameState, sizeof(int), 1, f);
        fread(highestCheckout, sizeof(int), 2, f);
        fread(focusUses, sizeof(int), 2, f);
        fread(magnetUses, sizeof(int), 2, f);
        fread(undoUses, sizeof(int), 2, f);
        fread(&soundEnabled, sizeof(int), 1, f);
        fread(&dartStyle, sizeof(int), 1, f);
        fclose(f);
        dartsCount = 0;
        
        char buf[30];
        if (aiDifficulty == 0) strcpy(buf, "Vs AI: Easy");
        else if (aiDifficulty == 1) strcpy(buf, "Vs AI: Medium");
        else if (aiDifficulty == 2) strcpy(buf, "Vs AI: Hard");
        else if (aiDifficulty == 3) strcpy(buf, "Vs AI: Legend");
        else strcpy(buf, "Vs Human");
        SetWindowText(GetDlgItem(hwnd, 103), buf);
        
        sprintf(statusMsg, "Game Loaded!");
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void NextTurn(HWND hwnd) {
    if (gameState == 1) {
        dartsCount = 0;
        dartsLeft = 3;
        prevScores[currentPlayer] = scores[currentPlayer];
        focusActive[currentPlayer] = 0;
        magnetActive[currentPlayer] = 0;
        currentPlayer = 1 - currentPlayer;
        gameState = 0;
        UpdateWind();
        
        const char* p2Name = isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI");
        if (currentPlayer == 0) sprintf(statusMsg, "Player 1's Turn - Throw 3 Darts");
        else sprintf(statusMsg, "%s's Turn", p2Name);
    } else if (gameState == 2) {
        if (isCampaign) {
            if (currentPlayer == 0) {
                if (campaignStage < 14) {
                    StartCampaign(campaignStage + 1);
                } else {
                    sprintf(statusMsg, "CAMPAIGN VICTORY! You are the World Champion!");
                    gameState = 3;
                }
            } else {
                sprintf(statusMsg, "Defeated! Click Campaign to retry stage.");
                gameState = 3;
            }
        } else {
            SetMode(gameMode);
        }
    } else if (gameState == 3) {
        if (isCampaign) StartCampaign(campaignStage);
        else SetMode(gameMode);
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void ThrowDart(HWND hwnd, int tx, int ty, int isAI) {
    PushHistory();
    if (isAI) {
        int targetNum = 20;
        int targetMult = 3;
        
        if (gameMode == MODE_501 || gameMode == MODE_301) {
            if (scores[1] > 60) { targetNum = 20; targetMult = 3; }
            else {
                if (scores[1] <= 40 && scores[1] % 2 == 0) { targetNum = scores[1] / 2; targetMult = 2; }
                else if (scores[1] <= 20) { targetNum = scores[1]; targetMult = 1; }
                else { targetNum = 20; targetMult = 1; }
            }
        } else if (gameMode == MODE_CRICKET) {
            int targets[] = {20,19,18,17,16,15,25};
            for (int i=0; i<7; i++) {
                if (cricketHits[1][i] < 3) {
                    targetNum = targets[i];
                    targetMult = (targetNum == 25) ? 2 : 3;
                    break;
                }
            }
        } else if (gameMode == MODE_ATC) {
            targetNum = atcTarget[1];
            targetMult = (targetNum == 20) ? 1 : 2;
        } else if (gameMode == MODE_BLITZ) {
            targetNum = 25;
            targetMult = 2;
        }
        
        float rad = 0;
        if (targetNum == 25) {
            rad = targetMult == 2 ? R * 0.02f : R * 0.06f;
        } else {
            if (targetMult == 3) rad = R * 0.605f;
            else if (targetMult == 2) rad = R * 0.976f;
            else rad = R * 0.75f;
        }
        
        float idealX = CX, idealY = CY;
        if (targetNum != 25) {
            int scoresArray[] = {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
            int idx = 0;
            for(int i=0; i<20; i++) { if(scoresArray[i]==targetNum) { idx=i; break; } }
            float a = -(float)PI / 2.0f + idx * ((float)PI / 10.0f);
            idealX = CX + cosf(a) * rad;
            idealY = CY + sinf(a) * rad;
        }
        
        float errorMag = 50.0f;
        if (isCampaign) {
            errorMag = (float)CAMPAIGN_STAGES[campaignStage].aiErr;
        } else {
            if (aiDifficulty == 0) errorMag = 120.0f;
            else if (aiDifficulty == 1) errorMag = 70.0f;
            else if (aiDifficulty == 2) errorMag = 30.0f;
            else if (aiDifficulty == 3) errorMag = 10.0f;
        }
        
        float rx = (((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) - 1.5f) * errorMag;
        float ry = (((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) - 1.5f) * errorMag;
        
        tx = (int)(idealX + rx);
        ty = (int)(idealY + ry);
    }
    
    int finalX = tx + (int)windX;
    int finalY = ty + (int)windY;
    
    int number, mult;
    GetHitDetails(finalX, finalY, &number, &mult);
    int pts = number * mult;
    
    if (dartsCount < 3) {
        PlayGameSound(1);
        darts[dartsCount].targetX = finalX;
        darts[dartsCount].targetY = finalY;
        darts[dartsCount].x = (float)CX;
        darts[dartsCount].y = 750.0f;
        darts[dartsCount].pts = pts;
        darts[dartsCount].number = number;
        darts[dartsCount].mult = mult;
        darts[dartsCount].progress = 0.0f;
        darts[dartsCount].animating = 1;
        dartsCount++;
    }
    
    dartsLeft--;
    totalDarts[currentPlayer]++;
    
    const char* turnName = currentPlayer == 0 ? "Player 1" : (isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI"));
    
    if (gameMode == MODE_501 || gameMode == MODE_301) {
        scores[currentPlayer] -= pts;
        if (scores[currentPlayer] < 0 || scores[currentPlayer] == 1) {
            sprintf(statusMsg, "%s Bust!", turnName);
            scores[currentPlayer] = prevScores[currentPlayer];
            gameState = 1;
        } else if (scores[currentPlayer] == 0) {
            sprintf(statusMsg, "%s Wins!", turnName);
            if (prevScores[currentPlayer] > highestCheckout[currentPlayer]) {
                highestCheckout[currentPlayer] = prevScores[currentPlayer];
            }
            gameState = 2;
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over. Score: %d", turnName, scores[currentPlayer]);
            gameState = 1;
        } else {
            sprintf(statusMsg, "%s Hit %d! Darts left: %d", turnName, pts, dartsLeft);
        }
    } else if (gameMode == MODE_CRICKET) {
        if (mult > 0) {
            int tIdx = -1;
            if (number == 20) tIdx = 0; else if (number == 19) tIdx = 1;
            else if (number == 18) tIdx = 2; else if (number == 17) tIdx = 3;
            else if (number == 16) tIdx = 4; else if (number == 15) tIdx = 5;
            else if (number == 25) tIdx = 6;
            
            if (tIdx != -1) {
                cricketHits[currentPlayer][tIdx] += mult;
                if (cricketHits[currentPlayer][tIdx] > 3) cricketHits[currentPlayer][tIdx] = 3;
            }
        }
        
        int allClosed = 1;
        for (int i=0; i<7; i++) {
            if (cricketHits[currentPlayer][i] < 3) allClosed = 0;
        }
        
        if (allClosed) {
            sprintf(statusMsg, "%s Wins in %d darts!", turnName, totalDarts[currentPlayer]);
            gameState = 2;
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over.", turnName);
            gameState = 1;
        } else {
            if (mult == 0) sprintf(statusMsg, "Miss! - left: %d", dartsLeft);
            else {
                const char* mStr = mult == 1 ? "Single" : (mult == 2 ? "Double" : "Triple");
                if (number == 25) sprintf(statusMsg, "%s: %s Bull - left: %d", turnName, mStr, dartsLeft);
                else sprintf(statusMsg, "%s: %s %d - left: %d", turnName, mStr, number, dartsLeft);
            }
        }
    } else if (gameMode == MODE_ATC) {
        if (number == atcTarget[currentPlayer]) {
            atcTarget[currentPlayer] += mult;
            if (atcTarget[currentPlayer] > 20) {
                sprintf(statusMsg, "%s Completed Around the Clock!", turnName);
                gameState = 2;
            } else {
                sprintf(statusMsg, "%s Hit %d! Next target: %d", turnName, number, atcTarget[currentPlayer]);
            }
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over. Target: %d", turnName, atcTarget[currentPlayer]);
            gameState = 1;
        } else {
            sprintf(statusMsg, "Miss Target %d! Darts left: %d", atcTarget[currentPlayer], dartsLeft);
        }
    } else if (gameMode == MODE_BLITZ) {
        if (number == 25) {
            blitzHits[currentPlayer] += mult;
            if (blitzHits[currentPlayer] >= 10) {
                sprintf(statusMsg, "%s Blitz Victory! 10 Bullseyes!", turnName);
                gameState = 2;
            } else {
                sprintf(statusMsg, "%s Bullseye Hit! Total: %d/10", turnName, blitzHits[currentPlayer]);
            }
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over. Bullseyes: %d/10", turnName, blitzHits[currentPlayer]);
            gameState = 1;
        } else {
            sprintf(statusMsg, "Miss Bullseye! Darts left: %d", dartsLeft);
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void DrawPieSlice(HDC hdc, int r, float a1, float a2, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    int left = CX - r;
    int top = CY - r;
    int right = CX + r;
    int bottom = CY + r;
    
    int dxs = (int)(cosf(a2) * 1000.0f);
    int dys = (int)(sinf(a2) * 1000.0f);
    int dxe = (int)(cosf(a1) * 1000.0f);
    int dye = (int)(sinf(a1) * 1000.0f);
    
    Pie(hdc, left, top, right, bottom, CX + dxs, CY + dys, CX + dxe, CY + dye);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void DrawCircle(HDC hdc, int r, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    Ellipse(hdc, CX - r, CY - r, CX + r, CY + r);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            // Row 1: Modes & Campaign
            CreateWindow("BUTTON", "501", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         20, 15, 50, 26, hwnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "301", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         75, 15, 50, 26, hwnd, (HMENU)111, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Cricket", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         130, 15, 60, 26, hwnd, (HMENU)102, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "ATC", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         195, 15, 50, 26, hwnd, (HMENU)112, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Blitz", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         250, 15, 50, 26, hwnd, (HMENU)113, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Campaign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         305, 15, 80, 26, hwnd, (HMENU)114, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 2: Power-Ups & State
            CreateWindow("BUTTON", "Focus (F)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         390, 15, 75, 26, hwnd, (HMENU)115, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Magnet (M)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         470, 15, 85, 26, hwnd, (HMENU)116, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Undo Dart", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         560, 15, 75, 26, hwnd, (HMENU)117, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         640, 15, 50, 26, hwnd, (HMENU)104, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 3: Settings & Utility
            CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         20, 46, 50, 26, hwnd, (HMENU)105, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Undo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         75, 46, 50, 26, hwnd, (HMENU)108, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Redo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         130, 46, 50, 26, hwnd, (HMENU)109, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Vs AI: Medium", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         185, 46, 110, 26, hwnd, (HMENU)103, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Sound", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         300, 46, 60, 26, hwnd, (HMENU)106, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Style", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         365, 46, 55, 26, hwnd, (HMENU)107, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         425, 46, 50, 26, hwnd, (HMENU)110, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            SetTimer(hwnd, TIMER_ID, 16, NULL);
            return 0;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 101) { SetMode(MODE_501); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 111) { SetMode(MODE_301); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 102) { SetMode(MODE_CRICKET); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 112) { SetMode(MODE_ATC); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 113) { SetMode(MODE_BLITZ); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 114) { StartCampaign(0); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 115) { ActivateFocus(hwnd); }
            else if (LOWORD(wParam) == 116) { ActivateMagnet(hwnd); }
            else if (LOWORD(wParam) == 117) { ActivateUndoDart(hwnd); }
            else if (LOWORD(wParam) == 103) {
                aiDifficulty = (aiDifficulty + 1) % 5;
                char buf[30];
                if (aiDifficulty == 0) strcpy(buf, "Vs AI: Easy");
                else if (aiDifficulty == 1) strcpy(buf, "Vs AI: Medium");
                else if (aiDifficulty == 2) strcpy(buf, "Vs AI: Hard");
                else if (aiDifficulty == 3) strcpy(buf, "Vs AI: Legend");
                else strcpy(buf, "Vs Human");
                SetWindowText((HWND)lParam, buf);
            } else if (LOWORD(wParam) == 104) { SaveState(hwnd); }
            else if (LOWORD(wParam) == 105) { LoadState(hwnd); }
            else if (LOWORD(wParam) == 106) {
                soundEnabled = !soundEnabled;
                sprintf(statusMsg, "Sound: %s", soundEnabled ? "ON" : "OFF");
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 107) {
                dartStyle = (dartStyle + 1) % 3;
                const char* styles[] = {"Cyan", "Red", "Gold"};
                sprintf(statusMsg, "Dart Style: %s", styles[dartStyle]);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 108) { Undo(hwnd); }
            else if (LOWORD(wParam) == 109) { Redo(hwnd); }
            else if (LOWORD(wParam) == 110) {
                MessageBox(hwnd, 
                    "KDarts - Loop 6 Campaign & Expansion:\n\n"
                    "Controls:\n"
                    "- Aim with mouse, click to throw. Press F for Focus (slows wobble), M for Magnet (expands double/triple rings), U for Undo Dart (re-throw 1 bad dart).\n"
                    "- Keyboard: 1-5 for Modes, C for Campaign, F for Focus, M for Magnet, U for Undo Dart.\n\n"
                    "Game Modes:\n"
                    "1. 501 / 301: Reach exactly 0. Bust if score goes below 0 or to 1.\n"
                    "2. Cricket: Close 15-20 and Bullseye (3 hits each).\n"
                    "3. Around the Clock (ATC): Hit numbers 1 to 20 in sequence.\n"
                    "4. Bullseye Blitz: First to score 10 Bullseye hits (Outer=1, Inner=2).\n\n"
                    "Campaign Mode:\n"
                    "Advance through 15 progressive stages featuring increasing AI difficulty, wind mechanics, and wobble!", 
                    "How to Play KDarts", MB_OK | MB_ICONINFORMATION);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == 'F' || wParam == 'f') ActivateFocus(hwnd);
            else if (wParam == 'M' || wParam == 'm') ActivateMagnet(hwnd);
            else if (wParam == 'U' || wParam == 'u') ActivateUndoDart(hwnd);
            else if (wParam == 'C' || wParam == 'c') { StartCampaign(0); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '1') { SetMode(MODE_501); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '2') { SetMode(MODE_301); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '3') { SetMode(MODE_CRICKET); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '4') { SetMode(MODE_ATC); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '5') { SetMode(MODE_BLITZ); InvalidateRect(hwnd, NULL, FALSE); }
            return 0;

        case WM_TIMER: {
            float speedMult = focusActive[currentPlayer] ? 0.5f : 1.0f;
            t += 0.05f * speedMult;
            
            float amp = (float)currentWobbleAmp;
            if (focusActive[currentPlayer]) amp *= 0.5f;
            wobbleX = (int)(sinf(t * 1.3f) * amp + sinf(t * 0.8f) * (amp * 0.6f));
            wobbleY = (int)(cosf(t * 1.5f) * amp + sinf(t * 0.9f) * (amp * 0.6f));
            
            for (int i = 0; i < dartsCount; i++) {
                if (darts[i].animating) {
                    darts[i].progress += 0.1f;
                    if (darts[i].progress >= 1.0f) {
                        darts[i].progress = 1.0f;
                        darts[i].animating = 0;
                        if (darts[i].number == 25) PlayGameSound(3);
                        else PlayGameSound(2);
                    }
                    float ease = 1.0f - powf(1.0f - darts[i].progress, 3.0f);
                    darts[i].x = CX + (darts[i].targetX - CX) * ease;
                    darts[i].y = 750.0f + (darts[i].targetY - 750.0f) * ease;
                } else {
                    darts[i].x = (float)darts[i].targetX;
                    darts[i].y = (float)darts[i].targetY;
                }
            }
            
            int isP2AI = (isCampaign || aiDifficulty != 4);
            if (currentPlayer == 1 && gameState == 0 && isP2AI) {
                int allAnimated = 1;
                for (int i = 0; i < dartsCount; i++) {
                    if (darts[i].animating) allAnimated = 0;
                }
                if (allAnimated) {
                    aiTimer++;
                    if (aiTimer > 50) {
                        aiTimer = 0;
                        ThrowDart(hwnd, 0, 0, 1);
                    }
                }
            } else if (currentPlayer == 1 && gameState == 1 && isP2AI) {
                aiTimer++;
                if (aiTimer > 100) {
                    aiTimer = 0;
                    NextTurn(hwnd);
                }
            }
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
            
        case WM_MOUSEMOVE:
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            return 0;
            
        case WM_LBUTTONDOWN: {
            if (mouseY < 170 && mouseX > 15 && mouseX < 700) return 0; // Ignore top UI panel clicks
            
            int isP1OrHuman = (currentPlayer == 0 || (!isCampaign && aiDifficulty == 4));
            if (gameState == 0 && isP1OrHuman) {
                int targetX = mouseX + wobbleX;
                int targetY = mouseY + wobbleY;
                ThrowDart(hwnd, targetX, targetY, 0);
            } else if ((gameState == 1 || gameState == 2 || gameState == 3) && isP1OrHuman) {
                NextTurn(hwnd);
            }
            return 0;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbmMem);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);
            
            // Header UI panel
            HBRUSH uiBrush = CreateSolidBrush(RGB(28, 28, 32));
            HPEN uiPen = CreatePen(PS_SOLID, 1, RGB(55, 55, 65));
            HBRUSH oldUIBrush = (HBRUSH)SelectObject(memDC, uiBrush);
            HPEN oldUIPen = (HPEN)SelectObject(memDC, uiPen);
            RoundRect(memDC, 10, 5, 710, 165, 12, 12);
            SelectObject(memDC, oldUIBrush);
            SelectObject(memDC, oldUIPen);
            DeleteObject(uiBrush);
            DeleteObject(uiPen);
            
            // Board background
            HBRUSH boardBg = CreateSolidBrush(RGB(12, 12, 14));
            HBRUSH oldBr = (HBRUSH)SelectObject(memDC, boardBg);
            Ellipse(memDC, CX - BOARD_R, CY - BOARD_R, CX + BOARD_R, CY + BOARD_R);
            SelectObject(memDC, oldBr);
            DeleteObject(boardBg);
            
            // Draw segments
            float angleStep = (2.0f * (float)PI) / 20.0f;
            float startOffset = -(float)PI / 2.0f - angleStep / 2.0f;
            int scoresArray[] = {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
            
            for (int i = 0; i < 20; i++) {
                float a1 = startOffset + i * angleStep;
                float a2 = a1 + angleStep;
                int isRed = (i % 2 == 0);
                
                COLORREF colorRed = RGB(208, 0, 0);
                COLORREF colorGreen = RGB(0, 163, 0);
                COLORREF colorBlack = RGB(17, 17, 17);
                COLORREF colorWhite = RGB(232, 230, 209);
                
                if (magnetActive[currentPlayer]) {
                    colorRed = RGB(255, 80, 80);
                    colorGreen = RGB(80, 220, 80);
                }
                
                DrawPieSlice(memDC, (int)(R * 1.0f), a1, a2, isRed ? colorRed : colorGreen);
                DrawPieSlice(memDC, (int)(R * 0.952f), a1, a2, isRed ? colorBlack : colorWhite);
                DrawPieSlice(memDC, (int)(R * 0.629f), a1, a2, isRed ? colorRed : colorGreen);
                DrawPieSlice(memDC, (int)(R * 0.582f), a1, a2, isRed ? colorBlack : colorWhite);
            }
            
            DrawCircle(memDC, (int)(R * 0.093f), RGB(0, 163, 0));
            DrawCircle(memDC, (int)(R * 0.037f), RGB(208, 0, 0));
            
            // Draw numbers
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(memDC, font);
            
            for (int i = 0; i < 20; i++) {
                float a = startOffset + i * angleStep + angleStep / 2.0f;
                int nx = CX + (int)(cosf(a) * (R * 1.15f));
                int ny = CY + (int)(sinf(a) * (R * 1.15f));
                char numStr[3];
                sprintf(numStr, "%d", scoresArray[i]);
                SIZE sz;
                GetTextExtentPoint32(memDC, numStr, strlen(numStr), &sz);
                TextOut(memDC, nx - sz.cx/2, ny - sz.cy/2, numStr, strlen(numStr));
            }
            
            HFONT largeFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            const char* p2Name = isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "P2" : "AI");
            
            SelectObject(memDC, font);
            SetTextColor(memDC, RGB(255, 215, 0));
            char titleLine[128];
            if (isCampaign) {
                sprintf(titleLine, "CAMPAIGN STAGE %d/15 - %s (%s)", campaignStage + 1, CAMPAIGN_STAGES[campaignStage].name, p2Name);
            } else {
                const char* mNames[] = {"501", "301", "Cricket", "Around the Clock", "Bullseye Blitz"};
                sprintf(titleLine, "QUICK MATCH - Mode: %s vs %s", mNames[gameMode], p2Name);
            }
            SIZE sz;
            GetTextExtentPoint32(memDC, titleLine, strlen(titleLine), &sz);
            TextOut(memDC, CX - sz.cx/2, 75, titleLine, strlen(titleLine));
            
            if (gameMode == MODE_501 || gameMode == MODE_301) {
                SelectObject(memDC, largeFont);
                SetTextColor(memDC, RGB(255, 255, 255));
                char scoreStr[40];
                sprintf(scoreStr, "P1: %d  |  %s: %d", scores[0], p2Name, scores[1]);
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, CX - sz.cx/2, 98, scoreStr, strlen(scoreStr));
            } else if (gameMode == MODE_CRICKET) {
                SelectObject(memDC, font);
                SetTextColor(memDC, RGB(255, 255, 255));
                char scoreStrP[128] = "P1: ";
                char scoreStrA[128];
                sprintf(scoreStrA, "%s: ", p2Name);
                int targets[] = {20, 19, 18, 17, 16, 15, 25};
                for (int i=0; i<7; i++) {
                    int hits = cricketHits[0][i];
                    char mark = hits == 0 ? '-' : (hits == 1 ? '/' : (hits == 2 ? 'X' : 'O'));
                    char buf[16];
                    if (targets[i] == 25) sprintf(buf, "B:%c ", mark);
                    else sprintf(buf, "%d:%c ", targets[i], mark);
                    strcat(scoreStrP, buf);
                    
                    hits = cricketHits[1][i];
                    mark = hits == 0 ? '-' : (hits == 1 ? '/' : (hits == 2 ? 'X' : 'O'));
                    if (targets[i] == 25) sprintf(buf, "B:%c ", mark);
                    else sprintf(buf, "%d:%c ", targets[i], mark);
                    strcat(scoreStrA, buf);
                }
                GetTextExtentPoint32(memDC, scoreStrP, strlen(scoreStrP), &sz);
                TextOut(memDC, CX - sz.cx/2, 95, scoreStrP, strlen(scoreStrP));
                GetTextExtentPoint32(memDC, scoreStrA, strlen(scoreStrA), &sz);
                TextOut(memDC, CX - sz.cx/2, 115, scoreStrA, strlen(scoreStrA));
            } else if (gameMode == MODE_ATC) {
                SelectObject(memDC, largeFont);
                SetTextColor(memDC, RGB(255, 255, 255));
                char scoreStr[64];
                sprintf(scoreStr, "P1 Target: %d/20  |  %s Target: %d/20", atcTarget[0], p2Name, atcTarget[1]);
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, CX - sz.cx/2, 98, scoreStr, strlen(scoreStr));
            } else if (gameMode == MODE_BLITZ) {
                SelectObject(memDC, largeFont);
                SetTextColor(memDC, RGB(255, 255, 255));
                char scoreStr[64];
                sprintf(scoreStr, "P1 Bulls: %d/10  |  %s Bulls: %d/10", blitzHits[0], p2Name, blitzHits[1]);
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, CX - sz.cx/2, 98, scoreStr, strlen(scoreStr));
            }
            
            SelectObject(memDC, font);
            SetTextColor(memDC, RGB(180, 220, 255));
            GetTextExtentPoint32(memDC, statusMsg, strlen(statusMsg), &sz);
            TextOut(memDC, CX - sz.cx/2, 138, statusMsg, strlen(statusMsg));
            
            // Power-ups & Wind Badge at bottom of UI panel
            char pwrBadge[128];
            sprintf(pwrBadge, "Focus(F): %d/2 %s | Magnet(M): %d/2 %s | Undo(U): %d/1 | Wind: %d mph", 
                    focusUses[currentPlayer], focusActive[currentPlayer] ? "[ACTIVE]" : "",
                    magnetUses[currentPlayer], magnetActive[currentPlayer] ? "[ACTIVE]" : "",
                    undoUses[currentPlayer], windSpeed);
            SetTextColor(memDC, RGB(150, 150, 160));
            GetTextExtentPoint32(memDC, pwrBadge, strlen(pwrBadge), &sz);
            TextOut(memDC, CX - sz.cx/2, 152, pwrBadge, strlen(pwrBadge));
            
            // Draw thrown darts
            for (int i = 0; i < dartsCount; i++) {
                float scale = darts[i].animating ? (1.5f - darts[i].progress * 0.5f) : 1.0f;
                int dx = (int)darts[i].x;
                int dy = (int)darts[i].y;
                
                HPEN shaftPen = CreatePen(PS_SOLID, (int)(2.0f * scale), RGB(200, 200, 200));
                HPEN oldPen = (HPEN)SelectObject(memDC, shaftPen);
                MoveToEx(memDC, dx, dy, NULL);
                LineTo(memDC, dx + (int)(15.0f * scale), dy + (int)(25.0f * scale));
                SelectObject(memDC, oldPen);
                DeleteObject(shaftPen);
                
                POINT flight[4];
                flight[0].x = dx + (int)(15.0f * scale); flight[0].y = dy + (int)(25.0f * scale);
                flight[1].x = dx + (int)(25.0f * scale); flight[1].y = dy + (int)(15.0f * scale);
                flight[2].x = dx + (int)(35.0f * scale); flight[2].y = dy + (int)(25.0f * scale);
                flight[3].x = dx + (int)(25.0f * scale); flight[3].y = dy + (int)(35.0f * scale);
                
                COLORREF fColor = RGB(0, 255, 255);
                if (dartStyle == 1) fColor = RGB(255, 51, 51);
                else if (dartStyle == 2) fColor = RGB(255, 215, 0);
                HBRUSH fBrush = CreateSolidBrush(fColor);
                HPEN fPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HBRUSH oldFBr = (HBRUSH)SelectObject(memDC, fBrush);
                HPEN oldFPen = (HPEN)SelectObject(memDC, fPen);
                Polygon(memDC, flight, 4);
                SelectObject(memDC, oldFBr);
                SelectObject(memDC, oldFPen);
                DeleteObject(fBrush);
                DeleteObject(fPen);
                
                HBRUSH tBrush = CreateSolidBrush(RGB(255, 255, 255));
                HBRUSH oldTBr = (HBRUSH)SelectObject(memDC, tBrush);
                int r = (int)(2.0f * scale);
                Ellipse(memDC, dx - r, dy - r, dx + r, dy + r);
                SelectObject(memDC, oldTBr);
                DeleteObject(tBrush);
            }
            
            // Draw crosshair
            int isP1OrHuman = (currentPlayer == 0 || (!isCampaign && aiDifficulty == 4));
            if (gameState == 0 && isP1OrHuman) {
                int tx = mouseX + wobbleX;
                int ty = mouseY + wobbleY;
                
                COLORREF crossColor = focusActive[currentPlayer] ? RGB(0, 255, 128) : RGB(255, 255, 0);
                HPEN cPen = CreatePen(PS_SOLID, 2, crossColor);
                HPEN oldCPen = (HPEN)SelectObject(memDC, cPen);
                
                MoveToEx(memDC, tx - 15, ty, NULL); LineTo(memDC, tx + 15, ty);
                MoveToEx(memDC, tx, ty - 15, NULL); LineTo(memDC, tx, ty + 15);
                Arc(memDC, tx - 10, ty - 10, tx + 10, ty + 10, tx, ty - 10, tx, ty - 10);
                
                // Wind indicator vector line on crosshair
                if (windSpeed > 0) {
                    HPEN wPen = CreatePen(PS_DOT, 1, RGB(255, 100, 100));
                    SelectObject(memDC, wPen);
                    MoveToEx(memDC, tx, ty, NULL);
                    LineTo(memDC, tx + (int)windX, ty + (int)windY);
                    DeleteObject(wPen);
                }
                
                SelectObject(memDC, oldCPen);
                DeleteObject(cPen);
            }
            
            SelectObject(memDC, oldFont);
            DeleteObject(font);
            DeleteObject(largeFont);
            
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, hOld);
            DeleteObject(hbmMem);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
            
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "KDartsClass";
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0, "KDartsClass", "KDarts - Loop 6 Campaign Expansion", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 735, 780,
        NULL, NULL, wc.hInstance, NULL
    );
    
    if (hwnd == NULL) ExitProcess(0);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
