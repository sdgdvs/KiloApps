#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define MAX_ROWS 40
#define MAX_COLS 40
int rows = 10;
int cols = 10;
int mines = 10;
#define CELL_SIZE 22
#define HEADER_HEIGHT 64

// Bitmasks for grid
#define CELL_MINE     0x01
#define CELL_REVEALED 0x02
#define CELL_FLAGGED  0x04
#define CELL_QUESTION 0x08

int grid[MAX_ROWS][MAX_COLS];
int gameOver = 0;
int initialized = 0;
int timeElapsed = 0;
int bestTimes[3] = {999, 999, 999};
int bestRush = 0;
int currentDiff = 0; // 0=Easy, 1=Medium, 2=Hard, 3=Rush
int flagsPlaced = 0;
int campaignMode = 0;
int campaignLevel = 1;
int shields = 0;
int scans = 0;
int sonars = 0;
int rushMode = 0;
int rushTime = 0;
int rushScore = 0;
int totalPlayed = 0;
int totalWins = 0;
HWND mainHwnd = NULL;
int mouseCellPressed = 0; // 1 if mouse button currently pressed on board

// Particle Explosion System
typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    float size;
    COLORREF color;
} Particle;

#define MAX_PARTICLES 150
Particle particles[MAX_PARTICLES];
int particleCount = 0;

static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

static int parse_int(const char** p) {
    while (**p && (**p < '0' || **p > '9')) (*p)++;
    if (!**p) return 0;
    int val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10 + (**p - '0');
        (*p)++;
    }
    return val;
}

void SpawnExplosion(float cx, float cy) {
    COLORREF colors[5] = {
        RGB(255, 60, 20),   // Bright Orange-Red Fire
        RGB(255, 200, 40),  // Yellow Spark
        RGB(255, 120, 0),   // Flame Ember
        RGB(80, 80, 90),    // Dark Smoke
        RGB(240, 240, 240)  // Debris Spark
    };
    for (int i = 0; i < 35; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx;
            p->y = cy;
            p->vx = (float)((my_rand() % 100) - 50) / 10.0f;
            p->vy = (float)((my_rand() % 100) - 50) / 10.0f - 1.2f; // slight upward launch
            p->maxLife = 15.0f + (float)(my_rand() % 25);
            p->life = p->maxLife;
            p->size = 2.0f + (float)(my_rand() % 4);
            p->color = colors[my_rand() % 5];
        }
    }
}

void UpdateParticles() {
    int active = 0;
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            p->x += p->vx;
            p->y += p->vy;
            p->vy += 0.15f; // gravity
            p->vx *= 0.96f; // air drag
            p->life -= 1.0f;
            particles[active++] = *p;
        }
    }
    particleCount = active;
}

void DrawParticles(HDC hdc) {
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            int r = (int)p->size;
            HBRUSH hbr = CreateSolidBrush(p->color);
            HPEN hPen = CreatePen(PS_SOLID, 1, p->color);
            HGDIOBJ oldBr = SelectObject(hdc, hbr);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);
            Ellipse(hdc, (int)(p->x - r), (int)(p->y - r), (int)(p->x + r), (int)(p->y + r));
            SelectObject(hdc, oldBr);
            SelectObject(hdc, oldPen);
            DeleteObject(hbr);
            DeleteObject(hPen);
        }
    }
}

void LoadBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            const char* ptr = buf;
            bestTimes[0] = parse_int(&ptr);
            bestTimes[1] = parse_int(&ptr);
            bestTimes[2] = parse_int(&ptr);
            totalPlayed = parse_int(&ptr);
            totalWins = parse_int(&ptr);
            bestRush = parse_int(&ptr);
        }
        CloseHandle(hFile);
    }
}

void SaveBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128];
        wsprintfA(buf, "%d %d %d %d %d %d", bestTimes[0], bestTimes[1], bestTimes[2], totalPlayed, totalWins, bestRush);
        DWORD bytesWritten;
        int len = 0; while(buf[len]) len++;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void InitGame(int firstClickX, int firstClickY) {
    memset(grid, 0, sizeof(grid));
    gameOver = 0;
    if (!rushMode) timeElapsed = 0;
    flagsPlaced = 0;
    seed = GetTickCount();

    int placed = 0;
    while (placed < mines) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if ((grid[r][c] & CELL_MINE) == 0) {
            // Avoid placing mine on first click 3x3 area
            if (r < firstClickY - 1 || r > firstClickY + 1 || c < firstClickX - 1 || c > firstClickX + 1) {
                grid[r][c] |= CELL_MINE;
                placed++;
            }
        }
    }
    initialized = 1;
    totalPlayed++;
    SaveBest();
    SetTimer(mainHwnd, 1, 1000, NULL);
    SetTimer(mainHwnd, 2, 33, NULL); // 30 FPS animation & particle timer
}

void InitCampaignLevel(HWND hwnd) {
    if (campaignLevel == 1) { rows=8; cols=8; mines=10; }
    else if (campaignLevel == 2) { rows=12; cols=12; mines=25; }
    else if (campaignLevel == 3) { rows=16; cols=16; mines=45; }
    else if (campaignLevel == 4) { rows=20; cols=20; mines=75; }
    else if (campaignLevel == 5) { rows=24; cols=24; mines=120; }
    else if (campaignLevel == 6) { rows=24; cols=30; mines=150; }
    else if (campaignLevel == 7) { rows=26; cols=30; mines=180; }
    else if (campaignLevel == 8) { rows=28; cols=30; mines=200; }
    else if (campaignLevel == 9) { rows=30; cols=30; mines=220; }
    else if (campaignLevel == 10) { rows=30; cols=30; mines=250; }
    else if (campaignLevel == 11) { rows=30; cols=30; mines=275; }
    else if (campaignLevel == 12) { rows=32; cols=32; mines=310; }
    else if (campaignLevel == 13) { rows=34; cols=34; mines=350; }
    else if (campaignLevel == 14) { rows=36; cols=36; mines=390; }
    else if (campaignLevel == 15) { rows=40; cols=40; mines=450; }
    else { 
        campaignMode = 0; 
        rows=10; cols=10; mines=10; currentDiff=0; 
        MessageBoxA(hwnd, "Campaign Complete!", "Victory", MB_OK); 
    }
    shields = campaignLevel >= 5 ? (campaignLevel >= 11 ? 3 : 2) : 1;
    scans = campaignLevel >= 11 ? 5 : 3;
    sonars = campaignLevel / 3;
    initialized = 0; gameOver = 0; timeElapsed = 0; flagsPlaced = 0; memset(grid,0,sizeof(grid));
    void ResizeWindow(HWND);
    ResizeWindow(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (grid[nr][nc] & CELL_MINE) count++;
            }
        }
    }
    return count;
}

void Reveal(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    if (grid[r][c] & (CELL_REVEALED | CELL_FLAGGED)) return;
    
    grid[r][c] |= CELL_REVEALED;
    
    if (grid[r][c] & CELL_MINE) {
        // Detonation explosion particles!
        int px = c * CELL_SIZE + CELL_SIZE / 2;
        int py = r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2;
        SpawnExplosion((float)px, (float)py);

        if (shields > 0) {
            shields--;
            grid[r][c] &= ~CELL_MINE;
            mines--;
            Beep(500, 100);
        } else {
            gameOver = 1;
            KillTimer(mainHwnd, 1);
            Beep(200, 500);
            return;
        }
    }
    
    if (CountMines(r, c) == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                Reveal(r + i, c + j);
            }
        }
    }
}

// GDI Graphics Rendering Functions

