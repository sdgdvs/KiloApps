#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <process.h>

#define W 800
#define H 600

int _fltused = 0;

HBITMAP hBitmap = NULL;
DWORD* pixels = NULL;
int bmpW = 0, bmpH = 0;

double minRe = -2.0, maxRe = 1.0;
double minIm = -1.2, maxIm = 1.2;
unsigned int max_iter = 100;
int theme = 0; // 0: Fire, 1: Ocean, 2: Cyberpunk, 3: BW, 4: Emerald, 5: Sunset, 6: Custom
int fractalType = 0; // 0: Mandelbrot, 1: Burning Ship, 2: Tricorn, 3: Celtic, 4: Buffalo
int isJulia = 0;
double juliaCRe = 0.0, juliaCIm = 0.0;

unsigned char customColor1[3] = {255, 0, 85};
unsigned char customColor2[3] = {0, 229, 255};

const char* fractalNames[] = {
    "Mandelbrot",
    "Burning Ship",
    "Tricorn",
    "Celtic",
    "Buffalo"
};
#define NUM_FRACTAL_TYPES 5

typedef struct {
    const char* name;
    int fType;
    int isJ;
    double jRe, jIm;
    double minRe, maxRe, minIm, maxIm;
    unsigned int iter;
} Landmark;

static const Landmark landmarks[] = {
    {"Mandelbrot Overview", 0, 0, 0.0, 0.0, -2.0, 1.0, -1.2, 1.2, 100},
    {"Seahorse Valley", 0, 0, 0.0, 0.0, -0.755, -0.745, 0.095, 0.105, 350},
    {"Elephant Valley", 0, 0, 0.0, 0.0, 0.265, 0.285, -0.010, 0.010, 300},
    {"Triple Spiral", 0, 0, 0.0, 0.0, -0.0885, -0.0865, 0.654, 0.656, 450},
    {"Mini Mandelbrot", 0, 0, 0.0, 0.0, -1.775, -1.765, -0.005, 0.005, 500},
    {"Burning Ship Main", 1, 0, 0.0, 0.0, -1.8, 1.0, -1.8, 1.0, 150},
    {"Ship Needle", 1, 0, 0.0, 0.0, -0.46, -0.44, -0.58, -0.56, 400},
    {"Tricorn Main", 2, 0, 0.0, 0.0, -2.0, 1.0, -1.5, 1.5, 120},
    {"Celtic Ring", 3, 0, 0.0, 0.0, -2.0, 1.0, -1.2, 1.2, 150},
    {"Buffalo Heart", 4, 0, 0.0, 0.0, -2.0, 1.0, -1.5, 1.5, 150},
    {"Julia Dendrite", 0, 1, -0.4, 0.6, -1.5, 1.5, -1.5, 1.5, 200},
    {"Julia San Marco", 0, 1, -0.75, 0.0, -1.6, 1.6, -1.2, 1.2, 200}
};
#define NUM_LANDMARKS (sizeof(landmarks) / sizeof(landmarks[0]))
int currentLandmark = 0;

#define MAX_HISTORY 256
typedef struct {
    double minRe, maxRe, minIm, maxIm;
    unsigned int max_iter;
    int isJulia;
    double juliaCRe, juliaCIm;
    int theme;
    int fractalType;
    unsigned char cc1[3], cc2[3];
} ViewState;

ViewState history[MAX_HISTORY];
int history_idx = -1;
int history_max = -1;

// ==========================================
// FAST MATH & VISUAL EFFECTS ENGINE (LOOP 3)
// ==========================================
static double FastSin(double x) {
    while (x > 3.141592653589793) x -= 6.283185307179586;
    while (x < -3.141592653589793) x += 6.283185307179586;
    double x2 = x * x;
    double x3 = x * x2;
    double x5 = x3 * x2;
    double x7 = x5 * x2;
    return x - (x3 / 6.0) + (x5 / 120.0) - (x7 / 5040.0);
}

static double FastCos(double x) {
    return FastSin(x + 1.5707963267948966);
}

static unsigned int rng_state = 123456789;
static unsigned int XorShift32() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}
static double RandomDouble(double minVal, double maxVal) {
    return minVal + (double)(XorShift32() % 10000) / 10000.0 * (maxVal - minVal);
}

// Screen Shake
double shakeMagnitude = 0.0;
double shakeAngle = 0.0;
double glintProgress = 0.0;

// Atmospheric Floating Quantum Motes
#define NUM_MOTES 35
typedef struct {
    double x, y;
    double vx, vy;
    double size;
    double pulse;
    COLORREF color;
} AmbientMote;
AmbientMote motes[NUM_MOTES];

// 4-Layer Kinematic Particle Engine
#define MAX_PARTICLES 128
typedef struct {
    int active;
    int layer; // 0: Spark, 1: Plasma Puff, 2: Crystal Shard, 3: Celebration Star
    double x, y;
    double vx, vy;
    double size, growth;
    double alpha, decay;
    double gravity, drag;
    double angle, rotSpeed;
    COLORREF color;
} Particle;
Particle particles[MAX_PARTICLES];

