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

#define CARD_W 80
#define CARD_H 110
#define GAP_X  12
#define GAP_Y  15

typedef struct {
    int suit;   // 0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades
    int rank;   // 1 (Ace) to 13 (King)
    int faceUp;
} Card;

typedef struct {
    Card stock[52];
    int stock_cnt;

    Card waste[52];
    int waste_cnt;

    Card foundations[4][13];
    int foundation_cnt[4];

    Card tableau[7][21];
    int tableau_cnt[7];

    int moves;
    int score;
    int timerSeconds;
    int gameStarted;
    int drawMode; // 1 or 3
} SolitaireState;

typedef struct {
    int played;
    int wins;
    int currentStreak;
    int bestStreak;
    int highScore;
    int bestTimeD1;
    int bestTimeD3;
} SolitaireStats;

// --- Global Variables ---
SolitaireState state;
SolitaireState undoStack[256];
int undoCount = 0;
SolitaireState redoStack[256];
int redoCount = 0;

SolitaireStats stats = {0};
int themeId = 0; // 0=Blue, 1=Crimson, 2=Emerald, 3=Cyber, 4=Gold
int feltId = 0;  // 0=Green, 1=Slate, 2=Indigo, 3=Crimson
int selectedType = -1; // 0=waste, 1=tableau, 2=foundation
int selectedPile = -1;
int selectedCardIdx = -1;

int hintSrcType = -1, hintSrcPile = -1, hintSrcIdx = -1;
int hintDstType = -1, hintDstPile = -1;

int autoFinishActive = 0;
int gameWon = 0;

// Random Seed generator
unsigned int seed = 1337;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

// --- Helpers ---
COLORREF GetFeltColor() {
    switch(feltId) {
        case 1: return RGB(30, 41, 59);    // Slate Dark
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
    HANDLE hFile = CreateFileA("ksolitaire.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &stats, sizeof(SolitaireStats), &readBytes, NULL);
        CloseHandle(hFile);
    }
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
        undoStack[undoCount++] = state;
    } else {
        for (int i = 0; i < 255; i++) undoStack[i] = undoStack[i+1];
        undoStack[255] = state;
    }
    redoCount = 0; // Clear redo
    SaveGameState();
}

void PerformUndo() {
    if (undoCount == 0) return;
    if (redoCount < 256) redoStack[redoCount++] = state;
    state = undoStack[--undoCount];
    selectedType = -1;
    hintSrcType = -1;
    SaveGameState();
}

void PerformRedo() {
    if (redoCount == 0) return;
    if (undoCount < 256) undoStack[undoCount++] = state;
    state = redoStack[--redoCount];
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
    state.score = 0;
    state.timerSeconds = 0;
    state.gameStarted = 0;
    if (state.drawMode != 1 && state.drawMode != 3) state.drawMode = 1;

    undoCount = 0;
    redoCount = 0;
    gameWon = 0;
    autoFinishActive = 0;
    ClearSelectionAndHints();

    Card deck[52];
    int idx = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 1; r <= 13; r++) {
            deck[idx].suit = s;
            deck[idx].rank = r;
            deck[idx].faceUp = 0;
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

    // Foundations & Waste
    for (int i = 0; i < 4; i++) state.foundation_cnt[i] = 0;
    state.waste_cnt = 0;

    // Tableau
    idx = 0;
    for (int col = 0; col < 7; col++) {
        state.tableau_cnt[col] = 0;
        for (int row = 0; row <= col; row++) {
            Card c = deck[idx++];
            if (row == col) c.faceUp = 1;
            state.tableau[col][state.tableau_cnt[col]++] = c;
        }
    }

    // Remaining to Stock
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
    if (card.suit != fIdx) return 0;
    if (state.foundation_cnt[fIdx] == 0) return (card.rank == 1); // Ace
    Card topCard = state.foundations[fIdx][state.foundation_cnt[fIdx] - 1];
    return (card.rank == topCard.rank + 1);
}

