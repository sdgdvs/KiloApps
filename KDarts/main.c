#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846
#define TIMER_ID 1
#define CX 325
#define CY 380
#define BOARD_R 300
#define R 220

#include <stdint.h>

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // whoosh
        Beep(800, 30);
        Beep(400, 30);
        Beep(200, 30);
    } else if (type == 2) { // thud
        Beep(100, 50);
    } else if (type == 3) { // cheer
        Beep(440, 100);
        Beep(554, 100);
        Beep(659, 200);
    }
    return 0;
}

void PlayGameSound(int type) {
    CreateThread(NULL, 0, PlaySoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

typedef struct {
    int targetX, targetY;
    float x, y;
    int pts;
    int number;
    float progress;
    int animating;
} Dart;

int gameMode = 0; // 0=501, 1=Cricket
int aiDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int currentPlayer = 0; // 0=Player, 1=AI
int cricketHits[2][7] = {{0}}; // 20, 19, 18, 17, 16, 15, 25
int totalDarts[2] = {0};

int scores[2] = {501, 501};
int prevScores[2] = {501, 501};
int dartsLeft = 3;
Dart darts[3];
int dartsCount = 0;
int gameState = 0; // 0=PLAYING, 1=TURN_END, 2=WON
int mouseX = 325, mouseY = 380;
int wobbleX = 0, wobbleY = 0;
float t = 0.0f;
int aiTimer = 0;
char statusMsg[100] = "Game On! Player Turn - Throw 3 Darts";

void SetMode(int mode);
void NextTurn(HWND hwnd);
void ThrowDart(HWND hwnd, int tx, int ty, int isAI);

void SetMode(int mode) {
    gameMode = mode;
    gameState = 0;
    dartsCount = 0;
    dartsLeft = 3;
    currentPlayer = 0;
    totalDarts[0] = 0;
    totalDarts[1] = 0;
    if (mode == 0) {
        scores[0] = 501; scores[1] = 501;
        prevScores[0] = 501; prevScores[1] = 501;
        sprintf(statusMsg, "Game On! Player Turn - Throw 3 Darts");
    } else {
        for(int i=0; i<7; i++) { cricketHits[0][i] = 0; cricketHits[1][i] = 0; }
        sprintf(statusMsg, "Player Turn - Close 15-20 and Bullseye");
    }
}

void GetHitDetails(int x, int y, int* number, int* mult) {
    float dx = (float)(x - CX);
    float dy = (float)(y - CY);
    float d = sqrtf(dx*dx + dy*dy) / (float)R;
    
    *number = 0;
    *mult = 0;
    
    if (d > 1.0f) return;
    if (d <= 0.037f) { *number = 25; *mult = 2; return; }
    if (d <= 0.093f) { *number = 25; *mult = 1; return; }
    
    *mult = 1;
    if (d >= 0.582f && d <= 0.629f) *mult = 3;
    if (d >= 0.952f && d <= 1.0f) *mult = 2;
    
    float angle = atan2f(dy, dx) + (float)PI / 2.0f;
    if (angle < 0) angle += 2.0f * (float)PI;
    
    int idx = (int)floorf((angle + (float)PI / 20.0f) / ((float)PI / 10.0f)) % 20;
    int scores[] = {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
    *number = scores[idx];
}

void DrawPieSlice(HDC hdc, int r, float a1, float a2, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
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
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    Ellipse(hdc, CX - r, CY - r, CX + r, CY + r);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void NextTurn(HWND hwnd) {
    if (gameState == 1) {
        dartsCount = 0;
        dartsLeft = 3;
        prevScores[currentPlayer] = scores[currentPlayer];
        currentPlayer = 1 - currentPlayer;
        gameState = 0;
        if (currentPlayer == 0) sprintf(statusMsg, "Player's Turn - Throw Dart 1");
        else sprintf(statusMsg, "AI's Turn");
    } else if (gameState == 2) {
        SetMode(gameMode);
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void ThrowDart(HWND hwnd, int tx, int ty, int isAI) {
    if (isAI) {
        int targetNum = 20;
        int targetMult = 3;
        
        if (gameMode == 0) {
            if (scores[1] > 60) { targetNum = 20; targetMult = 3; }
            else {
                if (scores[1] <= 20) { targetNum = scores[1]; targetMult = 1; }
                else { targetNum = 20; targetMult = 1; }
            }
        } else {
            int targets[] = {20,19,18,17,16,15,25};
            for (int i=0; i<7; i++) {
                if (cricketHits[1][i] < 3) {
                    targetNum = targets[i];
                    targetMult = (targetNum == 25) ? 2 : 3;
                    break;
                }
            }
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
        if (aiDifficulty == 0) errorMag = 120.0f;
        else if (aiDifficulty == 2) errorMag = 20.0f;
        
        float rx = (((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) - 1.5f) * errorMag;
        float ry = (((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) + ((float)rand()/RAND_MAX) - 1.5f) * errorMag;
        
        tx = (int)(idealX + rx);
        ty = (int)(idealY + ry);
    }
    
    int number, mult;
    GetHitDetails(tx, ty, &number, &mult);
    int pts = number * mult;
    
    if (dartsCount < 3) {
        PlayGameSound(1);
        darts[dartsCount].targetX = tx;
        darts[dartsCount].targetY = ty;
        darts[dartsCount].x = (float)CX;
        darts[dartsCount].y = 750.0f;
        darts[dartsCount].pts = pts;
        darts[dartsCount].number = number;
        darts[dartsCount].progress = 0.0f;
        darts[dartsCount].animating = 1;
        dartsCount++;
    }
    
    dartsLeft--;
    totalDarts[currentPlayer]++;
    
    const char* turnName = currentPlayer == 0 ? "Player" : "AI";
    
    if (gameMode == 0) {
        scores[currentPlayer] -= pts;
        if (scores[currentPlayer] < 0) {
            sprintf(statusMsg, "%s Bust!", turnName);
            scores[currentPlayer] = prevScores[currentPlayer];
            gameState = 1;
        } else if (scores[currentPlayer] == 0) {
            sprintf(statusMsg, "%s Wins!", turnName);
            gameState = 2;
        } else if (dartsLeft == 0) {
            sprintf(statusMsg, "%s Turn Over. Score: %d", turnName, scores[currentPlayer]);
            gameState = 1;
        } else {
            sprintf(statusMsg, "%s Hit %d! left: %d", turnName, pts, dartsLeft);
        }
    } else {
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
                const char* mStr = mult == 1 ? "S" : (mult == 2 ? "D" : "T");
                if (number == 25) sprintf(statusMsg, "%s: %s Bull - left: %d", turnName, mStr, dartsLeft);
                else sprintf(statusMsg, "%s: %s %d - left: %d", turnName, mStr, number, dartsLeft);
            }
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            CreateWindow("BUTTON", "501", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         150, 120, 100, 30, hwnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Cricket", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         270, 120, 100, 30, hwnd, (HMENU)102, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "AI: Medium", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         390, 120, 100, 30, hwnd, (HMENU)103, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SetTimer(hwnd, TIMER_ID, 16, NULL);
            return 0;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 101) {
                SetMode(0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 102) {
                SetMode(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 103) {
                aiDifficulty = (aiDifficulty + 1) % 3;
                char buf[20];
                if (aiDifficulty == 0) strcpy(buf, "AI: Easy");
                else if (aiDifficulty == 1) strcpy(buf, "AI: Medium");
                else strcpy(buf, "AI: Hard");
                SetWindowText((HWND)lParam, buf);
            }
            return 0;

        case WM_TIMER:
            t += 0.05f;
            wobbleX = (int)(sinf(t * 1.3f) * 15.0f + sinf(t * 0.8f) * 10.0f);
            wobbleY = (int)(cosf(t * 1.5f) * 15.0f + sinf(t * 0.9f) * 10.0f);
            
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
            
            if (currentPlayer == 1 && gameState == 0) {
                int allAnimated = 1;
                for (int i = 0; i < dartsCount; i++) {
                    if (darts[i].animating) allAnimated = 0;
                }
                if (allAnimated) {
                    aiTimer++;
                    if (aiTimer > 60) {
                        aiTimer = 0;
                        ThrowDart(hwnd, 0, 0, 1);
                    }
                }
            } else if (currentPlayer == 1 && gameState == 1) {
                aiTimer++;
                if (aiTimer > 120) {
                    aiTimer = 0;
                    NextTurn(hwnd);
                }
            }
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
            
        case WM_MOUSEMOVE:
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            return 0;
            
        case WM_LBUTTONDOWN: {
            if (mouseY < 160 && mouseX > 50 && mouseX < 600) return 0; // Ignore UI clicks
            
            if (gameState == 0 && currentPlayer == 0) { // PLAYING
                int targetX = mouseX + wobbleX;
                int targetY = mouseY + wobbleY;
                ThrowDart(hwnd, targetX, targetY, 0);
            } else if ((gameState == 1 || gameState == 2) && currentPlayer == 0) {
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
            
            // Draw UI panel
            HBRUSH uiBrush = CreateSolidBrush(RGB(30, 30, 30));
            HPEN uiPen = CreatePen(PS_SOLID, 1, RGB(51, 51, 51));
            HBRUSH oldUIBrush = (HBRUSH)SelectObject(memDC, uiBrush);
            HPEN oldUIPen = (HPEN)SelectObject(memDC, uiPen);
            RoundRect(memDC, 82, 10, 568, 160, 15, 15);
            SelectObject(memDC, oldUIBrush);
            SelectObject(memDC, oldUIPen);
            DeleteObject(uiBrush);
            DeleteObject(uiPen);
            
            // Board background
            HBRUSH boardBg = CreateSolidBrush(RGB(17, 17, 17));
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
            HFONT font = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Arial");
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
            
            HFONT largeFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, "Arial");
            
            if (gameMode == 0) {
                SelectObject(memDC, largeFont);
                char scoreStr[40];
                sprintf(scoreStr, "%d | %d", scores[0], scores[1]);
                SIZE sz;
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, 325 - sz.cx/2, 25, scoreStr, strlen(scoreStr));
                
                SelectObject(memDC, font);
                TextOut(memDC, 325 - sz.cx/2 - 50, 5, "P1", 2);
                TextOut(memDC, 325 + sz.cx/2 + 20, 5, "AI", 2);
            } else {
                SelectObject(memDC, font);
                char scoreStrP[100] = "P: ";
                char scoreStrA[100] = "AI: ";
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
                SIZE sz;
                GetTextExtentPoint32(memDC, scoreStrP, strlen(scoreStrP), &sz);
                TextOut(memDC, 325 - sz.cx/2, 20, scoreStrP, strlen(scoreStrP));
                GetTextExtentPoint32(memDC, scoreStrA, strlen(scoreStrA), &sz);
                TextOut(memDC, 325 - sz.cx/2, 50, scoreStrA, strlen(scoreStrA));
            }
            
            SelectObject(memDC, font);
            SetTextColor(memDC, RGB(170, 170, 170));
            SIZE sz;
            GetTextExtentPoint32(memDC, statusMsg, strlen(statusMsg), &sz);
            TextOut(memDC, 325 - sz.cx/2, 85, statusMsg, strlen(statusMsg));
            
            // Draw darts
            for (int i = 0; i < dartsCount; i++) {
                float scale = darts[i].animating ? (1.5f - darts[i].progress * 0.5f) : 1.0f;
                int dx = (int)darts[i].x;
                int dy = (int)darts[i].y;
                
                // shaft
                HPEN shaftPen = CreatePen(PS_SOLID, (int)(2.0f * scale), RGB(200, 200, 200));
                HPEN oldPen = (HPEN)SelectObject(memDC, shaftPen);
                MoveToEx(memDC, dx, dy, NULL);
                LineTo(memDC, dx + (int)(15.0f * scale), dy + (int)(25.0f * scale));
                SelectObject(memDC, oldPen);
                DeleteObject(shaftPen);
                
                // flight
                POINT flight[4];
                flight[0].x = dx + (int)(15.0f * scale); flight[0].y = dy + (int)(25.0f * scale);
                flight[1].x = dx + (int)(25.0f * scale); flight[1].y = dy + (int)(15.0f * scale);
                flight[2].x = dx + (int)(35.0f * scale); flight[2].y = dy + (int)(25.0f * scale);
                flight[3].x = dx + (int)(25.0f * scale); flight[3].y = dy + (int)(35.0f * scale);
                
                HBRUSH fBrush = CreateSolidBrush(RGB(0, 255, 255));
                HPEN fPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HBRUSH oldFBr = (HBRUSH)SelectObject(memDC, fBrush);
                HPEN oldFPen = (HPEN)SelectObject(memDC, fPen);
                Polygon(memDC, flight, 4);
                SelectObject(memDC, oldFBr);
                SelectObject(memDC, oldFPen);
                DeleteObject(fBrush);
                DeleteObject(fPen);
                
                // tip
                HBRUSH tBrush = CreateSolidBrush(RGB(255, 255, 255));
                HBRUSH oldTBr = (HBRUSH)SelectObject(memDC, tBrush);
                int r = (int)(2.0f * scale);
                Ellipse(memDC, dx - r, dy - r, dx + r, dy + r);
                SelectObject(memDC, oldTBr);
                DeleteObject(tBrush);
            }
            
            // Draw crosshair
            if (gameState == 0 && currentPlayer == 0) {
                int tx = mouseX + wobbleX;
                int ty = mouseY + wobbleY;
                HPEN cPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
                HPEN oldCPen = (HPEN)SelectObject(memDC, cPen);
                
                MoveToEx(memDC, tx - 15, ty, NULL);
                LineTo(memDC, tx + 15, ty);
                MoveToEx(memDC, tx, ty - 15, NULL);
                LineTo(memDC, tx, ty + 15);
                
                Arc(memDC, tx - 10, ty - 10, tx + 10, ty + 10, tx, ty - 10, tx, ty - 10);
                
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[]  = "KDartsClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KDarts", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 750,
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
