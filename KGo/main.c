#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ID_BTN_PASS 101
#define ID_BTN_RESIGN 102
#define ID_BTN_NEW 103
#define ID_CB_SIZE 104
#define ID_BTN_SCORE 105
#define ID_CB_AI 106
#define ID_BTN_SAVE 107
#define ID_BTN_LOAD 108
#define ID_BTN_UNDO 109
#define ID_BTN_REDO 110
#define ID_BTN_STATS 111
#define ID_CB_DIFFICULTY 112
#define ID_BTN_HELP 113
#define ID_BTN_CAMPAIGN 114
#define ID_BTN_HINT 115
#define ID_BTN_ESTIMATE 116
#define ID_BTN_ANALYZER 117
#define ID_BTN_TSUMEGO 118

typedef struct {
    int x;
    int y;
    int color; // 1 = Black, 2 = White
} StonePos;

typedef struct {
    int size;          // 9, 13, 19
    int handicap;      // 0..9
    float komi;        // 0.5, 6.5, 7.5
    int aiPersonality; // 0=Territorial, 1=Influence, 2=Balanced, 3=Grandmaster
    char name[64];
    int isTsumego;     // 1 if puzzle stage
    int stoneCount;
    StonePos initialStones[16];
    char targetDesc[128];
} CampaignStage;

CampaignStage campaign[20] = {
    {9, 5, 0.5f, 0, "Stage 1: Novice Field", 0, 0, {}, ""},
    {9, 3, 0.5f, 1, "Stage 2: Corner Skirmish", 0, 0, {}, ""},
    {9, 0, 0.5f, 0, "Stage 3: Tsumego - Corner Capture", 1, 7, 
     {{1,0,2}, {1,1,2}, {0,1,2}, {2,0,1}, {2,1,1}, {1,2,1}, {0,2,1}}, 
     "Capture White's corner group at (0,0)!"},
    {9, 2, 0.5f, 2, "Stage 4: Tactical Border", 0, 0, {}, ""},
    {9, 0, 6.5f, 3, "Stage 5: 9x9 Master Duel", 0, 0, {}, ""},
    {13, 7, 0.5f, 0, "Stage 6: Medium Horizon", 0, 0, {}, ""},
    {13, 0, 0.5f, 1, "Stage 7: Tsumego - Side Crane's Nest", 1, 8,
     {{3,2,2}, {4,2,2}, {5,2,2}, {3,3,1}, {6,2,1}, {4,1,1}, {5,1,1}, {4,3,2}},
     "Expose White's weakness on the side!"},
    {13, 4, 0.5f, 2, "Stage 8: Pincer Conflict", 0, 0, {}, ""},
    {13, 2, 6.5f, 1, "Stage 9: Influence Battle", 0, 0, {}, ""},
    {13, 0, 6.5f, 3, "Stage 10: 13x13 Grandmaster", 0, 0, {}, ""},
    {19, 0, 0.5f, 0, "Stage 11: Tsumego - Making Two Eyes", 1, 11,
     {{0,1,1}, {1,1,1}, {2,1,1}, {3,1,1}, {3,0,1}, {0,2,2}, {1,2,2}, {2,2,2}, {3,2,2}, {4,1,2}, {4,0,2}},
     "Play the vital point to secure two eyes for Black!"},
    {19, 8, 0.5f, 1, "Stage 12: Great Wall Siege", 0, 0, {}, ""},
    {19, 5, 0.5f, 2, "Stage 13: Dragon Slayer", 0, 0, {}, ""},
    {19, 0, 0.5f, 3, "Stage 14: Tsumego - Surround the Center", 1, 9,
     {{9,9,2}, {10,9,2}, {9,10,2}, {10,10,2}, {8,9,1}, {11,9,1}, {8,10,1}, {11,10,1}, {9,11,1}},
     "Surround and capture White's central shape!"},
    {19, 3, 0.5f, 3, "Stage 15: Dragon Slayer Elite", 0, 0, {}, ""},
    {19, 0, 0.5f, 2, "Stage 16: Tsumego - Belly Attachment", 1, 10,
     {{5,5,2}, {5,6,2}, {6,5,2}, {4,5,1}, {4,6,1}, {5,4,1}, {6,4,1}, {7,5,1}, {6,7,1}, {5,7,1}},
     "Play the belly attachment tesuji at (6,6) to capture White!"},
    {19, 2, 6.5f, 1, "Stage 17: Center Star Invasion", 0, 0, {}, ""},
    {19, 1, 6.5f, 2, "Stage 18: Even Corner Clash", 0, 0, {}, ""},
    {19, 0, 7.5f, 3, "Stage 19: KGo Championship Final", 0, 0, {}, ""},
    {19, 0, 7.5f, 3, "Stage 20: Grandmaster Go Legend Challenge", 0, 0, {}, ""}
};

int currentCampaignStage = -1;
float currentKomi = 6.5f;
int currentTsumegoIndex = 0;

typedef struct {
    int played;
    int aiWins;
    int aiLosses;
    int localB;
    int localW;
} GameStats;
GameStats stats = {0};