void DrawSmiley(HDC hdc, int cx, int cy, int size, int state) {
    int r = size / 2;
    // Outer yellow face with gradient/highlight
    HBRUSH hbrFace = CreateSolidBrush(RGB(255, 215, 0));
    HPEN hPenOutline = CreatePen(PS_SOLID, 2, RGB(180, 130, 0));
    HGDIOBJ oldBr = SelectObject(hdc, hbrFace);
    HGDIOBJ oldPen = SelectObject(hdc, hPenOutline);
    
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    
    // Top highlight curve
    HPEN hPenShine = CreatePen(PS_SOLID, 1, RGB(255, 255, 200));
    SelectObject(hdc, hPenShine);
    Arc(hdc, cx - r + 3, cy - r + 3, cx + r - 3, cy + r - 3, cx + r, cy - r, cx - r, cy - r);
    DeleteObject(hPenShine);

    if (state == 1) { // Pressed / Shocked (:o)
        // Wide open black eyes
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        
        // Open circular mouth
        Ellipse(hdc, cx - 4, cy + 1, cx + 4, cy + 9);
        DeleteObject(hbrEye);
    } else if (state == 2) { // Dead / Lost (X_X)
        // X eyes
        HPEN hPenX = CreatePen(PS_SOLID, 2, RGB(60, 40, 20));
        SelectObject(hdc, hPenX);
        // Left X
        MoveToEx(hdc, cx - 7, cy - 6, NULL); LineTo(hdc, cx - 3, cy - 2);
        MoveToEx(hdc, cx - 3, cy - 6, NULL); LineTo(hdc, cx - 7, cy - 2);
        // Right X
        MoveToEx(hdc, cx + 3, cy - 6, NULL); LineTo(hdc, cx + 7, cy - 2);
        MoveToEx(hdc, cx + 7, cy - 6, NULL); LineTo(hdc, cx + 3, cy - 2);
        
        // Sad frowning mouth
        Arc(hdc, cx - 6, cy + 2, cx + 6, cy + 12, cx + 6, cy + 6, cx - 6, cy + 6);
        DeleteObject(hPenX);
    } else if (state == 3) { // Cool / Victory (B-)
        // Black sunglasses
        HBRUSH hbrGlass = CreateSolidBrush(RGB(30, 30, 35));
        HPEN hPenGlass = CreatePen(PS_SOLID, 1, RGB(10, 10, 10));
        SelectObject(hdc, hbrGlass);
        SelectObject(hdc, hPenGlass);
        
        POINT ptL[4] = { {cx - 9, cy - 6}, {cx - 1, cy - 6}, {cx - 2, cy + 1}, {cx - 8, cy + 1} };
        POINT ptR[4] = { {cx + 1, cy - 6}, {cx + 9, cy - 6}, {cx + 8, cy + 1}, {cx + 2, cy + 1} };
        Polygon(hdc, ptL, 4);
        Polygon(hdc, ptR, 4);
        MoveToEx(hdc, cx - 1, cy - 4, NULL); LineTo(hdc, cx + 1, cy - 4); // bridge
        
        // Sunglasses white reflective stripe
        HPEN hPenSheen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, hPenSheen);
        MoveToEx(hdc, cx - 8, cy - 4, NULL); LineTo(hdc, cx - 4, cy - 4);
        MoveToEx(hdc, cx + 2, cy - 4, NULL); LineTo(hdc, cx + 6, cy - 4);
        DeleteObject(hPenSheen);
        DeleteObject(hbrGlass);
        DeleteObject(hPenGlass);
        
        // Smirk
        HPEN hPenSmirk = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmirk);
        Arc(hdc, cx - 5, cy + 1, cx + 6, cy + 8, cx - 5, cy + 4, cx + 5, cy + 6);
        DeleteObject(hPenSmirk);
    } else { // Normal Idle Smile
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        DeleteObject(hbrEye);
        
        HPEN hPenSmile = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmile);
        Arc(hdc, cx - 6, cy - 3, cx + 6, cy + 7, cx + 6, cy + 3, cx - 6, cy + 3);
        DeleteObject(hPenSmile);
    }

    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(hbrFace);
    DeleteObject(hPenOutline);
}

