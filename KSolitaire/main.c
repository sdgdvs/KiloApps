#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define ID_NEW_GAME       1001
#define ID_DRAW_1         1002
#define ID_DRAW_3         1003
#define ID_UNDO           1004
#define ID_REDO           1005
#define ID_HINT           1006
#define ID_AUTOFINISH     1007
#define ID_SAVE_GAME      1008
#define ID_LOAD_GAME      1009
#define ID_STATS          1010
#define ID_RESET_STATS    1011

#define ID_THEME_BLUE     1020
#define ID_THEME_CRIMSON  1021
#define ID_THEME_EMERALD  1022
#define ID_THEME_CYBER    1023
#define ID_THEME_GOLD     1024

#define ID_FELT_GREEN     1030
#define ID_FELT_SLATE     1031
#define ID_FELT_INDIGO    1032
#define ID_FELT_CRIMSON   1033

#define ID_MODE_CLASSIC   1040
#define ID_MODE_VEGAS     1041
#define ID_MODE_CAMPAIGN  1042
#define ID_STAGE_PREV     1043
#define ID_STAGE_NEXT     1044

#define ID_SKILL_WAND     1050
#define ID_SKILL_XRAY     1051
#define ID_SKILL_SHUFFLE  1052
#define ID_SKILL_UNDO     1053

#define CARD_W 84
#define CARD_H 118
#define GAP_X  14
#define GAP_Y  16

typedef struct {
    int suit;       // 0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades
    int rank;       // 1 (Ace) to 13 (King)
    int faceUp;
    int isFrozen;   // 1 if frozen card
} Card;

typedef struct {
    Card stock[52];
    int stock_cnt;

    Card waste[52];
    int waste_cnt;

    Card foundations[4][13];
    int foundation_cnt[4];
    int foundation_suit[4]; // Suit bound to foundation pile (-1 if empty & not locked)

    Card tableau[7][21];
    int tableau_cnt[7];

    int moves;
    int score;
    int timerSeconds;
    int gameStarted;
    int drawMode;       // 1 or 3
    int gameMode;       // 0 = Classic, 1 = Campaign, 2 = Vegas Money Mode
    int campaignStage;  // 1 to 20
    int vegasRules;     // 1 if Vegas scoring applies (-$52 buyin, +$5 per foundation card)
    int stockPasses;    // Stock recycles count
    int maxStockPasses; // Pass cap (1 for D1 Vegas, 3 for D3 Vegas, 0 = infinite)
    int suitLocked;     // 1 if foundations are suit locked (0=H, 1=D, 2=C, 3=S)
    int targetCards;    // Foundation card goal to win

    int wandCharges;    // Magic Wand uses (default 3)
    int xrayCharges;    // X-Ray Vision uses (default 3)
    int shuffleCharges; // Stock Shuffle uses (default 3)
    int xrayTimer;      // X-Ray vision active timer (seconds)
} SolitaireState;

typedef struct {
    int played;
    int wins;
    int currentStreak;
    int bestStreak;
    int highScore;
    int bestTimeD1;
    int bestTimeD3;
    int vegasCash;         // Cumulative Vegas Bankroll ($)
    int maxCampaignStage;  // Highest stage unlocked (1..20)
} SolitaireStats;

typedef struct {
    int stage;
    const char* name;
    int drawMode;
    int vegasRules;
    int suitLocked;
    int frozenCards;
    int targetCards;
    const char* desc;
} StageConfig;

static const StageConfig CAMPAIGN_STAGES[20] = {
    {1,  "Beginner's Luck",       1, 0, 0, 0, 20, "Draw 1 - Target: 20 Foundation Cards"},
    {2,  "Double Pass",           1, 0, 0, 0, 24, "Draw 1 - Target: 24 Foundation Cards"},
    {3,  "Cold Snap",             1, 0, 0, 1, 28, "Draw 1 - 1 Frozen Card in Tableau"},
    {4,  "Suit Lock 101",         1, 0, 1, 0, 32, "Draw 1 - Foundations Suit-Locked"},
    {5,  "Vegas Debut",           1, 1, 0, 0, 36, "Draw 1 - Vegas Money Rules (-$52 Start)"},
    {6,  "Triple Draw",           3, 0, 0, 0, 36, "Draw 3 - Classic Rules"},
    {7,  "Frostbite",             1, 0, 0, 2, 40, "Draw 1 - 2 Frozen Cards"},
    {8,  "High Roller Vegas",     3, 1, 0, 0, 40, "Draw 3 - Vegas Money Rules"},
    {9,  "Lock & Freeze",         1, 0, 1, 2, 44, "Draw 1 - Suit Lock + 2 Frozen Cards"},
    {10, "Deep Freeze",           3, 0, 0, 3, 44, "Draw 3 - 3 Frozen Cards"},
    {11, "Vegas Strict",          3, 1, 0, 0, 44, "Draw 3 - Vegas Rules (1 Pass Only)"},
    {12, "Glacial Suits",         1, 0, 1, 3, 48, "Draw 1 - Suit Lock + 3 Frozen Cards"},
    {13, "Suit Master",           3, 0, 1, 0, 48, "Draw 3 - Suit-Locked Foundations"},
    {14, "Arctic Blast",          1, 0, 0, 4, 48, "Draw 1 - 4 Frozen Cards"},
    {15, "Vegas Mastery",         3, 1, 1, 0, 48, "Draw 3 - Vegas Rules + Suit Lock"},
    {16, "Glacial Fortress",      3, 0, 1, 3, 52, "Draw 3 - Suit Lock + 3 Frozen Cards"},
    {17, "Vegas Peril",           3, 1, 0, 3, 52, "Draw 3 - Vegas Rules + 3 Frozen Cards"},
    {18, "High Stakes Ice",       3, 1, 1, 2, 52, "Draw 3 - Vegas Rules + Suit Lock + 2 Frozen"},
    {19, "Ice Citadel",           3, 0, 1, 4, 52, "Draw 3 - Suit Lock + 4 Frozen Cards"},
    {20, "Grandmaster Challenge", 3, 1, 1, 4, 52, "Draw 3 - Vegas + Suit Lock + 4 Frozen Cards"}
};

typedef struct {
    Card card;
    float x, y;
    float vx, vy;
    int delay;
    int active;
    int finished;
} CascadeCard;

// Memory copy helpers for CRT-less build
#pragma function(memcpy)
void *memcpy(void *dest, const void *src, size_t count) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (count--) *d++ = *s++;
    return dest;
}

#pragma function(memset)
void *memset(void *dest, int c, size_t count) {
    char *d = (char *)dest;
    while (count--) *d++ = (char)c;
    return dest;
}

void CopyState(SolitaireState *dst, const SolitaireState *src) {
    memcpy(dst, src, sizeof(SolitaireState));
}

void ZeroMem(void *ptr, size_t size) {
    memset(ptr, 0, size);
}

// --- Global Variables ---
SolitaireState state;
SolitaireState undoStack[256];
int undoCount = 0;
SolitaireState redoStack[256];
int redoCount = 0;

SolitaireStats stats;
int themeId = 0; // 0=Blue, 1=Crimson, 2=Emerald, 3=Cyber, 4=Gold
int feltId = 0;  // 0=Green, 1=Slate, 2=Indigo, 3=Crimson
int selectedType = -1; // 0=waste, 1=tableau, 2=foundation
int selectedPile = -1;
int selectedCardIdx = -1;

int hintSrcType = -1, hintSrcPile = -1, hintSrcIdx = -1;
int hintDstType = -1, hintDstPile = -1;

int autoFinishActive = 0;
int gameWon = 0;

CascadeCard cascadeCards[52];
int cascadeFrame = 0;
int cascadeActive = 0;

// Random Seed generator
unsigned int seed = 1337;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