void LoadStats() {
    FILE *f = fopen("kgo_stats.dat", "rb");
    if (f) {
        fread(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}
void SaveStats() {
    FILE *f = fopen("kgo_stats.dat", "wb");
    if (f) {
        fwrite(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}
void RecordGameEnd(int winnerPlayer, HWND hwnd) {
    stats.played++;
    if (SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        if (winnerPlayer == 1) stats.aiWins++;
        else stats.aiLosses++;
    } else {
        if (winnerPlayer == 1) stats.localB++;
        else stats.localW++;
    }
    SaveStats();
}

typedef struct {
    char board[19][19];
    char prevBoard[19][19];
    int currentPlayer;
    int captures[3];
} GameState;

#define MAX_HISTORY 500
GameState undoStack[MAX_HISTORY];
int undoCount = 0;
GameState redoStack[MAX_HISTORY];
int redoCount = 0;
int consecutivePasses = 0;

int boardSize = 9;
char board[19][19] = {0}; // 0 = empty, 1 = black, 2 = white
char prevBoard[19][19] = {0};
int currentPlayer = 1;
int captures[3] = {0}; // 1 = black, 2 = white
int lastMoveX = -1, lastMoveY = -1;
int hoverX = -1, hoverY = -1;
int animX = -1, animY = -1;
int animRadius = 13;
int rippleRadius = 0;
POINT capturedAnimStones[19*19];
int capturedAnimColor[19*19];
int capturedAnimCount = 0;
int captureAnimRadius = 0;

int hintX = -1, hintY = -1;
int hintScore = 0;
int showEstimator = 0;
int showAnalyzer = 0;
char territoryMap[19][19] = {0}; // 1=Black, 2=White
char atariMap[19][19] = {0};     // 1=Group in Atari
char koMap[19][19] = {0};        // 1=Forbidden Ko / Superko intersection

typedef enum {
    PARTICLE_SPARK = 0,   // Layer 1: Incandescent Core Spark
    PARTICLE_SMOKE = 1,   // Layer 2: Expanding Floating Smoke Puff
    PARTICLE_SHARD = 2,   // Layer 3: Heavy Kinematic Stone Fragment (gravity & tumble)
    PARTICLE_STAR = 3     // Layer 4: Radiant Celebration Starburst
} ParticleType;

typedef struct {
    float x, y;
    float vx, vy;
    float rot, vrot;
    int color; // 1 = Black, 2 = White, 3 = Gold, 4 = Cyan
    int life, maxLife;
    float size;
    ParticleType type;
} AdvancedParticle;

#define MAX_ADVANCED_PARTICLES 512
AdvancedParticle advParticles[MAX_ADVANCED_PARTICLES];
int advParticleCount = 0;

float shakeMagnitude = 0.0f;
float shakeAngle = 0.0f;

void TriggerScreenShake(float mag) {
    if (mag > shakeMagnitude) {
        shakeMagnitude = mag;
    } else {
        shakeMagnitude += mag * 0.4f;
    }
    if (shakeMagnitude > 22.0f) shakeMagnitude = 22.0f;
}

void SpawnCaptureExplosion(int cx, int cy, int stoneColor) {
    // Layer 1: Core incandescent sparks (14 particles)
    for (int p = 0; p < 14 && advParticleCount < MAX_ADVANCED_PARTICLES; p++) {
        AdvancedParticle *pt = &advParticles[advParticleCount++];
        pt->x = (float)cx;
        pt->y = (float)cy;
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 2.5f + (float)(rand() % 45) / 10.0f;
        pt->vx = cosf(angle) * speed;
        pt->vy = sinf(angle) * speed;
        pt->rot = 0.0f;
        pt->vrot = 0.0f;
        pt->color = (stoneColor == 1) ? ((rand() % 2 == 0) ? 4 : 3) : ((rand() % 2 == 0) ? 3 : 2);
        pt->life = 12 + rand() % 10;
        pt->maxLife = pt->life;
        pt->size = 2.0f + (float)(rand() % 3);
        pt->type = PARTICLE_SPARK;
    }
    // Layer 2: Expanding smoke puffs (6 particles)
    for (int p = 0; p < 6 && advParticleCount < MAX_ADVANCED_PARTICLES; p++) {
        AdvancedParticle *pt = &advParticles[advParticleCount++];
        pt->x = (float)cx + (float)(rand() % 9 - 4);
        pt->y = (float)cy + (float)(rand() % 9 - 4);
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.5f + (float)(rand() % 15) / 10.0f;
        pt->vx = cosf(angle) * speed;
        pt->vy = sinf(angle) * speed - 0.8f; // buoyancy
        pt->rot = (float)(rand() % 360);
        pt->vrot = ((float)(rand() % 10) - 5.0f) * 0.02f;
        pt->color = (stoneColor == 1) ? 1 : 2;
        pt->life = 18 + rand() % 12;
        pt->maxLife = pt->life;
        pt->size = 4.0f + (float)(rand() % 4);
        pt->type = PARTICLE_SMOKE;
    }
    // Layer 3: Heavy kinematic stone fragments (8 particles)
    for (int p = 0; p < 8 && advParticleCount < MAX_ADVANCED_PARTICLES; p++) {
        AdvancedParticle *pt = &advParticles[advParticleCount++];
        pt->x = (float)cx;
        pt->y = (float)cy;
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.8f + (float)(rand() % 35) / 10.0f;
        pt->vx = cosf(angle) * speed;
        pt->vy = sinf(angle) * speed - 1.5f; // initial upward kick
        pt->rot = (float)(rand() % 360);
        pt->vrot = ((float)(rand() % 20) - 10.0f) * 0.1f;
        pt->color = stoneColor; // Slate or Clam
        pt->life = 22 + rand() % 12;
        pt->maxLife = pt->life;
        pt->size = 3.0f + (float)(rand() % 3);
        pt->type = PARTICLE_SHARD;
    }
    // Layer 4: Radiant celebration stars (6 particles)
    for (int p = 0; p < 6 && advParticleCount < MAX_ADVANCED_PARTICLES; p++) {
        AdvancedParticle *pt = &advParticles[advParticleCount++];
        pt->x = (float)cx;
        pt->y = (float)cy;
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (float)(rand() % 25) / 10.0f;
        pt->vx = cosf(angle) * speed;
        pt->vy = sinf(angle) * speed;
        pt->rot = 0.0f;
        pt->vrot = 0.05f;
        pt->color = 3; // Golden celebratory star
        pt->life = 25 + rand() % 15;
        pt->maxLife = pt->life;
        pt->size = 4.0f;
        pt->type = PARTICLE_STAR;
    }
}

typedef struct {
    float x, y;
    float vx, vy;
    float size;
} PetalParticle;
#define MAX_PETALS 30
PetalParticle petals[MAX_PETALS];

typedef struct {
    float x, y;
    float vx, vy;
    float size;
} ZenMote;
#define MAX_ZEN_MOTES 25
ZenMote zenMotes[MAX_ZEN_MOTES];

float animTime = 0.0f;

void InitBoard();
int GetLiberties(int x, int y, int color, char visited[19][19]);
void GetGroup(int x, int y, int color, char visited[19][19], POINT group[], int *groupSize);
int EvaluateMoveGrandmaster(int x, int y, int color);

int IsHoshi(int x, int y, int size) {
    if (size == 9) {
        return ((x == 2 || x == 6) && (y == 2 || y == 6)) || (x == 4 && y == 4);
    } else if (size == 13) {
        return ((x == 3 || x == 9) && (y == 3 || y == 9)) || (x == 6 && y == 6);
    } else if (size == 19) {
        return ((x == 3 || x == 9 || x == 15) && (y == 3 || y == 9 || y == 15));
    }
    return 0;
}

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) {
        Beep(300, 80);
    } else if (type == 2) {
        Beep(150, 100);
        Beep(400, 100);
    } else if (type == 3) {
        Beep(440, 150);
        Beep(554, 150);
        Beep(659, 400);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, PlaySoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

void CopyBoard(char dst[19][19], char src[19][19]) {
    memcpy(dst, src, sizeof(char) * 19 * 19);
}

int IsSuperko(char testBoard[19][19]) {
    if (memcmp(testBoard, prevBoard, sizeof(char) * 19 * 19) == 0) return 1;
    for (int i = 0; i < undoCount; i++) {
        if (memcmp(testBoard, undoStack[i].board, sizeof(char) * 19 * 19) == 0) {
            return 1;
        }
    }
    return 0;
}

void PushUndo(char bBackup[19][19], int cBackup[3], int pBackup) {
    if (undoCount < MAX_HISTORY) {
        CopyBoard(undoStack[undoCount].board, bBackup);
        CopyBoard(undoStack[undoCount].prevBoard, prevBoard);
        undoStack[undoCount].currentPlayer = pBackup;
        undoStack[undoCount].captures[1] = cBackup[1];
        undoStack[undoCount].captures[2] = cBackup[2];
        undoCount++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            undoStack[i-1] = undoStack[i];
        }
        CopyBoard(undoStack[MAX_HISTORY-1].board, bBackup);
        CopyBoard(undoStack[MAX_HISTORY-1].prevBoard, prevBoard);
        undoStack[MAX_HISTORY-1].currentPlayer = pBackup;
        undoStack[MAX_HISTORY-1].captures[1] = cBackup[1];
        undoStack[MAX_HISTORY-1].captures[2] = cBackup[2];
    }
    redoCount = 0;
}

void DoUndo(HWND hwnd) {
    int isVsAI = (SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED);
    int stepsToUndo = (isVsAI && undoCount >= 2) ? 2 : 1;
    
    while (stepsToUndo > 0 && undoCount > 0) {
        if (redoCount < MAX_HISTORY) {
            CopyBoard(redoStack[redoCount].board, board);
            CopyBoard(redoStack[redoCount].prevBoard, prevBoard);
            redoStack[redoCount].currentPlayer = currentPlayer;
            redoStack[redoCount].captures[1] = captures[1];
            redoStack[redoCount].captures[2] = captures[2];
            redoCount++;
        }
        undoCount--;
        CopyBoard(board, undoStack[undoCount].board);
        CopyBoard(prevBoard, undoStack[undoCount].prevBoard);
        currentPlayer = undoStack[undoCount].currentPlayer;
        captures[1] = undoStack[undoCount].captures[1];
        captures[2] = undoStack[undoCount].captures[2];
        stepsToUndo--;
    }
    capturedAnimCount = 0;
    hintX = -1; hintY = -1;
    InvalidateRect(hwnd, NULL, TRUE);
}

void DoRedo(HWND hwnd) {
    if (redoCount > 0) {
        if (undoCount < MAX_HISTORY) {
            CopyBoard(undoStack[undoCount].board, board);
            CopyBoard(undoStack[undoCount].prevBoard, prevBoard);
            undoStack[undoCount].currentPlayer = currentPlayer;
            undoStack[undoCount].captures[1] = captures[1];
            undoStack[undoCount].captures[2] = captures[2];
            undoCount++;
        }
        redoCount--;
        CopyBoard(board, redoStack[redoCount].board);
        CopyBoard(prevBoard, redoStack[redoCount].prevBoard);
        currentPlayer = redoStack[redoCount].currentPlayer;
        captures[1] = redoStack[redoCount].captures[1];
        captures[2] = redoStack[redoCount].captures[2];
        capturedAnimCount = 0;
        hintX = -1; hintY = -1;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

int GetLiberties(int x, int y, int color, char visited[19][19]) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return 0;
    if (visited[y][x]) return 0;
    visited[y][x] = 1;
    
    if (board[y][x] == 0) return 1;
    if (board[y][x] != color) return 0;
    
    return GetLiberties(x-1, y, color, visited) +
           GetLiberties(x+1, y, color, visited) +
           GetLiberties(x, y-1, color, visited) +
           GetLiberties(x, y+1, color, visited);
}

void GetGroup(int x, int y, int color, char visited[19][19], POINT group[], int *groupSize) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return;
    if (visited[y][x]) return;
    if (board[y][x] != color) return;
    
    visited[y][x] = 1;
    group[*groupSize].x = x;
    group[*groupSize].y = y;
    (*groupSize)++;
    
    GetGroup(x-1, y, color, visited, group, groupSize);
    GetGroup(x+1, y, color, visited, group, groupSize);
    GetGroup(x, y-1, color, visited, group, groupSize);
    GetGroup(x, y+1, color, visited, group, groupSize);
}

void GetGroupLibertiesList(POINT group[], int groupSize, int color, POINT liberties[], int *libertyCount) {
    *libertyCount = 0;
    char visLib[19][19] = {0};
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    
    for (int i = 0; i < groupSize; i++) {
        for (int d = 0; d < 4; d++) {
            int nx = group[i].x + dx[d];
            int ny = group[i].y + dy[d];
            if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
                if (board[ny][nx] == 0 && !visLib[ny][nx]) {
                    visLib[ny][nx] = 1;
                    liberties[*libertyCount].x = nx;
                    liberties[*libertyCount].y = ny;
                    (*libertyCount)++;
                }
            }
        }
    }
}

int CheckCaptures(int color, int realMove) {
    int totalCaptured = 0;
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == color) {
                char visited[19][19] = {0};
                if (GetLiberties(x, y, color, visited) == 0) {
                    char groupVisited[19][19] = {0};
                    POINT group[19*19];
                    int groupSize = 0;
                    GetGroup(x, y, color, groupVisited, group, &groupSize);
                    for (int i = 0; i < groupSize; i++) {
                        board[group[i].y][group[i].x] = 0;
                        if (realMove) {
                            capturedAnimStones[capturedAnimCount].x = group[i].x;
                            capturedAnimStones[capturedAnimCount].y = group[i].y;
                            capturedAnimColor[capturedAnimCount] = color;
                            capturedAnimCount++;
                        }
                        totalCaptured++;
                    }
                }
            }
        }
    }
    if (totalCaptured > 0) {
        if (color == 1) captures[2] += totalCaptured;
        else captures[1] += totalCaptured;
    }
    return totalCaptured;
}

int IsValidMove(int x, int y, int color, int *outCaptures) {
    if (board[y][x] != 0) return 0;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int opp = color == 1 ? 2 : 1;
    int capCount = CheckCaptures(opp, 0);
    if (outCaptures) *outCaptures = capCount;
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, color, visited) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // suicide
    }
    
    if (IsSuperko(board)) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // ko / superko
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return 1;
}

void ComputeKoOverlay() {
    memset(koMap, 0, sizeof(koMap));
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == 0) {
                int caps = 0;
                if (!IsValidMove(x, y, currentPlayer, &caps)) {
                    koMap[y][x] = 1;
                }
            }
        }
    }
}

int EvaluateMoveTerritorial(int x, int y, int color) {
    int opp = color == 1 ? 2 : 1;
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int caps = CheckCaptures(opp, 0);
    
    char visited[19][19] = {0};
    int myLiberties = GetLiberties(x, y, color, visited);
    
    int score = caps * 40 + myLiberties * 3;
    if (caps == 0 && myLiberties == 1) score -= 500;
    
    int distEdgeX = x < (boardSize - 1 - x) ? x : (boardSize - 1 - x);
    int distEdgeY = y < (boardSize - 1 - y) ? y : (boardSize - 1 - y);
    if ((distEdgeX == 2 || distEdgeX == 3) && (distEdgeY == 2 || distEdgeY == 3)) {
        score += 25;
    }
    
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if (board[ny][nx] == color) {
                score += 15;
                char v2[19][19] = {0};
                if (GetLiberties(nx, ny, color, v2) <= 2) {
                    score += 35;
                }
            }
        }
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return score;
}

int EvaluateMoveInfluence(int x, int y, int color) {
    int opp = color == 1 ? 2 : 1;
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int caps = CheckCaptures(opp, 0);
    
    char visited[19][19] = {0};
    int myLiberties = GetLiberties(x, y, color, visited);
    
    int score = caps * 60 + myLiberties * 2;
    if (caps == 0 && myLiberties == 1) score -= 500;
    
    int cx = boardSize / 2;
    int cy = boardSize / 2;
    int distFromCenter = abs(x - cx) + abs(y - cy);
    score += (boardSize - distFromCenter) * 3;
    
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if (board[ny][nx] == opp) {
                char v2[19][19] = {0};
                if (GetLiberties(nx, ny, opp, v2) <= 2) {
                    score += 30;
                }
            }
        }
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return score;
}

