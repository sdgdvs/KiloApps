#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int s; // 0:S, 1:H, 2:C, 3:D
    int r; // 1-13
} Card;

Card deck[52];
Card freecells[4]; // r=0 if empty
int foundations[4]; // max r (1-13)
Card tab[8][20];
int tab_counts[8];

int sel_type = -1; // 0=free, 1=tab, -1=none
int sel_idx = -1;
int sel_card_idx = -1;

#define CARD_W 60
#define CARD_H 85
#define MARGIN 10
#define TAB_DY 20

void Shuffle() {
    int i, j;
    for (i = 0; i < 52; i++) {
        deck[i].s = i % 4;
        deck[i].r = (i / 4) + 1;
    }
    for (i = 51; i > 0; i--) {
        j = rand() % (i + 1);
        Card t = deck[i]; deck[i] = deck[j]; deck[j] = t;
    }
}

void InitGame() {
    Shuffle();
    memset(freecells, 0, sizeof(freecells));
    memset(foundations, 0, sizeof(foundations));
    memset(tab_counts, 0, sizeof(tab_counts));
    
    int i;
    for (i = 0; i < 52; i++) {
        int c = i % 8;
        tab[c][tab_counts[c]++] = deck[i];
    }
    sel_type = -1;
}

int GetColor(int s) {
    return (s == 1 || s == 3) ? 1 : 0; // 1=red, 0=black
}

int CanMoveToTab(Card c, int dst_idx) {
    if (tab_counts[dst_idx] == 0) return 1;
    Card top = tab[dst_idx][tab_counts[dst_idx] - 1];
    if (GetColor(c.s) != GetColor(top.s) && c.r == top.r - 1) return 1;
    return 0;
}

int CanMoveToFound(Card c) {
    if (c.r == foundations[c.s] + 1) return 1;
    return 0;
}

int GetMaxMove() {
    int empty_free = 0, empty_tab = 0;
    for(int i=0; i<4; i++) if (freecells[i].r == 0) empty_free++;
    for(int i=0; i<8; i++) if (tab_counts[i] == 0) empty_tab++;
    return (empty_free + 1) * (1 << empty_tab);
}

void HandleClick(int type, int idx, int c_idx) {
    if (sel_type == -1) { // Select
        if (type == 1) { // Tab
            int n = tab_counts[idx];
            if (c_idx < 0 || c_idx >= n) return;
            // check valid stack
            int valid = 1;
            for (int i = c_idx + 1; i < n; i++) {
                if (GetColor(tab[idx][i].s) == GetColor(tab[idx][i-1].s) || tab[idx][i].r != tab[idx][i-1].r - 1) {
                    valid = 0; break;
                }
            }
            if (valid && (n - c_idx) <= GetMaxMove()) {
                sel_type = type; sel_idx = idx; sel_card_idx = c_idx;
            }
        } else if (type == 0) { // Free
            if (freecells[idx].r > 0) {
                sel_type = type; sel_idx = idx; sel_card_idx = 0;
            }
        }
    } else { // Move
        Card cards_to_move[20];
        int n_move = 0;
        
        if (sel_type == 1) {
            n_move = tab_counts[sel_idx] - sel_card_idx;
            for(int i=0; i<n_move; i++) cards_to_move[i] = tab[sel_idx][sel_card_idx + i];
        } else if (sel_type == 0) {
            n_move = 1;
            cards_to_move[0] = freecells[sel_idx];
        }
        
        int moved = 0;
        if (type == 1) { // to Tab
            int max_move = GetMaxMove();
            if (tab_counts[idx] == 0) {
                int empty_free = 0, empty_tab = 0;
                for(int i=0; i<4; i++) if (freecells[i].r == 0) empty_free++;
                for(int i=0; i<8; i++) if (tab_counts[i] == 0 && i != idx) empty_tab++;
                max_move = (empty_free + 1) * (1 << empty_tab);
            }
            if (n_move <= max_move && CanMoveToTab(cards_to_move[0], idx)) {
                for(int i=0; i<n_move; i++) tab[idx][tab_counts[idx]++] = cards_to_move[i];
                moved = 1;
            }
        } else if (type == 0 && n_move == 1) { // to Free
            if (freecells[idx].r == 0) {
                freecells[idx] = cards_to_move[0];
                moved = 1;
            }
        } else if (type == 2 && n_move == 1) { // to Found
            if (cards_to_move[0].s == idx && CanMoveToFound(cards_to_move[0])) {
                foundations[idx] = cards_to_move[0].r;
                moved = 1;
            }
        }
        
        if (moved) {
            if (sel_type == 1) tab_counts[sel_idx] = sel_card_idx;
            else if (sel_type == 0) freecells[sel_idx].r = 0;
        }
        sel_type = -1;
    }
}