// Dual-Tier Shockwaves
#define MAX_SHOCKWAVES 12
typedef struct {
    int active;
    double x, y;
    double radius, maxRadius;
    double speed;
    double alpha, decay;
    int width;
    COLORREF color;
} Shockwave;
Shockwave shockwaves[MAX_SHOCKWAVES];

void InitAmbientMotes() {
    for (int i = 0; i < NUM_MOTES; i++) {
        motes[i].x = RandomDouble(0, W);
        motes[i].y = RandomDouble(0, H);
        motes[i].vx = RandomDouble(-0.3, 0.3);
        motes[i].vy = RandomDouble(-0.6, -0.2);
        motes[i].size = RandomDouble(1.5, 3.5);
        motes[i].pulse = RandomDouble(0, 6.28);
        int pick = XorShift32() % 3;
        if (pick == 0) motes[i].color = RGB(100, 200, 255);
        else if (pick == 1) motes[i].color = RGB(255, 215, 0);
        else motes[i].color = RGB(180, 100, 255);
    }
}

void TriggerScreenShake(double intensity) {
    shakeMagnitude += intensity;
    if (shakeMagnitude > 25.0) shakeMagnitude = 25.0;
}

void SpawnShockwave(double x, double y, double maxR, COLORREF color) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!shockwaves[i].active) {
            shockwaves[i].active = 1;
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].radius = 2.0;
            shockwaves[i].maxRadius = maxR;
            shockwaves[i].speed = 6.0;
            shockwaves[i].alpha = 1.0;
            shockwaves[i].decay = 0.045;
            shockwaves[i].width = 3;
            shockwaves[i].color = RGB(255, 255, 255);
            break;
        }
    }
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!shockwaves[i].active) {
            shockwaves[i].active = 1;
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].radius = 1.0;
            shockwaves[i].maxRadius = maxR * 1.35;
            shockwaves[i].speed = 4.0;
            shockwaves[i].alpha = 0.85;
            shockwaves[i].decay = 0.028;
            shockwaves[i].width = 4;
            shockwaves[i].color = color;
            break;
        }
    }
}

void SpawnParticleBurst(double x, double y, int count, COLORREF baseColor) {
    for (int c = 0; c < count; c++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].active = 1;
                particles[i].layer = XorShift32() % 4;
                particles[i].x = x;
                particles[i].y = y;
                double angle = RandomDouble(0, 6.28318);
                double speed = RandomDouble(2.0, 7.5);
                particles[i].vx = FastCos(angle) * speed;
                particles[i].vy = FastSin(angle) * speed;
                particles[i].alpha = 1.0;
                particles[i].angle = RandomDouble(0, 6.28);
                particles[i].rotSpeed = RandomDouble(-0.3, 0.3);

                if (particles[i].layer == 0) { // Needle Spark
                    particles[i].size = 2.0;
                    particles[i].growth = 0.0;
                    particles[i].decay = 0.05;
                    particles[i].gravity = 0.0;
                    particles[i].drag = 0.94;
                    particles[i].color = RGB(255, 255, 255);
                } else if (particles[i].layer == 1) { // Plasma Puff
                    particles[i].size = 6.0;
                    particles[i].growth = 0.3;
                    particles[i].decay = 0.035;
                    particles[i].gravity = -0.05;
                    particles[i].drag = 0.92;
                    particles[i].color = baseColor;
                } else if (particles[i].layer == 2) { // Crystal Shard
                    particles[i].size = 4.0;
                    particles[i].growth = -0.05;
                    particles[i].decay = 0.028;
                    particles[i].gravity = 0.22;
                    particles[i].drag = 0.97;
                    particles[i].color = RGB(100, 220, 255);
                } else { // Celebration Star
                    particles[i].size = 5.0;
                    particles[i].growth = 0.1;
                    particles[i].decay = 0.025;
                    particles[i].gravity = 0.02;
                    particles[i].drag = 0.96;
                    particles[i].color = RGB(255, 215, 0);
                }
                break;
            }
        }
    }
}

void TriggerImpact(double x, double y, double intensity, int count) {
    TriggerScreenShake(intensity);
    COLORREF c = RGB(100, 180, 255);
    if (theme == 0) c = RGB(255, 120, 30);
    else if (theme == 1) c = RGB(0, 200, 255);
    else if (theme == 2) c = RGB(255, 0, 180);
    else if (theme == 4) c = RGB(50, 220, 100);
    else if (theme == 5) c = RGB(255, 80, 140);
    
    SpawnShockwave(x, y, 60.0 + intensity * 6.0, c);
    SpawnParticleBurst(x, y, count, c);
}