int EvaluateMoveBalanced(int x, int y, int color) {
    int opp = color == 1 ? 2 : 1;
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int caps = CheckCaptures(opp, 0);
    
    char visited[19][19] = {0};
    int myLiberties = GetLiberties(x, y, color, visited);
    
    int score = caps * 50 + myLiberties * 3;
    if (caps == 0 && myLiberties == 1) score -= 500;
    
    int distEdgeX = x < (boardSize - 1 - x) ? x : (boardSize - 1 - x);
    int distEdgeY = y < (boardSize - 1 - y) ? y : (boardSize - 1 - y);
    if (distEdgeX >= 2 && distEdgeX <= 4 && distEdgeY >= 2 && distEdgeY <= 4) {
        score += 20;
    }
    
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if (board[ny][nx] == opp) {
                char v2[19][19] = {0};
                if (GetLiberties(nx, ny, opp, v2) <= 2) {
                    score += 20;
                }
            } else if (board[ny][nx] == color) {
                score += 10;
            }
        }
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return score;
}

int EvaluateMoveGrandmaster(int x, int y, int color) {
    int opp = color == 1 ? 2 : 1;
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    int savingAtari = 0;
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if (board[ny][nx] == color) {
                char v2[19][19] = {0};
                if (GetLiberties(nx, ny, color, v2) == 1) {
                    savingAtari = 1;
                }
            }
        }
    }
    
    board[y][x] = (char)color;
    int caps = CheckCaptures(opp, 0);
    
    int score = caps * 80;
    char visited[19][19] = {0};
    int myLiberties = GetLiberties(x, y, color, visited);
    score += myLiberties * 5;
    if (caps == 0 && myLiberties == 1) score -= 500;
    
    if (savingAtari && myLiberties > 1) {
        score += 60;
    }
    
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if (board[ny][nx] == opp) {
                char v2[19][19] = {0};
                int oppLib = GetLiberties(nx, ny, opp, v2);
                if (oppLib == 1) {
                    score += 50;
                } else if (oppLib == 2) {
                    score += 25;
                }
            }
        }
    }
    
    int cx = boardSize / 2;
    int cy = boardSize / 2;
    int dist = abs(x - cx) + abs(y - cy);
    score += (boardSize - dist) * 1;
    if (IsHoshi(x, y, boardSize)) score += 15;
    
    int distEdgeX = x < (boardSize - 1 - x) ? x : (boardSize - 1 - x);
    int distEdgeY = y < (boardSize - 1 - y) ? y : (boardSize - 1 - y);
    if ((distEdgeX == 2 || distEdgeX == 3) && (distEdgeY == 2 || distEdgeY == 3)) {
        score += 40;
    } else if (distEdgeX == 2 || distEdgeX == 3 || distEdgeY == 2 || distEdgeY == 3) {
        score += 20;
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return score;
}

void CalculateHint() {
    hintX = -1;
    hintY = -1;
    hintScore = -9999;
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            int caps = 0;
            if (IsValidMove(x, y, currentPlayer, &caps)) {
                int score = EvaluateMoveGrandmaster(x, y, currentPlayer);
                if (score > hintScore) {
                    hintScore = score;
                    hintX = x;
                    hintY = y;
                }
            }
        }
    }
}

void ComputeTerritoryAndAtari() {
    memset(territoryMap, 0, sizeof(territoryMap));
    memset(atariMap, 0, sizeof(atariMap));
    
    char visited[19][19] = {0};
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == 0 && !visited[y][x]) {
                int touchesB = 0, touchesW = 0;
                POINT queue[19*19];
                int head = 0, tail = 0;
                queue[tail].x = x; queue[tail].y = y; tail++;
                visited[y][x] = 1;
                
                POINT region[19*19];
                int regionCount = 0;
                
                while (head < tail) {
                    POINT p = queue[head++];
                    region[regionCount++] = p;
                    
                    int dx[] = {0, 1, 0, -1};
                    int dy[] = {1, 0, -1, 0};
                    for (int i = 0; i < 4; i++) {
                        int nx = p.x + dx[i];
                        int ny = p.y + dy[i];
                        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
                            if (board[ny][nx] == 1) touchesB = 1;
                            else if (board[ny][nx] == 2) touchesW = 1;
                            else if (!visited[ny][nx]) {
                                visited[ny][nx] = 1;
                                queue[tail].x = nx; queue[tail].y = ny; tail++;
                            }
                        }
                    }
                }
                
                int owner = 0;
                if (touchesB && !touchesW) owner = 1;
                else if (touchesW && !touchesB) owner = 2;
                
                for (int r = 0; r < regionCount; r++) {
                    territoryMap[region[r].y][region[r].x] = (char)owner;
                }
            }
        }
    }
    
    char groupVisited[19][19] = {0};
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] != 0 && !groupVisited[y][x]) {
                char vLib[19][19] = {0};
                int libCount = GetLiberties(x, y, board[y][x], vLib);
                if (libCount == 1) {
                    POINT grp[19*19];
                    int grpSize = 0;
                    GetGroup(x, y, board[y][x], groupVisited, grp, &grpSize);
                    for (int g = 0; g < grpSize; g++) {
                        atariMap[grp[g].y][grp[g].x] = 1;
                    }
                }
            }
        }
    }
}

void MakeAIMove(HWND hwnd);

void PlaceStone(HWND hwnd, int x, int y) {
    if (SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED && currentPlayer == 2) return;
    if (board[y][x] != 0) return;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)currentPlayer;
    
    int opp = currentPlayer == 1 ? 2 : 1;
    capturedAnimCount = 0;
    int caps = CheckCaptures(opp, 1);
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, currentPlayer, visited) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        TriggerScreenShake(5.0f);
        if (currentPlayer == 1) MessageBox(hwnd, "Suicide move is not allowed.", "Invalid Move", MB_OK);
        return;
    }
    
    if (IsSuperko(board)) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        TriggerScreenShake(5.0f);
        if (currentPlayer == 1) MessageBox(hwnd, "Superko rule violation (recreating previous position).", "Invalid Move", MB_OK);
        return;
    }
    
    PushUndo(boardBackup, capBackup, currentPlayer);
    CopyBoard(prevBoard, boardBackup);
    currentPlayer = opp;
    consecutivePasses = 0;
    hintX = -1; hintY = -1;
    lastMoveX = x;
    lastMoveY = y;
    
    if (caps > 0) {
        PlayGameSound(2);
    } else {
        PlayGameSound(1);
        TriggerScreenShake(2.0f);
    }
    
    animX = x;
    animY = y;
    animRadius = 0;
    rippleRadius = 13;
    SetTimer(hwnd, 1, 16, NULL);
    
    if (caps > 0) {
        captureAnimRadius = 13;
        TriggerScreenShake(6.0f + (float)caps * 3.5f);
        for (int i = 0; i < capturedAnimCount; i++) {
            int cx = 40 + capturedAnimStones[i].x * 30;
            int cy = 40 + capturedAnimStones[i].y * 30;
            SpawnCaptureExplosion(cx, cy, capturedAnimColor[i]);
        }
        SetTimer(hwnd, 3, 16, NULL);
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
    
    if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        SetTimer(hwnd, 2, 500, NULL);
    }
}

void PlaceHandicapStones(int size, int count) {
    if (count <= 0) return;
    int pts9[5][2] = {{6,2}, {2,6}, {6,6}, {2,2}, {4,4}};
    int pts13[9][2] = {{9,3}, {3,9}, {9,9}, {3,3}, {6,6}, {3,6}, {9,6}, {6,3}, {6,9}};
    int pts19[9][2] = {{15,3}, {3,15}, {15,15}, {3,3}, {9,9}, {3,9}, {15,9}, {9,3}, {9,15}};
    int (*pts)[2];
    int maxPts = 0;
    if (size == 9) { pts = pts9; maxPts = 5; }
    else if (size == 13) { pts = pts13; maxPts = 9; }
    else { pts = pts19; maxPts = 9; }
    if (count > maxPts) count = maxPts;
    for(int i=0; i<count; i++) {
        board[pts[i][1]][pts[i][0]] = 1; 
    }
    currentPlayer = 2; 
    CopyBoard(prevBoard, board);
}

void StartCampaignStage(HWND hwnd) {
    if (currentCampaignStage >= 20 || currentCampaignStage < 0) return;
    boardSize = campaign[currentCampaignStage].size;
    int handicap = campaign[currentCampaignStage].handicap;
    currentKomi = campaign[currentCampaignStage].komi;
    int diff = campaign[currentCampaignStage].aiPersonality;
    
    SendMessage(GetDlgItem(hwnd, ID_CB_SIZE), CB_SETCURSEL, boardSize==9?0:(boardSize==13?1:2), 0);
    SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, ID_CB_DIFFICULTY), CB_SETCURSEL, diff, 0);
    
    InitBoard();
    
    if (campaign[currentCampaignStage].isTsumego) {
        for (int i = 0; i < campaign[currentCampaignStage].stoneCount; i++) {
            StonePos sp = campaign[currentCampaignStage].initialStones[i];
            board[sp.y][sp.x] = (char)sp.color;
        }
        CopyBoard(prevBoard, board);
    } else {
        PlaceHandicapStones(boardSize, handicap);
    }
    
    hintX = -1; hintY = -1;
    InvalidateRect(hwnd, NULL, TRUE);
    
    const char *pName[] = {"Territorial", "Influence", "Balanced", "Grandmaster"};
    char msg[384];
    sprintf(msg, "%s\nBoard: %dx%d | Handicap: %d | Komi: %.1f\nAI Personality: %s\n\n%s", 
        campaign[currentCampaignStage].name, boardSize, boardSize, handicap, currentKomi,
        pName[diff],
        campaign[currentCampaignStage].isTsumego ? campaign[currentCampaignStage].targetDesc : "Goal: Surround more territory than White!");
    MessageBox(hwnd, msg, "Campaign Stage", MB_OK);
    
    if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        SetTimer(hwnd, 2, 500, NULL);
    }
}

void LoadTsumegoPuzzle(HWND hwnd, int index) {
    int puzzleStages[] = {2, 6, 10, 13, 15}; // Indices 0..4 mapped to stage 3, 7, 11, 14, 16
    if (index < 0 || index >= 5) index = 0;
    currentTsumegoIndex = index;
    currentCampaignStage = puzzleStages[index];
    StartCampaignStage(hwnd);
}

