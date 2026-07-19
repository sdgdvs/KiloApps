#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define CELL_W 70
#define CELL_H 100
#define PAD 20
#define TOP_Y 50
#define TAB_Y 170
#define TAB_PAD_X 10
#define TAB_PAD_Y 25

typedef struct {
    int s; // 0=Spades, 1=Hearts, 2=Clubs, 3=Diamonds
    int r; // 1-13
    int color; // 0=Black, 1=Red
} Card;

Card deck[52];
Card freeCells[4];
int freeCellsOccupied[4];
int found[4]; // 0-13

Card tab[8][52];
int tabCount[8];

int selType = -1; // 0=free, 1=tab, -1=none
int selIdx = -1;
int selCardIdx = -1;
int won = 0;

void InitGame() {
    won = 0;
    selType = -1;
    for(int i=0; i<4; i++) {
        freeCellsOccupied[i] = 0;
        found[i] = 0;
    }
    for(int i=0; i<8; i++) {
        tabCount[i] = 0;
    }
    
    Card* deckPtrs[52];
    int c = 0;
    for(int s=0; s<4; s++) {
        for(int r=1; r<=13; r++) {
            deck[c].s = s;
            deck[c].r = r;
            deck[c].color = (s==1 || s==3) ? 1 : 0;
            deckPtrs[c] = &deck[c];
            c++;
        }
    }
    
    for(int i=51; i>0; i--) {
        int j = rand() % (i + 1);
        Card *t = deckPtrs[i];
        deckPtrs[i] = deckPtrs[j];
        deckPtrs[j] = t;
    }
    
    c = 0;
    for(int i=0; i<52; i++) {
        tab[c][tabCount[c]++] = *deckPtrs[i];
        c = (c + 1) % 8;
    }
}

int GetMaxMoveCount() {
    int emptyFree = 0;
    for(int i=0; i<4; i++) if(!freeCellsOccupied[i]) emptyFree++;
    int emptyTab = 0;
    for(int i=0; i<8; i++) if(tabCount[i] == 0) emptyTab++;
    return (emptyFree + 1) * (1 << emptyTab);
}

int CanMoveToTab(Card c, int tIdx) {
    if(tabCount[tIdx] == 0) return 1;
    Card top = tab[tIdx][tabCount[tIdx]-1];
    return c.color != top.color && c.r == top.r - 1;
}

int GetDraggableGroup(int tIdx, int startIdx) {
    if(startIdx >= tabCount[tIdx]) return 0;
    for(int i=startIdx+1; i<tabCount[tIdx]; i++) {
        Card prev = tab[tIdx][i-1];
        Card curr = tab[tIdx][i];
        if(curr.color == prev.color || curr.r != prev.r - 1) return 0;
    }
    return tabCount[tIdx] - startIdx;
}