void SaveState() {
    if (history_idx < MAX_HISTORY - 1) {
        history_idx++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i+1];
        }
    }
    history[history_idx].minRe = minRe;
    history[history_idx].maxRe = maxRe;
    history[history_idx].minIm = minIm;
    history[history_idx].maxIm = maxIm;
    history[history_idx].max_iter = max_iter;
    history[history_idx].isJulia = isJulia;
    history[history_idx].juliaCRe = juliaCRe;
    history[history_idx].juliaCIm = juliaCIm;
    history[history_idx].theme = theme;
    history[history_idx].fractalType = fractalType;
    history[history_idx].cc1[0] = customColor1[0];
    history[history_idx].cc1[1] = customColor1[1];
    history[history_idx].cc1[2] = customColor1[2];
    history[history_idx].cc2[0] = customColor2[0];
    history[history_idx].cc2[1] = customColor2[1];
    history[history_idx].cc2[2] = customColor2[2];
    history_max = history_idx;
}

void LoadState(int idx) {
    if (idx >= 0 && idx <= history_max) {
        minRe = history[idx].minRe;
        maxRe = history[idx].maxRe;
        minIm = history[idx].minIm;
        maxIm = history[idx].maxIm;
        max_iter = history[idx].max_iter;
        isJulia = history[idx].isJulia;
        juliaCRe = history[idx].juliaCRe;
        juliaCIm = history[idx].juliaCIm;
        theme = history[idx].theme;
        fractalType = history[idx].fractalType;
        customColor1[0] = history[idx].cc1[0];
        customColor1[1] = history[idx].cc1[1];
        customColor1[2] = history[idx].cc1[2];
        customColor2[0] = history[idx].cc2[0];
        customColor2[1] = history[idx].cc2[1];
        customColor2[2] = history[idx].cc2[2];
    }
}

void GetColors(unsigned int n, unsigned int iter, int t, unsigned char* r, unsigned char* g, unsigned char* b) {
    if (t == 0) { // Fire
        *r = (unsigned char)((n * 255) / iter);
        *g = (unsigned char)((n * n * 255) / (iter * iter));
        *b = (unsigned char)((n * 128) / iter);
    } else if (t == 1) { // Ocean
        *r = (unsigned char)((n * 128) / iter);
        *g = (unsigned char)((n * n * 255) / (iter * iter));
        *b = (unsigned char)((n * 255) / iter);
    } else if (t == 2) { // Cyberpunk
        *r = (unsigned char)((n * 5) % 256);
        *g = (unsigned char)((n * 2) % 128);
        *b = (unsigned char)(255 - ((n * 3) % 256));
    } else if (t == 3) { // BW
        unsigned char v = ((n % 20) > 10) ? 255 : 0;
        *r = v; *g = v; *b = v;
    } else if (t == 4) { // Emerald Matrix
        *r = (unsigned char)((n * 30) % 100);
        *g = (unsigned char)((n * 255) / iter);
        *b = (unsigned char)((n * 90) % 180);
    } else if (t == 5) { // Sunset Neon
        *r = (unsigned char)((n * 255) / iter);
        *g = (unsigned char)((n * 70) / iter);
        *b = (unsigned char)((n * 190) / iter);
    } else if (t == 6) { // Custom
        double f = (double)n / iter;
        *r = (unsigned char)(customColor1[0] + f * (customColor2[0] - customColor1[0]));
        *g = (unsigned char)(customColor1[1] + f * (customColor2[1] - customColor1[1]));
        *b = (unsigned char)(customColor1[2] + f * (customColor2[2] - customColor1[2]));
    }
}

typedef struct {
    DWORD* buffer;
    int width;
    int height;
    int startY;
    int endY;
    double mRe, mxRe, mIm, mxIm;
    unsigned int mIter;
    int isJ;
    double jCRe, jCIm;
    int th;
    int fType;
} RenderTask;

DWORD WINAPI RenderThreadProc(LPVOID lpParam) {
    RenderTask* task = (RenderTask*)lpParam;
    double re_factor = (task->mxRe - task->mRe) / (task->width - 1);
    double im_factor = (task->mxIm - task->mIm) / (task->height - 1);
    
    for (int y = task->startY; y < task->endY; ++y) {
        double c_im_view = task->mxIm - y * im_factor;
        for (int x = 0; x < task->width; ++x) {
            double c_re_view = task->mRe + x * re_factor;
            double c_re = task->isJ ? task->jCRe : c_re_view;
            double c_im = task->isJ ? task->jCIm : c_im_view;
            double Z_re = c_re_view, Z_im = c_im_view;
            int isInside = 1;
            unsigned int n = 0;
            
            for (n = 0; n < task->mIter; ++n) {
                double Z_re2 = Z_re * Z_re, Z_im2 = Z_im * Z_im;
                if (Z_re2 + Z_im2 > 4.0) {
                    isInside = 0;
                    break;
                }
                
                if (task->fType == 1) { // Burning Ship
                    double a = (Z_re < 0.0) ? -Z_re : Z_re;
                    double b = (Z_im < 0.0) ? -Z_im : Z_im;
                    Z_im = -2.0 * a * b + c_im;
                    Z_re = Z_re2 - Z_im2 + c_re;
                } else if (task->fType == 2) { // Tricorn
                    Z_im = -2.0 * Z_re * Z_im + c_im;
                    Z_re = Z_re2 - Z_im2 + c_re;
                } else if (task->fType == 3) { // Celtic
                    double re_temp = Z_re2 - Z_im2;
                    if (re_temp < 0.0) re_temp = -re_temp;
                    Z_im = 2.0 * Z_re * Z_im + c_im;
                    Z_re = re_temp + c_re;
                } else if (task->fType == 4) { // Buffalo
                    double re_temp = Z_re2 - Z_im2;
                    if (re_temp < 0.0) re_temp = -re_temp;
                    double a = (Z_re < 0.0) ? -Z_re : Z_re;
                    Z_im = -2.0 * a * Z_im + c_im;
                    Z_re = re_temp + c_re;
                } else { // 0: Standard Mandelbrot
                    Z_im = 2.0 * Z_re * Z_im + c_im;
                    Z_re = Z_re2 - Z_im2 + c_re;
                }
            }
            if (isInside) {
                task->buffer[y * task->width + x] = 0; // Black
            } else {
                unsigned char r, g, b;
                GetColors(n, task->mIter, task->th, &r, &g, &b);
                task->buffer[y * task->width + x] = (r << 16) | (g << 8) | b;
            }
        }
    }
    return 0;
}