void DrawMineSprite(HDC hdc, int x, int y, int size, int isDetonated, DWORD tick) {
    int cx = x + size / 2;
    int cy = y + size / 2;
    int r = size / 3;

    if (isDetonated) {
        // Red background glow for triggered mine
        HBRUSH hbrGlow = CreateSolidBrush(RGB(240, 50, 50));
        RECT rcCell = { x, y, x + size, y + size };
        FillRect(hdc, &rcCell, hbrGlow);
        DeleteObject(hbrGlow);
    }

    // Spikes (4 diagonal + 4 orthogonal)
    HPEN hPenSpike = CreatePen(PS_SOLID, 2, RGB(30, 30, 35));
    HGDIOBJ oldPen = SelectObject(hdc, hPenSpike);
    MoveToEx(hdc, cx - r - 2, cy, NULL); LineTo(hdc, cx + r + 2, cy);
    MoveToEx(hdc, cx, cy - r - 2, NULL); LineTo(hdc, cx, cy + r + 2);
    MoveToEx(hdc, cx - r + 1, cy - r + 1, NULL); LineTo(hdc, cx + r - 1, cy + r - 1);
    MoveToEx(hdc, cx + r - 1, cy - r + 1, NULL); LineTo(hdc, cx - r + 1, cy + r - 1);
    DeleteObject(hPenSpike);

    // Iron Mine Body sphere
    HBRUSH hbrBody = CreateSolidBrush(RGB(40, 44, 52));
    HPEN hPenBody = CreatePen(PS_SOLID, 1, RGB(15, 18, 22));
    SelectObject(hdc, hbrBody);
    SelectObject(hdc, hPenBody);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    DeleteObject(hbrBody);
    DeleteObject(hPenBody);

    // Specular Shine spot on body
    HBRUSH hbrShine = CreateSolidBrush(RGB(200, 210, 230));
    SelectObject(hdc, hbrShine);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, cx - r + 3, cy - r + 3, cx - r + 7, cy - r + 7);
    DeleteObject(hbrShine);

    // Fuse wire
    HPEN hPenFuse = CreatePen(PS_SOLID, 2, RGB(180, 130, 70));
    SelectObject(hdc, hPenFuse);
    MoveToEx(hdc, cx + 2, cy - r, NULL);
    LineTo(hdc, cx + 6, cy - r - 4);
    DeleteObject(hPenFuse);

    // Flickering Fuse Spark!
    int sparkRadius = 2 + ((tick / 80) % 3);
    COLORREF sparkColor = ((tick / 60) % 2 == 0) ? RGB(255, 230, 40) : RGB(255, 60, 20);
    HBRUSH hbrSpark = CreateSolidBrush(sparkColor);
    HPEN hPenSpark = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    SelectObject(hdc, hbrSpark);
    SelectObject(hdc, hPenSpark);
    int sx = cx + 6, sy = cy - r - 4;
    Ellipse(hdc, sx - sparkRadius, sy - sparkRadius, sx + sparkRadius, sy + sparkRadius);
    DeleteObject(hbrSpark);
    DeleteObject(hPenSpark);
    
    SelectObject(hdc, oldPen);
}

void DrawFlagSprite(HDC hdc, int x, int y, int size, DWORD tick) {
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Flagpole base
    HBRUSH hbrBase = CreateSolidBrush(RGB(60, 60, 70));
    HPEN hPenBase = CreatePen(PS_SOLID, 1, RGB(30, 30, 35));
    HGDIOBJ oldBr = SelectObject(hdc, hbrBase);
    HGDIOBJ oldPen = SelectObject(hdc, hPenBase);
    Ellipse(hdc, cx - 6, y + size - 6, cx + 6, y + size - 2);
    DeleteObject(hbrBase);
    DeleteObject(hPenBase);

    // Silver Pole
    HPEN hPenPole = CreatePen(PS_SOLID, 2, RGB(200, 205, 215));
    SelectObject(hdc, hPenPole);
    MoveToEx(hdc, cx - 3, y + 4, NULL);
    LineTo(hdc, cx - 3, y + size - 4);
    DeleteObject(hPenPole);

    // Gold Finial Ball at top
    HBRUSH hbrGold = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, hbrGold);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, cx - 5, y + 2, cx - 1, y + 6);
    DeleteObject(hbrGold);

    // Waving Cloth Flag (Crimson red silk)
    int waveOffset = ((tick / 150) % 2) * 2;
    POINT pts[3] = {
        { cx - 2, y + 5 },
        { cx + size / 2 + 3 + waveOffset, y + 9 },
        { cx - 2, y + 15 }
    };
    HBRUSH hbrCloth = CreateSolidBrush(RGB(240, 45, 65));
    HPEN hPenCloth = CreatePen(PS_SOLID, 1, RGB(160, 20, 35));
    SelectObject(hdc, hbrCloth);
    SelectObject(hdc, hPenCloth);
    Polygon(hdc, pts, 3);
    DeleteObject(hbrCloth);
    DeleteObject(hPenCloth);

    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
}