// --- Helpers ---
COLORREF GetFeltColor() {
    switch(feltId) {
        case 1: return RGB(15, 23, 42);    // Slate Dark
        case 2: return RGB(30, 27, 75);    // Indigo
        case 3: return RGB(69, 10, 10);    // Deep Crimson
        default: return RGB(13, 92, 46);   // Classic Green
    }
}

COLORREF GetCardBackBg() {
    switch(themeId) {
        case 1: return RGB(128, 0, 32);   // Crimson
        case 2: return RGB(6, 78, 59);    // Emerald
        case 3: return RGB(15, 23, 42);   // Cyber
        case 4: return RGB(120, 53, 15);  // Gold
        default: return RGB(30, 60, 114); // Navy Blue
    }
}

// --- Storage Persistence ---
void LoadStats() {
    ZeroMem(&stats, sizeof(SolitaireStats));
    HANDLE hFile = CreateFileA("ksolitaire.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &stats, sizeof(SolitaireStats), &readBytes, NULL);
        CloseHandle(hFile);
    }
    if (stats.maxCampaignStage < 1) stats.maxCampaignStage = 1;
}

void SaveStats() {
    HANDLE hFile = CreateFileA("ksolitaire.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &stats, sizeof(SolitaireStats), &written, NULL);
        CloseHandle(hFile);
    }
}

void SaveGameState() {
    HANDLE hFile = CreateFileA("ksolitaire.sav", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &state, sizeof(SolitaireState), &written, NULL);
        CloseHandle(hFile);
    }
}

int LoadGameState() {
    HANDLE hFile = CreateFileA("ksolitaire.sav", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &state, sizeof(SolitaireState), &readBytes, NULL);
        CloseHandle(hFile);
        return 1;
    }
    return 0;
}

// --- Undo / Redo Snapshot ---
void PushUndoState() {
    if (undoCount < 256) {
        CopyState(&undoStack[undoCount++], &state);
    } else {
        for (int i = 0; i < 255; i++) CopyState(&undoStack[i], &undoStack[i+1]);
        CopyState(&undoStack[255], &state);
    }
    redoCount = 0;
    SaveGameState();
}

void PerformUndo() {
    if (undoCount == 0) return;
    if (redoCount < 256) CopyState(&redoStack[redoCount++], &state);
    CopyState(&state, &undoStack[--undoCount]);
    selectedType = -1;
    hintSrcType = -1;
    SaveGameState();
}

void PerformRedo() {
    if (redoCount == 0) return;
    if (undoCount < 256) CopyState(&undoStack[undoCount++], &state);
    CopyState(&state, &redoStack[--redoCount]);
    selectedType = -1;
    hintSrcType = -1;
    SaveGameState();
}

// --- Solitaire Game Engine ---
void ClearSelectionAndHints() {
    selectedType = -1;
    selectedPile = -1;
    selectedCardIdx = -1;
    hintSrcType = -1;
}

void NewGame(HWND hwnd) {
    state.moves = 0;
    state.timerSeconds = 0;
    state.gameStarted = 0;
    state.stockPasses = 0;

    if (state.campaignStage < 1) state.campaignStage = 1;
    if (state.campaignStage > 20) state.campaignStage = 20;

    int frozenCardsToPlace = 0;

    if (state.gameMode == 1) { // Campaign Mode
        const StageConfig *cfg = &CAMPAIGN_STAGES[state.campaignStage - 1];
        state.drawMode = cfg->drawMode;
        state.vegasRules = cfg->vegasRules;
        state.suitLocked = cfg->suitLocked;
        state.targetCards = cfg->targetCards;
        frozenCardsToPlace = cfg->frozenCards;
        state.score = cfg->vegasRules ? -52 : 0;
        state.maxStockPasses = cfg->vegasRules ? (cfg->drawMode == 1 ? 1 : 3) : (cfg->stage == 2 ? 2 : 0);
    } else if (state.gameMode == 2) { // Vegas Money Mode
        if (state.drawMode != 1 && state.drawMode != 3) state.drawMode = 1;
        state.vegasRules = 1;
        state.suitLocked = 0;
        state.targetCards = 52;
        state.score = -52;
        state.maxStockPasses = (state.drawMode == 1 ? 1 : 3);
        frozenCardsToPlace = 0;
    } else { // Classic Mode
        if (state.drawMode != 1 && state.drawMode != 3) state.drawMode = 1;
        state.vegasRules = 0;
        state.suitLocked = 0;
        state.targetCards = 52;
        state.score = 0;
        state.maxStockPasses = 0;
        frozenCardsToPlace = 0;
    }

    state.wandCharges = 3;
    state.xrayCharges = 3;
    state.shuffleCharges = 3;
    state.xrayTimer = 0;

    undoCount = 0;
    redoCount = 0;
    gameWon = 0;
    autoFinishActive = 0;
    cascadeActive = 0;
    KillTimer(hwnd, 3);
    ClearSelectionAndHints();

    Card deck[52];
    int idx = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 1; r <= 13; r++) {
            deck[idx].suit = s;
            deck[idx].rank = r;
            deck[idx].faceUp = 0;
            deck[idx].isFrozen = 0;
            idx++;
        }
    }

    // Fisher-Yates Shuffle
    for (int i = 51; i > 0; i--) {
        int j = rnd() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }

    for (int i = 0; i < 4; i++) {
        state.foundation_cnt[i] = 0;
        state.foundation_suit[i] = state.suitLocked ? i : -1;
    }
    state.waste_cnt = 0;

    idx = 0;
    for (int col = 0; col < 7; col++) {
        state.tableau_cnt[col] = 0;
        for (int row = 0; row <= col; row++) {
            Card c = deck[idx++];
            if (row == col) c.faceUp = 1;
            state.tableau[col][state.tableau_cnt[col]++] = c;
        }
    }

    // Apply Ice/Frozen Cards to initial face-up cards if required
    if (frozenCardsToPlace > 0) {
        int placed = 0;
        int attempts = 0;
        while (placed < frozenCardsToPlace && attempts < 50) {
            attempts++;
            int col = rnd() % 7;
            int topIdx = state.tableau_cnt[col] - 1;
            if (!state.tableau[col][topIdx].isFrozen) {
                state.tableau[col][topIdx].isFrozen = 1;
                placed++;
            }
        }
    }

    state.stock_cnt = 0;
    while (idx < 52) {
        state.stock[state.stock_cnt++] = deck[idx++];
    }

    SaveGameState();
    InvalidateRect(hwnd, NULL, FALSE);
}

int IsRedSuit(int suit) {
    return (suit == 0 || suit == 1);
}

int CanMoveToFoundation(Card card, int fIdx) {
    if (state.suitLocked) {
        if (card.suit != fIdx) return 0;
        if (state.foundation_cnt[fIdx] == 0) return (card.rank == 1);
        Card topCard = state.foundations[fIdx][state.foundation_cnt[fIdx] - 1];
        return (card.rank == topCard.rank + 1);
    } else {
        if (state.foundation_cnt[fIdx] == 0) {
            return (card.rank == 1);
        } else {
            Card topCard = state.foundations[fIdx][state.foundation_cnt[fIdx] - 1];
            if (card.suit != topCard.suit) return 0;
            return (card.rank == topCard.rank + 1);
        }
    }
}

int CanMoveToTableau(Card topMoveCard, int tIdx) {
    if (state.tableau_cnt[tIdx] == 0) return (topMoveCard.rank == 13);
    Card targetCard = state.tableau[tIdx][state.tableau_cnt[tIdx] - 1];
    if (!targetCard.faceUp) return 0;
    if (IsRedSuit(targetCard.suit) == IsRedSuit(topMoveCard.suit)) return 0;
    return (targetCard.rank == topMoveCard.rank + 1);
}

void ThawAdjacent(int colA, int colB) {
    int minC = (colA < colB ? colA : colB) - 1;
    int maxC = (colA > colB ? colA : colB) + 1;
    if (minC < 0) minC = 0;
    if (maxC > 6) maxC = 6;

    for (int c = minC; c <= maxC; c++) {
        for (int i = 0; i < state.tableau_cnt[c]; i++) {
            state.tableau[c][i].isFrozen = 0;
        }
    }
}