void RenderMandelbrotToBuffer(DWORD* buffer, int width, int height) {
    if (!buffer || width <= 1 || height <= 1) return;
    
    int numThreads = 8;
    HANDLE threads[8];
    RenderTask tasks[8];
    
    int chunkH = height / numThreads;
    for (int i = 0; i < numThreads; i++) {
        tasks[i].buffer = buffer;
        tasks[i].width = width;
        tasks[i].height = height;
        tasks[i].startY = i * chunkH;
        tasks[i].endY = (i == numThreads - 1) ? height : (i + 1) * chunkH;
        tasks[i].mRe = minRe; tasks[i].mxRe = maxRe;
        tasks[i].mIm = minIm; tasks[i].mxIm = maxIm;
        tasks[i].mIter = max_iter;
        tasks[i].isJ = isJulia;
        tasks[i].jCRe = juliaCRe; tasks[i].jCIm = juliaCIm;
        tasks[i].th = theme;
        tasks[i].fType = fractalType;
        
        threads[i] = CreateThread(NULL, 0, RenderThreadProc, &tasks[i], 0, NULL);
    }
    
    WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);
    for (int i = 0; i < numThreads; i++) {
        CloseHandle(threads[i]);
    }
}

void ResizeBitmap(HWND hwnd, int width, int height) {
    if (width == 0 || height == 0) return;
    if (hBitmap) DeleteObject(hBitmap);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    HDC hdc = GetDC(hwnd);
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    ReleaseDC(hwnd, hdc);
    
    bmpW = width;
    bmpH = height;
    
    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
}

void Zoom(double factor, int mouseX, int mouseY) {
    if (bmpW <= 1 || bmpH <= 1) return;
    
    double re_factor = (maxRe - minRe) / (bmpW - 1);
    double im_factor = (maxIm - minIm) / (bmpH - 1);
    
    double centerRe = minRe + mouseX * re_factor;
    double centerIm = maxIm - mouseY * im_factor;
    
    double newWRe = (maxRe - minRe) * factor;
    double newWIm = (maxIm - minIm) * factor;
    
    if (newWRe < 1e-13 || newWRe > 10.0) return;
    
    minRe = centerRe - ((double)mouseX / bmpW) * newWRe;
    maxRe = minRe + newWRe;
    
    maxIm = centerIm + ((double)mouseY / bmpH) * newWIm;
    minIm = maxIm - newWIm;
    
    double zoomLevel = 3.0 / newWRe;
    unsigned int needed_iter = 100;
    double temp_z = zoomLevel;
    int limit = 0;
    while (temp_z > 5.0 && limit < 50) {
        needed_iter += 30;
        temp_z /= 2.0;
        limit++;
    }
    if (needed_iter > max_iter && zoomLevel > 5.0) {
        max_iter = needed_iter;
        if (max_iter > 2000) max_iter = 2000;
    }
    
    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
}

void SaveImage4K(HWND hwnd) {
    int expW = 3840;
    int expH = 2160;
    int imageSize32 = expW * expH * 4;
    DWORD* buffer = (DWORD*)HeapAlloc(GetProcessHeap(), 0, imageSize32);
    if (!buffer) {
        MessageBox(hwnd, "Failed to allocate memory for 4K export.", "Error", MB_OK);
        return;
    }
    
    SetWindowText(hwnd, "KMandel - Rendering 4K image...");
    RenderMandelbrotToBuffer(buffer, expW, expH);
    SetWindowText(hwnd, "KMandel - Press F1 or H for Help");
    
    OPENFILENAME ofn = {0};
    char szFileName[MAX_PATH] = "kmandel_4k.bmp";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "bmp";
    
    if (GetSaveFileName(&ofn)) {
        HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BITMAPFILEHEADER bfh = {0};
            BITMAPINFOHEADER bih = {0};
            
            bfh.bfType = 0x4D42;
            bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize32;
            bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            
            bih.biSize = sizeof(BITMAPINFOHEADER);
            bih.biWidth = expW;
            bih.biHeight = -expH;
            bih.biPlanes = 1;
            bih.biBitCount = 32;
            bih.biCompression = BI_RGB;
            bih.biSizeImage = imageSize32;
            
            DWORD dwWritten;
            WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
            WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
            WriteFile(hFile, buffer, imageSize32, &dwWritten, NULL);
            CloseHandle(hFile);
        }
    }
    HeapFree(GetProcessHeap(), 0, buffer);
}

