#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;

#define W 400
#define H 400
#define TIMER_ID 1

int pad_x = W / 2 - 30;
int pad_w = 60;
int pad_h = 10;
int ball_x = W / 2;
int ball_y = H / 2;
int ball_size = 8;
int ball_dx = 0;
int ball_dy = 0;
int score = 0;
int high_score = 0;
int state = 0; // 0=start, 1=play, 2=gameover
int diff = 0; // 0=Easy, 1=Hard
int speed = 3;
int lives = 3;
int power_x = 0;
int power_y = 0;
int power_active = 0;
int paddle_timer = 0;
int level = 1;
int lifetime_bricks = 0;
int power_type = 0;
int pierce_timer = 0;
int ufo_active = 0;
int ufo_x = 0;
int ufo_y = 25;
int ufo_dx = 2;
int ufo_timer = 500;

int sticky_timer = 0;
int ball_stuck = 0;
int ball_stuck_offset = 0;

int laser_timer = 0;
#define MAX_LASERS 5
int lasers_x[MAX_LASERS];
int lasers_y[MAX_LASERS];
int lasers_active[MAX_LASERS];

#define ROWS 5
#define COLS 10
int bricks[ROWS][COLS];
int bricks_left = 0;

#define MAX_PARTICLES 64
typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
} Particle;
Particle particles[MAX_PARTICLES];

#define MAX_TRAIL 6
typedef struct { int x, y; } TrailPoint;
TrailPoint ball_trail[MAX_TRAIL];
int trail_idx = 0;

static unsigned int rng_seed = 1337;
static int MyRand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (int)(rng_seed & 0x7fffffff);
}

static int MyAbs(int x) {
    return x < 0 ? -x : x;
}

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x;
                particles[p].y = y;
                particles[p].vx = ((float)(MyRand() % 61 - 30)) / 10.0f;
                particles[p].vy = ((float)(MyRand() % 61 - 30)) / 10.0f;
                particles[p].life = 10 + (MyRand() % 12);
                particles[p].color = color;
                break;
            }
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].life--;
        }
    }
}

void RecordBallTrail(int x, int y) {
    ball_trail[trail_idx].x = x;
    ball_trail[trail_idx].y = y;
    trail_idx = (trail_idx + 1) % MAX_TRAIL;
}

COLORREF GetBrickColor(int r, int c, int type) {
    if (type == 9) return RGB(140, 150, 160);
    if (type == 2) return RGB(240, 60, 60);
    COLORREF rowColors[5] = {
        RGB(255, 50, 100),
        RGB(255, 150, 50),
        RGB(255, 200, 0),
        RGB(50, 200, 100),
        RGB(50, 200, 255)
    };
    return rowColors[r % 5];
}

void DrawGDIBrick(HDC hdc, int r, int c, int type, int bx, int by, int bw, int bh) {
    COLORREF baseClr = GetBrickColor(r, c, type);
    HBRUSH br = CreateSolidBrush(baseClr);
    RECT rr = { bx + 1, by + 1, bx + bw - 1, by + bh - 1 };
    FillRect(hdc, &rr, br);
    DeleteObject(br);

    HPEN lightPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, lightPen);
    MoveToEx(hdc, bx + 1, by + bh - 2, NULL);
    LineTo(hdc, bx + 1, by + 1);
    LineTo(hdc, bx + bw - 2, by + 1);

    HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(30, 30, 30));
    SelectObject(hdc, darkPen);
    LineTo(hdc, bx + bw - 2, by + bh - 2);
    LineTo(hdc, bx + 1, by + bh - 2);

    if (type == 9) { // Steel cross pattern
        HPEN steelPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
        SelectObject(hdc, steelPen);
        MoveToEx(hdc, bx + 4, by + 4, NULL); LineTo(hdc, bx + bw - 4, by + bh - 4);
        MoveToEx(hdc, bx + bw - 4, by + 4, NULL); LineTo(hdc, bx + 4, by + bh - 4);
        DeleteObject(steelPen);
    } else if (type == 2) { // Reinforced armor plate
        HPEN armorPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, armorPen);
        MoveToEx(hdc, bx + bw/2, by + 3, NULL); LineTo(hdc, bx + bw/2 - 4, by + bh/2);
        LineTo(hdc, bx + bw/2 + 4, by + bh - 3);
        DeleteObject(armorPen);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(lightPen);
    DeleteObject(darkPen);
}