void DrawQuestionSprite(HDC hdc, int x, int y, int size) {
    RECT rc = { x, y, x + size, y + size };
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(56, 189, 248));
    DrawTextA(hdc, "?", 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

void Draw3DTile(HDC hdc, int x, int y, int size, int isRevealed, int isPressed) {
    RECT rc = { x, y, x + size, y + size };
    if (isRevealed) {
        HBRUSH hbrRev = CreateSolidBrush(RGB(24, 28, 40));
        FillRect(hdc, &rc, hbrRev);
        DeleteObject(hbrRev);

        // Sunken subtle inner shadow border
        HPEN hDarkInner = CreatePen(PS_SOLID, 1, RGB(15, 18, 26));
        HGDIOBJ oldPen = SelectObject(hdc, hDarkInner);
        MoveToEx(hdc, x, y + size - 1, NULL);
        LineTo(hdc, x, y);
        LineTo(hdc, x + size - 1, y);
        SelectObject(hdc, oldPen);
        DeleteObject(hDarkInner);
    } else {
        HBRUSH hbrUnrev = CreateSolidBrush(RGB(65, 75, 100));
        FillRect(hdc, &rc, hbrUnrev);
        DeleteObject(hbrUnrev);

        // 3D Bevel Highlights & Shadows
        HPEN hWhiteHi = CreatePen(PS_SOLID, 2, RGB(140, 155, 190));
        HPEN hDarkLo  = CreatePen(PS_SOLID, 2, RGB(30, 35, 50));
        
        HGDIOBJ oldPen = SelectObject(hdc, hWhiteHi);
        MoveToEx(hdc, x, y + size - 1, NULL);
        LineTo(hdc, x, y);
        LineTo(hdc, x + size - 1, y);

        SelectObject(hdc, hDarkLo);
        LineTo(hdc, x + size - 1, y + size - 1);
        LineTo(hdc, x - 1, y + size - 1);

        SelectObject(hdc, oldPen);
        DeleteObject(hWhiteHi);
        DeleteObject(hDarkLo);
    }
}

void DrawBoard(HWND hwnd, HDC hdc) {
    DWORD tick = GetTickCount();

    // Dark sleek window background
    RECT rcFull = { 0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT };
    HBRUSH hbrBg = CreateSolidBrush(RGB(18, 20, 29));
    FillRect(hdc, &rcFull, hbrBg);
    DeleteObject(hbrBg);

    // Header Toolbar Panel
    RECT rcHeader = { 0, 0, cols * CELL_SIZE, HEADER_HEIGHT };
    HBRUSH hbrHeader = CreateSolidBrush(RGB(30, 35, 50));
    FillRect(hdc, &rcHeader, hbrHeader);
    DeleteObject(hbrHeader);

    // Header 3D bottom border line
    HPEN hHeaderBorder = CreatePen(PS_SOLID, 2, RGB(60, 70, 95));
    HGDIOBJ oldPen = SelectObject(hdc, hHeaderBorder);
    MoveToEx(hdc, 0, HEADER_HEIGHT - 1, NULL);
    LineTo(hdc, cols * CELL_SIZE, HEADER_HEIGHT - 1);
    SelectObject(hdc, oldPen);
    DeleteObject(hHeaderBorder);

    // Header LCD Counters & Status Smiley
    HFONT hLcdFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
    HGDIOBJ oldFont = SelectObject(hdc, hLcdFont);
    SetBkMode(hdc, OPAQUE);
    SetBkColor(hdc, RGB(10, 10, 15));

    // Mine Counter (Left)
    char szMines[16];
    wsprintfA(szMines, "%03d", mines - flagsPlaced);
    RECT rcMineBox = { 10, 12, 60, 42 };
    HBRUSH hbrLcd = CreateSolidBrush(RGB(10, 10, 15));
    FillRect(hdc, &rcMineBox, hbrLcd);
    SetTextColor(hdc, RGB(247, 118, 142)); // Danger Red LED
    DrawTextA(hdc, szMines, -1, &rcMineBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Timer / Rush Score (Right)
    char szTime[16];
    wsprintfA(szTime, "%03d", rushMode ? rushTime : timeElapsed);
    RECT rcTimeBox = { cols * CELL_SIZE - 60, 12, cols * CELL_SIZE - 10, 42 };
    FillRect(hdc, &rcTimeBox, hbrLcd);
    SetTextColor(hdc, RGB(122, 162, 247)); // Blue LED
    DrawTextA(hdc, szTime, -1, &rcTimeBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hbrLcd);

    // Interactive Smiley Status Button (Center)
    int faceCx = cols * CELL_SIZE / 2;
    int faceCy = 27;
    int smileyState = 0;
    if (gameOver) {
        smileyState = 2; // Dead
    } else if (CheckWin()) {
        smileyState = 3; // Cool Win
    } else if (mouseCellPressed) {
        smileyState = 1; // Shocked click
    }
    DrawSmiley(hdc, faceCx, faceCy, 32, smileyState);

    // Mode text below toolbar
    SetBkMode(hdc, TRANSPARENT);
    HFONT hSubFont = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, hSubFont);
    SetTextColor(hdc, RGB(180, 190, 210));
    RECT rcMode = { 0, 44, cols * CELL_SIZE, HEADER_HEIGHT - 2 };
    char szMode[128];
    if (rushMode) {
        wsprintfA(szMode, "RUSH MODE - Score:%d Best:%d", rushScore, bestRush);
    } else if (campaignMode) {
        wsprintfA(szMode, "CAMPAIGN Lvl %d | Sh:%d R:%d S:%d", campaignLevel, shields, scans, sonars);
    } else {
        wsprintfA(szMode, "1-Ez 2-Md 3-Hd C-Camp 4-Rush | P:%d W:%d", totalPlayed, totalWins);
    }
    DrawTextA(hdc, szMode, -1, &rcMode, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hSubFont);

    // Draw Grid Cells
    HFONT hNumFont = CreateFontA(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, hNumFont);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE + HEADER_HEIGHT;
            RECT rcCell = { x, y, x + CELL_SIZE, y + CELL_SIZE };
            
            int isRev = (grid[r][c] & CELL_REVEALED);
            Draw3DTile(hdc, x, y, CELL_SIZE, isRev, 0);

            if (isRev) {
                if (grid[r][c] & CELL_MINE) {
                    DrawMineSprite(hdc, x, y, CELL_SIZE, 1, tick);
                } else {
                    int m = CountMines(r, c);
                    if (m > 0) {
                        char szNum[2] = { m + '0', 0 };
                        COLORREF numColors[8] = {
                            RGB(56, 189, 248),  // 1: Cyan Blue
                            RGB(74, 222, 128),  // 2: Emerald Green
                            RGB(248, 113, 113), // 3: Bright Red
                            RGB(192, 132, 252), // 4: Vivid Purple
                            RGB(250, 204, 21),  // 5: Gold Yellow
                            RGB(45, 212, 191),  // 6: Teal
                            RGB(226, 232, 240), // 7: Platinum Silver
                            RGB(148, 163, 184)  // 8: Slate
                        };
                        SetTextColor(hdc, numColors[m - 1]);
                        DrawTextA(hdc, szNum, 1, &rcCell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            } else {
                if (grid[r][c] & CELL_FLAGGED) {
                    DrawFlagSprite(hdc, x, y, CELL_SIZE, tick);
                } else if (grid[r][c] & CELL_QUESTION) {
                    DrawQuestionSprite(hdc, x, y, CELL_SIZE);
                }
            }
        }
    }
    DeleteObject(hNumFont);
    SelectObject(hdc, oldFont);
    DeleteObject(hLcdFont);

    // Draw active particle overlays
    DrawParticles(hdc);
}

int CheckWin() {
    int revealed = 0;
    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols; c++) {
            if (grid[r][c] & CELL_REVEALED) revealed++;
        }
    }
    return (revealed == (rows * cols - mines));
}