void PickColor(HWND hwnd, unsigned char* color) {
    CHOOSECOLOR cc;
    static COLORREF acrCustClr[16]; 
    memset(&cc, 0, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = RGB(color[0], color[1], color[2]);
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColor(&cc)) {
        color[0] = GetRValue(cc.rgbResult);
        color[1] = GetGValue(cc.rgbResult);
        color[2] = GetBValue(cc.rgbResult);
    }
}

void ApplyLandmark(int idx) {
    if (idx < 0 || idx >= NUM_LANDMARKS) return;
    fractalType = landmarks[idx].fType;
    isJulia = landmarks[idx].isJ;
    juliaCRe = landmarks[idx].jRe;
    juliaCIm = landmarks[idx].jIm;
    minRe = landmarks[idx].minRe;
    maxRe = landmarks[idx].maxRe;
    minIm = landmarks[idx].minIm;
    maxIm = landmarks[idx].maxIm;
    max_iter = landmarks[idx].iter;
    SaveState();
    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
}

void DrawCornerFiligreeGDI(HDC hdc, int w, int h) {
    int margin = 10;
    int len = 32;
    int notch = 8;
    
    HPEN hPenBlue = CreatePen(PS_SOLID, 2, RGB(96, 165, 250));
    HPEN hPenGold = CreatePen(PS_SOLID, 1, RGB(251, 191, 36));
    HBRUSH hBrushGold = CreateSolidBrush(RGB(251, 191, 36));
    
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBlue);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushGold);
    
    // 4 corners
    int cx[4] = { margin, w - margin, margin, w - margin };
    int cy[4] = { margin, margin, h - margin, h - margin };
    int dx[4] = { 1, -1, 1, -1 };
    int dy[4] = { 1, 1, -1, -1 };
    
    for (int i = 0; i < 4; i++) {
        SelectObject(hdc, hPenBlue);
        MoveToEx(hdc, cx[i], cy[i] + dy[i] * len, NULL);
        LineTo(hdc, cx[i], cy[i] + dy[i] * notch);
        LineTo(hdc, cx[i] + dx[i] * notch, cy[i]);
        LineTo(hdc, cx[i] + dx[i] * len, cy[i]);
        
        SelectObject(hdc, hPenGold);
        MoveToEx(hdc, cx[i] + dx[i] * 6, cy[i] + dy[i] * (len - 6), NULL);
        LineTo(hdc, cx[i] + dx[i] * 6, cy[i] + dy[i] * 6);
        LineTo(hdc, cx[i] + dx[i] * (len - 6), cy[i] + dy[i] * 6);
        
        // Gold Rivet
        Ellipse(hdc, cx[i] + dx[i] * 10 - 2, cy[i] + dy[i] * 10 - 2, cx[i] + dx[i] * 10 + 3, cy[i] + dy[i] * 10 + 3);
    }
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPenBlue);
    DeleteObject(hPenGold);
    DeleteObject(hBrushGold);
}