int CanMoveToTableau(Card topMoveCard, int tIdx) {
    if (state.tableau_cnt[tIdx] == 0) return (topMoveCard.rank == 13); // King
    Card targetCard = state.tableau[tIdx][state.tableau_cnt[tIdx] - 1];
    if (!targetCard.faceUp) return 0;
    if (IsRedSuit(targetCard.suit) == IsRedSuit(topMoveCard.suit)) return 0;
    return (targetCard.rank == topMoveCard.rank + 1);
}

void CheckWin(HWND hwnd) {
    int total = 0;
    for (int i = 0; i < 4; i++) total += state.foundation_cnt[i];
    if (total == 52 && !gameWon) {
        gameWon = 1;
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2); // Auto Finish Timer

        stats.played++;
        stats.wins++;
        stats.currentStreak++;
        if (stats.currentStreak > stats.bestStreak) stats.bestStreak = stats.currentStreak;
        if (state.score > stats.highScore) stats.highScore = state.score;

        if (state.drawMode == 1) {
            if (stats.bestTimeD1 == 0 || state.timerSeconds < stats.bestTimeD1) stats.bestTimeD1 = state.timerSeconds;
        } else {
            if (stats.bestTimeD3 == 0 || state.timerSeconds < stats.bestTimeD3) stats.bestTimeD3 = state.timerSeconds;
        }

        SaveStats();

        char msg[256];
        wsprintfA(msg, "Congratulations! You completed Klondike Solitaire!\n\nFinal Score: %d\nTime: %d seconds\nMoves: %d", state.score, state.timerSeconds, state.moves);
        MessageBoxA(hwnd, msg, "KSolitaire Victory!", MB_OK | MB_ICONINFORMATION);
    }
}