void MakeAIMove(HWND hwnd) {
    if (currentPlayer != 2) return;
    
    int personality = 1;
    if (currentCampaignStage >= 0 && currentCampaignStage < 20) {
        personality = campaign[currentCampaignStage].aiPersonality;
    } else {
        personality = SendMessage(GetDlgItem(hwnd, ID_CB_DIFFICULTY), CB_GETCURSEL, 0, 0);
    }
    
    POINT validMoves[19*19];
    int validCount = 0;
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            int caps = 0;
            if (IsValidMove(x, y, 2, &caps)) {
                validMoves[validCount].x = x;
                validMoves[validCount].y = y;
                validCount++;
            }
        }
    }
    
    if (validCount > 0) {
        int bestScore = -9999;
        int bestMoves[19*19];
        int bestCount = 0;
        for (int i = 0; i < validCount; i++) {
            int score = 0;
            if (personality == 0) {
                score = EvaluateMoveTerritorial(validMoves[i].x, validMoves[i].y, 2);
            } else if (personality == 1) {
                score = EvaluateMoveInfluence(validMoves[i].x, validMoves[i].y, 2);
            } else if (personality == 2) {
                score = EvaluateMoveBalanced(validMoves[i].x, validMoves[i].y, 2);
            } else {
                score = EvaluateMoveGrandmaster(validMoves[i].x, validMoves[i].y, 2);
            }
            if (score > bestScore) {
                bestScore = score;
                bestCount = 0;
                bestMoves[bestCount++] = i;
            } else if (score == bestScore) {
                bestMoves[bestCount++] = i;
            }
        }
        if (bestCount > 0) {
            int r = bestMoves[rand() % bestCount];
            PlaceStone(hwnd, validMoves[r].x, validMoves[r].y);
        } else {
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PASS, 0), 0);
        }
    } else {
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PASS, 0), 0);
    }
}

void SaveGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "wb");
    if (f) {
        fwrite(&boardSize, sizeof(int), 1, f);
        fwrite(board, sizeof(char), 19*19, f);
        fwrite(prevBoard, sizeof(char), 19*19, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(captures, sizeof(int), 3, f);
        fwrite(&undoCount, sizeof(int), 1, f);
        fwrite(undoStack, sizeof(GameState), undoCount, f);
        fwrite(&redoCount, sizeof(int), 1, f);
        fwrite(redoStack, sizeof(GameState), redoCount, f);
        fwrite(&currentKomi, sizeof(float), 1, f);
        fclose(f);
        MessageBox(hwnd, "Game saved.", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK);
    }
}

void LoadGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "rb");
    if (f) {
        fread(&boardSize, sizeof(int), 1, f);
        fread(board, sizeof(char), 19*19, f);
        fread(prevBoard, sizeof(char), 19*19, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(captures, sizeof(int), 3, f);
        fread(&undoCount, sizeof(int), 1, f);
        fread(undoStack, sizeof(GameState), undoCount, f);
        fread(&redoCount, sizeof(int), 1, f);
        fread(redoStack, sizeof(GameState), redoCount, f);
        fread(&currentKomi, sizeof(float), 1, f);
        fclose(f);
        
        int sel = 0;
        if (boardSize == 13) sel = 1;
        else if (boardSize == 19) sel = 2;
        SendMessage(GetDlgItem(hwnd, ID_CB_SIZE), CB_SETCURSEL, sel, 0);
        
        hintX = -1; hintY = -1;
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Game loaded.", "Load", MB_OK);
        
        if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
            SetTimer(hwnd, 2, 500, NULL);
        }
    } else {
        MessageBox(hwnd, "No saved game found.", "Error", MB_OK);
    }
}

void InitBoard() {
    consecutivePasses = 0;
    memset(board, 0, sizeof(board));
    memset(prevBoard, 0, sizeof(prevBoard));
    currentPlayer = 1;
    captures[1] = 0;
    captures[2] = 0;
    animX = -1;
    animY = -1;
    rippleRadius = 0;
    capturedAnimCount = 0;
    undoCount = 0;
    redoCount = 0;
    hintX = -1;
    hintY = -1;
    lastMoveX = -1;
    lastMoveY = -1;
    advParticleCount = 0;
    shakeMagnitude = 0.0f;
}