void DrawPerimeterGlintGDI(HDC hdc, int w, int h, double prog) {
    int pad = 10;
    int rw = w - pad * 2;
    int rh = h - pad * 2;
    int totalP = (rw + rh) * 2;
    int curDist = (int)(prog * totalP);
    
    int gx = pad, gy = pad;
    if (curDist < rw) {
        gx = pad + curDist; gy = pad;
    } else if (curDist < rw + rh) {
        gx = pad + rw; gy = pad + (curDist - rw);
    } else if (curDist < rw * 2 + rh) {
        gx = pad + rw - (curDist - (rw + rh)); gy = pad + rh;
    } else {
        gx = pad; gy = pad + rh - (curDist - (rw * 2 + rh));
    }
    
    HBRUSH hGlintBrush = CreateSolidBrush(RGB(255, 255, 255));
    HPEN hGlintPen = CreatePen(PS_SOLID, 1, RGB(147, 197, 253));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hGlintBrush);
    HPEN hOldP = (HPEN)SelectObject(hdc, hGlintPen);
    
    Ellipse(hdc, gx - 5, gy - 5, gx + 6, gy + 6);
    
    SelectObject(hdc, hOldB);
    SelectObject(hdc, hOldP);
    DeleteObject(hGlintBrush);
    DeleteObject(hGlintPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitAmbientMotes();
            SetTimer(hwnd, 1, 33, NULL); // 30 FPS visual effects loop
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                // Update screen shake
                if (shakeMagnitude > 0.05) {
                    shakeAngle += 0.8;
                    shakeMagnitude *= 0.88;
                } else {
                    shakeMagnitude = 0.0;
                }
                
                // Update traveling glint
                glintProgress += 0.015;
                if (glintProgress > 1.0) glintProgress -= 1.0;
                
                // Update ambient motes
                for (int i = 0; i < NUM_MOTES; i++) {
                    motes[i].x += motes[i].vx;
                    motes[i].y += motes[i].vy;
                    motes[i].pulse += 0.04;
                    if (motes[i].y < 0) motes[i].y = bmpH;
                    if (motes[i].x < 0) motes[i].x = bmpW;
                    if (motes[i].x > bmpW) motes[i].x = 0;
                }
                
                // Update shockwaves
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].active) {
                        shockwaves[i].radius += shockwaves[i].speed;
                        shockwaves[i].alpha -= shockwaves[i].decay;
                        if (shockwaves[i].alpha <= 0.0 || shockwaves[i].radius >= shockwaves[i].maxRadius) {
                            shockwaves[i].active = 0;
                        }
                    }
                }
                
                // Update particles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].active) {
                        particles[i].x += particles[i].vx;
                        particles[i].y += particles[i].vy;
                        particles[i].vx *= particles[i].drag;
                        particles[i].vy *= particles[i].drag;
                        particles[i].vy += particles[i].gravity;
                        particles[i].size += particles[i].growth;
                        particles[i].alpha -= particles[i].decay;
                        particles[i].angle += particles[i].rotSpeed;
                        if (particles[i].alpha <= 0.0 || particles[i].size <= 0.5) {
                            particles[i].active = 0;
                        }
                    }
                }
                
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            ResizeBitmap(hwnd, nw, nh);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            if ((wParam & MK_SHIFT) && !isJulia) {
                // Interactive Julia bridging
                double re_factor = (maxRe - minRe) / (bmpW - 1);
                double im_factor = (maxIm - minIm) / (bmpH - 1);
                juliaCRe = minRe + x * re_factor;
                juliaCIm = maxIm - y * im_factor;
                
                TriggerImpact(x, y, 9.0, 35);
                
                isJulia = 1;
                minRe = -2.0; maxRe = 2.0;
                minIm = -2.0; maxIm = 2.0;
                max_iter = 100;
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
            } else {
                TriggerImpact(x, y, 4.5, 20);
                Zoom(0.5, x, y); // Zoom in
                SaveState();
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            TriggerImpact(x, y, 3.5, 16);
            Zoom(2.0, x, y); // Zoom out
            SaveState();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == 'F') {
                fractalType = (fractalType + 1) % NUM_FRACTAL_TYPES;
                TriggerImpact(bmpW / 2, bmpH / 2, 6.0, 25);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'L' || wParam == 'P') {
                currentLandmark = (currentLandmark + 1) % NUM_LANDMARKS;
                TriggerImpact(bmpW / 2, bmpH / 2, 8.0, 35);
                ApplyLandmark(currentLandmark);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'R') {
                if (isJulia) {
                    minRe = -2.0; maxRe = 2.0;
                    minIm = -2.0; maxIm = 2.0;
                } else {
                    minRe = -2.0; maxRe = 1.0;
                    minIm = -1.2; maxIm = 1.2;
                }
                max_iter = 100;
                TriggerImpact(bmpW / 2, bmpH / 2, 7.0, 30);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T') {
                theme = (theme + 1) % 7;
                TriggerImpact(bmpW / 2, bmpH / 2, 5.0, 20);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'C') {
                theme = 6; // Switch to Custom
                PickColor(hwnd, customColor1);
                PickColor(hwnd, customColor2);
                TriggerImpact(bmpW / 2, bmpH / 2, 5.0, 20);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'J') {
                if (!isJulia) {
                    isJulia = 1;
                    juliaCRe = minRe + (maxRe - minRe) / 2.0;
                    juliaCIm = minIm + (maxIm - minIm) / 2.0;
                    minRe = -2.0; maxRe = 2.0;
                    minIm = -2.0; maxIm = 2.0;
                } else {
                    isJulia = 0;
                    minRe = -2.0; maxRe = 1.0;
                    minIm = -1.2; maxIm = 1.2;
                }
                max_iter = 100;
                TriggerImpact(bmpW / 2, bmpH / 2, 6.0, 25);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'Z') {
                if (history_idx > 0) {
                    history_idx--;
                    LoadState(history_idx);
                    TriggerImpact(bmpW / 2, bmpH / 2, 4.0, 15);
                    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'Y') {
                if (history_idx < history_max) {
                    history_idx++;
                    LoadState(history_idx);
                    TriggerImpact(bmpW / 2, bmpH / 2, 4.0, 15);
                    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'S') {
                TriggerImpact(bmpW / 2, bmpH / 2, 8.0, 40);
                SaveImage4K(hwnd);
            } else if (wParam == 'H' || wParam == VK_F1) {
                MessageBox(hwnd, 
                    "KMandel Pro Fractal Explorer\n\n"
                    "L / R Click: Zoom In / Out\n"
                    "F: Switch Formula (Mandelbrot, Burning Ship, Tricorn, Celtic, Buffalo)\n"
                    "L / P: Landmark / Bookmark Presets\n"
                    "Shift + Click: Pick Julia Set\n"
                    "J: Toggle Julia / Mandelbrot Mode\n"
                    "T: Toggle Color Theme\n"
                    "C: Customize Dual Palette\n"
                    "Z / Y: Undo / Redo View History\n"
                    "S: Export Ultra-HD 4K Image\n"
                    "R: Reset Viewport Coordinates\n"
                    "F1 / H: Toggle Help",
                    "KMandel Help", MB_OK);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (bmpW > 0 && bmpH > 0) {
                // Double-buffered rendering to prevent flicker
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hbmMem = CreateCompatibleBitmap(hdc, bmpW, bmpH);
                HBITMAP hOldBm = (HBITMAP)SelectObject(hdcMem, hbmMem);
                
                // 1. Calculate Screen-Shake offset
                int shakeX = 0, shakeY = 0;
                if (shakeMagnitude > 0.05) {
                    shakeX = (int)(FastSin(shakeAngle * 2.3) * shakeMagnitude);
                    shakeY = (int)(FastCos(shakeAngle * 1.9) * shakeMagnitude);
                }
                
                // Clear background with deep space dark tone
                HBRUSH hBgBrush = CreateSolidBrush(RGB(7, 10, 18));
                RECT fullRc = { 0, 0, bmpW, bmpH };
                FillRect(hdcMem, &fullRc, hBgBrush);
                DeleteObject(hBgBrush);
                
                // 2. Blit Fractal Bitmap with shake offset
                if (hBitmap) {
                    HDC hdcFractal = CreateCompatibleDC(hdc);
                    HBITMAP hOldF = (HBITMAP)SelectObject(hdcFractal, hBitmap);
                    BitBlt(hdcMem, shakeX, shakeY, bmpW, bmpH, hdcFractal, 0, 0, SRCCOPY);
                    SelectObject(hdcFractal, hOldF);
                    DeleteDC(hdcFractal);
                }
                
                // 3. Draw Ambient Floating Motes
                for (int i = 0; i < NUM_MOTES; i++) {
                    HBRUSH hMoteBrush = CreateSolidBrush(motes[i].color);
                    HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
                    HPEN hOldP = (HPEN)SelectObject(hdcMem, hNullPen);
                    HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hMoteBrush);
                    
                    int mx = (int)motes[i].x + shakeX;
                    int my = (int)motes[i].y + shakeY;
                    int sz = (int)motes[i].size;
                    Ellipse(hdcMem, mx - sz, my - sz, mx + sz + 1, my + sz + 1);
                    
                    SelectObject(hdcMem, hOldP);
                    SelectObject(hdcMem, hOldB);
                    DeleteObject(hMoteBrush);
                }
                
                // 4. Draw Concentric Shockwaves
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].active) {
                        HPEN hSwPen = CreatePen(PS_SOLID, shockwaves[i].width, shockwaves[i].color);
                        HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HPEN hOldP = (HPEN)SelectObject(hdcMem, hSwPen);
                        HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hNullBrush);
                        
                        int sx = (int)shockwaves[i].x + shakeX;
                        int sy = (int)shockwaves[i].y + shakeY;
                        int r = (int)shockwaves[i].radius;
                        Ellipse(hdcMem, sx - r, sy - r, sx + r, sy + r);
                        
                        SelectObject(hdcMem, hOldP);
                        SelectObject(hdcMem, hOldB);
                        DeleteObject(hSwPen);
                    }
                }
                
                // 5. Draw 4-Layer Particles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].active) {
                        int px = (int)particles[i].x + shakeX;
                        int py = (int)particles[i].y + shakeY;
                        int psz = (int)particles[i].size;
                        if (psz < 1) psz = 1;
                        
                        if (particles[i].layer == 0) { // Needle Spark
                            HPEN hSpkPen = CreatePen(PS_SOLID, (int)particles[i].size, particles[i].color);
                            HPEN hOldP = (HPEN)SelectObject(hdcMem, hSpkPen);
                            MoveToEx(hdcMem, px - (int)(particles[i].vx * 2), py - (int)(particles[i].vy * 2), NULL);
                            LineTo(hdcMem, px + (int)(particles[i].vx * 2), py + (int)(particles[i].vy * 2));
                            SelectObject(hdcMem, hOldP);
                            DeleteObject(hSpkPen);
                        } else if (particles[i].layer == 1) { // Plasma Puff
                            HBRUSH hPuffBrush = CreateSolidBrush(particles[i].color);
                            HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
                            HPEN hOldP = (HPEN)SelectObject(hdcMem, hNullPen);
                            HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hPuffBrush);
                            Ellipse(hdcMem, px - psz, py - psz, px + psz + 1, py + psz + 1);
                            SelectObject(hdcMem, hOldP);
                            SelectObject(hdcMem, hOldB);
                            DeleteObject(hPuffBrush);
                        } else if (particles[i].layer == 2) { // Crystal Shard
                            POINT pts[4];
                            double a = particles[i].angle;
                            pts[0].x = px + (int)(FastCos(a) * psz);
                            pts[0].y = py + (int)(FastSin(a) * psz);
                            pts[1].x = px + (int)(FastCos(a + 1.57) * psz * 0.6);
                            pts[1].y = py + (int)(FastSin(a + 1.57) * psz * 0.6);
                            pts[2].x = px + (int)(FastCos(a + 3.14) * psz);
                            pts[2].y = py + (int)(FastSin(a + 3.14) * psz);
                            pts[3].x = px + (int)(FastCos(a + 4.71) * psz * 0.6);
                            pts[3].y = py + (int)(FastSin(a + 4.71) * psz * 0.6);
                            
                            HBRUSH hShardBrush = CreateSolidBrush(particles[i].color);
                            HPEN hShardPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                            HPEN hOldP = (HPEN)SelectObject(hdcMem, hShardPen);
                            HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hShardBrush);
                            Polygon(hdcMem, pts, 4);
                            SelectObject(hdcMem, hOldP);
                            SelectObject(hdcMem, hOldB);
                            DeleteObject(hShardBrush);
                            DeleteObject(hShardPen);
                        } else { // Celebration Star
                            POINT starPts[8];
                            double a = particles[i].angle;
                            for (int s = 0; s < 8; s++) {
                                double curA = a + s * 0.785398;
                                double r = (s % 2 == 0) ? psz * 1.6 : psz * 0.5;
                                starPts[s].x = px + (int)(FastCos(curA) * r);
                                starPts[s].y = py + (int)(FastSin(curA) * r);
                            }
                            HBRUSH hStarBrush = CreateSolidBrush(particles[i].color);
                            HPEN hStarPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                            HPEN hOldP = (HPEN)SelectObject(hdcMem, hStarPen);
                            HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hStarBrush);
                            Polygon(hdcMem, starPts, 8);
                            SelectObject(hdcMem, hOldP);
                            SelectObject(hdcMem, hOldB);
                            DeleteObject(hStarBrush);
                            DeleteObject(hStarPen);
                        }
                    }
                }
                
                // 6. Draw Corner Filigree & Perimeter Glint
                DrawCornerFiligreeGDI(hdcMem, bmpW, bmpH);
                DrawPerimeterGlintGDI(hdcMem, bmpW, bmpH, glintProgress);
                
                // 7. Draw HUD Badge
                RECT textBg = { 12, bmpH - 46, 440, bmpH - 12 };
                HBRUSH hHudBrush = CreateSolidBrush(RGB(11, 19, 36));
                HPEN hHudPen = CreatePen(PS_SOLID, 2, RGB(96, 165, 250));
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hHudBrush);
                HPEN hOldPen = (HPEN)SelectObject(hdcMem, hHudPen);
                RoundRect(hdcMem, textBg.left, textBg.top, textBg.right, textBg.bottom, 12, 12);
                
                // Stylized composed Info Icon
                HPEN iconPen = CreatePen(PS_SOLID, 2, RGB(96, 165, 250));
                SelectObject(hdcMem, iconPen);
                Arc(hdcMem, 22, bmpH - 38, 38, bmpH - 22, 0, 0, 0, 0); // Circle
                MoveToEx(hdcMem, 30, bmpH - 34, NULL); // 'i' dot
                LineTo(hdcMem, 30, bmpH - 33);
                MoveToEx(hdcMem, 30, bmpH - 30, NULL); // 'i' body
                LineTo(hdcMem, 30, bmpH - 25);
                
                SelectObject(hdcMem, hOldBrush);
                SelectObject(hdcMem, hOldPen);
                DeleteObject(hHudBrush);
                DeleteObject(hHudPen);
                DeleteObject(iconPen);
                
                // Draw HUD Text
                SetBkMode(hdcMem, TRANSPARENT);
                int dpi = GetDeviceCaps(hdcMem, LOGPIXELSY);
                int fontHeight = -MulDiv(11, dpi, 72);
                HFONT hFont = CreateFont(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                         DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
                HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
                SetTextColor(hdcMem, RGB(248, 250, 252));
                
                char hudMsg[128];
                const char* curF = (fractalType >= 0 && fractalType < NUM_FRACTAL_TYPES) ? fractalNames[fractalType] : "Fractal";
                wsprintf(hudMsg, "%s%s | [F]ormula [L]andmark [T]heme [F1]Help", curF, isJulia ? " (Julia)" : "");
                
                int len = 0;
                while (hudMsg[len]) len++;
                TextOut(hdcMem, 46, bmpH - 36, hudMsg, len);
                
                SelectObject(hdcMem, hOldFont);
                DeleteObject(hFont);
                
                // 8. BitBlt memory buffer to screen
                BitBlt(hdc, 0, 0, bmpW, bmpH, hdcMem, 0, 0, SRCCOPY);
                
                SelectObject(hdcMem, hOldBm);
                DeleteObject(hbmMem);
                DeleteDC(hdcMem);
            }
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1; // Handled in double-buffered WM_PAINT
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hBitmap) DeleteObject(hBitmap);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(floor)
double __cdecl floor(double x) {
    return (double)((int)x);
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMandelApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    RECT rect = { 0, 0, W, H };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);

    HWND hwnd = CreateWindowEx(0, "KMandelApp", "KMandel - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    SaveState(); // Save initial state

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}