void DrawGDIPaddle(HDC hdc, int px, int py, int pw, int ph) {
    HBRUSH pBrush = CreateSolidBrush(RGB(50, 70, 100));
    HPEN pPen = CreatePen(PS_SOLID, 1, RGB(120, 150, 190));
    HPEN oldPen = (HPEN)SelectObject(hdc, pPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, pBrush);

    RoundRect(hdc, px, py, px + pw, py + ph, 6, 6);

    HPEN hiPen = CreatePen(PS_SOLID, 1, RGB(200, 230, 255));
    SelectObject(hdc, hiPen);
    MoveToEx(hdc, px + 4, py + 1, NULL);
    LineTo(hdc, px + pw - 4, py + 1);
    DeleteObject(hiPen);

    COLORREF ledClr = ((GetTickCount() / 200) % 2 == 0) ? RGB(0, 255, 255) : RGB(0, 120, 255);
    HBRUSH ledBr = CreateSolidBrush(ledClr);
    RECT l1 = { px + 2, py + 3, px + 5, py + ph - 3 };
    RECT l2 = { px + pw - 5, py + 3, px + pw - 2, py + ph - 3 };
    FillRect(hdc, &l1, ledBr);
    FillRect(hdc, &l2, ledBr);
    DeleteObject(ledBr);

    COLORREF coreClr = sticky_timer > 0 ? RGB(0, 120, 255) :
                       laser_timer > 0 ? RGB(0, 255, 255) : RGB(255, 180, 0);
    HBRUSH coreBr = CreateSolidBrush(coreClr);
    RECT coreRc = { px + pw/2 - 8, py + 3, px + pw/2 + 8, py + ph - 3 };
    FillRect(hdc, &coreRc, coreBr);
    DeleteObject(coreBr);

    if (laser_timer > 0) {
        HBRUSH lasNoz = CreateSolidBrush(RGB(0, 255, 255));
        RECT n1 = { px, py - 4, px + 3, py };
        RECT n2 = { px + pw - 3, py - 4, px + pw, py };
        FillRect(hdc, &n1, lasNoz);
        FillRect(hdc, &n2, lasNoz);
        DeleteObject(lasNoz);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pPen);
    DeleteObject(pBrush);
}

void DrawGDIBall(HDC hdc) {
    if (state == 0) return;
    
    if (!ball_stuck) {
        for (int i = 0; i < MAX_TRAIL; i++) {
            int idx = (trail_idx + i) % MAX_TRAIL;
            if (ball_trail[idx].x != 0 || ball_trail[idx].y != 0) {
                int sz = 2 + (i * (ball_size - 2)) / MAX_TRAIL;
                COLORREF trailClr = pierce_timer > 0 ? RGB(200, 50, 50) : RGB(0, 180, 255);
                HBRUSH tBr = CreateSolidBrush(trailClr);
                HPEN tPen = CreatePen(PS_NULL, 0, 0);
                HGDIOBJ oP = SelectObject(hdc, tPen);
                HGDIOBJ oB = SelectObject(hdc, tBr);
                Ellipse(hdc, ball_trail[idx].x - sz/2, ball_trail[idx].y - sz/2, ball_trail[idx].x + sz/2, ball_trail[idx].y + sz/2);
                SelectObject(hdc, oP); SelectObject(hdc, oB);
                DeleteObject(tPen); DeleteObject(tBr);
            }
        }
    }

    COLORREF outerClr = pierce_timer > 0 ? RGB(255, 30, 30) : RGB(0, 180, 255);
    COLORREF innerClr = RGB(255, 255, 255);

    HBRUSH oBr = CreateSolidBrush(outerClr);
    HPEN oPen = CreatePen(PS_SOLID, 1, outerClr);
    HGDIOBJ oldP = SelectObject(hdc, oPen);
    HGDIOBJ oldB = SelectObject(hdc, oBr);

    Ellipse(hdc, ball_x, ball_y, ball_x + ball_size, ball_y + ball_size);

    HBRUSH iBr = CreateSolidBrush(innerClr);
    SelectObject(hdc, iBr);
    Ellipse(hdc, ball_x + 2, ball_y + 2, ball_x + ball_size - 2, ball_y + ball_size - 2);

    SelectObject(hdc, oldP);
    SelectObject(hdc, oldB);
    DeleteObject(oPen); DeleteObject(oBr); DeleteObject(iBr);
}