void StartWinCascade(HWND hwnd) {
    cascadeActive = 1;
    cascadeFrame = 0;
    int idx = 0;
    int foundationStartX = GAP_X + 3 * (CARD_W + GAP_X);
    int stockY = 35 + GAP_Y;

    for (int f = 0; f < 4; f++) {
        int fx = foundationStartX + f * (CARD_W + GAP_X);
        for (int c = state.foundation_cnt[f] - 1; c >= 0; c--) {
            cascadeCards[idx].card = state.foundations[f][c];
            cascadeCards[idx].x = (float)fx;
            cascadeCards[idx].y = (float)stockY;
            cascadeCards[idx].vx = (float)((rnd() % 7 + 3) * ((rnd() % 2 == 0) ? 1 : -1));
            cascadeCards[idx].vy = -(float)(rnd() % 5 + 2);
            cascadeCards[idx].delay = idx * 5;
            cascadeCards[idx].active = 0;
            cascadeCards[idx].finished = 0;
            idx++;
        }
    }
    SetTimer(hwnd, 3, 20, NULL);
}

void CheckWin(HWND hwnd) {
    int total = 0;
    for (int i = 0; i < 4; i++) total += state.foundation_cnt[i];
    if (total >= state.targetCards && !gameWon) {
        gameWon = 1;
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);

        stats.played++;
        stats.wins++;
        stats.currentStreak++;
        if (stats.currentStreak > stats.bestStreak) stats.bestStreak = stats.currentStreak;
        if (state.score > stats.highScore) stats.highScore = state.score;
        if (state.vegasRules) stats.vegasCash += state.score;

        if (state.drawMode == 1) {
            if (stats.bestTimeD1 == 0 || state.timerSeconds < stats.bestTimeD1) stats.bestTimeD1 = state.timerSeconds;
        } else {
            if (stats.bestTimeD3 == 0 || state.timerSeconds < stats.bestTimeD3) stats.bestTimeD3 = state.timerSeconds;
        }

        if (state.gameMode == 1) {
            if (state.campaignStage < 20 && state.campaignStage + 1 > stats.maxCampaignStage) {
                stats.maxCampaignStage = state.campaignStage + 1;
            }
        }

        SaveStats();
        StartWinCascade(hwnd);

        char msg[384];
        if (state.gameMode == 1) {
            if (state.campaignStage < 20) {
                wsprintfA(msg, "STAGE CLEARED!\n\nStage %d: %s Completed!\nScore: %d\nTime: %d s\nMoves: %d\n\nStage %d Unlocked!",
                    state.campaignStage, CAMPAIGN_STAGES[state.campaignStage - 1].name, state.score, state.timerSeconds, state.moves, state.campaignStage + 1);
            } else {
                wsprintfA(msg, "GRANDMASTER CHALLENGE VICTORIOUS!\n\nYou have mastered all 20 Solitaire Campaign Stages!\n\nFinal Score: %d\nTime: %d s", state.score, state.timerSeconds);
            }
        } else if (state.gameMode == 2) {
            wsprintfA(msg, "VEGAS SOLITAIRE WIN!\n\nEarnings this match: $%d\nTotal Vegas Cash: $%d\nTime: %d s", state.score, stats.vegasCash, state.timerSeconds);
        } else {
            wsprintfA(msg, "Congratulations! Solitaire Victory!\n\nFinal Score: %d\nTime: %d s\nMoves: %d", state.score, state.timerSeconds, state.moves);
        }

        if (MessageBoxA(hwnd, msg, "KSolitaire Victory!", MB_OK | MB_ICONINFORMATION) == IDOK) {
            if (state.gameMode == 1 && state.campaignStage < 20) {
                state.campaignStage++;
                NewGame(hwnd);
            }
        }
    }
}

int AttemptMove(int srcType, int srcPile, int srcIdx, int dstType, int dstPile, HWND hwnd) {
    Card cardsToMove[21];
    int moveCount = 0;

    if (srcType == 0) {
        if (state.waste_cnt == 0) return 0;
        cardsToMove[0] = state.waste[state.waste_cnt - 1];
        moveCount = 1;
    } else if (srcType == 1) {
        if (srcIdx < 0 || srcIdx >= state.tableau_cnt[srcPile]) return 0;
        // Check if any card in the sequence is frozen
        for (int i = srcIdx; i < state.tableau_cnt[srcPile]; i++) {
            if (state.tableau[srcPile][i].isFrozen) {
                MessageBeep(MB_ICONHAND);
                return 0;
            }
        }
        moveCount = state.tableau_cnt[srcPile] - srcIdx;
        for (int i = 0; i < moveCount; i++) {
            cardsToMove[i] = state.tableau[srcPile][srcIdx + i];
        }
    } else if (srcType == 2) {
        if (state.foundation_cnt[srcPile] == 0) return 0;
        cardsToMove[0] = state.foundations[srcPile][state.foundation_cnt[srcPile] - 1];
        moveCount = 1;
    }

    if (moveCount == 0) return 0;

    int legal = 0;
    if (dstType == 2 && moveCount == 1) {
        if (CanMoveToFoundation(cardsToMove[0], dstPile)) legal = 1;
    } else if (dstType == 1) {
        if (CanMoveToTableau(cardsToMove[0], dstPile)) legal = 1;
    }

    if (!legal) return 0;

    PushUndoState();

    if (srcType == 0) state.waste_cnt--;
    else if (srcType == 1) state.tableau_cnt[srcPile] -= moveCount;
    else if (srcType == 2) state.foundation_cnt[srcPile]--;

    if (dstType == 2) {
        state.foundations[dstPile][state.foundation_cnt[dstPile]++] = cardsToMove[0];
        if (state.foundation_cnt[dstPile] == 1 && !state.suitLocked) {
            state.foundation_suit[dstPile] = cardsToMove[0].suit;
        }
        state.score += (state.vegasRules ? 5 : (srcType == 0 ? 10 : (srcType == 1 ? 10 : 0)));
        MessageBeep(MB_OK);
    } else if (dstType == 1) {
        for (int i = 0; i < moveCount; i++) {
            state.tableau[dstPile][state.tableau_cnt[dstPile]++] = cardsToMove[i];
        }
        if (!state.vegasRules) {
            state.score += (srcType == 0 ? 5 : (srcType == 2 ? -15 : 0));
        }
    }

    if (srcType == 1 && state.tableau_cnt[srcPile] > 0) {
        Card *top = &state.tableau[srcPile][state.tableau_cnt[srcPile] - 1];
        if (!top->faceUp) {
            top->faceUp = 1;
            if (!state.vegasRules) state.score += 5;
        }
    }

    // Thaw adjacent frozen cards when tableau move happens
    if (srcType == 1 || dstType == 1) {
        ThawAdjacent(srcType == 1 ? srcPile : dstPile, dstType == 1 ? dstPile : srcPile);
    }

    state.moves++;
    ClearSelectionAndHints();
    InvalidateRect(hwnd, NULL, FALSE);
    CheckWin(hwnd);
    return 1;
}

// --- Active Skills & Power-ups ---
int UseMagicWand(HWND hwnd) {
    if (state.wandCharges <= 0 || gameWon) {
        MessageBeep(MB_ICONHAND);
        return 0;
    }
    // Search tableau top cards
    for (int t = 0; t < 7; t++) {
        if (state.tableau_cnt[t] > 0) {
            Card c = state.tableau[t][state.tableau_cnt[t] - 1];
            if (c.faceUp && !c.isFrozen) {
                for (int f = 0; f < 4; f++) {
                    if (CanMoveToFoundation(c, f)) {
                        state.wandCharges--;
                        AttemptMove(1, t, state.tableau_cnt[t] - 1, 2, f, hwnd);
                        return 1;
                    }
                }
            }
        }
    }
    // Search waste
    if (state.waste_cnt > 0) {
        Card c = state.waste[state.waste_cnt - 1];
        for (int f = 0; f < 4; f++) {
            if (CanMoveToFoundation(c, f)) {
                state.wandCharges--;
                AttemptMove(0, 0, state.waste_cnt - 1, 2, f, hwnd);
                return 1;
            }
        }
    }
    MessageBeep(MB_ICONASTERISK);
    return 0;
}