void CheckWin(HWND hwnd) {
    if(found[0]==13 && found[1]==13 && found[2]==13 && found[3]==13) {
        won = 1;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void DrawCard(HDC hdc, int x, int y, Card c, int selected) {
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    HPEN pen = CreatePen(PS_SOLID, selected ? 3 : 1, selected ? RGB(255, 235, 59) : RGB(204, 204, 204));
    
    SelectObject(hdc, bg);
    SelectObject(hdc, pen);
    Rectangle(hdc, x, y, x + CELL_W, y + CELL_H);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, c.color ? RGB(211, 47, 47) : RGB(0, 0, 0));
    
    WCHAR *suits[] = {L"\x2660", L"\x2665", L"\x2663", L"\x2666"};
    WCHAR *ranks[] = {L"A",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9",L"10",L"J",L"Q",L"K"};
    WCHAR text[16];
    wsprintfW(text, L"%s%s", ranks[c.r-1], suits[c.s]);
    
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    RECT rT = {x+5, y+5, x+CELL_W-5, y+25};
    DrawTextW(hdc, text, -1, &rT, DT_LEFT | DT_TOP);
    
    RECT rB = {x+5, y+CELL_H-25, x+CELL_W-5, y+CELL_H-5};
    DrawTextW(hdc, text, -1, &rB, DT_RIGHT | DT_BOTTOM);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    DeleteObject(bg);
    DeleteObject(pen);
}

void DrawEmptyCell(HDC hdc, int x, int y, int isFound, int suitIdx) {
    HBRUSH bg = CreateSolidBrush(RGB(37, 37, 38));
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(85, 85, 85));
    SelectObject(hdc, bg);
    SelectObject(hdc, pen);
    Rectangle(hdc, x, y, x + CELL_W, y + CELL_H);
    DeleteObject(bg);
    DeleteObject(pen);
    
    if(isFound) {
        WCHAR *suits[] = {L"\x2660", L"\x2665", L"\x2663", L"\x2666"};
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (suitIdx==1||suitIdx==3) ? RGB(139, 0, 0) : RGB(68, 68, 68));
        HFONT hFont = CreateFontW(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        
        RECT r = {x, y, x+CELL_W, y+CELL_H};
        DrawTextW(hdc, suits[suitIdx], -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            InitGame();
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            HBRUSH hbrBg = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdcMem, &clientRect, hbrBg);
            DeleteObject(hbrBg);
            
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            HFONT hTitleFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hTitleFont);
            RECT titleRect = {0, 10, clientRect.right, 40};
            DrawTextW(hdcMem, L"KFreecell", -1, &titleRect, DT_CENTER | DT_TOP);
            if(won) {
                RECT winRect = {0, 10, clientRect.right - 20, 40};
                DrawTextW(hdcMem, L"You Win!", -1, &winRect, DT_RIGHT | DT_TOP);
            }
            SelectObject(hdcMem, hOldFont);
            DeleteObject(hTitleFont);
            
            // Draw Free Cells
            int group1X = PAD;
            for(int i=0; i<4; i++) {
                int x = group1X + i*(CELL_W + TAB_PAD_X);
                if(freeCellsOccupied[i]) {
                    DrawCard(hdcMem, x, TOP_Y, freeCells[i], (selType==0 && selIdx==i));
                } else {
                    DrawEmptyCell(hdcMem, x, TOP_Y, 0, 0);
                }
            }
            
            // Draw Foundations
            int group2X = clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X);
            for(int i=0; i<4; i++) {
                int x = group2X + i*(CELL_W + TAB_PAD_X);
                if(found[i] > 0) {
                    Card c = {i, found[i], (i==1||i==3)?1:0};
                    DrawCard(hdcMem, x, TOP_Y, c, 0);
                } else {
                    DrawEmptyCell(hdcMem, x, TOP_Y, 1, i);
                }
            }
            
            // Draw Tableau
            int totalTabW = 8 * CELL_W + 7 * TAB_PAD_X;
            int tabStartX = (clientRect.right - totalTabW) / 2;
            for(int i=0; i<8; i++) {
                int x = tabStartX + i*(CELL_W + TAB_PAD_X);
                if(tabCount[i] == 0) {
                    DrawEmptyCell(hdcMem, x, TAB_Y, 0, 0);
                } else {
                    for(int j=0; j<tabCount[i]; j++) {
                        int y = TAB_Y + j*TAB_PAD_Y;
                        int selected = (selType==1 && selIdx==i && j>=selCardIdx);
                        DrawCard(hdcMem, x, y, tab[i][j], selected);
                    }
                }
            }
            
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
            
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            if(won) return 0;
            int mx = (short)LOWORD(lParam);
            int my = (short)HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            int clickedType = -1; // 0=free, 1=tab, 2=found
            int clickedIdx = -1;
            int clickedCardIdx = 0;
            
            // Check Free Cells
            int group1X = PAD;
            for(int i=0; i<4; i++) {
                int x = group1X + i*(CELL_W + TAB_PAD_X);
                if(mx >= x && mx <= x+CELL_W && my >= TOP_Y && my <= TOP_Y+CELL_H) {
                    clickedType = 0;
                    clickedIdx = i;
                    break;
                }
            }
            
            // Check Foundations
            if(clickedType == -1) {
                int group2X = clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X);
                for(int i=0; i<4; i++) {
                    int x = group2X + i*(CELL_W + TAB_PAD_X);
                    if(mx >= x && mx <= x+CELL_W && my >= TOP_Y && my <= TOP_Y+CELL_H) {
                        clickedType = 2;
                        clickedIdx = i;
                        break;
                    }
                }
            }
            
            // Check Tableau
            if(clickedType == -1) {
                int totalTabW = 8 * CELL_W + 7 * TAB_PAD_X;
                int tabStartX = (clientRect.right - totalTabW) / 2;
                for(int i=0; i<8; i++) {
                    int x = tabStartX + i*(CELL_W + TAB_PAD_X);
                    if(mx >= x && mx <= x+CELL_W) {
                        if(tabCount[i] == 0) {
                            if(my >= TAB_Y && my <= TAB_Y+CELL_H) {
                                clickedType = 1;
                                clickedIdx = i;
                                clickedCardIdx = 0;
                                break;
                            }
                        } else {
                            for(int j=tabCount[i]-1; j>=0; j--) {
                                int y = TAB_Y + j*TAB_PAD_Y;
                                int h = (j == tabCount[i]-1) ? CELL_H : TAB_PAD_Y;
                                if(my >= y && my <= y+h) {
                                    clickedType = 1;
                                    clickedIdx = i;
                                    clickedCardIdx = j;
                                    break;
                                }
                            }
                            if(clickedType != -1) break;
                        }
                    }
                }
            }
            
            if(selType == -1) { // Select
                if(clickedType == 1 && tabCount[clickedIdx] > 0) {
                    int groupSize = GetDraggableGroup(clickedIdx, clickedCardIdx);
                    if(groupSize > 0 && groupSize <= GetMaxMoveCount()) {
                        selType = clickedType;
                        selIdx = clickedIdx;
                        selCardIdx = clickedCardIdx;
                    }
                } else if(clickedType == 0 && freeCellsOccupied[clickedIdx]) {
                    selType = clickedType;
                    selIdx = clickedIdx;
                    selCardIdx = 0;
                }
            } else { // Move
                int moved = 0;
                
                Card cardsToMove[52];
                int nToMove = 0;
                if(selType == 1) {
                    nToMove = tabCount[selIdx] - selCardIdx;
                    for(int i=0; i<nToMove; i++) {
                        cardsToMove[i] = tab[selIdx][selCardIdx + i];
                    }
                } else if(selType == 0) {
                    nToMove = 1;
                    cardsToMove[0] = freeCells[selIdx];
                }
                
                if(clickedType == 1) { // move to tableau
                    int emptyFree = 0;
                    for(int i=0; i<4; i++) if(!freeCellsOccupied[i]) emptyFree++;
                    int emptyTab = 0;
                    for(int i=0; i<8; i++) if(tabCount[i] == 0 && i != clickedIdx) emptyTab++;
                    int maxMove = (emptyFree + 1) * (1 << emptyTab);
                    
                    if(nToMove <= maxMove && CanMoveToTab(cardsToMove[0], clickedIdx)) {
                        for(int i=0; i<nToMove; i++) {
                            tab[clickedIdx][tabCount[clickedIdx]++] = cardsToMove[i];
                        }
                        moved = 1;
                    }
                } else if(clickedType == 0 && nToMove == 1 && !freeCellsOccupied[clickedIdx]) {
                    freeCells[clickedIdx] = cardsToMove[0];
                    freeCellsOccupied[clickedIdx] = 1;
                    moved = 1;
                } else if(clickedType == 2 && nToMove == 1 && cardsToMove[0].s == clickedIdx) {
                    if(cardsToMove[0].r == found[clickedIdx] + 1) {
                        found[clickedIdx] = cardsToMove[0].r;
                        moved = 1;
                    }
                }
                
                if(moved) {
                    if(selType == 1) {
                        tabCount[selIdx] = selCardIdx;
                    } else if(selType == 0) {
                        freeCellsOccupied[selIdx] = 0;
                    }
                    CheckWin(hwnd);
                }
                selType = -1;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_KEYDOWN:
            if(wParam == 'R' || wParam == 'N') {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if(wParam == VK_ESCAPE) {
                selType = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"KFreecellClass";
    RegisterClassW(&wc);
    
    HWND hwnd = CreateWindowW(L"KFreecellClass", L"KFreecell",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 700,
        NULL, NULL, hInstance, NULL);
        
    MSG msg;
    while(GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ExitProcess(0);
}
