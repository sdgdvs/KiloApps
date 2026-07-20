#include <windows.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846
#define TIMER_ID 1
#define CX 325
#define CY 380
#define BOARD_R 300
#define R 220

typedef struct {
    int x, y, pts;
} Dart;

int gameMode = 0; // 0=501, 1=Cricket
int cricketHits[7] = {0}; // 20, 19, 18, 17, 16, 15, 25
int totalDarts = 0;

int score = 501;
int prevScore = 501;
int dartsLeft = 3;
Dart darts[3];
int dartsCount = 0;
int gameState = 0; // 0=PLAYING, 1=TURN_END, 2=WON
int mouseX = 325, mouseY = 380;
int wobbleX = 0, wobbleY = 0;
float t = 0.0f;
char statusMsg[100] = "Game On! Throw 3 Darts";

void SetMode(int mode) {
    gameMode = mode;
    gameState = 0;
    dartsCount = 0;
    dartsLeft = 3;
    totalDarts = 0;
    if (mode == 0) {
        score = 501;
        prevScore = 501;
        sprintf(statusMsg, "Game On! Throw 3 Darts");
    } else {
        for(int i=0; i<7; i++) cricketHits[i] = 0;
        sprintf(statusMsg, "Close 15-20 and Bullseye");
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindow("BUTTON", "501", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         210, 120, 100, 30, hwnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindow("BUTTON", "Cricket", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                         340, 120, 100, 30, hwnd, (HMENU)102, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SetTimer(hwnd, TIMER_ID, 16, NULL);
            return 0;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 101) {
                SetMode(0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 102) {
                SetMode(1);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        case WM_TIMER:
            t += 0.05f;
            wobbleX = (int)(sinf(t * 1.3f) * 15.0f + sinf(t * 0.8f) * 10.0f);
            wobbleY = (int)(cosf(t * 1.5f) * 15.0f + sinf(t * 0.9f) * 10.0f);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
            
        case WM_MOUSEMOVE:
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            return 0;
            
        case WM_LBUTTONDOWN: {
            if (mouseY < 160 && mouseX > 50 && mouseX < 600) return 0; // Ignore UI clicks
            
            if (gameState == 0) { // PLAYING
                int targetX = mouseX + wobbleX;
                int targetY = mouseY + wobbleY;
                int number, mult;
                GetHitDetails(targetX, targetY, &number, &mult);
                int pts = number * mult;
                
                if (dartsCount < 3) {
                    darts[dartsCount].x = targetX;
                    darts[dartsCount].y = targetY;
                    darts[dartsCount].pts = pts;
                    dartsCount++;
                }
                
                dartsLeft--;
                totalDarts++;
                
                if (gameMode == 0) {
                    score -= pts;
                    if (score < 0) {
                        sprintf(statusMsg, "Bust!");
                        score = prevScore;
                        gameState = 1;
                    } else if (score == 0) {
                        sprintf(statusMsg, "You Win!");
                        gameState = 2;
                    } else if (dartsLeft == 0) {
                        sprintf(statusMsg, "Turn Over. Score: %d", score);
                        gameState = 1;
                    } else {
                        sprintf(statusMsg, "Hit %d! Darts left: %d", pts, dartsLeft);
                    }
                } else { // Cricket
                    if (mult > 0) {
                        int tIdx = -1;
                        if (number == 20) tIdx = 0;
                        else if (number == 19) tIdx = 1;
                        else if (number == 18) tIdx = 2;
                        else if (number == 17) tIdx = 3;
                        else if (number == 16) tIdx = 4;
                        else if (number == 15) tIdx = 5;
                        else if (number == 25) tIdx = 6;
                        
                        if (tIdx != -1) {
                            cricketHits[tIdx] += mult;
                            if (cricketHits[tIdx] > 3) cricketHits[tIdx] = 3;
                        }
                    }
                    
                    int allClosed = 1;
                    for (int i=0; i<7; i++) {
                        if (cricketHits[i] < 3) allClosed = 0;
                    }
                    
                    if (allClosed) {
                        sprintf(statusMsg, "You Win in %d darts!", totalDarts);
                        gameState = 2;
                    } else if (dartsLeft == 0) {
                        sprintf(statusMsg, "Turn Over.");
                        gameState = 1;
                    } else {
                        if (mult == 0) sprintf(statusMsg, "Miss! - Darts left: %d", dartsLeft);
                        else {
                            const char* mStr = mult == 1 ? "Single" : (mult == 2 ? "Double" : "Triple");
                            if (number == 25) sprintf(statusMsg, "%s Bull - Darts left: %d", mStr, dartsLeft);
                            else sprintf(statusMsg, "%s %d - Darts left: %d", mStr, number, dartsLeft);
                        }
                    }
                }
            } else if (gameState == 1) { // TURN_END
                dartsCount = 0;
                dartsLeft = 3;
                prevScore = score;
                gameState = 0; // PLAYING
                sprintf(statusMsg, "Throw Dart 1");
            } else if (gameState == 2) { // WON
                SetMode(gameMode);
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
                char scoreStr[20];
                sprintf(scoreStr, "%d", score);
                SIZE sz;
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, 325 - sz.cx/2, 25, scoreStr, strlen(scoreStr));
            } else {
                SelectObject(memDC, font);
                char scoreStr[100] = "";
                int targets[] = {20, 19, 18, 17, 16, 15, 25};
                for (int i=0; i<7; i++) {
                    int hits = cricketHits[i];
                    char mark = hits == 0 ? '-' : (hits == 1 ? '/' : (hits == 2 ? 'X' : 'O'));
                    char buf[16];
                    if (targets[i] == 25) sprintf(buf, "B:%c  ", mark);
                    else sprintf(buf, "%d:%c  ", targets[i], mark);
                    strcat(scoreStr, buf);
                }
                SIZE sz;
                GetTextExtentPoint32(memDC, scoreStr, strlen(scoreStr), &sz);
                TextOut(memDC, 325 - sz.cx/2, 40, scoreStr, strlen(scoreStr));
            }
            
            SelectObject(memDC, font);
            SetTextColor(memDC, RGB(170, 170, 170));
            SIZE sz;
            GetTextExtentPoint32(memDC, statusMsg, strlen(statusMsg), &sz);
            TextOut(memDC, 325 - sz.cx/2, 85, statusMsg, strlen(statusMsg));
            
            // Draw darts
            for (int i = 0; i < dartsCount; i++) {
                HBRUSH dBrush = CreateSolidBrush(RGB(0, 255, 255));
                HBRUSH oldDBr = (HBRUSH)SelectObject(memDC, dBrush);
                HPEN dPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN oldDPen = (HPEN)SelectObject(memDC, dPen);
                Ellipse(memDC, darts[i].x - 4, darts[i].y - 4, darts[i].x + 4, darts[i].y + 4);
                SelectObject(memDC, oldDBr);
                SelectObject(memDC, oldDPen);
                DeleteObject(dBrush);
                DeleteObject(dPen);
            }
            
            // Draw crosshair
            if (gameState == 0) {
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