void CalculateScore(HWND hwnd) {
    TriggerScreenShake(12.0f);
    int terrB = 0;
    int terrW = 0;
    char visited[19][19] = {0};
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == 0 && !visited[y][x]) {
                int touchesB = 0;
                int touchesW = 0;
                
                POINT queue[19*19];
                int head = 0, tail = 0;
                queue[tail].x = x;
                queue[tail].y = y;
                tail++;
                visited[y][x] = 1;
                
                int emptyCount = 0;
                
                while (head < tail) {
                    POINT p = queue[head++];
                    emptyCount++;
                    
                    int dx[] = {0, 1, 0, -1};
                    int dy[] = {1, 0, -1, 0};
                    for (int i = 0; i < 4; i++) {
                        int nx = p.x + dx[i];
                        int ny = p.y + dy[i];
                        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
                            if (board[ny][nx] == 1) touchesB = 1;
                            else if (board[ny][nx] == 2) touchesW = 1;
                            else if (!visited[ny][nx]) {
                                visited[ny][nx] = 1;
                                queue[tail].x = nx;
                                queue[tail].y = ny;
                                tail++;
                            }
                        }
                    }
                }
                
                if (touchesB && !touchesW) terrB += emptyCount;
                if (touchesW && !touchesB) terrW += emptyCount;
            }
        }
    }
    
    float totalB = captures[1] + terrB;
    float totalW = captures[2] + terrW + currentKomi;
    
    int winner = (totalB > totalW) ? 1 : 2;
    RecordGameEnd(winner, hwnd);

    int pad = 40;
    int cSize = 30;
    for (int i = 0; i < 8; i++) {
        int rx = pad + (rand() % boardSize) * cSize;
        int ry = pad + (rand() % boardSize) * cSize;
        SpawnCaptureExplosion(rx, ry, winner);
    }
    SetTimer(hwnd, 3, 16, NULL);

    char msg[512];
    if (currentCampaignStage != -1) {
        if (winner == 1) {
            currentCampaignStage++;
            if (currentCampaignStage >= 20) {
                sprintf(msg, "Black: %.1f vs White: %.1f\n\nBlack wins!\n\nCONGRATULATIONS! You completed all 20 Campaign Stages and conquered the Stage 20 Grandmaster Go Legend Challenge!", totalB, totalW);
                currentCampaignStage = -1;
            } else {
                sprintf(msg, "Black: %.1f vs White: %.1f\n\nBlack wins!\n\nAdvancing to Campaign Stage %d!", totalB, totalW, currentCampaignStage + 1);
            }
        } else {
            sprintf(msg, "Black: %.1f vs White: %.1f\n\nWhite wins!\n\nYou failed Campaign Stage %d (%s). Try again.", totalB, totalW, currentCampaignStage + 1, campaign[currentCampaignStage].name);
        }
        PlayGameSound(3);
        MessageBox(hwnd, msg, "Game Score", MB_OK);
        if (currentCampaignStage != -1) {
            StartCampaignStage(hwnd);
        } else {
            InitBoard();
            InvalidateRect(hwnd, NULL, TRUE);
        }
    } else {
        sprintf(msg, "Black: %d territory + %d captures = %.1f\n"
                     "White: %d territory + %d captures + %.1f komi = %.1f\n\n"
                     "%s wins!", 
                     terrB, captures[1], totalB,
                     terrW, captures[2], currentKomi, totalW,
                     (totalB > totalW) ? "Black" : "White");
        PlayGameSound(3);
        MessageBox(hwnd, msg, "Game Score", MB_OK);
        InitBoard();
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnPass, hBtnResign, hBtnNew, hCbSize, hCbAi, hCbDifficulty;

    switch (uMsg) {
        case WM_CREATE:
            for (int i = 0; i < MAX_PETALS; i++) {
                petals[i].x = rand() % 800;
                petals[i].y = rand() % 800;
                petals[i].vx = (rand() % 20 - 10) / 10.0f;
                petals[i].vy = (rand() % 20 + 20) / 10.0f;
                petals[i].size = (rand() % 5) + 3;
            }
            for (int i = 0; i < MAX_ZEN_MOTES; i++) {
                zenMotes[i].x = (float)(rand() % 800);
                zenMotes[i].y = (float)(rand() % 800);
                zenMotes[i].vx = ((rand() % 16) - 8) / 10.0f;
                zenMotes[i].vy = -((rand() % 15) + 5) / 10.0f;
                zenMotes[i].size = (float)((rand() % 3) + 2);
            }
            SetTimer(hwnd, 4, 50, NULL);
            hBtnPass = CreateWindow("BUTTON", "Pass", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                15, 620, 45, 30, hwnd, (HMENU)ID_BTN_PASS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnResign = CreateWindow("BUTTON", "Resign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                65, 620, 50, 30, hwnd, (HMENU)ID_BTN_RESIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Score", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                120, 620, 45, 30, hwnd, (HMENU)ID_BTN_SCORE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnNew = CreateWindow("BUTTON", "New", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                170, 620, 40, 30, hwnd, (HMENU)ID_BTN_NEW, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hCbSize = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                215, 620, 70, 100, hwnd, (HMENU)ID_CB_SIZE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"9x9");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"13x13");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"19x19");
            SendMessage(hCbSize, CB_SETCURSEL, 0, 0);

            hCbAi = CreateWindow("BUTTON", "VS AI (White)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                290, 620, 100, 30, hwnd, (HMENU)ID_CB_AI, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbAi, BM_SETCHECK, BST_CHECKED, 0);

            hCbDifficulty = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                395, 620, 115, 120, hwnd, (HMENU)ID_CB_DIFFICULTY, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Territorial");
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Influence");
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Balanced");
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Grandmaster");
            SendMessage(hCbDifficulty, CB_SETCURSEL, 3, 0);

            CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                15, 660, 45, 30, hwnd, (HMENU)ID_BTN_SAVE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                65, 660, 45, 30, hwnd, (HMENU)ID_BTN_LOAD, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Undo(U)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                115, 660, 55, 30, hwnd, (HMENU)ID_BTN_UNDO, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Hint(H)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                175, 660, 55, 30, hwnd, (HMENU)ID_BTN_HINT, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Est(T)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                235, 660, 45, 30, hwnd, (HMENU)ID_BTN_ESTIMATE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Analyzer(S)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                285, 660, 75, 30, hwnd, (HMENU)ID_BTN_ANALYZER, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Campaign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                365, 660, 65, 30, hwnd, (HMENU)ID_BTN_CAMPAIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Tsumego", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                435, 660, 60, 30, hwnd, (HMENU)ID_BTN_TSUMEGO, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Stats", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                500, 660, 45, 30, hwnd, (HMENU)ID_BTN_STATS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                550, 660, 45, 30, hwnd, (HMENU)ID_BTN_HELP, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                if (animRadius < 13) animRadius += 2;
                if (animRadius > 13) animRadius = 13;
                
                if (rippleRadius > 0) {
                    rippleRadius += 2;
                    if (rippleRadius > 44) rippleRadius = 0;
                }
                
                if (animRadius == 13 && rippleRadius == 0) {
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 3) {
                captureAnimRadius -= 2;
                int newCount = 0;
                for (int i = 0; i < advParticleCount; i++) {
                    advParticles[i].x += advParticles[i].vx;
                    advParticles[i].y += advParticles[i].vy;
                    advParticles[i].rot += advParticles[i].vrot;

                    if (advParticles[i].type == PARTICLE_SPARK) {
                        advParticles[i].vx *= 0.93f;
                        advParticles[i].vy *= 0.93f;
                    } else if (advParticles[i].type == PARTICLE_SMOKE) {
                        advParticles[i].vy -= 0.12f;
                        advParticles[i].size += 0.35f;
                    } else if (advParticles[i].type == PARTICLE_SHARD) {
                        advParticles[i].vy += 0.38f;
                        advParticles[i].vx *= 0.97f;
                        if (advParticles[i].y > 600.0f) {
                            advParticles[i].y = 600.0f;
                            advParticles[i].vy = -advParticles[i].vy * 0.45f;
                        }
                    } else if (advParticles[i].type == PARTICLE_STAR) {
                        advParticles[i].vx += sinf(animTime * 4.0f) * 0.15f;
                    }

                    advParticles[i].life--;
                    if (advParticles[i].life > 0) {
                        advParticles[newCount++] = advParticles[i];
                    }
                }
                advParticleCount = newCount;

                if (shakeMagnitude > 0.05f) {
                    shakeAngle += 2.3f;
                    shakeMagnitude *= 0.85f;
                    if (shakeMagnitude < 0.05f) shakeMagnitude = 0.0f;
                }

                if (captureAnimRadius <= 0 && advParticleCount <= 0 && shakeMagnitude <= 0.05f) {
                    captureAnimRadius = 0;
                    capturedAnimCount = 0;
                    shakeMagnitude = 0.0f;
                    KillTimer(hwnd, 3);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 4) {
                animTime += 0.1f;
                if (shakeMagnitude > 0.05f) {
                    shakeAngle += 2.3f;
                    shakeMagnitude *= 0.85f;
                    if (shakeMagnitude < 0.05f) shakeMagnitude = 0.0f;
                }
                for (int i = 0; i < MAX_PETALS; i++) {
                    petals[i].x += petals[i].vx;
                    petals[i].y += petals[i].vy;
                    petals[i].vx += (rand() % 5 - 2) / 10.0f;
                    if (petals[i].vx > 2.0f) petals[i].vx = 2.0f;
                    if (petals[i].vx < -2.0f) petals[i].vx = -2.0f;
                    if (petals[i].y > 800) {
                        petals[i].y = -10;
                        petals[i].x = rand() % 800;
                    }
                }
                for (int i = 0; i < MAX_ZEN_MOTES; i++) {
                    zenMotes[i].x += zenMotes[i].vx;
                    zenMotes[i].y += zenMotes[i].vy;
                    zenMotes[i].vx += ((rand() % 5) - 2) / 20.0f;
                    if (zenMotes[i].vx > 1.2f) zenMotes[i].vx = 1.2f;
                    if (zenMotes[i].vx < -1.2f) zenMotes[i].vx = -1.2f;
                    if (zenMotes[i].y < -10) {
                        zenMotes[i].y = 810;
                        zenMotes[i].x = (float)(rand() % 800);
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                KillTimer(hwnd, 2);
                MakeAIMove(hwnd);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == 'U' || wParam == 'u') {
                DoUndo(hwnd);
            } else if (wParam == 'H' || wParam == 'h') {
                CalculateHint();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'T' || wParam == 't') {
                showEstimator = !showEstimator;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'S' || wParam == 's') {
                showAnalyzer = !showAnalyzer;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            if (x >= padding - cellSize/2 && x <= padding + (boardSize-1)*cellSize + cellSize/2 &&
                y >= padding - cellSize/2 && y <= padding + (boardSize-1)*cellSize + cellSize/2) {
                int col = (x - padding + cellSize / 2) / cellSize;
                int row = (y - padding + cellSize / 2) / cellSize;
                if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                    if (hoverX != col || hoverY != row) {
                        hoverX = col;
                        hoverY = row;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else {
                if (hoverX != -1 || hoverY != -1) {
                    hoverX = -1; hoverY = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            int col = (x - padding + cellSize / 2) / cellSize;
            int row = (y - padding + cellSize / 2) / cellSize;
            
            if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                PlaceStone(hwnd, col, row);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int winW = clientRect.right - clientRect.left;
            int winH = clientRect.bottom - clientRect.top;
            if (winW < 800) winW = 800;
            if (winH < 720) winH = 720;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, winW, winH);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
            HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);

            RECT fullRect = { 0, 0, winW, winH };

            // Tatami mat texture background floor
            HBRUSH tatamiBrush = CreateSolidBrush(RGB(158, 148, 107));
            FillRect(memDC, &fullRect, tatamiBrush);
            DeleteObject(tatamiBrush);
            HPEN tatamiPen = CreatePen(PS_SOLID, 1, RGB(140, 130, 90));
            HPEN oldT_Pen = SelectObject(memDC, tatamiPen);
            for (int i = 0; i < winW; i += 20) {
                MoveToEx(memDC, i, 0, NULL);
                LineTo(memDC, i, winH);
            }
            for (int i = 0; i < winH; i += 20) {
                MoveToEx(memDC, 0, i, NULL);
                LineTo(memDC, winW, i);
            }
            SelectObject(memDC, oldT_Pen);
            DeleteObject(tatamiPen);

            // Ambient lantern lighting glow
            for (int i = 0; i < 8; i++) {
                HBRUSH glowBrush = CreateSolidBrush(RGB(158 + i * 10, 148 + i * 5, 107));
                SelectObject(memDC, glowBrush);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                Ellipse(memDC, 400 - (8-i)*60, 350 - (8-i)*60, 400 + (8-i)*60, 350 + (8-i)*60);
                DeleteObject(glowBrush);
            }

            int padding = 40;
            int cellSize = 30;
            int boardW = (boardSize - 1) * cellSize;
            
            ComputeKoOverlay();

            // Outer shadow
            HBRUSH shadowBrush = CreateSolidBrush(RGB(10, 10, 10));
            RECT shadowRect = { padding - 15 + 16, padding - 15 + 24, padding + boardW + 15 + 24, padding + boardW + 15 + 32 };
            FillRect(memDC, &shadowRect, shadowBrush);
            DeleteObject(shadowBrush);

            // 3D Procedural wood grain ring layers visible on the edges of the 3D Kaya wood board
            for (int edge = 14; edge >= 1; edge--) {
                int cR = 92 - edge * 3; if (cR < 0) cR = 0;
                int cG = 58 - edge * 3; if (cG < 0) cG = 0;
                int cB = 33 - edge * 2; if (cB < 0) cB = 0;
                if (edge % 3 == 0) { cR = max(0, cR - 15); cG = max(0, cG - 15); cB = max(0, cB - 10); } // procedural grain rings
                HBRUSH edgeBrush = CreateSolidBrush(RGB(cR, cG, cB));
                RECT edgeRect = { padding - 15 + edge, padding - 15 + edge, padding + boardW + 15 + edge, padding + boardW + 15 + edge };
                FillRect(memDC, &edgeRect, edgeBrush);
                DeleteObject(edgeBrush);
            }

            // Outer frame mahogany
            HBRUSH frameBrush = CreateSolidBrush(RGB(74, 46, 20));
            RECT frameRect = { padding - 15, padding - 15, padding + boardW + 15, padding + boardW + 15 };
            FillRect(memDC, &frameRect, frameBrush);
            DeleteObject(frameBrush);

            // Inner Kaya board surface
            HBRUSH boardBrush = CreateSolidBrush(RGB(220, 179, 92));
            RECT boardRect = { padding - 8, padding - 8, padding + boardW + 8, padding + boardW + 8 };
            FillRect(memDC, &boardRect, boardBrush);
            DeleteObject(boardBrush);

            // Fine wood grain texturing
            HPEN grainPen = CreatePen(PS_SOLID, 1, RGB(205, 160, 78));
            HPEN oldPen = SelectObject(memDC, grainPen);
            for (int g = 0; g < 18; g++) {
                int gy = padding - 6 + g * (boardW + 12) / 17;
                MoveToEx(memDC, padding - 6, gy, NULL);
                LineTo(memDC, padding + boardW + 6, gy + (g % 2 == 0 ? 2 : -2));
            }
            SelectObject(memDC, oldPen);
            DeleteObject(grainPen);

            // Ornate Japanese Lacquer & Gold Leaf Corner Filigree Brackets
            int bracketSize = 28;
            for (int cIdx = 0; cIdx < 4; cIdx++) {
                int bx = (cIdx % 2 == 0) ? (padding - 15) : (padding + boardW + 15);
                int by = (cIdx < 2) ? (padding - 15) : (padding + boardW + 15);
                int dirX = (cIdx % 2 == 0) ? 1 : -1;
                int dirY = (cIdx < 2) ? 1 : -1;

                // Dark lacquer base plate
                HBRUSH bBase = CreateSolidBrush(RGB(18, 14, 10));
                SelectObject(memDC, bBase);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                POINT bPoly[6] = {
                    {bx, by},
                    {bx + dirX * bracketSize, by},
                    {bx + dirX * bracketSize, by + dirY * 9},
                    {bx + dirX * 9, by + dirY * 9},
                    {bx + dirX * 9, by + dirY * bracketSize},
                    {bx, by + dirY * bracketSize}
                };
                Polygon(memDC, bPoly, 6);
                DeleteObject(bBase);

                // Polished gold leaf border
                HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
                HPEN oldP = SelectObject(memDC, goldPen);
                MoveToEx(memDC, bx + dirX * bracketSize, by, NULL);
                LineTo(memDC, bx, by);
                LineTo(memDC, bx, by + dirY * bracketSize);
                SelectObject(memDC, oldP);
                DeleteObject(goldPen);

                // Gold scroll filigree accent
                HPEN filigreePen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
                oldP = SelectObject(memDC, filigreePen);
                MoveToEx(memDC, bx + dirX * (bracketSize - 3), by + dirY * 4, NULL);
                LineTo(memDC, bx + dirX * 4, by + dirY * 4);
                LineTo(memDC, bx + dirX * 4, by + dirY * (bracketSize - 3));
                SelectObject(memDC, oldP);
                DeleteObject(filigreePen);

                // Brass corner rivet / stud
                HBRUSH studBrush = CreateSolidBrush(RGB(255, 235, 140));
                SelectObject(memDC, studBrush);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                Ellipse(memDC, bx + dirX * 6 - 3, by + dirY * 6 - 3, bx + dirX * 6 + 3, by + dirY * 6 + 3);
                DeleteObject(studBrush);
            }

            // Pulsing golden inlay perimeter line around the Kaya board
            float goldPulse = (sinf(animTime * 2.5f) + 1.0f) * 0.5f;
            int gR = 190 + (int)(goldPulse * 65.0f);
            int gG = 150 + (int)(goldPulse * 75.0f);
            int gB = 30 + (int)(goldPulse * 50.0f);
            HPEN inlayPen = CreatePen(PS_SOLID, 1, RGB(gR, gG, gB));
            HPEN oldPenInlay = SelectObject(memDC, inlayPen);
            HBRUSH hollowB = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
            HBRUSH oldBInlay = SelectObject(memDC, hollowB);
            Rectangle(memDC, padding - 7, padding - 7, padding + boardW + 7, padding + boardW + 7);
            SelectObject(memDC, oldPenInlay);
            SelectObject(memDC, oldBInlay);
            DeleteObject(inlayPen);

            // Traveling specular glint traversing the outer playfield Kaya board frame
            float periT = fmodf(animTime * 0.6f, 1.0f);
            float totalPeri = (float)((boardW + 30) * 4);
            float curDist = periT * totalPeri;
            int glintX = padding - 15;
            int glintY = padding - 15;
            int sideLen = boardW + 30;
            if (curDist < sideLen) {
                glintX = (padding - 15) + (int)curDist;
                glintY = padding - 15;
            } else if (curDist < sideLen * 2) {
                glintX = padding + boardW + 15;
                glintY = (padding - 15) + (int)(curDist - sideLen);
            } else if (curDist < sideLen * 3) {
                glintX = (padding + boardW + 15) - (int)(curDist - sideLen * 2);
                glintY = padding + boardW + 15;
            } else {
                glintX = padding - 15;
                glintY = (padding + boardW + 15) - (int)(curDist - sideLen * 3);
            }
            HBRUSH glintBrush1 = CreateSolidBrush(RGB(255, 255, 255));
            HBRUSH glintBrush2 = CreateSolidBrush(RGB(255, 225, 120));
            SelectObject(memDC, nullPen);
            SelectObject(memDC, glintBrush2);
            Ellipse(memDC, glintX - 7, glintY - 7, glintX + 7, glintY + 7);
            SelectObject(memDC, glintBrush1);
            Ellipse(memDC, glintX - 3, glintY - 3, glintX + 3, glintY + 3);
            DeleteObject(glintBrush1);
            DeleteObject(glintBrush2);

            // Grid lines
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(30, 22, 14));
            SelectObject(memDC, hPen);
            for (int i = 0; i < boardSize; i++) {
                MoveToEx(memDC, padding, padding + i * cellSize, NULL);
                LineTo(memDC, padding + boardW, padding + i * cellSize);
                MoveToEx(memDC, padding + i * cellSize, padding, NULL);
                LineTo(memDC, padding + i * cellSize, padding + boardW);
            }
            DeleteObject(hPen);

            // Star points (Hoshi dots)
            HBRUSH hoshiBrush = CreateSolidBrush(RGB(25, 18, 10));
            for (int r = 0; r < boardSize; r++) {
                for (int c = 0; c < boardSize; c++) {
                    if (IsHoshi(c, r, boardSize)) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        HBRUSH oldB = SelectObject(memDC, hoshiBrush);
                        HPEN oldP = SelectObject(memDC, nullPen);
                        Ellipse(memDC, cx - 3, cy - 3, cx + 3, cy + 3);
                        SelectObject(memDC, oldB);
                        SelectObject(memDC, oldP);
                    }
                }
            }
            DeleteObject(hoshiBrush);
            
            // Ko & Superko forbidden overlay indicators
            for (int r = 0; r < boardSize; r++) {
                for (int c = 0; c < boardSize; c++) {
                    if (koMap[r][c] && board[r][c] == 0) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        HPEN koPen = CreatePen(PS_DOT, 1, RGB(255, 50, 50));
                        HPEN oldP = SelectObject(memDC, koPen);
                        HBRUSH oldB = SelectObject(memDC, hollowB);
                        Ellipse(memDC, cx - 8, cy - 8, cx + 8, cy + 8);
                        MoveToEx(memDC, cx - 4, cy - 4, NULL);
                        LineTo(memDC, cx + 4, cy + 4);
                        MoveToEx(memDC, cx + 4, cy - 4, NULL);
                        LineTo(memDC, cx - 4, cy + 4);
                        SelectObject(memDC, oldP);
                        SelectObject(memDC, oldB);
                        DeleteObject(koPen);
                    }
                }
            }

            // Territory estimation visualizer
            if (showEstimator) {
                ComputeTerritoryAndAtari();
                float pulse = (sinf(animTime) + 1.0f) * 0.5f;
                int pOff = (int)(pulse * 3.0f);
                for (int r = 0; r < boardSize; r++) {
                    for (int c = 0; c < boardSize; c++) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        if (board[r][c] == 0) {
                            if (territoryMap[r][c] == 1) { // Black territory
                                HBRUSH tBrush = CreateSolidBrush(RGB(0, 150 - pOff*10, 255));
                                HBRUSH tInner = CreateSolidBrush(RGB(180, 230, 255));
                                SelectObject(memDC, nullPen);
                                SelectObject(memDC, tBrush);
                                Ellipse(memDC, cx - (6 + pOff), cy - (6 + pOff), cx + (6 + pOff), cy + (6 + pOff));
                                SelectObject(memDC, tInner);
                                Ellipse(memDC, cx - 2, cy - 2, cx + 2, cy + 2);
                                DeleteObject(tBrush);
                                DeleteObject(tInner);
                            } else if (territoryMap[r][c] == 2) { // White territory
                                HBRUSH tBrush = CreateSolidBrush(RGB(255, 60 - pOff*10, 60 - pOff*10));
                                HBRUSH tInner = CreateSolidBrush(RGB(255, 200, 200));
                                SelectObject(memDC, nullPen);
                                SelectObject(memDC, tBrush);
                                Ellipse(memDC, cx - (6 + pOff), cy - (6 + pOff), cx + (6 + pOff), cy + (6 + pOff));
                                SelectObject(memDC, tInner);
                                Ellipse(memDC, cx - 2, cy - 2, cx + 2, cy + 2);
                                DeleteObject(tBrush);
                                DeleteObject(tInner);
                            }
                        }
                    }
                }
            }

            // Group Liberty Analyzer highlighting
            int targetX = -1, targetY = -1;
            if (hoverX >= 0 && hoverY >= 0 && board[hoverY][hoverX] != 0) {
                targetX = hoverX; targetY = hoverY;
            } else if (lastMoveX >= 0 && lastMoveY >= 0 && board[lastMoveY][lastMoveX] != 0) {
                targetX = lastMoveX; targetY = lastMoveY;
            }
            
            char analyzerStatusStr[128] = "";
            if (showAnalyzer && targetX >= 0 && targetY >= 0 && board[targetY][targetX] != 0) {
                int targetColor = board[targetY][targetX];
                char visGrp[19][19] = {0};
                POINT grpStones[19*19];
                int grpSize = 0;
                GetGroup(targetX, targetY, targetColor, visGrp, grpStones, &grpSize);
                
                POINT liberties[19*19];
                int libCount = 0;
                GetGroupLibertiesList(grpStones, grpSize, targetColor, liberties, &libCount);
                
                // Highlight group stones
                HPEN grpPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 200));
                HPEN oldP = SelectObject(memDC, grpPen);
                HBRUSH oldB = SelectObject(memDC, hollowB);
                for (int i = 0; i < grpSize; i++) {
                    int cx = padding + grpStones[i].x * cellSize;
                    int cy = padding + grpStones[i].y * cellSize;
                    Ellipse(memDC, cx - 14, cy - 14, cx + 14, cy + 14);
                }
                SelectObject(memDC, oldP);
                SelectObject(memDC, oldB);
                DeleteObject(grpPen);
                
                // Render green numbered liberty indicators
                HBRUSH libBrush = CreateSolidBrush(RGB(40, 200, 80));
                HFONT fontSmall = CreateFont(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
                HFONT oldFont = SelectObject(memDC, fontSmall);
                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, RGB(255, 255, 255));
                for (int l = 0; l < libCount; l++) {
                    int cx = padding + liberties[l].x * cellSize;
                    int cy = padding + liberties[l].y * cellSize;
                    SelectObject(memDC, nullPen);
                    SelectObject(memDC, libBrush);
                    Ellipse(memDC, cx - 7, cy - 7, cx + 7, cy + 7);
                    char numBuf[8];
                    sprintf(numBuf, "%d", l + 1);
                    RECT txtRect = {cx - 7, cy - 7, cx + 7, cy + 7};
                    DrawText(memDC, numBuf, -1, &txtRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                SelectObject(memDC, oldFont);
                DeleteObject(fontSmall);
                DeleteObject(libBrush);
                
                sprintf(analyzerStatusStr, "Group Analyzer: %s group | Size: %d stones | Liberties: %d (%s)",
                    targetColor == 1 ? "Black" : "White", grpSize, libCount,
                    libCount == 1 ? "ATARI DANGER!" : (libCount == 2 ? "Vulnerable" : "Safe"));
            }

            // Draw 3D Slate & Shell stones
            for (int r = 0; r < boardSize; r++) {
                for (int c = 0; c < boardSize; c++) {
                    if (board[r][c] != 0) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        int radius = (r == animY && c == animX) ? animRadius : 13;
                        
                        // Dynamic 3D drop shadow based on board position
                        int shX = (int)((c - (boardSize-1)/2.0f) * 0.4f) + 2;
                        int shY = (int)((r - (boardSize-1)/2.0f) * 0.4f) + 3;

                        HBRUSH shBrush = CreateSolidBrush(RGB(40, 28, 16));
                        SelectObject(memDC, nullPen);
                        SelectObject(memDC, shBrush);
                        Ellipse(memDC, cx - radius + shX, cy - radius + shY, cx + radius + shX, cy + radius + shY);
                        DeleteObject(shBrush);

                        if (board[r][c] == 1) { // 3D Black Slate Stone (Obsidian sheen)
                            HBRUSH slateBrush = CreateSolidBrush(RGB(24, 27, 34));
                            HPEN slatePen = CreatePen(PS_SOLID, 1, RGB(10, 10, 12));
                            HBRUSH oldB = SelectObject(memDC, slateBrush);
                            HPEN oldP = SelectObject(memDC, slatePen);
                            Ellipse(memDC, cx - radius, cy - radius, cx + radius, cy + radius);
                            SelectObject(memDC, oldB);
                            SelectObject(memDC, oldP);
                            DeleteObject(slateBrush);
                            DeleteObject(slatePen);

                            // Micro-texture (slate grain)
                            int pseudoRand = (r * 13 + c * 17) % 360;
                            float radT = pseudoRand * 3.14159f / 180.0f;
                            float cosR = cosf(radT);
                            float sinR = sinf(radT);
                            HPEN grainP = CreatePen(PS_SOLID, 1, RGB(35, 38, 45));
                            SelectObject(memDC, grainP);
                            for(int g=0; g<5; g++) {
                                float sx = -radius + 4 + g*3;
                                float sy = -radius + 4;
                                float ex = sx + 2;
                                float ey = radius - 4;
                                MoveToEx(memDC, cx + (int)(sx*cosR - sy*sinR), cy + (int)(sx*sinR + sy*cosR), NULL);
                                LineTo(memDC, cx + (int)(ex*cosR - ey*sinR), cy + (int)(ex*sinR + ey*cosR));
                            }
                            DeleteObject(grainP);

                            // Specular obsidian crescent highlight
                            HPEN crescentPen = CreatePen(PS_SOLID, 1, RGB(115, 130, 150));
                            SelectObject(memDC, crescentPen);
                            Arc(memDC, cx - radius + 3, cy - radius + 3, cx + radius - 3, cy + radius - 3, cx - radius + 3, cy, cx, cy - radius + 3);
                            DeleteObject(crescentPen);

                            // Specular point sheen highlight
                            HBRUSH sheenBrush = CreateSolidBrush(RGB(100, 110, 125));
                            SelectObject(memDC, nullPen);
                            SelectObject(memDC, sheenBrush);
                            Ellipse(memDC, cx - 7, cy - 7, cx - 2, cy - 2);
                            HBRUSH sheenBrush2 = CreateSolidBrush(RGB(220, 230, 245));
                            SelectObject(memDC, sheenBrush2);
                            Ellipse(memDC, cx - 5, cy - 5, cx - 3, cy - 3);
                            DeleteObject(sheenBrush);
                            DeleteObject(sheenBrush2);
                        } else { // 3D White Clam Shell Stone (Natural striations & pearlescent glint)
                            HBRUSH clamBrush = CreateSolidBrush(RGB(246, 243, 235));
                            HPEN clamPen = CreatePen(PS_SOLID, 1, RGB(190, 180, 165));
                            HBRUSH oldB = SelectObject(memDC, clamBrush);
                            HPEN oldP = SelectObject(memDC, clamPen);
                            Ellipse(memDC, cx - radius, cy - radius, cx + radius, cy + radius);
                            
                            // Fine clam shell natural growth grain lines
                            int varO = ((r * 13 + c * 17) % 5) - 2;
                            int varOy = ((r * 11 + c * 19) % 3) - 1;
                            HPEN grainP = CreatePen(PS_SOLID, 1, RGB(220, 210, 195));
                            SelectObject(memDC, grainP);
                            Arc(memDC, cx - radius + 2 + varO, cy - radius + 3 + varOy, cx + radius - 2 - varO, cy + radius + 3 + varOy, cx - radius + 2 + varO, cy + varOy, cx + radius - 2 - varO, cy + varOy);
                            Arc(memDC, cx - radius + 3 + varO, cy - radius + 7 + varOy, cx + radius - 3 - varO, cy + radius + 7 + varOy, cx - radius + 3 + varO, cy + 4 + varOy, cx + radius - 3 - varO, cy + 4 + varOy);
                            Arc(memDC, cx - radius + 4 + varO, cy - radius + 11 + varOy, cx + radius - 4 - varO, cy + radius + 11 + varOy, cx - radius + 4 + varO, cy + 8 + varOy, cx + radius - 4 - varO, cy + 8 + varOy);
                            DeleteObject(grainP);

                            // Delicate iridescent top arc
                            HPEN arcPen2 = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                            SelectObject(memDC, arcPen2);
                            Arc(memDC, cx - radius + 2, cy - radius + 2, cx + radius - 2, cy + radius - 2, cx - radius + 2, cy, cx, cy - radius + 2);
                            DeleteObject(arcPen2);

                            SelectObject(memDC, oldB);
                            SelectObject(memDC, oldP);
                            DeleteObject(clamBrush);
                            DeleteObject(clamPen);

                            // Specular highlight
                            HBRUSH sheenBrush = CreateSolidBrush(RGB(255, 255, 255));
                            SelectObject(memDC, nullPen);
                            SelectObject(memDC, sheenBrush);
                            Ellipse(memDC, cx - 8, cy - 8, cx - 2, cy - 2);
                            DeleteObject(sheenBrush);
                        }

                        // Traveling diagonal specular sheen glint traversing stone face
                        float sheenPos = fmodf(animTime * 1.5f + (r * 0.25f + c * 0.35f), 4.0f);
                        if (sheenPos < 0.85f) {
                            int shOff = (int)((sheenPos / 0.85f - 0.5f) * (radius * 1.5f));
                            HPEN sheenLinePen = CreatePen(PS_SOLID, 1, board[r][c] == 1 ? RGB(90, 110, 135) : RGB(255, 255, 255));
                            HPEN oldShP = SelectObject(memDC, sheenLinePen);
                            MoveToEx(memDC, cx - radius/2 + shOff, cy + radius/2 + shOff, NULL);
                            LineTo(memDC, cx + radius/2 + shOff, cy - radius/2 + shOff);
                            SelectObject(memDC, oldShP);
                            DeleteObject(sheenLinePen);
                        }

                        // Last Move Marker Ring
                        if (c == lastMoveX && r == lastMoveY) {
                            COLORREF ringColor = (board[r][c] == 1) ? RGB(0, 240, 255) : RGB(255, 215, 0);
                            HBRUSH oldB = SelectObject(memDC, hollowB);

                            // Animated glowing particle aura
                            float auraPhase = (sinf(animTime * 3.0f) + 1.0f) * 0.5f;
                            int auraSize = 16 + (int)(auraPhase * 6.0f);
                            HPEN auraPen1 = CreatePen(PS_SOLID, 2, ringColor);
                            HPEN oldP = SelectObject(memDC, auraPen1);
                            Ellipse(memDC, cx - auraSize, cy - auraSize, cx + auraSize, cy + auraSize);
                            DeleteObject(auraPen1);

                            HPEN ringPen = CreatePen(PS_SOLID, 2, ringColor);
                            SelectObject(memDC, ringPen);
                            Ellipse(memDC, cx - 5, cy - 5, cx + 5, cy + 5);
                            
                            // Multi-tier Placement shockwave ripple rings
                            if (rippleRadius > 0) {
                                HPEN ripplePen = CreatePen(PS_SOLID, max(1, 4 - (rippleRadius - 13)/7), ringColor);
                                SelectObject(memDC, ripplePen);
                                Ellipse(memDC, cx - rippleRadius, cy - rippleRadius, cx + rippleRadius, cy + rippleRadius);
                                DeleteObject(ripplePen);

                                // Outer gold ripple ring
                                int outRad = rippleRadius + 7;
                                if (outRad <= 44) {
                                    HPEN outRipPen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
                                    SelectObject(memDC, outRipPen);
                                    Ellipse(memDC, cx - outRad, cy - outRad, cx + outRad, cy + outRad);
                                    DeleteObject(outRipPen);
                                }
                            }
                            
                            SelectObject(memDC, oldP);
                            SelectObject(memDC, oldB);
                            DeleteObject(ringPen);
                        }

                        // Atari danger warning ring
                        if (showEstimator && atariMap[r][c]) {
                            HPEN atariPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
                            HPEN oldP = SelectObject(memDC, atariPen);
                            HBRUSH oldB = SelectObject(memDC, hollowB);
                            Ellipse(memDC, cx - radius - 1, cy - radius - 1, cx + radius + 1, cy + radius + 1);
                            SelectObject(memDC, oldP);
                            SelectObject(memDC, oldB);
                            DeleteObject(atariPen);
                        }
                    }
                }
            }

            // Captured stone animation - pop effect
            for (int i = 0; i < capturedAnimCount; i++) {
                int cx = padding + capturedAnimStones[i].x * cellSize;
                int cy = padding + capturedAnimStones[i].y * cellSize;
                HBRUSH stoneBrush = CreateSolidBrush(capturedAnimColor[i] == 1 ? RGB(20, 22, 26) : RGB(246, 243, 235));
                
                int rCol = rand() % 255;
                int gCol = rand() % 255;
                int bCol = rand() % 255;
                HPEN stonePen = CreatePen(PS_SOLID, 3, RGB(rCol, gCol, bCol));
                HBRUSH oldBrush = SelectObject(memDC, stoneBrush);
                HPEN oldPen = SelectObject(memDC, stonePen);
                
                int animRad = captureAnimRadius;
                if (captureAnimRadius < 6) {
                    animRad = 15 - captureAnimRadius;
                }
                Ellipse(memDC, cx - animRad, cy - animRad, cx + animRad, cy + animRad);
                
                SelectObject(memDC, oldBrush);
                SelectObject(memDC, oldPen);
                DeleteObject(stoneBrush);
                DeleteObject(stonePen);
            }

            // 4-Layer Advanced Particles
            for (int i = 0; i < advParticleCount; i++) {
                int px = (int)advParticles[i].x;
                int py = (int)advParticles[i].y;
                float pLifeRatio = (float)advParticles[i].life / (float)(advParticles[i].maxLife > 0 ? advParticles[i].maxLife : 1);
                
                if (advParticles[i].type == PARTICLE_SPARK) {
                    // Layer 1: Core Incandescent Spark
                    COLORREF sparkColor = (advParticles[i].color == 4) ? RGB(110, 230, 255) : 
                                         ((advParticles[i].color == 3) ? RGB(255, 215, 0) : RGB(255, 255, 255));
                    HBRUSH sBrush = CreateSolidBrush(sparkColor);
                    SelectObject(memDC, nullPen);
                    SelectObject(memDC, sBrush);
                    int spSize = (int)advParticles[i].size;
                    if (spSize < 1) spSize = 1;
                    Ellipse(memDC, px - spSize, py - spSize, px + spSize, py + spSize);
                    SetPixel(memDC, px, py, RGB(255, 255, 255));
                    DeleteObject(sBrush);
                } else if (advParticles[i].type == PARTICLE_SMOKE) {
                    // Layer 2: Expanding Smoke Puff
                    int smR = (int)(210 * pLifeRatio + 30);
                    int smG = (int)(200 * pLifeRatio + 30);
                    int smB = (int)(185 * pLifeRatio + 30);
                    HBRUSH smBrush = CreateSolidBrush(RGB(smR, smG, smB));
                    SelectObject(memDC, nullPen);
                    SelectObject(memDC, smBrush);
                    int smSize = (int)advParticles[i].size;
                    Ellipse(memDC, px - smSize, py - smSize, px + smSize, py + smSize);
                    DeleteObject(smBrush);
                } else if (advParticles[i].type == PARTICLE_SHARD) {
                    // Layer 3: Heavy Kinematic Stone Shard / Chip
                    HBRUSH shBrush = CreateSolidBrush(advParticles[i].color == 1 ? RGB(22, 24, 30) : RGB(242, 238, 228));
                    HPEN shPen = CreatePen(PS_SOLID, 1, advParticles[i].color == 1 ? RGB(60, 70, 85) : RGB(190, 180, 160));
                    SelectObject(memDC, shBrush);
                    SelectObject(memDC, shPen);
                    float rot = advParticles[i].rot;
                    float cosT = cosf(rot);
                    float sinT = sinf(rot);
                    int sLen = (int)advParticles[i].size;
                    POINT shPts[4] = {
                        {px + (int)(-sLen*cosT - (-sLen/2)*sinT), py + (int)(-sLen*sinT + (-sLen/2)*cosT)},
                        {px + (int)(sLen*cosT - (-sLen/2)*sinT),  py + (int)(sLen*sinT + (-sLen/2)*cosT)},
                        {px + (int)(sLen*cosT - (sLen/2)*sinT),   py + (int)(sLen*sinT + (sLen/2)*cosT)},
                        {px + (int)(-sLen*cosT - (sLen/2)*sinT),  py + (int)(-sLen*sinT + (sLen/2)*cosT)}
                    };
                    Polygon(memDC, shPts, 4);
                    DeleteObject(shBrush);
                    DeleteObject(shPen);
                } else if (advParticles[i].type == PARTICLE_STAR) {
                    // Layer 4: Radiant Celebration Star
                    HBRUSH stBrush = CreateSolidBrush(RGB(255, 225, 80));
                    SelectObject(memDC, stBrush);
                    SelectObject(memDC, nullPen);
                    int stRad = (int)(advParticles[i].size * pLifeRatio) + 1;
                    POINT starPts[4] = {
                        {px, py - stRad * 2},
                        {px + stRad * 2, py},
                        {px, py + stRad * 2},
                        {px - stRad * 2, py}
                    };
                    Polygon(memDC, starPts, 4);
                    SetPixel(memDC, px, py, RGB(255, 255, 255));
                    DeleteObject(stBrush);
                }
            }

            if (hintX != -1 && hintY != -1 && board[hintY][hintX] == 0) {
                int cx = padding + hintX * cellSize;
                int cy = padding + hintY * cellSize;
                HPEN hintPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
                HPEN oldPen = SelectObject(memDC, hintPen);
                HBRUSH oldBrush = SelectObject(memDC, hollowB);
                Ellipse(memDC, cx - 14, cy - 14, cx + 14, cy + 14);
                SelectObject(memDC, oldBrush);
                SelectObject(memDC, oldPen);
                DeleteObject(hintPen);
            }

            if (hoverX != -1 && hoverY != -1 && board[hoverY][hoverX] == 0) {
                int cx = padding + hoverX * cellSize;
                int cy = padding + hoverY * cellSize;
                HPEN ghostPen = CreatePen(PS_SOLID, 2, currentPlayer == 1 ? RGB(100, 100, 100) : RGB(200, 200, 200));
                HPEN oldPen = SelectObject(memDC, ghostPen);
                HBRUSH oldBrush = SelectObject(memDC, hollowB);
                Ellipse(memDC, cx - 13, cy - 13, cx + 13, cy + 13);
                SelectObject(memDC, oldBrush);
                SelectObject(memDC, oldPen);
                DeleteObject(ghostPen);
            }
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(224, 224, 224));
            char status[256];
            if (currentCampaignStage != -1) {
                sprintf(status, "%s | Turn: %s | Komi: %.1f%s%s%s", 
                    campaign[currentCampaignStage].name,
                    currentPlayer == 1 ? "Black" : "White",
                    currentKomi,
                    hintX != -1 ? " | HINT ACTIVE" : "",
                    showEstimator ? " | EST ON" : "",
                    showAnalyzer ? " | ANALYZER ON" : "");
            } else {
                sprintf(status, "KGo - Turn: %s | Komi: %.1f%s%s%s", 
                    currentPlayer == 1 ? "Black" : "White",
                    currentKomi,
                    hintX != -1 ? " | HINT ACTIVE" : "",
                    showEstimator ? " | EST ON" : "",
                    showAnalyzer ? " | ANALYZER ON" : "");
            }
            TextOut(memDC, 20, 10, status, strlen(status));

            if (strlen(analyzerStatusStr) > 0) {
                SetTextColor(memDC, RGB(0, 255, 200));
                TextOut(memDC, 20, 595, analyzerStatusStr, strlen(analyzerStatusStr));
            } else if (hintX != -1 && hintY != -1) {
                char hintStr[128];
                sprintf(hintStr, "AI Hint: Optimal Move at (%d, %d) with score %d", hintX, hintY, hintScore);
                SetTextColor(memDC, RGB(255, 215, 0));
                TextOut(memDC, 20, 595, hintStr, strlen(hintStr));
            }

            // Atmospheric falling cherry blossom petals
            HBRUSH petalBrush = CreateSolidBrush(RGB(255, 183, 197));
            SelectObject(memDC, nullPen);
            SelectObject(memDC, petalBrush);
            for (int i = 0; i < MAX_PETALS; i++) {
                int px = (int)petals[i].x;
                int py = (int)petals[i].y;
                int s = (int)petals[i].size;
                Ellipse(memDC, px, py, px + s*2, py + s);
            }
            DeleteObject(petalBrush);

            // Atmospheric floating golden Zen dust motes
            HBRUSH zenBrush = CreateSolidBrush(RGB(255, 215, 0));
            HBRUSH zenCore = CreateSolidBrush(RGB(255, 255, 255));
            for (int i = 0; i < MAX_ZEN_MOTES; i++) {
                int px = (int)zenMotes[i].x;
                int py = (int)zenMotes[i].y;
                int s = (int)zenMotes[i].size;
                SelectObject(memDC, nullPen);
                SelectObject(memDC, zenBrush);
                Ellipse(memDC, px - s, py - s, px + s, py + s);
                SelectObject(memDC, zenCore);
                Ellipse(memDC, px - 1, py - 1, px + 1, py + 1);
            }
            DeleteObject(zenBrush);
            DeleteObject(zenCore);

            // Double-buffered BitBlt with procedural screen-shake offset
            int offX = 0, offY = 0;
            if (shakeMagnitude > 0.05f) {
                offX = (int)(cosf(shakeAngle) * shakeMagnitude);
                offY = (int)(sinf(shakeAngle * 1.35f) * shakeMagnitude);
            }
            BitBlt(hdc, offX, offY, winW, winH, memDC, 0, 0, SRCCOPY);
            if (offX != 0 || offY != 0) {
                HBRUSH bgBrush = CreateSolidBrush(RGB(158, 148, 107));
                if (offX > 0) { RECT r = {0, 0, offX, winH}; FillRect(hdc, &r, bgBrush); }
                else if (offX < 0) { RECT r = {winW + offX, 0, winW, winH}; FillRect(hdc, &r, bgBrush); }
                if (offY > 0) { RECT r = {0, 0, winW, offY}; FillRect(hdc, &r, bgBrush); }
                else if (offY < 0) { RECT r = {0, winH + offY, winW, winH}; FillRect(hdc, &r, bgBrush); }
                DeleteObject(bgBrush);
            }

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_PASS) {
                consecutivePasses++;
                TriggerScreenShake(2.5f);
                if (consecutivePasses >= 2) {
                    CalculateScore(hwnd);
                    return 0;
                }
                int capBackup[3] = { captures[0], captures[1], captures[2] };
                char bBackup[19][19];
                CopyBoard(bBackup, board);
                PushUndo(bBackup, capBackup, currentPlayer);
                currentPlayer = currentPlayer == 1 ? 2 : 1;
                hintX = -1; hintY = -1;
                InvalidateRect(hwnd, NULL, TRUE);
                if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    SetTimer(hwnd, 2, 500, NULL);
                }
            } else if (LOWORD(wParam) == ID_BTN_RESIGN) {
                int winner = currentPlayer == 1 ? 2 : 1;
                RecordGameEnd(winner, hwnd);
                TriggerScreenShake(10.0f);
                char msg[256];
                if (currentCampaignStage != -1) {
                    sprintf(msg, "%s wins by resignation!\n\nYou failed Campaign Stage %d (%s). Try again.", 
                            currentPlayer == 1 ? "White" : "Black", currentCampaignStage + 1, campaign[currentCampaignStage].name);
                } else {
                    sprintf(msg, "%s wins by resignation!", currentPlayer == 1 ? "White" : "Black");
                }
                PlayGameSound(3);
                MessageBox(hwnd, msg, "Game Over", MB_OK);
                if (currentCampaignStage != -1) {
                    StartCampaignStage(hwnd);
                } else {
                    InitBoard();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == ID_BTN_SCORE) {
                CalculateScore(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                currentCampaignStage = -1;
                currentKomi = 6.5f;
                TriggerScreenShake(4.0f);
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_SAVE) {
                SaveGame(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_LOAD) {
                TriggerScreenShake(4.0f);
                LoadGame(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_UNDO) {
                TriggerScreenShake(3.0f);
                DoUndo(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_REDO) {
                TriggerScreenShake(3.0f);
                DoRedo(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_HINT) {
                TriggerScreenShake(3.5f);
                CalculateHint();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_ESTIMATE) {
                showEstimator = !showEstimator;
                TriggerScreenShake(2.5f);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_ANALYZER) {
                showAnalyzer = !showAnalyzer;
                TriggerScreenShake(2.5f);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_CAMPAIGN) {
                currentCampaignStage = 0;
                TriggerScreenShake(5.0f);
                StartCampaignStage(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_TSUMEGO) {
                currentTsumegoIndex = (currentTsumegoIndex + 1) % 5;
                TriggerScreenShake(5.0f);
                LoadTsumegoPuzzle(hwnd, currentTsumegoIndex);
            } else if (LOWORD(wParam) == ID_BTN_STATS) {
                char smsg[256];
                sprintf(smsg, "Statistics:\nGames Played: %d\n\nvs AI Mode:\nWins: %d\nLosses: %d\n\nLocal Mode:\nBlack Wins: %d\nWhite Wins: %d",
                        stats.played, stats.aiWins, stats.aiLosses, stats.localB, stats.localW);
                MessageBox(hwnd, smsg, "Statistics", MB_OK);
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBox(hwnd, 
                    "Goal: Control more territory (empty points) than your opponent.\n\n"
                    "- Players (Black & White) take turns placing stones on intersections.\n"
                    "- Stones must have at least one empty adjacent point (liberty).\n"
                    "- If surrounded, stones are captured and removed.\n"
                    "- Superko Rule: Cannot recreate ANY previous board position.\n"
                    "- Suicide Rule: Cannot place a stone with no liberties unless it captures.\n\n"
                    "Active Skills & Assistance:\n"
                    "- Press 'U' (or Undo): Revert last turn move pair.\n"
                    "- Press 'H' (or Hint): Calculate & highlight optimal AI candidate move.\n"
                    "- Press 'T' (or Est): Toggle Territory map overlay & group danger warning.\n"
                    "- Press 'S' (or Analyzer): Toggle Group Liberty Analyzer overlay & numbered liberties.\n"
                    "- Click 'Tsumego': Practice life-and-death Tsumego puzzles.",
                    "How to Play KGo", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == ID_CB_SIZE && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hCbSize, CB_GETCURSEL, 0, 0);
                if (sel == 0) boardSize = 9;
                else if (sel == 1) boardSize = 13;
                else if (sel == 2) boardSize = 19;
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KGoWindowClass";
    srand(GetTickCount());

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KGo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 750,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    LoadStats();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