void DrawCard(HDC hdc, int x, int y, Card c, int is_sel) {
    if (c.r == 0) return;
    
    HBRUSH bg = CreateSolidBrush(RGB(255,255,255));
    HPEN pen = CreatePen(PS_SOLID, is_sel ? 3 : 1, is_sel ? RGB(255,215,0) : RGB(100,100,100));
    SelectObject(hdc, bg); SelectObject(hdc, pen);
    Rectangle(hdc, x, y, x + CARD_W, y + CARD_H);
    DeleteObject(bg); DeleteObject(pen);
    
    const char* suits[] = {"S", "H", "C", "D"};
    const char* ranks[] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
    
    char text[10];
    sprintf(text, "%s %s", ranks[c.r-1], suits[c.s]);
    
    SetTextColor(hdc, GetColor(c.s) ? RGB(200,0,0) : RGB(0,0,0));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x + 5, y + 5, text, strlen(text));
}

void DrawEmptyCell(HDC hdc, int x, int y, int is_found, int suit) {
    HBRUSH bg = CreateSolidBrush(RGB(40,40,40));
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(80,80,80));
    SelectObject(hdc, bg); SelectObject(hdc, pen);
    Rectangle(hdc, x, y, x + CARD_W, y + CARD_H);
    DeleteObject(bg); DeleteObject(pen);
    
    if (is_found) {
        const char* suits[] = {"S", "H", "C", "D"};
        SetTextColor(hdc, RGB(80,80,80));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, x + 20, y + 30, suits[suit], 1);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            InitGame();
            return 0;
            
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            // Check Freecells
            for(int i=0; i<4; i++) {
                int cx = MARGIN + i * (CARD_W + MARGIN);
                int cy = MARGIN;
                if (mx >= cx && mx <= cx + CARD_W && my >= cy && my <= cy + CARD_H) {
                    HandleClick(0, i, 0);
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                }
            }
            
            // Check Foundations
            for(int i=0; i<4; i++) {
                int cx = 800 - MARGIN - (4-i) * (CARD_W + MARGIN);
                int cy = MARGIN;
                if (mx >= cx && mx <= cx + CARD_W && my >= cy && my <= cy + CARD_H) {
                    HandleClick(2, i, 0);
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                }
            }
            
            // Check Tableau
            for(int i=0; i<8; i++) {
                int cx = MARGIN + i * (CARD_W + MARGIN);
                int cy = MARGIN + CARD_H + MARGIN * 2;
                if (mx >= cx && mx <= cx + CARD_W) {
                    if (tab_counts[i] == 0) {
                        if (my >= cy && my <= cy + CARD_H) {
                            HandleClick(1, i, 0);
                            InvalidateRect(hwnd, NULL, TRUE);
                            return 0;
                        }
                    } else {
                        int c_idx = -1;
                        for(int j=tab_counts[i]-1; j>=0; j--) {
                            int card_y = cy + j * TAB_DY;
                            int card_h = (j == tab_counts[i]-1) ? CARD_H : TAB_DY;
                            if (my >= card_y && my < card_y + card_h) {
                                c_idx = j;
                                break;
                            }
                        }
                        if (c_idx != -1) {
                            HandleClick(1, i, c_idx);
                            InvalidateRect(hwnd, NULL, TRUE);
                            return 0;
                        }
                    }
                }
            }
            
            // Deselect on empty area click
            sel_type = -1;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Dark theme background
            HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
            
            // Draw Freecells
            for(int i=0; i<4; i++) {
                int cx = MARGIN + i * (CARD_W + MARGIN);
                int cy = MARGIN;
                if (freecells[i].r == 0) DrawEmptyCell(hdc, cx, cy, 0, 0);
                else DrawCard(hdc, cx, cy, freecells[i], (sel_type == 0 && sel_idx == i));
            }
            
            // Draw Foundations
            for(int i=0; i<4; i++) {
                int cx = 800 - MARGIN - (4-i) * (CARD_W + MARGIN);
                int cy = MARGIN;
                if (foundations[i] == 0) DrawEmptyCell(hdc, cx, cy, 1, i);
                else {
                    Card c = {i, foundations[i]};
                    DrawCard(hdc, cx, cy, c, 0);
                }
            }
            
            // Draw Tableau
            for(int i=0; i<8; i++) {
                int cx = MARGIN + i * (CARD_W + MARGIN);
                int cy = MARGIN + CARD_H + MARGIN * 2;
                if (tab_counts[i] == 0) {
                    DrawEmptyCell(hdc, cx, cy, 0, 0);
                } else {
                    for(int j=0; j<tab_counts[i]; j++) {
                        int is_sel = (sel_type == 1 && sel_idx == i && j >= sel_card_idx);
                        DrawCard(hdc, cx, cy + j * TAB_DY, tab[i][j], is_sel);
                    }
                }
            }
            
            // Check win
            int win = 1;
            for(int i=0; i<4; i++) if(foundations[i] != 13) win = 0;
            if(win) {
                SetTextColor(hdc, RGB(255,255,0));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 350, 50, "YOU WIN!", 8);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KFreecellClass";
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KFreecell",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) return 0;
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
