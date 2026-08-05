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
#define MODE_KILLER 5

typedef struct {
    const char* name;
    const char* opponent;
    int mode;
    int aiErr;
    int wobble;
    int wind;
} CampaignStage;

static const CampaignStage CAMPAIGN_STAGES[20] = {
    {"Local Pub", "Rookie Rick", MODE_301, 120, 10, 0},
    {"Amateur Open", "Amateur Andy", MODE_ATC, 110, 11, 2},
    {"League Qualifier", "Pub Regular Pete", MODE_301, 100, 12, 3},
    {"Town Championship", "Local Hero Luke", MODE_CRICKET, 90, 13, 4},
    {"County Clash", "District Champ Dave", MODE_BLITZ, 80, 14, 5},
    {"Regional Cup", "Regional Ace Ray", MODE_KILLER, 72, 15, 6},
    {"State Invitational", "State Master Sam", MODE_501, 65, 16, 7},
    {"National Qualifier", "Pro Qualifier Phil", MODE_ATC, 58, 18, 8},
    {"National Semifinal", "National Pro Ned", MODE_CRICKET, 50, 20, 9},
    {"National Final", "National Champ Nick", MODE_501, 44, 22, 10},
    {"International Masters", "Euro Champ Eric", MODE_BLITZ, 38, 24, 11},
    {"Grand Prix", "Grand Prix Finalist Gary", MODE_KILLER, 32, 26, 12},
    {"Premier League", "Premier Star Paul", MODE_301, 26, 28, 13},
    {"World Tour", "Continental Ace Connor", MODE_CRICKET, 22, 30, 14},
    {"World Semis", "World Contender Will", MODE_BLITZ, 18, 32, 15},
    {"World Trophy", "Elite Striker Sean", MODE_ATC, 14, 34, 16},
    {"Global Masters", "Master Thrower Max", MODE_KILLER, 10, 36, 17},
    {"Super League Final", "Champion Chris", MODE_501, 7, 38, 18},
    {"World Championship Semis", "Legend Lance", MODE_CRICKET, 4, 40, 19},
    {"World Championship Finals", "World Champion Vic", MODE_501, 2, 42, 20}
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
    float wobbleAmp;
} Dart;

typedef struct {
    float x, y, vx, vy;
    COLORREF color;
    int life, maxLife;
    float size;
} Particle;

typedef struct {
    float x, y, vy;
    char text[32];
    COLORREF color;
    int life, maxLife;
} ScoreText;

#define MAX_PARTICLES 64
#define MAX_SCORE_TEXTS 16
static Particle particles[MAX_PARTICLES];
static int particleCount = 0;
static ScoreText scoreTexts[MAX_SCORE_TEXTS];
static int scoreTextCount = 0;

static int shakeTime = 0;
static float shakeMag = 0.0f;

void TriggerShake(float mag, int duration) {
    shakeMag = mag;
    shakeTime = duration;
}

void SpawnSparks(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        if (particleCount >= MAX_PARTICLES) break;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)PI;
        float speed = 1.5f + ((float)rand() / RAND_MAX) * 4.5f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed - 1.0f;
        particles[particleCount].color = color;
        particles[particleCount].size = 1.5f + ((float)rand() / RAND_MAX) * 2.5f;
        particles[particleCount].life = 0;
        particles[particleCount].maxLife = 20 + (rand() % 20);
        particleCount++;
    }
}

void SpawnTrailParticle(float x, float y, COLORREF color) {
    if (particleCount >= MAX_PARTICLES) return;
    particles[particleCount].x = x;
    particles[particleCount].y = y;
    particles[particleCount].vx = (((float)rand() / RAND_MAX) - 0.5f) * 1.5f;
    particles[particleCount].vy = 1.5f + ((float)rand() / RAND_MAX) * 2.0f;
    particles[particleCount].color = color;
    particles[particleCount].size = 1.5f;
    particles[particleCount].life = 0;
    particles[particleCount].maxLife = 10;
    particleCount++;
}

void SpawnScoreText(float x, float y, const char* txt, COLORREF color) {
    if (scoreTextCount >= MAX_SCORE_TEXTS) {
        memmove(&scoreTexts[0], &scoreTexts[1], sizeof(ScoreText) * (MAX_SCORE_TEXTS - 1));
        scoreTextCount--;
    }
    scoreTexts[scoreTextCount].x = x;
    scoreTexts[scoreTextCount].y = y;
    scoreTexts[scoreTextCount].vy = -1.2f;
    strncpy(scoreTexts[scoreTextCount].text, txt, 31);
    scoreTexts[scoreTextCount].color = color;
    scoreTexts[scoreTextCount].life = 0;
    scoreTexts[scoreTextCount].maxLife = 50;
    scoreTextCount++;
}

int isCampaign = 0;
int campaignStage = 0;
int gameMode = MODE_501;
int aiDifficulty = 1;
int currentPlayer = 0;
int soundEnabled = 1;
int dartStyle = 0;

int scores[2] = {501, 501};
int prevScores[2] = {501, 501};
int cricketHits[2][7] = {{0}};
int atcTarget[2] = {1, 1};
int blitzHits[2] = {0, 0};
int killerLives[2] = {5, 5};
int isKiller[2] = {0, 0};

int totalDarts[2] = {0, 0};
int highestCheckout[2] = {0, 0};

int focusActive[2] = {0, 0};
int focusUses[2] = {2, 2};
int magnetActive[2] = {0, 0};
int magnetUses[2] = {2, 2};
int undoUses[2] = {1, 1};
int laserActive[2] = {0, 0};
int laserUses[2] = {2, 2};

float windX = 0.0f, windY = 0.0f;
int windSpeed = 0;
float windAngle = 0.0f;
int currentWobbleAmp = 15;