void HandleReveal(HWND hwnd, int x, int y) {
    if (gameOver || !initialized) return;
    if (grid[y][x] & CELL_FLAGGED) return;

    if (grid[y][x] & CELL_REVEALED) {
        int flagged = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = y + i, nc = x + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (grid[nr][nc] & CELL_FLAGGED) flagged++;
                }
            }
        }
        if (flagged == CountMines(y, x)) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nr = y + i, nc = x + j;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (!(grid[nr][nc] & CELL_FLAGGED) && !(grid[nr][nc] & CELL_REVEALED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        } else {
            return;
        }
    } else {
        Reveal(y, x);
    }

    if (!gameOver) Beep(1000, 20);
    InvalidateRect(hwnd, NULL, FALSE);
    
    if (gameOver) {
        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
        InvalidateRect(hwnd, NULL, FALSE);
        MessageBoxA(hwnd, "Boom! Click Smiley to restart.", "Game Over", MB_OK);
    } else if (CheckWin()) {
        gameOver = 1;
        KillTimer(hwnd, 1);
        Beep(1500, 300);
        totalWins++;
        SaveBest();
        if (!campaignMode && timeElapsed < bestTimes[currentDiff]) {
            bestTimes[currentDiff] = timeElapsed;
            SaveBest();
        }
        if (campaignMode) {
            MessageBoxA(hwnd, "Level Complete! Next Level...", "Campaign", MB_OK);
            campaignLevel++;
            InitCampaignLevel(hwnd);
        } else if (rushMode) {
            rushScore++;
            rushTime += 15;
            if (rushTime > 999) rushTime = 999;
            if (rushScore > bestRush) bestRush = rushScore;
            SaveBest();
            Beep(1200, 200);
            initialized = 0;
            memset(grid, 0, sizeof(grid));
            gameOver = 0;
            flagsPlaced = 0;
            InvalidateRect(hwnd, NULL, TRUE);
        } else {
            MessageBoxA(hwnd, "You Win! Click Smiley to restart.", "Congratulations", MB_OK);
        }
    }
}