int UseXRayVision(HWND hwnd) {
    if (state.xrayCharges <= 0 || gameWon) {
        MessageBeep(MB_ICONHAND);
        return 0;
    }
    state.xrayCharges--;
    state.xrayTimer = 5;
    MessageBeep(MB_OK);
    InvalidateRect(hwnd, NULL, FALSE);
    return 1;
}

int UseShuffleStock(HWND hwnd) {
    if (state.shuffleCharges <= 0 || gameWon) {
        MessageBeep(MB_ICONHAND);
        return 0;
    }
    if (state.stock_cnt == 0 && state.waste_cnt == 0) {
        MessageBeep(MB_ICONASTERISK);
        return 0;
    }
    PushUndoState();
    state.shuffleCharges--;
    Card temp[104];
    int total = 0;
    for (int i = 0; i < state.stock_cnt; i++) {
        temp[total] = state.stock[i];
        temp[total++].faceUp = 0;
    }
    for (int i = 0; i < state.waste_cnt; i++) {
        temp[total] = state.waste[i];
        temp[total++].faceUp = 0;
    }
    for (int i = total - 1; i > 0; i--) {
        int j = rnd() % (i + 1);
        Card t = temp[i];
        temp[i] = temp[j];
        temp[j] = t;
    }
    state.stock_cnt = 0;
    state.waste_cnt = 0;
    for (int i = 0; i < total; i++) {
        state.stock[state.stock_cnt++] = temp[i];
    }
    state.moves++;
    MessageBeep(MB_OK);
    InvalidateRect(hwnd, NULL, FALSE);
    return 1;
}