void DrawGDIPowerup(HDC hdc, int x, int y, int type) {
    COLORREF clr = RGB(255, 255, 255);
    char badge[2] = "?";
    if (type == 1) { clr = RGB(255, 200, 0); badge[0] = 'E'; }
    else if (type == 2) { clr = RGB(50, 220, 50); badge[0] = '+'; }
    else if (type == 3) { clr = RGB(255, 50, 50); badge[0] = 'P'; }
    else if (type == 4) { clr = RGB(50, 100, 255); badge[0] = 'S'; }
    else if (type == 5) { clr = RGB(0, 255, 255); badge[0] = 'L'; }

    HBRUSH pwBr = CreateSolidBrush(clr);
    HPEN pwPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HGDIOBJ oP = SelectObject(hdc, pwPen);
    HGDIOBJ oB = SelectObject(hdc, pwBr);

    RoundRect(hdc, x - 7, y - 7, x + 7, y + 7, 6, 6);

    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x - 3, y - 6, badge, 1);

    SelectObject(hdc, oP);
    SelectObject(hdc, oB);
    DeleteObject(pwPen); DeleteObject(pwBr);
}

void DrawGDIUFO(HDC hdc, int x, int y) {
    HBRUSH domeBr = CreateSolidBrush(RGB(0, 255, 255));
    HBRUSH bodyBr = CreateSolidBrush(RGB(180, 40, 180));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HGDIOBJ oP = SelectObject(hdc, nullPen);
    
    HGDIOBJ oB = SelectObject(hdc, domeBr);
    Ellipse(hdc, x + 8, y, x + 22, y + 10);

    SelectObject(hdc, bodyBr);
    Ellipse(hdc, x, y + 3, x + 30, y + 11);

    COLORREF lightClr = ((GetTickCount() / 150) % 2 == 0) ? RGB(255, 0, 255) : RGB(0, 255, 255);
    HBRUSH lightBr = CreateSolidBrush(lightClr);
    SelectObject(hdc, lightBr);
    RECT l1 = { x + 5, y + 8, x + 8, y + 10 };
    RECT l2 = { x + 14, y + 9, x + 17, y + 11 };
    RECT l3 = { x + 22, y + 8, x + 25, y + 10 };
    FillRect(hdc, &l1, lightBr);
    FillRect(hdc, &l2, lightBr);
    FillRect(hdc, &l3, lightBr);

    SelectObject(hdc, oP);
    SelectObject(hdc, oB);
    DeleteObject(domeBr); DeleteObject(bodyBr); DeleteObject(lightBr); DeleteObject(nullPen);
}

void LoadHighScore() {
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read;
        int buf[2] = {0, 0};
        if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
            high_score = buf[0];
            lifetime_bricks = buf[1];
        }
        CloseHandle(hFile);
    }
}

void SaveHighScore() {
    if (score > high_score) high_score = score;
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        int buf[2] = { high_score, lifetime_bricks };
        WriteFile(hFile, buf, sizeof(buf), &written, NULL);
        CloseHandle(hFile);
    }
}