void ResizeWindow(HWND hwnd) {
    RECT rcClient, rcWindow;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWindow);
    SetWindowPos(hwnd, NULL, 0, 0,
        (rcWindow.right - rcWindow.left) + (cols * CELL_SIZE - (rcClient.right - rcClient.left)),
        (rcWindow.bottom - rcWindow.top) + (rows * CELL_SIZE + HEADER_HEIGHT - (rcClient.bottom - rcClient.top)),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadBest();
            SetTimer(hwnd, 2, 33, NULL); // Animation loop timer
            break;
        case WM_TIMER:
            if (wParam == 1 && !gameOver && initialized) {
                if (rushMode) {
                    rushTime--;
                    if (rushTime <= 0) {
                        rushTime = 0;
                        gameOver = 1;
                        KillTimer(hwnd, 1);
                        Beep(200, 500);
                        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Time's Up! Click Smiley to restart.", "Game Over", MB_OK);
                    }
                } else {
                    timeElapsed++;
                    if (timeElapsed > 999) timeElapsed = 999;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                // Animation frame tick
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawBoard(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            // Check click on Smiley Button in header
            int faceCx = cols * CELL_SIZE / 2;
            int faceCy = 27;
            if ((mx - faceCx)*(mx - faceCx) + (my - faceCy)*(my - faceCy) <= 256) {
                if (campaignMode) { InitCampaignLevel(hwnd); return 0; }
                if (rushMode) { rushTime = 60; rushScore = 0; }
                initialized = 0;
                memset(grid, 0, sizeof(grid));
                gameOver = 0;
                if (!rushMode) timeElapsed = 0;
                flagsPlaced = 0;
                shields = 0;
                particleCount = 0;
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }

            if (gameOver) return 0;

            mouseCellPressed = 1;
            InvalidateRect(hwnd, NULL, FALSE);

            int x = mx / CELL_SIZE;
            int y = (my - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && my >= HEADER_HEIGHT) {
                if (!initialized) InitGame(x, y);
                HandleReveal(hwnd, x, y);
            }
            break;
        }
        case WM_LBUTTONUP: {
            mouseCellPressed = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver || !initialized) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = (HIWORD(lParam) - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && HIWORD(lParam) >= HEADER_HEIGHT) {
                if (!(grid[y][x] & CELL_REVEALED)) {
                    // Right Click Cycle: Unrevealed -> Flagged -> Question -> Unrevealed
                    if (grid[y][x] & CELL_FLAGGED) {
                        grid[y][x] &= ~CELL_FLAGGED;
                        grid[y][x] |= CELL_QUESTION;
                        flagsPlaced--;
                    } else if (grid[y][x] & CELL_QUESTION) {
                        grid[y][x] &= ~CELL_QUESTION;
                    } else {
                        grid[y][x] |= CELL_FLAGGED;
                        flagsPlaced++;
                    }
                    Beep(800, 20);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == '1') { rushMode=0; campaignMode=0; shields=0; scans=0; sonars=0; rows=10; cols=10; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '2') { rushMode=0; campaignMode=0; shields=0; scans=0; sonars=0; rows=16; cols=16; mines=40; currentDiff=1; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '3') { rushMode=0; campaignMode=0; shields=0; scans=0; sonars=0; rows=16; cols=30; mines=99; currentDiff=2; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '4') { rushMode=1; rushTime=60; rushScore=0; campaignMode=0; shields=0; scans=0; sonars=0; rows=10; cols=10; mines=10; currentDiff=3; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == 'C') { rushMode=0; campaignMode = !campaignMode; if (campaignMode) { campaignLevel = 1; InitCampaignLevel(hwnd); } else { campaignMode=0; shields=0; scans=0; sonars=0; rows=10; cols=10; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); } }
            if (wParam == 'R' && campaignMode && scans > 0 && initialized && !gameOver) {
                for (int r = 0; r < rows; r++) {
                    for (int c = 0; c < cols; c++) {
                        if ((grid[r][c] & CELL_MINE) && !(grid[r][c] & CELL_FLAGGED) && !(grid[r][c] & CELL_REVEALED)) {
                            grid[r][c] |= CELL_FLAGGED;
                            flagsPlaced++;
                            scans--;
                            Beep(1200, 100);
                            InvalidateRect(hwnd, NULL, FALSE);
                            goto end_radar;
                        }
                    }
                }
                end_radar:;
            }
            if (wParam == 'S' && campaignMode && sonars > 0 && initialized && !gameOver) {
                int targetR = -1, targetC = -1;
                for (int i = 0; i < 1000; i++) {
                    int r = my_rand() % rows;
                    int c = my_rand() % cols;
                    if (!(grid[r][c] & CELL_REVEALED)) {
                        targetR = r; targetC = c;
                        break;
                    }
                }
                if (targetR != -1) {
                    sonars--;
                    for (int i = -1; i <= 1; i++) {
                        for (int j = -1; j <= 1; j++) {
                            int nr = targetR + i, nc = targetC + j;
                            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                                if (grid[nr][nc] & CELL_MINE) {
                                    if (!(grid[nr][nc] & CELL_FLAGGED)) {
                                        grid[nr][nc] |= CELL_FLAGGED;
                                        flagsPlaced++;
                                    }
                                } else {
                                    if (!(grid[nr][nc] & CELL_REVEALED) && !(grid[nr][nc] & CELL_FLAGGED)) {
                                        Reveal(nr, nc);
                                    }
                                }
                            }
                        }
                    }
                    Beep(2000, 150);
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (CheckWin() && !gameOver) {
                        gameOver = 1;
                        KillTimer(hwnd, 1);
                        Beep(1500, 300);
                        totalWins++;
                        SaveBest();
                        MessageBoxA(hwnd, "Level Complete! Next Level...", "Campaign", MB_OK);
                        campaignLevel++;
                        InitCampaignLevel(hwnd);
                    }
                }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMinesClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    RECT rc = {0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMinesClass", "KMines", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ResizeWindow(hwnd);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