// --- Smart Hint System ---
void GiveHint(HWND hwnd) {
    ClearSelectionAndHints();
    if (gameWon) return;

    for (int t = 0; t < 7; t++) {
        if (state.tableau_cnt[t] > 0) {
            Card c = state.tableau[t][state.tableau_cnt[t] - 1];
            if (c.faceUp && !c.isFrozen) {
                for (int f = 0; f < 4; f++) {
                    if (CanMoveToFoundation(c, f)) {
                        hintSrcType = 1; hintSrcPile = t; hintSrcIdx = state.tableau_cnt[t] - 1;
                        hintDstType = 2; hintDstPile = f;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return;
                    }
                }
            }
        }
    }

    if (state.waste_cnt > 0) {
        Card c = state.waste[state.waste_cnt - 1];
        for (int f = 0; f < 4; f++) {
            if (CanMoveToFoundation(c, f)) {
                hintSrcType = 0; hintSrcPile = 0; hintSrcIdx = state.waste_cnt - 1;
                hintDstType = 2; hintDstPile = f;
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
        }
    }

    for (int srcT = 0; srcT < 7; srcT++) {
        int firstFaceUp = -1;
        for (int i = 0; i < state.tableau_cnt[srcT]; i++) {
            if (state.tableau[srcT][i].faceUp && !state.tableau[srcT][i].isFrozen) { firstFaceUp = i; break; }
        }
        if (firstFaceUp >= 0) {
            Card c = state.tableau[srcT][firstFaceUp];
            for (int dstT = 0; dstT < 7; dstT++) {
                if (srcT != dstT && CanMoveToTableau(c, dstT)) {
                    hintSrcType = 1; hintSrcPile = srcT; hintSrcIdx = firstFaceUp;
                    hintDstType = 1; hintDstPile = dstT;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return;
                }
            }
        }
    }

    if (state.waste_cnt > 0) {
        Card c = state.waste[state.waste_cnt - 1];
        for (int dstT = 0; dstT < 7; dstT++) {
            if (CanMoveToTableau(c, dstT)) {
                hintSrcType = 0; hintSrcPile = 0; hintSrcIdx = state.waste_cnt - 1;
                hintDstType = 1; hintDstPile = dstT;
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
        }
    }

    MessageBeep(MB_ICONASTERISK);
}

// --- Auto Finish Solver ---
int CanAutoFinish() {
    if (gameWon) return 0;
    if (state.stock_cnt > 0 || state.waste_cnt > 0) return 0;
    for (int t = 0; t < 7; t++) {
        for (int i = 0; i < state.tableau_cnt[t]; i++) {
            if (!state.tableau[t][i].faceUp || state.tableau[t][i].isFrozen) return 0;
        }
    }
    return 1;
}

void AutoFinishStep(HWND hwnd) {
    int moved = 0;
    for (int t = 0; t < 7; t++) {
        if (state.tableau_cnt[t] > 0) {
            Card c = state.tableau[t][state.tableau_cnt[t] - 1];
            for (int f = 0; f < 4; f++) {
                if (CanMoveToFoundation(c, f)) {
                    AttemptMove(1, t, state.tableau_cnt[t] - 1, 2, f, hwnd);
                    moved = 1;
                    break;
                }
            }
        }
        if (moved) break;
    }

    if (!moved || gameWon) {
        KillTimer(hwnd, 2);
        autoFinishActive = 0;
    }
}

// --- GDI Vector Suit Drawing ---
void DrawSuitGDI(HDC hdc, int suit, int cx, int cy, int size) {
    int r = size / 2;
    if (suit == 0) { // Hearts ♥
        HBRUSH brush = CreateSolidBrush(RGB(211, 47, 47));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        
        Ellipse(hdc, cx - r, cy - r, cx, cy + 2);
        Ellipse(hdc, cx, cy - r, cx + r, cy + 2);
        POINT pts[3] = { {cx - r, cy}, {cx + r, cy}, {cx, cy + r} };
        Polygon(hdc, pts, 3);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    } else if (suit == 1) { // Diamonds ♦
        HBRUSH brush = CreateSolidBrush(RGB(211, 47, 47));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        
        POINT pts[4] = { {cx, cy - r}, {cx + r, cy}, {cx, cy + r}, {cx - r, cy} };
        Polygon(hdc, pts, 4);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    } else if (suit == 2) { // Clubs ♣
        HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        
        int cr = (int)(r * 0.65f);
        Ellipse(hdc, cx - cr, cy - r, cx + cr, cy - r + 2 * cr);
        Ellipse(hdc, cx - r, cy - cr, cx - r + 2 * cr, cy + cr);
        Ellipse(hdc, cx + r - 2 * cr, cy - cr, cx + r, cy + cr);
        
        POINT stem[3] = { {cx - 2, cy}, {cx + 2, cy}, {cx, cy + r} };
        Polygon(hdc, stem, 3);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    } else if (suit == 3) { // Spades ♠
        HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        
        int cr = (int)(r * 0.5f);
        POINT head[3] = { {cx, cy - r}, {cx - r, cy + 2}, {cx + r, cy + 2} };
        Polygon(hdc, head, 3);
        Ellipse(hdc, cx - r, cy - 2, cx, cy + 2 * cr);
        Ellipse(hdc, cx, cy - 2, cx + r, cy + 2 * cr);
        
        POINT stem[3] = { {cx - 2, cy + 2}, {cx + 2, cy + 2}, {cx, cy + r} };
        Polygon(hdc, stem, 3);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    }
}

// --- Court Card Portrait GDI Rendering ---
void DrawCourtCardGDI(HDC hdc, int rank, int suit, int x, int y) {
    int fx = x + 16;
    int fy = y + 24;
    int fw = CARD_W - 32;
    int fh = CARD_H - 48;

    COLORREF bgCol = (rank == 13) ? RGB(69, 10, 10) : ((rank == 12) ? RGB(46, 16, 101) : RGB(30, 41, 59));
    HBRUSH bgBrush = CreateSolidBrush(bgCol);
    RECT fr = {fx, fy, fx + fw, fy + fh};
    FillRect(hdc, &fr, bgBrush);
    DeleteObject(bgBrush);

    HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    HPEN oldPen = (HPEN)SelectObject(hdc, goldPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, fx, fy, fx + fw, fy + fh);

    int cx = fx + fw / 2;
    int cy = fy + fh / 2;

    HBRUSH skinBrush = CreateSolidBrush(RGB(255, 224, 189));
    SelectObject(hdc, skinBrush);
    Ellipse(hdc, cx - 11, cy - 15, cx + 11, cy + 7);
    DeleteObject(skinBrush);

    if (rank == 13) { // King
        HBRUSH crownBrush = CreateSolidBrush(RGB(255, 215, 0));
        SelectObject(hdc, crownBrush);
        POINT crownPts[5] = { {cx - 12, cy - 12}, {cx - 8, cy - 22}, {cx, cy - 16}, {cx + 8, cy - 22}, {cx + 12, cy - 12} };
        Polygon(hdc, crownPts, 5);
        DeleteObject(crownBrush);

        HBRUSH robeBrush = CreateSolidBrush(RGB(185, 28, 28));
        SelectObject(hdc, robeBrush);
        POINT robePts[4] = { {cx - 14, cy + 5}, {cx + 14, cy + 5}, {cx + 18, fy + fh - 2}, {cx - 18, fy + fh - 2} };
        Polygon(hdc, robePts, 4);
        DeleteObject(robeBrush);

        HPEN scPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        SelectObject(hdc, scPen);
        MoveToEx(hdc, cx + 14, fy + 8, NULL);
        LineTo(hdc, cx + 14, fy + fh - 4);
        DeleteObject(scPen);
    } else if (rank == 12) { // Queen
        HBRUSH tiaraBrush = CreateSolidBrush(RGB(255, 215, 0));
        SelectObject(hdc, tiaraBrush);
        POINT tiaraPts[5] = { {cx - 10, cy - 12}, {cx - 6, cy - 20}, {cx, cy - 15}, {cx + 6, cy - 20}, {cx + 10, cy - 12} };
        Polygon(hdc, tiaraPts, 5);
        DeleteObject(tiaraBrush);

        HBRUSH gownBrush = CreateSolidBrush(RGB(126, 34, 206));
        SelectObject(hdc, gownBrush);
        POINT gownPts[4] = { {cx - 12, cy + 5}, {cx + 12, cy + 5}, {cx + 16, fy + fh - 2}, {cx - 16, fy + fh - 2} };
        Polygon(hdc, gownPts, 4);
        DeleteObject(gownBrush);

        HBRUSH roseBrush = CreateSolidBrush(RGB(244, 63, 94));
        SelectObject(hdc, roseBrush);
        Ellipse(hdc, cx + 8, cy + 10, cx + 16, cy + 18);
        DeleteObject(roseBrush);
    } else if (rank == 11) { // Jack
        HBRUSH capBrush = CreateSolidBrush(RGB(220, 38, 38));
        SelectObject(hdc, capBrush);
        Ellipse(hdc, cx - 12, cy - 20, cx + 12, cy - 10);
        DeleteObject(capBrush);

        HPEN fPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        SelectObject(hdc, fPen);
        MoveToEx(hdc, cx + 8, cy - 14, NULL);
        LineTo(hdc, cx + 16, cy - 24);
        DeleteObject(fPen);

        HBRUSH tabBrush = CreateSolidBrush(RGB(37, 99, 235));
        SelectObject(hdc, tabBrush);
        POINT tabPts[4] = { {cx - 12, cy + 5}, {cx + 12, cy + 5}, {cx + 14, fy + fh - 2}, {cx - 14, fy + fh - 2} };
        Polygon(hdc, tabPts, 4);
        DeleteObject(tabBrush);

        HPEN spPen = CreatePen(PS_SOLID, 2, RGB(148, 163, 184));
        SelectObject(hdc, spPen);
        MoveToEx(hdc, cx - 14, fy + 4, NULL);
        LineTo(hdc, cx - 14, fy + fh - 4);
        DeleteObject(spPen);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(goldPen);
}

// --- Card Back Rendering ---
void DrawCardBackGDI(HDC hdc, int x, int y, int isXRay) {
    RECT r = {x, y, x + CARD_W, y + CARD_H};
    HBRUSH bg = CreateSolidBrush(isXRay ? RGB(15, 45, 80) : GetCardBackBg());
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    HPEN borderPen = CreatePen(PS_SOLID, 2, isXRay ? RGB(0, 229, 255) : RGB(255, 215, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x + 3, y + 3, x + CARD_W - 3, y + CARD_H - 3, 6, 6);

    HPEN latPen = CreatePen(PS_SOLID, 1, isXRay ? RGB(0, 200, 255) : RGB(255, 215, 0));
    SelectObject(hdc, latPen);
    for (int offset = -CARD_H; offset < CARD_W + CARD_H; offset += 18) {
        MoveToEx(hdc, x + offset, y, NULL);
        LineTo(hdc, x + offset + CARD_H, y + CARD_H);
        MoveToEx(hdc, x + offset, y + CARD_H, NULL);
        LineTo(hdc, x + offset + CARD_H, y);
    }

    int cx = x + CARD_W / 2;
    int cy = y + CARD_H / 2;
    HBRUSH crownBrush = CreateSolidBrush(isXRay ? RGB(0, 229, 255) : RGB(255, 215, 0));
    SelectObject(hdc, crownBrush);
    Ellipse(hdc, cx - 14, cy - 14, cx + 14, cy + 14);
    DeleteObject(crownBrush);

    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, cx - 5, cy - 8, isXRay ? "X" : "K", 1);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(borderPen);
    DeleteObject(latPen);
}

// --- Main GDI Card Renderer ---
void DrawCardGDI(HDC hdc, Card card, int x, int y, int isSelected, int isHintSrc, int isHintDst) {
    if (!card.faceUp) {
        if (state.xrayTimer > 0) {
            DrawCardBackGDI(hdc, x, y, 1);
            COLORREF textColor = IsRedSuit(card.suit) ? RGB(255, 128, 128) : RGB(180, 220, 255);
            SetTextColor(hdc, textColor);
            SetBkMode(hdc, TRANSPARENT);
            char rankStr[4] = {0};
            if (card.rank == 1) wsprintfA(rankStr, "A");
            else if (card.rank == 11) wsprintfA(rankStr, "J");
            else if (card.rank == 12) wsprintfA(rankStr, "Q");
            else if (card.rank == 13) wsprintfA(rankStr, "K");
            else wsprintfA(rankStr, "%d", card.rank);
            TextOutA(hdc, x + 6, y + 4, rankStr, lstrlenA(rankStr));
            DrawSuitGDI(hdc, card.suit, x + 12, y + 26, 12);
        } else {
            DrawCardBackGDI(hdc, x, y, 0);
        }
        return;
    }

    RECT r = {x, y, x + CARD_W, y + CARD_H};
    HBRUSH bg = CreateSolidBrush(card.isFrozen ? RGB(224, 247, 250) : RGB(255, 255, 255));
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    COLORREF borderCol = card.isFrozen ? RGB(0, 229, 255) : 
        (isSelected ? RGB(255, 215, 0) : (isHintSrc ? RGB(0, 230, 118) : (isHintDst ? RGB(255, 145, 0) : RGB(100, 100, 100))));
    int penWidth = (card.isFrozen || isSelected || isHintSrc || isHintDst) ? 3 : 1;

    HPEN cardPen = CreatePen(PS_SOLID, penWidth, borderCol);
    HPEN oldPen = (HPEN)SelectObject(hdc, cardPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x, y, x + CARD_W, y + CARD_H, 8, 8);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(cardPen);

    COLORREF textColor = IsRedSuit(card.suit) ? RGB(211, 47, 47) : RGB(26, 26, 26);
    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);

    char rankStr[4] = {0};
    if (card.rank == 1) wsprintfA(rankStr, "A");
    else if (card.rank == 11) wsprintfA(rankStr, "J");
    else if (card.rank == 12) wsprintfA(rankStr, "Q");
    else if (card.rank == 13) wsprintfA(rankStr, "K");
    else wsprintfA(rankStr, "%d", card.rank);

    TextOutA(hdc, x + 6, y + 4, rankStr, lstrlenA(rankStr));
    DrawSuitGDI(hdc, card.suit, x + 12, y + 26, 12);

    if (card.isFrozen) {
        SetTextColor(hdc, RGB(0, 184, 212));
        TextOutA(hdc, x + CARD_W - 20, y + 4, "ICE", 3);
    } else {
        TextOutA(hdc, x + CARD_W - 14, y + CARD_H - 20, rankStr, lstrlenA(rankStr));
        DrawSuitGDI(hdc, card.suit, x + CARD_W - 12, y + CARD_H - 28, 12);
    }

    int cx = x + CARD_W / 2;
    int cy = y + CARD_H / 2;

    if (card.rank == 1) {
        DrawSuitGDI(hdc, card.suit, cx, cy, 32);
    } else if (card.rank >= 11) {
        DrawCourtCardGDI(hdc, card.rank, card.suit, x, y);
    } else {
        DrawSuitGDI(hdc, card.suit, cx, cy - 20, 16);
        DrawSuitGDI(hdc, card.suit, cx, cy + 20, 16);
        if (card.rank >= 4) {
            DrawSuitGDI(hdc, card.suit, cx - 18, cy - 20, 16);
            DrawSuitGDI(hdc, card.suit, cx + 18, cy - 20, 16);
            DrawSuitGDI(hdc, card.suit, cx - 18, cy + 20, 16);
            DrawSuitGDI(hdc, card.suit, cx + 18, cy + 20, 16);
        }
        if (card.rank == 3 || card.rank == 5 || card.rank == 7 || card.rank == 9) {
            DrawSuitGDI(hdc, card.suit, cx, cy, 16);
        }
    }
}

void DrawSlotOutline(HDC hdc, int x, int y, const char *label, int isHintDst) {
    RECT r = {x, y, x + CARD_W, y + CARD_H};
    HBRUSH slotBg = CreateSolidBrush(RGB(5, 30, 15));
    FillRect(hdc, &r, slotBg);
    DeleteObject(slotBg);

    HPEN slotPen = CreatePen(PS_SOLID, 2, isHintDst ? RGB(255, 145, 0) : RGB(212, 175, 55));
    HPEN oldPen = (HPEN)SelectObject(hdc, slotPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x, y, x + CARD_W, y + CARD_H, 8, 8);

    HBRUSH dotBrush = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, dotBrush);
    Ellipse(hdc, x + 4, y + 4, x + 8, y + 8);
    Ellipse(hdc, x + CARD_W - 8, y + 4, x + CARD_W - 4, y + 8);
    Ellipse(hdc, x + 4, y + CARD_H - 8, x + 8, y + CARD_H - 4);
    Ellipse(hdc, x + CARD_W - 8, y + CARD_H - 8, x + CARD_W - 4, y + CARD_H - 4);
    DeleteObject(dotBrush);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(slotPen);

    if (label) {
        SetTextColor(hdc, RGB(212, 175, 55));
        SetBkMode(hdc, TRANSPARENT);
        DrawTextA(hdc, label, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

// --- Window Procedure ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMENU hMenu = CreateMenu();
            
            HMENU hGameMenu = CreatePopupMenu();
            AppendMenu(hGameMenu, MF_STRING, ID_NEW_GAME, "New Game\tF2");
            AppendMenu(hGameMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hGameMenu, MF_STRING, ID_UNDO, "Undo\tCtrl+Z");
            AppendMenu(hGameMenu, MF_STRING, ID_REDO, "Redo\tCtrl+Y");
            AppendMenu(hGameMenu, MF_STRING, ID_HINT, "Hint\tCtrl+H");
            AppendMenu(hGameMenu, MF_STRING, ID_AUTOFINISH, "Auto-Finish\tCtrl+F");
            AppendMenu(hGameMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hGameMenu, MF_STRING, ID_STATS, "Statistics");
            AppendMenu(hGameMenu, MF_STRING, ID_RESET_STATS, "Reset Stats");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hGameMenu, "Game");

            HMENU hModeMenu = CreatePopupMenu();
            AppendMenu(hModeMenu, MF_STRING, ID_MODE_CLASSIC, "Classic Solitaire");
            AppendMenu(hModeMenu, MF_STRING, ID_MODE_VEGAS, "Vegas Money Mode");
            AppendMenu(hModeMenu, MF_STRING, ID_MODE_CAMPAIGN, "Campaign Mode (20 Stages)");
            AppendMenu(hModeMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hModeMenu, MF_STRING, ID_STAGE_PREV, "Previous Stage");
            AppendMenu(hModeMenu, MF_STRING, ID_STAGE_NEXT, "Next Stage");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hModeMenu, "Game Modes");

            HMENU hSkillMenu = CreatePopupMenu();
            AppendMenu(hSkillMenu, MF_STRING, ID_SKILL_WAND, "Magic Wand [W]");
            AppendMenu(hSkillMenu, MF_STRING, ID_SKILL_XRAY, "X-Ray Vision [X]");
            AppendMenu(hSkillMenu, MF_STRING, ID_SKILL_SHUFFLE, "Shuffle Stock [S]");
            AppendMenu(hSkillMenu, MF_STRING, ID_SKILL_UNDO, "Free Undo [U]");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSkillMenu, "Active Skills");

            HMENU hRulesMenu = CreatePopupMenu();
            AppendMenu(hRulesMenu, MF_STRING, ID_DRAW_1, "Draw 1 Card");
            AppendMenu(hRulesMenu, MF_STRING, ID_DRAW_3, "Draw 3 Cards");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRulesMenu, "Rules");

            HMENU hThemeMenu = CreatePopupMenu();
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_BLUE, "Deck: Navy Blue");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_CRIMSON, "Deck: Crimson Royal");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_EMERALD, "Deck: Emerald Forest");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_CYBER, "Deck: Cyber Dark");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_GOLD, "Deck: Gold Elegance");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hThemeMenu, "Deck Theme");

            HMENU hFeltMenu = CreatePopupMenu();
            AppendMenu(hFeltMenu, MF_STRING, ID_FELT_GREEN, "Felt: Classic Green");
            AppendMenu(hFeltMenu, MF_STRING, ID_FELT_SLATE, "Felt: Slate Dark");
            AppendMenu(hFeltMenu, MF_STRING, ID_FELT_INDIGO, "Felt: Royal Indigo");
            AppendMenu(hFeltMenu, MF_STRING, ID_FELT_CRIMSON, "Felt: Deep Crimson");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFeltMenu, "Felt Color");

            SetMenu(hwnd, hMenu);

            seed = GetTickCount();
            LoadStats();
            if (!LoadGameState()) {
                state.gameMode = 0;
                state.campaignStage = 1;
                state.drawMode = 1;
                NewGame(hwnd);
            }
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == ID_NEW_GAME) NewGame(hwnd);
            else if (id == ID_DRAW_1) { state.drawMode = 1; NewGame(hwnd); }
            else if (id == ID_DRAW_3) { state.drawMode = 3; NewGame(hwnd); }
            else if (id == ID_MODE_CLASSIC) { state.gameMode = 0; NewGame(hwnd); }
            else if (id == ID_MODE_VEGAS) { state.gameMode = 2; NewGame(hwnd); }
            else if (id == ID_MODE_CAMPAIGN) { state.gameMode = 1; NewGame(hwnd); }
            else if (id == ID_STAGE_PREV) {
                if (state.campaignStage > 1) { state.campaignStage--; state.gameMode = 1; NewGame(hwnd); }
            } else if (id == ID_STAGE_NEXT) {
                if (state.campaignStage < stats.maxCampaignStage && state.campaignStage < 20) {
                    state.campaignStage++; state.gameMode = 1; NewGame(hwnd);
                }
            } else if (id == ID_SKILL_WAND) UseMagicWand(hwnd);
            else if (id == ID_SKILL_XRAY) UseXRayVision(hwnd);
            else if (id == ID_SKILL_SHUFFLE) UseShuffleStock(hwnd);
            else if (id == ID_SKILL_UNDO) PerformUndo();
            else if (id == ID_UNDO) PerformUndo();
            else if (id == ID_REDO) PerformRedo();
            else if (id == ID_HINT) GiveHint(hwnd);
            else if (id == ID_AUTOFINISH) {
                if (CanAutoFinish()) {
                    autoFinishActive = 1;
                    SetTimer(hwnd, 2, 100, NULL);
                }
            } else if (id == ID_STATS) {
                char buf[512];
                int winrate = stats.played > 0 ? (stats.wins * 100 / stats.played) : 0;
                wsprintfA(buf, "Klondike Solitaire Statistics\n\nGames Played: %d\nWins: %d\nWin Rate: %d%%\nCurrent Streak: %d\nBest Streak: %d\nHigh Score: %d\nVegas Bankroll: $%d\nCampaign Max Stage: Stage %d / 20\nBest Time (Draw 1): %d s\nBest Time (Draw 3): %d s",
                    stats.played, stats.wins, winrate, stats.currentStreak, stats.bestStreak, stats.highScore, stats.vegasCash, stats.maxCampaignStage, stats.bestTimeD1, stats.bestTimeD3);
                MessageBoxA(hwnd, buf, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_RESET_STATS) {
                if (MessageBoxA(hwnd, "Reset all statistics?", "Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    ZeroMem(&stats, sizeof(SolitaireStats));
                    stats.maxCampaignStage = 1;
                    SaveStats();
                }
            } else if (id == ID_THEME_BLUE) themeId = 0;
            else if (id == ID_THEME_CRIMSON) themeId = 1;
            else if (id == ID_THEME_EMERALD) themeId = 2;
            else if (id == ID_THEME_CYBER) themeId = 3;
            else if (id == ID_THEME_GOLD) themeId = 4;
            else if (id == ID_FELT_GREEN) feltId = 0;
            else if (id == ID_FELT_SLATE) feltId = 1;
            else if (id == ID_FELT_INDIGO) feltId = 2;
            else if (id == ID_FELT_CRIMSON) feltId = 3;

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F2) NewGame(hwnd);
            else if (wParam == 'W' || wParam == 'w') UseMagicWand(hwnd);
            else if (wParam == 'X' || wParam == 'x') UseXRayVision(hwnd);
            else if (wParam == 'S' || wParam == 's') UseShuffleStock(hwnd);
            else if (wParam == 'U' || wParam == 'u') PerformUndo();
            else if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (wParam == 'Z') PerformUndo();
                else if (wParam == 'Y') PerformRedo();
                else if (wParam == 'H') GiveHint(hwnd);
                else if (wParam == 'F') {
                    if (CanAutoFinish()) { autoFinishActive = 1; SetTimer(hwnd, 2, 100, NULL); }
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (gameWon) return 0;
            if (!state.gameStarted) state.gameStarted = 1;

            int mx = LOWORD(lParam);
            int my = HIWORD(lParam) - 35;

            if (my < 0) return 0;

            ClearSelectionAndHints();

            // 1. Stock Click
            int stockX = GAP_X;
            int stockY = GAP_Y;
            if (mx >= stockX && mx <= stockX + CARD_W && my >= stockY && my <= stockY + CARD_H) {
                if (state.stock_cnt == 0) {
                    if (state.waste_cnt > 0) {
                        if (state.maxStockPasses > 0 && state.stockPasses >= state.maxStockPasses) {
                            MessageBeep(MB_ICONHAND);
                            return 0;
                        }
                        PushUndoState();
                        state.stockPasses++;
                        while (state.waste_cnt > 0) {
                            Card c = state.waste[--state.waste_cnt];
                            c.faceUp = 0;
                            state.stock[state.stock_cnt++] = c;
                        }
                    }
                } else {
                    PushUndoState();
                    int count = (state.drawMode < state.stock_cnt) ? state.drawMode : state.stock_cnt;
                    for (int i = 0; i < count; i++) {
                        Card c = state.stock[--state.stock_cnt];
                        c.faceUp = 1;
                        state.waste[state.waste_cnt++] = c;
                    }
                }
                state.moves++;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // 2. Waste Click
            int wasteX = stockX + CARD_W + GAP_X;
            int wasteY = stockY;
            if (state.waste_cnt > 0 && mx >= wasteX && mx <= wasteX + CARD_W && my >= wasteY && my <= wasteY + CARD_H) {
                selectedType = 0;
                selectedPile = 0;
                selectedCardIdx = state.waste_cnt - 1;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // 3. Foundations Click
            int foundationStartX = GAP_X + 3 * (CARD_W + GAP_X);
            for (int f = 0; f < 4; f++) {
                int fx = foundationStartX + f * (CARD_W + GAP_X);
                int fy = stockY;
                if (mx >= fx && mx <= fx + CARD_W && my >= fy && my <= fy + CARD_H) {
                    if (selectedType != -1) {
                        AttemptMove(selectedType, selectedPile, selectedCardIdx, 2, f, hwnd);
                    } else if (state.foundation_cnt[f] > 0) {
                        selectedType = 2;
                        selectedPile = f;
                        selectedCardIdx = state.foundation_cnt[f] - 1;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                }
            }

            // 4. Tableau Click
            int tableauStartY = stockY + CARD_H + GAP_Y;
            for (int t = 0; t < 7; t++) {
                int tx = GAP_X + t * (CARD_W + GAP_X);
                int tCount = state.tableau_cnt[t];

                if (tCount == 0) {
                    if (mx >= tx && mx <= tx + CARD_W && my >= tableauStartY && my <= tableauStartY + CARD_H) {
                        if (selectedType != -1) {
                            AttemptMove(selectedType, selectedPile, selectedCardIdx, 1, t, hwnd);
                        }
                        return 0;
                    }
                } else {
                    for (int c = tCount - 1; c >= 0; c--) {
                        int cy = tableauStartY + c * 24;
                        int cardHeight = (c == tCount - 1) ? CARD_H : 24;
                        if (mx >= tx && mx <= tx + CARD_W && my >= cy && my <= cy + cardHeight) {
                            Card card = state.tableau[t][c];
                            if (card.isFrozen) {
                                MessageBeep(MB_ICONHAND);
                                return 0;
                            }
                            if (!card.faceUp && c == tCount - 1) {
                                PushUndoState();
                                state.tableau[t][c].faceUp = 1;
                                if (!state.vegasRules) state.score += 5;
                                state.moves++;
                                InvalidateRect(hwnd, NULL, FALSE);
                                return 0;
                            }
                            if (card.faceUp) {
                                if (selectedType != -1) {
                                    AttemptMove(selectedType, selectedPile, selectedCardIdx, 1, t, hwnd);
                                } else {
                                    selectedType = 1;
                                    selectedPile = t;
                                    selectedCardIdx = c;
                                    InvalidateRect(hwnd, NULL, FALSE);
                                }
                                return 0;
                            }
                        }
                    }
                }
            }

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDBLCLK: {
            if (gameWon) return 0;
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam) - 35;

            int wasteX = GAP_X + CARD_W + GAP_X;
            int wasteY = GAP_Y;
            if (state.waste_cnt > 0 && mx >= wasteX && mx <= wasteX + CARD_W && my >= wasteY && my <= wasteY + CARD_H) {
                Card c = state.waste[state.waste_cnt - 1];
                for (int f = 0; f < 4; f++) {
                    if (CanMoveToFoundation(c, f)) {
                        AttemptMove(0, 0, state.waste_cnt - 1, 2, f, hwnd);
                        return 0;
                    }
                }
            }

            int tableauStartY = wasteY + CARD_H + GAP_Y;
            for (int t = 0; t < 7; t++) {
                int tx = GAP_X + t * (CARD_W + GAP_X);
                int tCount = state.tableau_cnt[t];
                if (tCount > 0) {
                    int topY = tableauStartY + (tCount - 1) * 24;
                    if (mx >= tx && mx <= tx + CARD_W && my >= topY && my <= topY + CARD_H) {
                        Card c = state.tableau[t][tCount - 1];
                        if (c.faceUp && !c.isFrozen) {
                            for (int f = 0; f < 4; f++) {
                                if (CanMoveToFoundation(c, f)) {
                                    AttemptMove(1, t, tCount - 1, 2, f, hwnd);
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) { // 1-sec timer
                if (state.gameStarted && !gameWon) {
                    state.timerSeconds++;
                    if (state.xrayTimer > 0) {
                        state.xrayTimer--;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 2) { // Auto finish step
                if (autoFinishActive) AutoFinishStep(hwnd);
            } else if (wParam == 3) { // Victory Cascade Waterfall Loop
                if (cascadeActive) {
                    cascadeFrame++;
                    RECT clientRc;
                    GetClientRect(hwnd, &clientRc);
                    int winW = clientRc.right;
                    int winH = clientRc.bottom;

                    HDC hdc = GetDC(hwnd);
                    for (int i = 0; i < 52; i++) {
                        CascadeCard *c = &cascadeCards[i];
                        if (cascadeFrame >= c->delay && !c->active && !c->finished) {
                            c->active = 1;
                        }
                        if (c->active) {
                            DrawCardGDI(hdc, c->card, (int)c->x, (int)c->y, 0, 0, 0);

                            c->x += c->vx;
                            c->y += c->vy;
                            c->vy += 0.85f;

                            if (c->y + CARD_H >= winH) {
                                c->y = (float)(winH - CARD_H);
                                c->vy = -c->vy * 0.82f;
                            }
                            if (c->x <= 0 || c->x + CARD_W >= winW) {
                                c->vx = -c->vx;
                            }
                            if (c->x < -100 || c->x > winW + 100 || c->y > winH + 150) {
                                c->active = 0;
                                c->finished = 1;
                            }
                        }
                    }
                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRc;
            GetClientRect(hwnd, &clientRc);
            int winW = clientRc.right;
            int winH = clientRc.bottom;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBm = CreateCompatibleBitmap(hdc, winW, winH);
            HBITMAP oldBm = (HBITMAP)SelectObject(memDC, memBm);

            // Draw Background Felt
            HBRUSH bgBrush = CreateSolidBrush(GetFeltColor());
            FillRect(memDC, &clientRc, bgBrush);
            DeleteObject(bgBrush);

            // Felt weave grid pattern
            HPEN weavePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            HPEN oldWeave = (HPEN)SelectObject(memDC, weavePen);
            for (int y = 35; y < winH; y += 6) {
                for (int x = (y % 12); x < winW; x += 12) {
                    SetPixel(memDC, x, y, RGB(18, 105, 53));
                }
            }
            SelectObject(memDC, oldWeave);
            DeleteObject(weavePen);

            // Draw Top Status Bar
            RECT statusRc = {0, 0, winW, 35};
            HBRUSH statusBrush = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(memDC, &statusRc, statusBrush);
            DeleteObject(statusBrush);

            SetTextColor(memDC, RGB(255, 215, 0));
            SetBkMode(memDC, TRANSPARENT);
            char statusText[320];

            if (state.gameMode == 1) {
                wsprintfA(statusText, "Time: %02d:%02d  Moves: %d  Score: %d  [Stg %d/20]  Wand(W):%d  XRay(X):%d  Shuf(S):%d",
                    state.timerSeconds / 60, state.timerSeconds % 60, state.moves, state.score, state.campaignStage,
                    state.wandCharges, state.xrayCharges, state.shuffleCharges);
            } else if (state.gameMode == 2) {
                wsprintfA(statusText, "Time: %02d:%02d  Moves: %d  Cash: $%d  Bank: $%d  Wand(W):%d  XRay(X):%d  Shuf(S):%d",
                    state.timerSeconds / 60, state.timerSeconds % 60, state.moves, state.score, stats.vegasCash,
                    state.wandCharges, state.xrayCharges, state.shuffleCharges);
            } else {
                wsprintfA(statusText, "Time: %02d:%02d  Moves: %d  Score: %d  (Draw %d)  Wand(W):%d  XRay(X):%d  Shuf(S):%d",
                    state.timerSeconds / 60, state.timerSeconds % 60, state.moves, state.score, state.drawMode,
                    state.wandCharges, state.xrayCharges, state.shuffleCharges);
            }
            TextOutA(memDC, 10, 8, statusText, lstrlenA(statusText));

            int stockX = GAP_X;
            int stockY = 35 + GAP_Y;

            // Stock Slot
            if (state.stock_cnt > 0) {
                Card dummy = {0, 0, 0, 0};
                DrawCardGDI(memDC, dummy, stockX, stockY, 0, 0, 0);
            } else {
                DrawSlotOutline(memDC, stockX, stockY, "Stock", 0);
            }

            // Waste Slot
            int wasteX = stockX + CARD_W + GAP_X;
            int wasteY = stockY;
            if (state.waste_cnt > 0) {
                Card topWaste = state.waste[state.waste_cnt - 1];
                int isSel = (selectedType == 0);
                int isHS = (hintSrcType == 0);
                DrawCardGDI(memDC, topWaste, wasteX, wasteY, isSel, isHS, 0);
            } else {
                DrawSlotOutline(memDC, wasteX, wasteY, "Waste", 0);
            }

            // Foundations
            const char *fLabels[4] = {"♥", "♦", "♣", "♠"};
            int foundationStartX = GAP_X + 3 * (CARD_W + GAP_X);
            for (int f = 0; f < 4; f++) {
                int fx = foundationStartX + f * (CARD_W + GAP_X);
                int fy = stockY;
                if (state.foundation_cnt[f] > 0) {
                    Card topF = state.foundations[f][state.foundation_cnt[f] - 1];
                    int isSel = (selectedType == 2 && selectedPile == f);
                    int isHD = (hintDstType == 2 && hintDstPile == f);
                    DrawCardGDI(memDC, topF, fx, fy, isSel, 0, isHD);
                } else {
                    int isHD = (hintDstType == 2 && hintDstPile == f);
                    const char *lbl = state.suitLocked ? fLabels[f] : "Ace";
                    DrawSlotOutline(memDC, fx, fy, lbl, isHD);
                }
            }

            // Tableau Columns
            int tableauStartY = stockY + CARD_H + GAP_Y;
            for (int t = 0; t < 7; t++) {
                int tx = GAP_X + t * (CARD_W + GAP_X);
                int tCount = state.tableau_cnt[t];

                if (tCount == 0) {
                    int isHD = (hintDstType == 1 && hintDstPile == t);
                    DrawSlotOutline(memDC, tx, tableauStartY, NULL, isHD);
                } else {
                    for (int c = 0; c < tCount; c++) {
                        Card card = state.tableau[t][c];
                        int cy = tableauStartY + c * 24;
                        int isSel = (selectedType == 1 && selectedPile == t && c >= selectedCardIdx);
                        int isHS = (hintSrcType == 1 && hintSrcPile == t && c == hintSrcIdx);
                        int isHD = (hintDstType == 1 && hintDstPile == t && c == tCount - 1);
                        DrawCardGDI(memDC, card, tx, cy, isSel, isHS, isHD);
                    }
                }
            }

            BitBlt(hdc, 0, 0, winW, winH, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBm);
            DeleteObject(memBm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            KillTimer(hwnd, 3);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSolitaireApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSolitaireApp", "KSolitaire - Klondike Solitaire", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 740, 700, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