int dartsLeft = 3;
Dart darts[3];
int dartsCount = 0;
int gameState = 0;
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
    int killerLives[2];
    int isKiller[2];
    int totalDarts[2];
    int highestCheckout[2];
    int scores[2];
    int prevScores[2];
    int focusActive[2];
    int focusUses[2];
    int magnetActive[2];
    int magnetUses[2];
    int undoUses[2];
    int laserActive[2];
    int laserUses[2];
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
    memcpy(st->killerLives, killerLives, sizeof(killerLives));
    memcpy(st->isKiller, isKiller, sizeof(isKiller));
    memcpy(st->totalDarts, totalDarts, sizeof(totalDarts));
    memcpy(st->highestCheckout, highestCheckout, sizeof(highestCheckout));
    memcpy(st->scores, scores, sizeof(scores));
    memcpy(st->prevScores, prevScores, sizeof(prevScores));
    memcpy(st->focusActive, focusActive, sizeof(focusActive));
    memcpy(st->focusUses, focusUses, sizeof(focusUses));
    memcpy(st->magnetActive, magnetActive, sizeof(magnetActive));
    memcpy(st->magnetUses, magnetUses, sizeof(magnetUses));
    memcpy(st->undoUses, undoUses, sizeof(undoUses));
    memcpy(st->laserActive, laserActive, sizeof(laserActive));
    memcpy(st->laserUses, laserUses, sizeof(laserUses));
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
    memcpy(killerLives, st->killerLives, sizeof(killerLives));
    memcpy(isKiller, st->isKiller, sizeof(isKiller));
    memcpy(totalDarts, st->totalDarts, sizeof(totalDarts));
    memcpy(highestCheckout, st->highestCheckout, sizeof(highestCheckout));
    memcpy(scores, st->scores, sizeof(scores));
    memcpy(prevScores, st->prevScores, sizeof(prevScores));
    memcpy(focusActive, st->focusActive, sizeof(focusActive));
    memcpy(focusUses, st->focusUses, sizeof(focusUses));
    memcpy(magnetActive, st->magnetActive, sizeof(magnetActive));
    memcpy(magnetUses, st->magnetUses, sizeof(magnetUses));
    memcpy(undoUses, st->undoUses, sizeof(undoUses));
    memcpy(laserActive, st->laserActive, sizeof(laserActive));
    memcpy(laserUses, st->laserUses, sizeof(laserUses));
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
    laserUses[0] = 2; laserUses[1] = 2;
    focusActive[0] = 0; focusActive[1] = 0;
    magnetActive[0] = 0; magnetActive[1] = 0;
    laserActive[0] = 0; laserActive[1] = 0;
    killerLives[0] = 5; killerLives[1] = 5;
    isKiller[0] = 0; isKiller[1] = 0;
    
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
    } else if (mode == MODE_KILLER) {
        killerLives[0] = 5; killerLives[1] = 5;
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
    laserUses[0] = 2; laserUses[1] = 2;
    focusActive[0] = 0; focusActive[1] = 0;
    magnetActive[0] = 0; magnetActive[1] = 0;
    laserActive[0] = 0; laserActive[1] = 0;
    killerLives[0] = 5; killerLives[1] = 5;
    isKiller[0] = 0; isKiller[1] = 0;
    
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
    } else if (gameMode == MODE_KILLER) {
        killerLives[0] = 5; killerLives[1] = 5;
    }
    
    UpdateWind();
    sprintf(statusMsg, "Stage %d/20: %s vs %s", stage + 1, CAMPAIGN_STAGES[stage].name, CAMPAIGN_STAGES[stage].opponent);
}