int AttemptMove(int srcType, int srcPile, int srcIdx, int dstType, int dstPile, HWND hwnd) {
    Card cardsToMove[21];
    int moveCount = 0;

    if (srcType == 0) { // Waste
        if (state.waste_cnt == 0) return 0;
        cardsToMove[0] = state.waste[state.waste_cnt - 1];
        moveCount = 1;
    } else if (srcType == 1) { // Tableau
        if (srcIdx < 0 || srcIdx >= state.tableau_cnt[srcPile]) return 0;
        moveCount = state.tableau_cnt[srcPile] - srcIdx;
        for (int i = 0; i < moveCount; i++) {
            cardsToMove[i] = state.tableau[srcPile][srcIdx + i];
        }
    } else if (srcType == 2) { // Foundation
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

    // Remove from source
    if (srcType == 0) state.waste_cnt--;
    else if (srcType == 1) state.tableau_cnt[srcPile] -= moveCount;
    else if (srcType == 2) state.foundation_cnt[srcPile]--;

    // Add to destination
    if (dstType == 2) {
        state.foundations[dstPile][state.foundation_cnt[dstPile]++] = cardsToMove[0];
        state.score += (srcType == 0 ? 10 : (srcType == 1 ? 10 : 0));
        MessageBeep(MB_OK);
    } else if (dstType == 1) {
        for (int i = 0; i < moveCount; i++) {
            state.tableau[dstPile][state.tableau_cnt[dstPile]++] = cardsToMove[i];
        }
        state.score += (srcType == 0 ? 5 : (srcType == 2 ? -15 : 0));
    }

    // Auto flip top card of source tableau
    if (srcType == 1 && state.tableau_cnt[srcPile] > 0) {
        Card *top = &state.tableau[srcPile][state.tableau_cnt[srcPile] - 1];
        if (!top->faceUp) {
            top->faceUp = 1;
            state.score += 5;
        }
    }

    state.moves++;
    ClearSelectionAndHints();
    InvalidateRect(hwnd, NULL, FALSE);
    CheckWin(hwnd);
    return 1;
}

// --- Smart Hint System ---
void GiveHint(HWND hwnd) {
    ClearSelectionAndHints();
    if (gameWon) return;

    // 1. Tableau to Foundation
    for (int t = 0; t < 7; t++) {
        if (state.tableau_cnt[t] > 0) {
            Card c = state.tableau[t][state.tableau_cnt[t] - 1];
            if (c.faceUp) {
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

    // 2. Waste to Foundation
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

    // 3. Move Tableau card revealing face-down card
    for (int srcT = 0; srcT < 7; srcT++) {
        int firstFaceUp = -1;
        for (int i = 0; i < state.tableau_cnt[srcT]; i++) {
            if (state.tableau[srcT][i].faceUp) { firstFaceUp = i; break; }
        }
        if (firstFaceUp > 0) {
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

    // 4. Waste to Tableau
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
            if (!state.tableau[t][i].faceUp) return 0;
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

// --- GDI Rendering ---
void DrawCardGDI(HDC hdc, Card card, int x, int y, int isSelected, int isHintSrc, int isHintDst) {
    RECT r = {x, y, x + CARD_W, y + CARD_H};

    if (!card.faceUp) {
        // Face down card
        HBRUSH bg = CreateSolidBrush(GetCardBackBg());
        FillRect(hdc, &r, bg);
        DeleteObject(bg);
        
        HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, x, y, x + CARD_W, y + CARD_H, 6, 6);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(borderPen);
        return;
    }

    // Face up card
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    HPEN cardPen = CreatePen(PS_SOLID, (isSelected || isHintSrc || isHintDst) ? 3 : 1, 
        isSelected ? RGB(255, 235, 59) : (isHintSrc ? RGB(0, 230, 118) : (isHintDst ? RGB(255, 145, 0) : RGB(100, 100, 100))));
    HPEN oldPen = (HPEN)SelectObject(hdc, cardPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x, y, x + CARD_W, y + CARD_H, 6, 6);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(cardPen);

    // Text & Suits
    COLORREF textColor = IsRedSuit(card.suit) ? RGB(211, 47, 47) : RGB(33, 33, 33);
    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);

    char rankStr[4] = {0};
    if (card.rank == 1) wsprintfA(rankStr, "A");
    else if (card.rank == 11) wsprintfA(rankStr, "J");
    else if (card.rank == 12) wsprintfA(rankStr, "Q");
    else if (card.rank == 13) wsprintfA(rankStr, "K");
    else wsprintfA(rankStr, "%d", card.rank);

    char suitStr[4] = {0};
    if (card.suit == 0) wsprintfA(suitStr, "H");      // Hearts
    else if (card.suit == 1) wsprintfA(suitStr, "D"); // Diamonds
    else if (card.suit == 2) wsprintfA(suitStr, "C"); // Clubs
    else if (card.suit == 3) wsprintfA(suitStr, "S"); // Spades

    char textBuf[16];
    wsprintfA(textBuf, "%s%s", rankStr, suitStr);

    TextOutA(hdc, x + 5, y + 5, textBuf, lstrlenA(textBuf));
    TextOutA(hdc, x + CARD_W / 2 - 6, y + CARD_H / 2 - 8, suitStr, lstrlenA(suitStr));
}

void DrawSlotOutline(HDC hdc, int x, int y, const char *label, int isHintDst) {
    RECT r = {x, y, x + CARD_W, y + CARD_H};
    HPEN slotPen = CreatePen(PS_DASH, 1, isHintDst ? RGB(255, 145, 0) : RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, slotPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x, y, x + CARD_W, y + CARD_H, 6, 6);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(slotPen);

    if (label) {
        SetTextColor(hdc, RGB(255, 255, 255));
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
                state.drawMode = 1;
                NewGame(hwnd);
            }
            SetTimer(hwnd, 1, 1000, NULL); // 1 sec Timer
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == ID_NEW_GAME) NewGame(hwnd);
            else if (id == ID_DRAW_1) { state.drawMode = 1; NewGame(hwnd); }
            else if (id == ID_DRAW_3) { state.drawMode = 3; NewGame(hwnd); }
            else if (id == ID_UNDO) PerformUndo();
            else if (id == ID_REDO) PerformRedo();
            else if (id == ID_HINT) GiveHint(hwnd);
            else if (id == ID_AUTOFINISH) {
                if (CanAutoFinish()) {
                    autoFinishActive = 1;
                    SetTimer(hwnd, 2, 100, NULL); // 100ms step timer
                }
            } else if (id == ID_STATS) {
                char buf[512];
                int winrate = stats.played > 0 ? (stats.wins * 100 / stats.played) : 0;
                wsprintfA(buf, "Klondike Solitaire Statistics\n\nGames Played: %d\nWins: %d\nWin Rate: %d%%\nCurrent Streak: %d\nBest Streak: %d\nHigh Score: %d\nBest Time (Draw 1): %d s\nBest Time (Draw 3): %d s",
                    stats.played, stats.wins, winrate, stats.currentStreak, stats.bestStreak, stats.highScore, stats.bestTimeD1, stats.bestTimeD3);
                MessageBoxA(hwnd, buf, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_RESET_STATS) {
                if (MessageBoxA(hwnd, "Reset all statistics?", "Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    SolitaireStats empty = {0};
                    stats = empty;
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
            int my = HIWORD(lParam) - 35; // Adjust for top status bar

            if (my < 0) return 0;

            ClearSelectionAndHints();

            // 1. Stock Click
            int stockX = GAP_X;
            int stockY = GAP_Y;
            if (mx >= stockX && mx <= stockX + CARD_W && my >= stockY && my <= stockY + CARD_H) {
                PushUndoState();
                if (state.stock_cnt == 0) {
                    while (state.waste_cnt > 0) {
                        Card c = state.waste[--state.waste_cnt];
                        c.faceUp = 0;
                        state.stock[state.stock_cnt++] = c;
                    }
                } else {
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
                    // Empty column click
                    if (mx >= tx && mx <= tx + CARD_W && my >= tableauStartY && my <= tableauStartY + CARD_H) {
                        if (selectedType != -1) {
                            AttemptMove(selectedType, selectedPile, selectedCardIdx, 1, t, hwnd);
                        }
                        return 0;
                    }
                } else {
                    for (int c = tCount - 1; c >= 0; c--) {
                        int cy = tableauStartY + c * 20;
                        int cardHeight = (c == tCount - 1) ? CARD_H : 20;
                        if (mx >= tx && mx <= tx + CARD_W && my >= cy && my <= cy + cardHeight) {
                            Card card = state.tableau[t][c];
                            if (!card.faceUp && c == tCount - 1) {
                                // Flip top card!
                                PushUndoState();
                                state.tableau[t][c].faceUp = 1;
                                state.score += 5;
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

            // Double click Waste or Tableau to Auto-Move to Foundation
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
                    int topY = tableauStartY + (tCount - 1) * 20;
                    if (mx >= tx && mx <= tx + CARD_W && my >= topY && my <= topY + CARD_H) {
                        Card c = state.tableau[t][tCount - 1];
                        if (c.faceUp) {
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
            if (wParam == 1) { // 1-sec game timer
                if (state.gameStarted && !gameWon) {
                    state.timerSeconds++;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 2) { // Auto finish step
                if (autoFinishActive) AutoFinishStep(hwnd);
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

            // Draw Top Status Bar
            RECT statusRc = {0, 0, winW, 35};
            HBRUSH statusBrush = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(memDC, &statusRc, statusBrush);
            DeleteObject(statusBrush);

            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            char statusText[256];
            wsprintfA(statusText, "Time: %02d:%02d   Moves: %d   Score: %d   (Draw %d)   Wins: %d",
                state.timerSeconds / 60, state.timerSeconds % 60, state.moves, state.score, state.drawMode, stats.wins);
            TextOutA(memDC, 15, 8, statusText, lstrlenA(statusText));

            // Offset for Game Table below status bar
            int stockX = GAP_X;
            int stockY = 35 + GAP_Y;

            // Draw Stock Slot
            if (state.stock_cnt > 0) {
                Card dummy = {0, 0, 0};
                DrawCardGDI(memDC, dummy, stockX, stockY, 0, 0, 0);
            } else {
                DrawSlotOutline(memDC, stockX, stockY, "Stock", 0);
            }

            // Draw Waste Slot
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

            // Draw Foundations
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
                    DrawSlotOutline(memDC, fx, fy, fLabels[f], isHD);
                }
            }

            // Draw Tableau Columns
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
                        int cy = tableauStartY + c * 20;
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

    HWND hwnd = CreateWindowEx(0, "KSolitaireApp", "KSolitaire - Klondike", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 720, 680, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