void InitLevel() {
    bricks_left = 0;
    power_active = 0;
    pierce_timer = 0;
    paddle_timer = 0;
    sticky_timer = 0;
    laser_timer = 0;
    ball_stuck = 0;
    for(int i=0; i<MAX_LASERS; i++) lasers_active[i] = 0;
    for(int i=0; i<MAX_PARTICLES; i++) particles[i].life = 0;
    for(int i=0; i<MAX_TRAIL; i++) { ball_trail[i].x = 0; ball_trail[i].y = 0; }
    ufo_active = 0;
    ufo_timer = 300;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int v = 0;
            int L = level % 10;
            if (L == 1) {
                if (r == 1 && c % 3 == 1) v = 9; else if (r == 2) v = 2; else v = 1;
            } else if (L == 2) {
                if ((r+c)%2 == 0) v = 1; else if (r == 0) v = 9;
            } else if (L == 3) {
                if (c >= r && c < COLS - r) v = (r == 0) ? 2 : 1;
            } else if (L == 4) {
                if (r==1 && (c==2 || c==7)) v = 9;
                else if (r==3 && (c>1 && c<8)) v = 2;
                else if (r==4 && (c==2 || c==7)) v = 1;
            } else if (L == 5) {
                if (c == 2 || c == 7) v = 9; else v = (r%2==0) ? 2 : 1;
            } else if (L == 6) {
                if ((r+c)%2 == 0) v = 2; else v = 9;
                if (v == 9 && (r==4 || r==0)) v = 1;
            } else if (L == 7) {
                if (MyAbs(r - 2) + MyAbs(c - 4) < 3) v = 2;
            } else if (L == 8) {
                if (c == 1 || c == 8) v = 9; else v = 2;
            } else if (L == 9) {
                if (c >= r && c < COLS - r) v = 2;
            } else {
                v = 2;
            }
            bricks[r][c] = v;
            if (v != 0 && v != 9) bricks_left++;
        }
    }
    if (bricks_left == 0) { bricks[0][0] = 1; bricks_left = 1; }
    
    pad_w = (diff == 1) ? 40 : 60;
    pad_x = W / 2 - pad_w / 2;
    ball_x = W / 2;
    ball_y = H - 50;
    ball_dx = speed * ((GetTickCount() % 2 == 0) ? 1 : -1);
    ball_dy = -speed;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            LoadHighScore();
            SetTimer(hwnd, TIMER_ID, 16, NULL);
            break;
        case WM_TIMER:
            if (state == 1) {
                if (GetAsyncKeyState(VK_LEFT) & 0x8000) pad_x -= 6;
                if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pad_x += 6;
                if (pad_x < 0) pad_x = 0;
                if (pad_x > W - pad_w) pad_x = W - pad_w;

                UpdateParticles();

                if (ball_stuck) {
                    ball_x = pad_x + ball_stuck_offset;
                    ball_y = H - 30 - ball_size;
                    if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
                        ball_stuck = 0;
                        ball_dy = -speed;
                    }
                } else {
                    ball_x += ball_dx;
                    ball_y += ball_dy;
                    RecordBallTrail(ball_x + ball_size/2, ball_y + ball_size/2);

                    // Wall collisions
                    if (ball_x < 0) { ball_x = 0; ball_dx = -ball_dx; }
                    if (ball_x > W - ball_size) { ball_x = W - ball_size; ball_dx = -ball_dx; }
                    if (ball_y < 0) { ball_y = 0; ball_dy = -ball_dy; }

                    // Paddle collision
                    if (ball_y + ball_size > H - 30 && ball_y < H - 30 + pad_h) {
                        if (ball_x + ball_size > pad_x && ball_x < pad_x + pad_w) {
                            ball_y = H - 30 - ball_size;
                            ball_dy = -ball_dy;
                            int hit_pos = (ball_x + ball_size / 2) - (pad_x + pad_w / 2);
                            ball_dx = (hit_pos / 5);
                            if (ball_dx == 0) ball_dx = (ball_dx > 0) ? 1 : -1;
                            MessageBeep(0xFFFFFFFF);
                            SpawnParticles(ball_x + ball_size/2, H - 30, RGB(0, 255, 255), 5);
                            if (sticky_timer > 0) {
                                ball_stuck = 1;
                                ball_stuck_offset = ball_x - pad_x;
                            }
                        }
                    }

                    // Bottom collision
                    if (ball_y > H) {
                        lives--;
                        power_active = 0;
                        sticky_timer = 0;
                        laser_timer = 0;
                        ball_stuck = 0;
                        if (lives <= 0) {
                            SaveHighScore();
                            state = 2; // Game Over
                            MessageBeep(MB_ICONEXCLAMATION);
                        } else {
                            pad_w = (diff == 1) ? 40 : 60;
                            paddle_timer = 0;
                            pad_x = W / 2 - pad_w / 2;
                            ball_x = W / 2;
                            ball_y = H - 50;
                            ball_dx = speed * (GetTickCount() % 2 == 0 ? 1 : -1);
                            ball_dy = -speed;
                            MessageBeep(MB_ICONASTERISK);
                        }
                    }
                }

                // Power-up falling
                if (power_active) {
                    power_y += 3;
                    if (power_y + 10 > H - 30 && power_y < H - 30 + pad_h) {
                        if (power_x + 10 > pad_x && power_x < pad_x + pad_w) {
                            power_active = 0;
                            MessageBeep(MB_OK);
                            SpawnParticles(power_x, power_y, RGB(255, 255, 255), 10);
                            if (power_type == 1) { paddle_timer = 300; pad_w = (diff == 1) ? 80 : 100; }
                            else if (power_type == 2) { lives++; }
                            else if (power_type == 3) { pierce_timer = 300; }
                            else if (power_type == 4) { sticky_timer = 300; }
                            else if (power_type == 5) { laser_timer = 300; }
                        }
                    }
                    if (power_y > H) power_active = 0;
                }

                // Timers
                if (paddle_timer > 0) { paddle_timer--; if (paddle_timer == 0) pad_w = (diff == 1) ? 40 : 60; }
                if (pierce_timer > 0) pierce_timer--;
                if (sticky_timer > 0) sticky_timer--;
                
                // Lasers logic
                if (laser_timer > 0) {
                    laser_timer--;
                    if (laser_timer % 30 == 0) {
                        for (int i=0; i<MAX_LASERS; i++) {
                            if (!lasers_active[i]) {
                                lasers_active[i] = 1;
                                lasers_x[i] = pad_x + pad_w/2;
                                lasers_y[i] = H - 30;
                                break;
                            }
                        }
                    }
                }

                // UFO logic
                if (ufo_active) {
                    ufo_x += ufo_dx;
                    if (ufo_x < 0 || ufo_x > W - 30) ufo_dx = -ufo_dx;
                    if (ball_x + ball_size > ufo_x && ball_x < ufo_x + 30 && ball_y + ball_size > ufo_y && ball_y < ufo_y + 10) {
                        ufo_active = 0;
                        ball_dy = -ball_dy;
                        score += 50 + diff * 50;
                        MessageBeep(0xFFFFFFFF);
                        SpawnParticles(ufo_x + 15, ufo_y + 5, RGB(255, 0, 255), 15);
                    }
                } else {
                    if (ufo_timer > 0) ufo_timer--;
                    else if (level >= 3) { ufo_active = 1; ufo_x = 0; ufo_dx = 2 + diff; ufo_timer = 400 + (GetTickCount() % 400); }
                }

                int br_w = W / COLS;
                int br_h = 20;

                // Move lasers
                for(int i=0; i<MAX_LASERS; i++) {
                    if (lasers_active[i]) {
                        lasers_y[i] -= 5;
                        if (lasers_y[i] < 0) lasers_active[i] = 0;
                        else {
                            int r = (lasers_y[i] - 40) / br_h;
                            int c = lasers_x[i] / br_w;
                            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                                if (bricks[r][c] != 0 && bricks[r][c] != 9) {
                                    lasers_active[i] = 0;
                                    if (bricks[r][c] == 2) {
                                        bricks[r][c] = 1; score += 5; lifetime_bricks++;
                                        SpawnParticles(lasers_x[i], lasers_y[i], RGB(240, 60, 60), 6);
                                    } else {
                                        SpawnParticles(lasers_x[i], lasers_y[i], GetBrickColor(r, c, bricks[r][c]), 10);
                                        bricks[r][c] = 0; bricks_left--; score += 10; lifetime_bricks++;
                                        if (bricks_left == 0) { speed++; level++; InitLevel(); }
                                    }
                                }
                            }
                        }
                    }
                }

                // Bricks collision
                if (!ball_stuck) {
                    for (int r = 0; r < ROWS; r++) {
                        for (int c = 0; c < COLS; c++) {
                            if (bricks[r][c]) {
                                int bx = c * br_w;
                                int by = r * br_h + 40;
                                if (ball_x + ball_size > bx && ball_x < bx + br_w &&
                                    ball_y + ball_size > by && ball_y < by + br_h) {
                                    if (bricks[r][c] == 9) {
                                        ball_dy = -ball_dy;
                                        MessageBeep(0xFFFFFFFF);
                                        SpawnParticles(ball_x, ball_y, RGB(150, 150, 150), 5);
                                    } else if (bricks[r][c] == 2) {
                                        if (pierce_timer > 0) {
                                            SpawnParticles(bx + br_w/2, by + br_h/2, RGB(240, 60, 60), 12);
                                            bricks[r][c] = 0; bricks_left--; score += 5 + (diff * 5); lifetime_bricks++;
                                        } else {
                                            SpawnParticles(bx + br_w/2, by + br_h/2, RGB(240, 60, 60), 6);
                                            bricks[r][c] = 1; ball_dy = -ball_dy; score += 5 + (diff * 5); lifetime_bricks++;
                                        }
                                        MessageBeep(0xFFFFFFFF);
                                    } else {
                                        SpawnParticles(bx + br_w/2, by + br_h/2, GetBrickColor(r, c, bricks[r][c]), 12);
                                        bricks[r][c] = 0;
                                        bricks_left--;
                                        if (pierce_timer <= 0) ball_dy = -ball_dy;
                                        score += 10 + (diff * 10);
                                        lifetime_bricks++;
                                        MessageBeep(0xFFFFFFFF);
                                        
                                        if (!power_active && (GetTickCount() % 5 == 0)) {
                                            power_active = 1;
                                            power_type = (GetTickCount() % 5) + 1; // 1 to 5
                                            power_x = bx + br_w / 2;
                                            power_y = by;
                                        }
                                    }
                                    if (bricks_left == 0) {
                                        if (level % 3 == 0) speed++;
                                        level++;
                                        InitLevel();
                                    }
                                    goto out_loops;
                                }
                            }
                        }
                    }
                out_loops:;
                }
            } else if (state == 0 || state == 2) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    diff = 0; speed = 3; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 60;
                }
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    diff = 1; speed = 5; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 40;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HGDIOBJ hOld = SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(20, 20, 30));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            
            if (state == 0) {
                char* t1 = "KBreakout";
                char* t2 = "Press ENTER for Easy";
                char* t3 = "Press SPACE for Hard";
                TextOutA(memDC, W/2 - 35, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t3, lstrlenA(t3));
            } else if (state == 2) {
                char* t1 = "GAME OVER";
                char* t2 = "Press ENTER for Easy";
                char* t3 = "Press SPACE for Hard";
                TextOutA(memDC, W/2 - 40, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t3, lstrlenA(t3));
            } else {
                DrawGDIPaddle(memDC, pad_x, H - 30, pad_w, pad_h);

                int br_w = W / COLS;
                int br_h = 20;
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            DrawGDIBrick(memDC, r, c, bricks[r][c], c * br_w, r * br_h + 40, br_w, br_h);
                        }
                    }
                }
                
                if (power_active) DrawGDIPowerup(memDC, power_x, power_y, power_type);
                if (ufo_active) DrawGDIUFO(memDC, ufo_x, ufo_y);

                HBRUSH las = CreateSolidBrush(RGB(0, 255, 255));
                for(int i=0; i<MAX_LASERS; i++) {
                    if(lasers_active[i]) {
                        RECT rl = { lasers_x[i]-2, lasers_y[i]-5, lasers_x[i]+2, lasers_y[i]+5 };
                        FillRect(memDC, &rl, las);
                    }
                }
                DeleteObject(las);

                // Render Particles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].life > 0) {
                        HBRUSH pBr = CreateSolidBrush(particles[i].color);
                        RECT prc = { (int)particles[i].x, (int)particles[i].y, (int)particles[i].x + 3, (int)particles[i].y + 3 };
                        FillRect(memDC, &prc, pBr);
                        DeleteObject(pBr);
                    }
                }

                DrawGDIBall(memDC);
            }
            
            char sStr[64];
            wsprintfA(sStr, "L%d Sc:%d Hi:%d Lv:%d Brks:%d", level, score, high_score, lives, lifetime_bricks);
            TextOutA(memDC, 10, 10, sStr, lstrlenA(sStr));
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            SaveHighScore();
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
    wc.lpszClassName = "KBreakoutApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KBreakoutApp", "KBreakout", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}