void ActivateFocus(HWND hwnd) {
    if (focusUses[currentPlayer] > 0 && !focusActive[currentPlayer]) {
        focusActive[currentPlayer] = 1;
        focusUses[currentPlayer]--;
        PlayGameSound(4);
        sprintf(statusMsg, "Precision Focus Active! Aim wobble slowed by 75%%");
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

void ActivateLaser(HWND hwnd) {
    if (laserUses[currentPlayer] > 0 && !laserActive[currentPlayer]) {
        laserActive[currentPlayer] = 1;
        laserUses[currentPlayer]--;
        PlayGameSound(4);
        sprintf(statusMsg, "Laser Sight Active! Trajectory prediction line enabled.");
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
        } else if (gameMode == MODE_KILLER) {
            if (killerLives[1 - currentPlayer] < 5) killerLives[1 - currentPlayer]++;
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
        fwrite(killerLives, sizeof(int), 2, f);
        fwrite(isKiller, sizeof(int), 2, f);
        fwrite(totalDarts, sizeof(int), 2, f);
        fwrite(scores, sizeof(int), 2, f);
        fwrite(prevScores, sizeof(int), 2, f);
        fwrite(&dartsLeft, sizeof(int), 1, f);
        fwrite(&gameState, sizeof(int), 1, f);
        fwrite(highestCheckout, sizeof(int), 2, f);
        fwrite(focusUses, sizeof(int), 2, f);
        fwrite(magnetUses, sizeof(int), 2, f);
        fwrite(undoUses, sizeof(int), 2, f);
        fwrite(laserUses, sizeof(int), 2, f);
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
        fread(killerLives, sizeof(int), 2, f);
        fread(isKiller, sizeof(int), 2, f);
        fread(totalDarts, sizeof(int), 2, f);
        fread(scores, sizeof(int), 2, f);
        fread(prevScores, sizeof(int), 2, f);
        fread(&dartsLeft, sizeof(int), 1, f);
        fread(&gameState, sizeof(int), 1, f);
        fread(highestCheckout, sizeof(int), 2, f);
        fread(focusUses, sizeof(int), 2, f);
        fread(magnetUses, sizeof(int), 2, f);
        fread(undoUses, sizeof(int), 2, f);
        fread(laserUses, sizeof(int), 2, f);
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
        laserActive[currentPlayer] = 0;
        currentPlayer = 1 - currentPlayer;
        gameState = 0;
        UpdateWind();
        
        const char* p2Name = isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI");
        if (currentPlayer == 0) sprintf(statusMsg, "Player 1's Turn - Throw 3 Darts");
        else sprintf(statusMsg, "%s's Turn", p2Name);
    } else if (gameState == 2) {
        if (isCampaign) {
            if (currentPlayer == 0) {
                if (campaignStage < 19) {
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
        } else if (gameMode == MODE_KILLER) {
            targetNum = (!isKiller[1]) ? 19 : 20;
            targetMult = 3;
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
        darts[dartsCount].wobbleAmp = 0.0f;
        dartsCount++;
    }
    
    dartsLeft--;
    totalDarts[currentPlayer]++;
    
    const char* turnName = currentPlayer == 0 ? "Player 1" : (isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "Player 2" : "AI"));
    
    if (gameMode == MODE_501) {
        scores[currentPlayer] -= pts;
        if (scores[currentPlayer] < 0 || scores[currentPlayer] == 1) {
            sprintf(statusMsg, "%s Bust!", turnName);
            scores[currentPlayer] = prevScores[currentPlayer];
            gameState = 1;
        } else if (scores[currentPlayer] == 0) {
            if (mult == 2) {
                sprintf(statusMsg, "%s Wins with Double Out!", turnName);
                if (prevScores[currentPlayer] > highestCheckout[currentPlayer]) {
                    highestCheckout[currentPlayer] = prevScores[currentPlayer];
                }
                gameState = 2;
            } else {
                sprintf(statusMsg, "%s Bust! Must finish on a Double.", turnName);
                scores[currentPlayer] = prevScores[currentPlayer];
                gameState = 1;
            }
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over. Score: %d", turnName, scores[currentPlayer]);
            gameState = 1;
        } else {
            sprintf(statusMsg, "%s Hit %d! Darts left: %d", turnName, pts, dartsLeft);
        }
    } else if (gameMode == MODE_301) {
        scores[currentPlayer] -= pts;
        if (scores[currentPlayer] < 0 || scores[currentPlayer] == 1) {
            sprintf(statusMsg, "%s Bust!", turnName);
            scores[currentPlayer] = prevScores[currentPlayer];
            gameState = 1;
        } else if (scores[currentPlayer] == 0) {
            sprintf(statusMsg, "%s Wins 301!", turnName);
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
    } else if (gameMode == MODE_KILLER) {
        int targetSector = (currentPlayer == 0) ? 20 : 19;
        int oppSector = (currentPlayer == 0) ? 19 : 20;
        if (!isKiller[currentPlayer]) {
            if (number == targetSector) {
                isKiller[currentPlayer] = 1;
                sprintf(statusMsg, "%s unlocked KILLER status!", turnName);
            } else {
                sprintf(statusMsg, "%s missed sector %d! Darts left: %d", turnName, targetSector, dartsLeft);
            }
        } else {
            if (number == oppSector || mult >= 2) {
                int dmg = mult;
                killerLives[1 - currentPlayer] -= dmg;
                if (killerLives[1 - currentPlayer] <= 0) {
                    killerLives[1 - currentPlayer] = 0;
                    sprintf(statusMsg, "%s KILLER VICTORY! Opponent eliminated!", turnName);
                    gameState = 2;
                } else {
                    sprintf(statusMsg, "%s HIT! Opponent -%d Lives (Left: %d)", turnName, dmg, killerLives[1 - currentPlayer]);
                }
            } else if (dartsLeft == 0) {
                sprintf(statusMsg, "%s Turn Over. Opponent Lives: %d", turnName, killerLives[1 - currentPlayer]);
                gameState = 1;
            } else {
                sprintf(statusMsg, "Miss! Darts left: %d", dartsLeft);
            }
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void DrawPieSliceGDI(HDC hdc, int cx, int cy, int r, float a1, float a2, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_NULL, 0, 0);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    int left = cx - r;
    int top = cy - r;
    int right = cx + r;
    int bottom = cy + r;
    
    int dxs = (int)(cosf(a2) * 1000.0f);
    int dys = (int)(sinf(a2) * 1000.0f);
    int dxe = (int)(cosf(a1) * 1000.0f);
    int dye = (int)(sinf(a1) * 1000.0f);
    
    Pie(hdc, left, top, right, bottom, cx + dxs, cy + dys, cx + dxe, cy + dye);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void DrawCircleGDI(HDC hdc, int cx, int cy, int r, COLORREF color, COLORREF border) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void Draw3DSisalDartboardGDI(HDC hdc, int cx, int cy) {
    // 1. Mahogany Cabinet Frame
    DrawCircleGDI(hdc, cx, cy, (int)(BOARD_R * 1.35f), RGB(26, 12, 5), RGB(9, 4, 2));
    DrawCircleGDI(hdc, cx, cy, (int)(BOARD_R * 1.02f), RGB(10, 10, 12), RGB(20, 20, 24));

    // Brass corner bolts
    for (int k = 0; k < 4; k++) {
        float a = k * ((float)PI / 2.0f) + (float)PI / 4.0f;
        int bx = cx + (int)(cosf(a) * (BOARD_R * 1.22f));
        int by = cy + (int)(sinf(a) * (BOARD_R * 1.22f));
        DrawCircleGDI(hdc, bx, by, 5, RGB(212, 175, 55), RGB(92, 71, 16));
    }

    // 2. 3D Sisal Segment Beds
    float angleStep = (2.0f * (float)PI) / 20.0f;
    float startOffset = -(float)PI / 2.0f - angleStep / 2.0f;
    int scoresArray[] = {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
    
    for (int i = 0; i < 20; i++) {
        float a1 = startOffset + i * angleStep;
        float a2 = a1 + angleStep;
        int isRed = (i % 2 == 0);
        
        COLORREF colorRed = RGB(200, 16, 46);
        COLORREF colorGreen = RGB(0, 135, 81);
        COLORREF colorBlack = RGB(24, 24, 28);
        COLORREF colorCream = RGB(244, 235, 208);
        
        if (magnetActive[currentPlayer]) {
            colorRed = RGB(255, 68, 68);
            colorGreen = RGB(0, 224, 112);
        }
        
        DrawPieSliceGDI(hdc, cx, cy, (int)(R * 1.0f), a1, a2, isRed ? colorRed : colorGreen);
        DrawPieSliceGDI(hdc, cx, cy, (int)(R * 0.952f), a1, a2, isRed ? colorBlack : colorCream);
        DrawPieSliceGDI(hdc, cx, cy, (int)(R * 0.629f), a1, a2, isRed ? colorRed : colorGreen);
        DrawPieSliceGDI(hdc, cx, cy, (int)(R * 0.582f), a1, a2, isRed ? colorBlack : colorCream);
    }
    
    // Outer Bull & Inner Bull
    DrawCircleGDI(hdc, cx, cy, (int)(R * 0.093f), RGB(0, 135, 81), RGB(0, 80, 50));
    DrawCircleGDI(hdc, cx, cy, (int)(R * 0.037f), RGB(200, 16, 46), RGB(255, 215, 0));
    DrawCircleGDI(hdc, cx - 1, cy - 1, (int)(R * 0.015f), RGB(255, 255, 255), RGB(255, 255, 255));

    // 3. Metallic Wire Spider Grid (Rings & Radial Spokes)
    float ringRadii[] = {R * 1.0f, R * 0.952f, R * 0.629f, R * 0.582f, R * 0.093f, R * 0.037f};
    for (int k = 0; k < 6; k++) {
        int r = (int)ringRadii[k];
        // Shadow line
        HPEN sPen = CreatePen(PS_SOLID, 2, RGB(20, 20, 20));
        HPEN oldP = (HPEN)SelectObject(hdc, sPen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, nullBr);
        Ellipse(hdc, cx - r + 1, cy - r + 1, cx + r + 1, cy + r + 1);
        SelectObject(hdc, oldP); DeleteObject(sPen);
        
        // Base wire
        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(180, 185, 195));
        SelectObject(hdc, wPen);
        Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
        SelectObject(hdc, oldP); SelectObject(hdc, oldB); DeleteObject(wPen);
    }

    for (int i = 0; i < 20; i++) {
        float a = startOffset + i * angleStep;
        int x1 = cx + (int)(cosf(a) * (R * 0.093f));
        int y1 = cy + (int)(sinf(a) * (R * 0.093f));
        int x2 = cx + (int)(cosf(a) * (R * 1.0f));
        int y2 = cy + (int)(sinf(a) * (R * 1.0f));
        
        // Wire Shadow
        HPEN sPen = CreatePen(PS_SOLID, 2, RGB(20, 20, 20));
        HPEN oldP = (HPEN)SelectObject(hdc, sPen);
        MoveToEx(hdc, x1 + 1, y1 + 1, NULL);
        LineTo(hdc, x2 + 1, y2 + 1);
        SelectObject(hdc, oldP); DeleteObject(sPen);
        
        // Base Wire
        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(210, 215, 225));
        SelectObject(hdc, wPen);
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, oldP); DeleteObject(wPen);

        // Metallic staples at intersections
        float stapleRadii[] = {R * 1.0f, R * 0.952f, R * 0.629f, R * 0.582f};
        for (int k = 0; k < 4; k++) {
            int sx = cx + (int)(cosf(a) * stapleRadii[k]);
            int sy = cy + (int)(sinf(a) * stapleRadii[k]);
            DrawCircleGDI(hdc, sx, sy, 2, RGB(230, 235, 240), RGB(50, 50, 50));
        }
    }

    // 4. Outer Metallic Number Wire Ring
    int numR = (int)(R * 1.15f);
    HPEN nrPen = CreatePen(PS_SOLID, 2, RGB(140, 145, 155));
    HPEN oldP = (HPEN)SelectObject(hdc, nrPen);
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, nullBr);
    Ellipse(hdc, cx - numR, cy - numR, cx + numR, cy + numR);
    SelectObject(hdc, oldP); SelectObject(hdc, oldB); DeleteObject(nrPen);

    // 1-20 Metal Wire Numbers
    SetBkMode(hdc, TRANSPARENT);
    HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                            DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    
    for (int i = 0; i < 20; i++) {
        float a = startOffset + i * angleStep + angleStep / 2.0f;
        int nx = cx + (int)(cosf(a) * numR);
        int ny = cy + (int)(sinf(a) * numR);
        char numStr[3];
        sprintf(numStr, "%d", scoresArray[i]);
        SIZE sz;
        GetTextExtentPoint32(hdc, numStr, strlen(numStr), &sz);
        
        // Shadow
        SetTextColor(hdc, RGB(0, 0, 0));
        TextOut(hdc, nx - sz.cx/2 + 1, ny - sz.cy/2 + 1, numStr, strlen(numStr));
        // Metallic Face
        SetTextColor(hdc, RGB(235, 240, 245));
        TextOut(hdc, nx - sz.cx/2, ny - sz.cy/2, numStr, strlen(numStr));
    }
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void DrawDetailed3DDart(HDC hdc, int x, int y, float scale, float angle, int style, int isAnimating) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    
    #define TRANSFORM_PT(px, py, outX, outY) do { \
        float sx = (float)(px) * scale; \
        float sy = (float)(py) * scale; \
        outX = x + (int)(sx * cosA - sy * sinA); \
        outY = y + (int)(sx * sinA + sy * cosA); \
    } while(0)

    if (!isAnimating) {
        POINT shadowPts[10];
        int pxs[] = {0, -4, -5, -2, -12, 0, 12, 2, 5, 4};
        int pys[] = {0, -12, -34, -54, -76, -82, -76, -54, -34, -12};
        for(int k=0; k<10; k++) {
            TRANSFORM_PT(pxs[k] + 6, pys[k] + 10, shadowPts[k].x, shadowPts[k].y);
        }
        HBRUSH shBr = CreateSolidBrush(RGB(10, 10, 10));
        HPEN shPen = CreatePen(PS_NULL, 0, 0);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, shBr);
        HPEN oldP = (HPEN)SelectObject(hdc, shPen);
        Polygon(hdc, shadowPts, 10);
        SelectObject(hdc, oldB); SelectObject(hdc, oldP);
        DeleteObject(shBr); DeleteObject(shPen);
    }

    // Steel Tip (0,0) to (0,-14)
    POINT tipPts[3];
    TRANSFORM_PT(0, 0, tipPts[0].x, tipPts[0].y);
    TRANSFORM_PT(-2, -14, tipPts[1].x, tipPts[1].y);
    TRANSFORM_PT(2, -14, tipPts[2].x, tipPts[2].y);
    
    HBRUSH tipBr = CreateSolidBrush(RGB(220, 225, 230));
    HPEN tipPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, tipBr);
    HPEN oldP = (HPEN)SelectObject(hdc, tipPen);
    Polygon(hdc, tipPts, 3);
    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
    DeleteObject(tipBr); DeleteObject(tipPen);

    // Brass / Tungsten Barrel (-14 to -38)
    POINT barPts[4];
    TRANSFORM_PT(-4, -14, barPts[0].x, barPts[0].y);
    TRANSFORM_PT(4, -14, barPts[1].x, barPts[1].y);
    TRANSFORM_PT(4, -38, barPts[2].x, barPts[2].y);
    TRANSFORM_PT(-4, -38, barPts[3].x, barPts[3].y);

    COLORREF bColor = (style == 2) ? RGB(230, 190, 50) : RGB(180, 185, 190);
    HBRUSH barBr = CreateSolidBrush(bColor);
    HPEN barPen = CreatePen(PS_SOLID, 1, RGB(40, 40, 40));
    oldB = (HBRUSH)SelectObject(hdc, barBr);
    oldP = (HPEN)SelectObject(hdc, barPen);
    Polygon(hdc, barPts, 4);
    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
    DeleteObject(barBr); DeleteObject(barPen);

    // Knurling ridges
    HPEN ridgePen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
    oldP = (HPEN)SelectObject(hdc, ridgePen);
    for(int bY = -34; bY <= -18; bY += 4) {
        int lx1, ly1, lx2, ly2;
        TRANSFORM_PT(-4, bY, lx1, ly1);
        TRANSFORM_PT(4, bY, lx2, ly2);
        MoveToEx(hdc, lx1, ly1, NULL);
        LineTo(hdc, lx2, ly2);
    }
    SelectObject(hdc, oldP); DeleteObject(ridgePen);

    // Aluminum Shaft (-38 to -58)
    POINT shfPts[4];
    TRANSFORM_PT(-2, -38, shfPts[0].x, shfPts[0].y);
    TRANSFORM_PT(2, -38, shfPts[1].x, shfPts[1].y);
    TRANSFORM_PT(2, -58, shfPts[2].x, shfPts[2].y);
    TRANSFORM_PT(-2, -58, shfPts[3].x, shfPts[3].y);

    HBRUSH shfBr = CreateSolidBrush(RGB(60, 60, 65));
    HPEN shfPen = CreatePen(PS_SOLID, 1, RGB(20, 20, 20));
    oldB = (HBRUSH)SelectObject(hdc, shfBr);
    oldP = (HPEN)SelectObject(hdc, shfPen);
    Polygon(hdc, shfPts, 4);
    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
    DeleteObject(shfBr); DeleteObject(shfPen);

    // 3D Tail Flights (-58 to -84)
    COLORREF fColor = RGB(0, 240, 255);
    if (style == 1) fColor = RGB(255, 42, 42);
    else if (style == 2) fColor = RGB(255, 215, 0);

    POINT fin1[6];
    TRANSFORM_PT(0, -58, fin1[0].x, fin1[0].y);
    TRANSFORM_PT(-14, -72, fin1[1].x, fin1[1].y);
    TRANSFORM_PT(-11, -84, fin1[2].x, fin1[2].y);
    TRANSFORM_PT(0, -78, fin1[3].x, fin1[3].y);
    TRANSFORM_PT(11, -84, fin1[4].x, fin1[4].y);
    TRANSFORM_PT(14, -72, fin1[5].x, fin1[5].y);

    HBRUSH fltBr = CreateSolidBrush(fColor);
    HPEN fltPen = CreatePen(PS_SOLID, 1, RGB(10, 10, 10));
    oldB = (HBRUSH)SelectObject(hdc, fltBr);
    oldP = (HPEN)SelectObject(hdc, fltPen);
    Polygon(hdc, fin1, 6);

    POINT fin2[6];
    TRANSFORM_PT(0, -60, fin2[0].x, fin2[0].y);
    TRANSFORM_PT(-6, -70, fin2[1].x, fin2[1].y);
    TRANSFORM_PT(-4, -78, fin2[2].x, fin2[2].y);
    TRANSFORM_PT(0, -76, fin2[3].x, fin2[3].y);
    TRANSFORM_PT(4, -78, fin2[4].x, fin2[4].y);
    TRANSFORM_PT(6, -70, fin2[5].x, fin2[5].y);

    Polygon(hdc, fin2, 6);
    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
    DeleteObject(fltBr); DeleteObject(fltPen);

    #undef TRANSFORM_PT
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            // Row 1: Modes & Campaign
            CreateWindow("BUTTON", "501", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         20, 15, 45, 26, hwnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "301", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         70, 15, 45, 26, hwnd, (HMENU)111, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Cricket", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         120, 15, 55, 26, hwnd, (HMENU)102, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "ATC", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         180, 15, 45, 26, hwnd, (HMENU)112, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Blitz", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         230, 15, 45, 26, hwnd, (HMENU)113, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Killer", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         280, 15, 50, 26, hwnd, (HMENU)118, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Campaign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         335, 15, 75, 26, hwnd, (HMENU)114, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 2: Power-Ups & State
            CreateWindow("BUTTON", "Focus (F)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         415, 15, 70, 26, hwnd, (HMENU)115, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Magnet (M)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         490, 15, 80, 26, hwnd, (HMENU)116, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Undo Dart", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         575, 15, 70, 26, hwnd, (HMENU)117, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Laser (L)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         650, 15, 65, 26, hwnd, (HMENU)119, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 3: Settings & Utility
            CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         20, 46, 50, 26, hwnd, (HMENU)104, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         75, 46, 50, 26, hwnd, (HMENU)105, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Undo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         130, 46, 50, 26, hwnd, (HMENU)108, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Redo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         185, 46, 50, 26, hwnd, (HMENU)109, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Vs AI: Medium", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         240, 46, 110, 26, hwnd, (HMENU)103, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Sound", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         355, 46, 60, 26, hwnd, (HMENU)106, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Style", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         420, 46, 55, 26, hwnd, (HMENU)107, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         480, 46, 50, 26, hwnd, (HMENU)110, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            SetTimer(hwnd, TIMER_ID, 16, NULL);
            return 0;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 101) { SetMode(MODE_501); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 111) { SetMode(MODE_301); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 102) { SetMode(MODE_CRICKET); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 112) { SetMode(MODE_ATC); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 113) { SetMode(MODE_BLITZ); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 118) { SetMode(MODE_KILLER); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 114) { StartCampaign(0); InvalidateRect(hwnd, NULL, FALSE); }
            else if (LOWORD(wParam) == 115) { ActivateFocus(hwnd); }
            else if (LOWORD(wParam) == 116) { ActivateMagnet(hwnd); }
            else if (LOWORD(wParam) == 117) { ActivateUndoDart(hwnd); }
            else if (LOWORD(wParam) == 119) { ActivateLaser(hwnd); }
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
                    "KDarts - 3D Visual Edition (Loop 7 Expansion):\n\n"
                    "Controls:\n"
                    "- Aim with mouse, click to throw.\n"
                    "- Press F: Focus (slows wobble 75%)\n"
                    "- Press M: Magnet (expands double/triple hitboxes)\n"
                    "- Press U: Undo Dart (re-throw 1 bad dart per leg)\n"
                    "- Press L: Laser Sight (predicts exact trajectory arc)\n"
                    "- Keys 1-6: Switch Modes | Key C: Campaign Mode\n\n"
                    "Game Modes:\n"
                    "1. 501 Double Out: Reach 0 with a Double checkout!\n"
                    "2. 301: Reach exactly 0.\n"
                    "3. Cricket: Close 15-20 and Bullseye.\n"
                    "4. Around the Clock (ATC): Hit numbers 1 to 20 in sequence.\n"
                    "5. Bullseye Blitz: First to score 10 Bullseye hits.\n"
                    "6. Killer Darts: Become Killer, reduce opponent's 5 lives to 0!\n\n"
                    "Campaign Mode:\n"
                    "20 progressive stages from Rookie Rick to World Championship Finals against World Champion Vic!", 
                    "How to Play KDarts", MB_OK | MB_ICONINFORMATION);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == 'F' || wParam == 'f') ActivateFocus(hwnd);
            else if (wParam == 'M' || wParam == 'm') ActivateMagnet(hwnd);
            else if (wParam == 'U' || wParam == 'u') ActivateUndoDart(hwnd);
            else if (wParam == 'L' || wParam == 'l') ActivateLaser(hwnd);
            else if (wParam == 'C' || wParam == 'c') { StartCampaign(0); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '1') { SetMode(MODE_501); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '2') { SetMode(MODE_301); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '3') { SetMode(MODE_CRICKET); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '4') { SetMode(MODE_ATC); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '5') { SetMode(MODE_BLITZ); InvalidateRect(hwnd, NULL, FALSE); }
            else if (wParam == '6') { SetMode(MODE_KILLER); InvalidateRect(hwnd, NULL, FALSE); }
            return 0;

        case WM_TIMER: {
            float speedMult = focusActive[currentPlayer] ? 0.25f : 1.0f;
            t += 0.05f * speedMult;
            
            float amp = (float)currentWobbleAmp;
            if (focusActive[currentPlayer]) amp *= 0.25f;
            wobbleX = (int)(sinf(t * 1.3f) * amp + sinf(t * 0.8f) * (amp * 0.6f));
            wobbleY = (int)(cosf(t * 1.5f) * amp + sinf(t * 0.9f) * (amp * 0.6f));
            
            for (int i = 0; i < dartsCount; i++) {
                if (darts[i].animating) {
                    darts[i].progress += 0.08f;
                    if (darts[i].progress >= 1.0f) {
                        darts[i].progress = 1.0f;
                        darts[i].animating = 0;
                        darts[i].wobbleAmp = (darts[i].number == 25) ? 12.0f : ((darts[i].mult == 3) ? 9.0f : 6.0f);
                        TriggerShake((darts[i].number == 25) ? 10.0f : ((darts[i].mult == 3) ? 7.0f : 4.0f), 10);
                        
                        if (darts[i].number == 25) {
                            PlayGameSound(3);
                            SpawnSparks((float)darts[i].targetX, (float)darts[i].targetY, RGB(255, 215, 0), 35);
                            SpawnSparks((float)darts[i].targetX, (float)darts[i].targetY, RGB(255, 51, 51), 20);
                            SpawnScoreText((float)darts[i].targetX, (float)darts[i].targetY - 20, 
                                           darts[i].mult == 2 ? "DOUBLE BULL! 50" : "BULLSEYE! 25", RGB(255, 215, 0));
                        } else {
                            PlayGameSound(2);
                            COLORREF col = (darts[i].mult == 3) ? RGB(0, 255, 136) : ((darts[i].mult == 2) ? RGB(255, 68, 68) : RGB(255, 255, 255));
                            SpawnSparks((float)darts[i].targetX, (float)darts[i].targetY, col, (darts[i].mult == 3) ? 25 : ((darts[i].mult == 2) ? 18 : 12));
                            
                            char txt[32];
                            if (darts[i].mult == 3) sprintf(txt, "TRIPLE %d! +%d", darts[i].number, darts[i].pts);
                            else if (darts[i].mult == 2) sprintf(txt, "DOUBLE %d! +%d", darts[i].number, darts[i].pts);
                            else if (darts[i].pts > 0) sprintf(txt, "+%d", darts[i].pts);
                            else strcpy(txt, "MISS!");
                            
                            COLORREF txtCol = (darts[i].mult == 3) ? RGB(0, 255, 204) : ((darts[i].mult == 2) ? RGB(255, 102, 102) : RGB(255, 255, 255));
                            SpawnScoreText((float)darts[i].targetX, (float)darts[i].targetY - 20, txt, txtCol);
                        }
                    }
                    float ease = 1.0f - powf(1.0f - darts[i].progress, 3.0f);
                    float arc = sinf(darts[i].progress * (float)PI) * 110.0f;
                    darts[i].x = CX + (darts[i].targetX - CX) * ease;
                    darts[i].y = 750.0f + (darts[i].targetY - 750.0f) * ease - arc;
                    
                    if ((rand() % 100) < 60) {
                        COLORREF tCol = RGB(0, 240, 255);
                        if (dartStyle == 1) tCol = RGB(255, 42, 42);
                        else if (dartStyle == 2) tCol = RGB(255, 215, 0);
                        float scale = 2.4f - darts[i].progress * 1.4f;
                        SpawnTrailParticle(darts[i].x, darts[i].y + 30.0f * scale, tCol);
                    }
                } else {
                    darts[i].x = (float)darts[i].targetX;
                    darts[i].y = (float)darts[i].targetY;
                }
            }
            
            // Update particles
            for (int i = particleCount - 1; i >= 0; i--) {
                particles[i].life++;
                particles[i].x += particles[i].vx;
                particles[i].y += particles[i].vy;
                particles[i].vy += 0.15f;
                if (particles[i].life >= particles[i].maxLife) {
                    particles[i] = particles[particleCount - 1];
                    particleCount--;
                }
            }

            // Update floating score text
            for (int i = scoreTextCount - 1; i >= 0; i--) {
                scoreTexts[i].life++;
                scoreTexts[i].y += scoreTexts[i].vy;
                if (scoreTexts[i].life >= scoreTexts[i].maxLife) {
                    scoreTexts[i] = scoreTexts[scoreTextCount - 1];
                    scoreTextCount--;
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
            if (mouseY < 170 && mouseX > 15 && mouseX < 700) return 0;
            
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
            
            HBRUSH mortarBr = CreateSolidBrush(RGB(34, 17, 17));
            FillRect(memDC, &rect, mortarBr);
            HBRUSH brickBr = CreateSolidBrush(RGB(51, 26, 26));
            for (int by = 0; by < height; by += 40) {
                int offsetX = ((by / 40) % 2 == 0) ? 0 : -40;
                for (int bx = offsetX; bx < width; bx += 80) {
                    RECT bRect = {bx + 2, by + 2, bx + 78, by + 38};
                    FillRect(memDC, &bRect, brickBr);
                }
            }
            DeleteObject(brickBr);
            DeleteObject(mortarBr);
            
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
            
            // Calculate screen shake
            int shakeX = 0, shakeY = 0;
            if (shakeTime > 0) {
                shakeX = (int)(((float)rand()/RAND_MAX - 0.5f) * shakeMag);
                shakeY = (int)(((float)rand()/RAND_MAX - 0.5f) * shakeMag);
                shakeTime--;
                shakeMag *= 0.85f;
            }

            // Draw 3D Sisal Dartboard with metallic wire spider
            Draw3DSisalDartboardGDI(memDC, CX + shakeX, CY + shakeY);
            
            // Header Text & Score UI
            SetBkMode(memDC, TRANSPARENT);
            HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT largeFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            const char* p2Name = isCampaign ? CAMPAIGN_STAGES[campaignStage].opponent : (aiDifficulty == 4 ? "P2" : "AI");
            
            HFONT oldFont = (HFONT)SelectObject(memDC, font);
            SetTextColor(memDC, RGB(255, 215, 0));
            char titleLine[128];
            if (isCampaign) {
                sprintf(titleLine, "CAMPAIGN STAGE %d/20 - %s (%s)", campaignStage + 1, CAMPAIGN_STAGES[campaignStage].name, p2Name);
            } else {
                const char* mNames[] = {"501 Double Out", "301", "Cricket", "Around the Clock", "Bullseye Blitz", "Killer Darts"};
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
            } else if (gameMode == MODE_KILLER) {
                SelectObject(memDC, largeFont);
                SetTextColor(memDC, RGB(255, 88, 88));
                char scoreStr[64];
                sprintf(scoreStr, "P1 Lives: %d  |  %s Lives: %d", killerLives[0], p2Name, killerLives[1]);
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, CX - sz.cx/2, 98, scoreStr, strlen(scoreStr));
            }
            
            SelectObject(memDC, font);
            SetTextColor(memDC, RGB(180, 220, 255));
            GetTextExtentPoint32(memDC, statusMsg, strlen(statusMsg), &sz);
            TextOut(memDC, CX - sz.cx/2, 138, statusMsg, strlen(statusMsg));
            
            char pwrBadge[140];
            sprintf(pwrBadge, "Focus(F): %d/2 %s | Magnet(M): %d/2 %s | Undo(U): %d/1 | Laser(L): %d/2 %s | Wind: %d mph", 
                    focusUses[currentPlayer], focusActive[currentPlayer] ? "[ACTIVE]" : "",
                    magnetUses[currentPlayer], magnetActive[currentPlayer] ? "[ACTIVE]" : "",
                    undoUses[currentPlayer],
                    laserUses[currentPlayer], laserActive[currentPlayer] ? "[ACTIVE]" : "",
                    windSpeed);
            SetTextColor(memDC, RGB(150, 150, 160));
            GetTextExtentPoint32(memDC, pwrBadge, strlen(pwrBadge), &sz);
            TextOut(memDC, CX - sz.cx/2, 152, pwrBadge, strlen(pwrBadge));
            
            // Draw Darts
            for (int i = 0; i < dartsCount; i++) {
                int renderX = (int)darts[i].x + shakeX;
                int renderY = (int)darts[i].y + shakeY;
                float scale = 1.0f;
                float angle = 0.0f;
                int isAnim = darts[i].animating;
                
                if (isAnim) {
                    scale = 2.4f - darts[i].progress * 1.4f;
                    angle = (1.0f - darts[i].progress) * -0.4f;
                } else {
                    if (darts[i].wobbleAmp > 0.05f) {
                        angle = sinf(t * 28.0f) * (darts[i].wobbleAmp * ((float)PI / 180.0f));
                        darts[i].wobbleAmp *= 0.88f;
                    }
                }
                
                DrawDetailed3DDart(memDC, renderX, renderY, scale, angle, dartStyle, isAnim);
            }
            
            // Draw Spark Particles
            for (int i = 0; i < particleCount; i++) {
                int px = (int)particles[i].x + shakeX;
                int py = (int)particles[i].y + shakeY;
                int pSize = (int)particles[i].size;
                HBRUSH pBr = CreateSolidBrush(particles[i].color);
                HPEN pPen = CreatePen(PS_NULL, 0, 0);
                HBRUSH oldB = (HBRUSH)SelectObject(memDC, pBr);
                HPEN oldP = (HPEN)SelectObject(memDC, pPen);
                Ellipse(memDC, px - pSize, py - pSize, px + pSize, py + pSize);
                SelectObject(memDC, oldB); SelectObject(memDC, oldP);
                DeleteObject(pBr); DeleteObject(pPen);
            }

            // Draw Floating Score Texts
            for (int i = 0; i < scoreTextCount; i++) {
                int tx = (int)scoreTexts[i].x + shakeX;
                int ty = (int)scoreTexts[i].y + shakeY;
                GetTextExtentPoint32(memDC, scoreTexts[i].text, strlen(scoreTexts[i].text), &sz);
                
                SetTextColor(memDC, RGB(0, 0, 0));
                TextOut(memDC, tx - sz.cx/2 + 1, ty - sz.cy/2 + 1, scoreTexts[i].text, strlen(scoreTexts[i].text));
                SetTextColor(memDC, scoreTexts[i].color);
                TextOut(memDC, tx - sz.cx/2, ty - sz.cy/2, scoreTexts[i].text, strlen(scoreTexts[i].text));
            }

            // Crosshair & Laser Sight Arc
            int isP1OrHuman = (currentPlayer == 0 || (!isCampaign && aiDifficulty == 4));
            if (gameState == 0 && isP1OrHuman) {
                int tx = mouseX + wobbleX + (int)windX + shakeX;
                int ty = mouseY + wobbleY + (int)windY + shakeY;
                
                if (laserActive[currentPlayer]) {
                    HPEN lPen = CreatePen(PS_DOT, 2, RGB(255, 0, 128));
                    HPEN oldLPen = (HPEN)SelectObject(memDC, lPen);
                    MoveToEx(memDC, CX, 750, NULL);
                    LineTo(memDC, tx, ty);
                    SelectObject(memDC, oldLPen);
                    DeleteObject(lPen);
                    DrawCircleGDI(memDC, tx, ty, 8, RGB(255, 0, 128), RGB(255, 255, 255));
                }

                COLORREF crossColor = focusActive[currentPlayer] ? RGB(0, 255, 128) : (laserActive[currentPlayer] ? RGB(255, 0, 128) : RGB(255, 255, 0));
                HPEN cPen = CreatePen(PS_SOLID, 2, crossColor);
                HPEN oldCPen = (HPEN)SelectObject(memDC, cPen);
                
                MoveToEx(memDC, tx - 18, ty, NULL); LineTo(memDC, tx + 18, ty);
                MoveToEx(memDC, tx, ty - 18, NULL); LineTo(memDC, tx, ty + 18);
                Arc(memDC, tx - 10, ty - 10, tx + 10, ty + 10, tx, ty - 10, tx, ty - 10);
                
                DrawCircleGDI(memDC, tx, ty, 2, crossColor, crossColor);

                if (windSpeed > 0) {
                    HPEN wPen = CreatePen(PS_DOT, 1, RGB(255, 100, 100));
                    SelectObject(memDC, wPen);
                    MoveToEx(memDC, mouseX + wobbleX + shakeX, mouseY + wobbleY + shakeY, NULL);
                    LineTo(memDC, tx, ty);
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
        0, "KDartsClass", "KDarts - 3D Visual Edition", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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
