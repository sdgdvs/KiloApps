#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <shellapi.h>

#define W 660
#define H 495

HWND hComboPreset, hComboWave, hBtnPlay, hBtnSeq, hBtnExportWav, hBtnHelp, hBtnSave, hBtnLoad, hBtnExportFm, hBtnImportFm, hBtnPlayRadio, hBtnShareTrap, hFreq, hAttack, hDecay, hSustain, hRelease;
HWND hDelayTime, hDelayFdbk, hDelayMix;
HWND hComboArp, hArpBpm, hArpOct;
HWND hScopeWnd, hPianoWnd, hStatusText;

HWAVEOUT hWaveOut = NULL;
WAVEHDR waveHdr;
static int g_octaveShift = 0;
static int g_activePianoKey = -1;
static WNDPROC g_oldEditProc = NULL;

void UpdateStatusText(const char* text) {
    if (hStatusText) {
        SetWindowTextA(hStatusText, text);
    }
}

int GetDPI() {
    HDC hdc = GetDC(NULL);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return dpi;
}
int Scale(int v) {
    return (v * GetDPI()) / 96;
}
HWND CreateScaledWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, Scale(X), Scale(Y), Scale(nWidth), Scale(nHeight), hWndParent, hMenu, hInstance, lpParam);
}

#define SAMPLE_RATE 44100
#define MAX_DURATION 5
short buffer[SAMPLE_RATE * MAX_DURATION];
int bufferSampleCount = 0;

int _fltused = 0;

int parse_int(const char* s) {
    int res = 0;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

double parse_float(const char* s) {
    double res = 0.0;
    int dec = 0;
    double frac = 1.0;
    while (*s) {
        if (*s == '.') { dec = 1; }
        else if (*s >= '0' && *s <= '9') {
            if (dec) {
                frac *= 0.1;
                res += (*s - '0') * frac;
            } else {
                res = res * 10.0 + (*s - '0');
            }
        }
        s++;
    }
    return res;
}

static unsigned int g_seed = 12345;
static unsigned int fast_rand() {
    g_seed = g_seed * 1103515245 + 12345;
    return (unsigned int)(g_seed / 65536) % 32768;
}

// Preset definitions
typedef struct {
    int wave;
    int freq;
    const char* attack;
    const char* decay;
    const char* sustain;
    const char* release;
} Preset;

static const char* g_morseEchoSubsystem =
    "1000"                      // e
    "11101011101000"            // c
    "1010101000"                // h
    "11101110111000"            // o
    "111010101010111000"        // -
    "10101000"                  // s
    "1010111000"                // u
    "111010101000"              // b
    "10101000"                  // s
    "1110101110111000"          // y
    "10101000"                  // s
    "111000"                    // t
    "1000"                      // e
    "1110111000"                // m
    "10111010111010111000"      // .
    "11101000"                  // n
    "1000"                      // e
    "111";                      // t

Preset g_presets[] = {
    { 0, 440, "0.05", "0.20", "0.60", "0.40" }, // 1: Neon Lead (Sine)
    { 1, 110, "0.01", "0.25", "0.40", "0.15" }, // 2: Sub Bass (Square)
    { 2, 330, "0.40", "0.60", "0.80", "1.00" }, // 3: Warm Pad (Sawtooth)
    { 1, 523, "0.001","0.10", "0.30", "0.05" }, // 4: 8-Bit Chiptune
    { 3, 880, "0.005","0.60", "0.10", "0.80" }, // 5: Glass Bell (Triangle)
    { 4, 440, "0.01", "0.15", "0.10", "0.10" }, // 6: Noise Generator
    { 1, 110, "0.01", "0.18", "0.30", "0.12" }, // 7: YM2612 FM Bass
    { 2, 261, "0.45", "0.50", "0.85", "1.20" }, // 8: SPC700 Echo Pad
    { 0, 1999, "0.02", "0.10", "0.90", "0.30" } // 9: 1999 Ghost Beacon
};

void GenerateWave(int type, int freq, double attack, double decay, double sustain, double release, double delayTime, double delayFdbk, double delayMix) {
    double phase = 0.0;
    double phaseInc = (double)freq / SAMPLE_RATE;
    
    double sustain_time = 0.5;
    double total_time = attack + decay + sustain_time + release;
    if (total_time > 5.0) total_time = 5.0;
    
    int total_samples = (int)(total_time * SAMPLE_RATE);
    if (total_samples < 1) total_samples = 1;
    bufferSampleCount = total_samples;
    
    for (int i = 0; i < total_samples; i++) {
        double val = 0.0;
        
        if (type == 0) { // Sine approximation
            double t = phase;
            if (t < 0.5) {
                double x = t * 2.0;
                val = 4.0 * x * (1.0 - x);
            } else {
                double x = (t - 0.5) * 2.0;
                val = -4.0 * x * (1.0 - x);
            }
        } else if (type == 1) { // Square
            val = (phase < 0.5) ? 0.6 : -0.6;
        } else if (type == 2) { // Sawtooth
            val = (phase * 2.0) - 1.0;
        } else if (type == 3) { // Triangle
            double t = phase;
            if (t < 0.25) val = t * 4.0;
            else if (t < 0.75) val = 1.0 - (t - 0.25) * 4.0;
            else val = -1.0 + (t - 0.75) * 4.0;
        } else if (type == 4) { // Noise
            val = ((double)fast_rand() / 32767.0) * 2.0 - 1.0;
        }
        
        double t = (double)i / SAMPLE_RATE;
        double env = 0.0;
        if (t < attack) {
            if (attack > 0) env = t / attack;
        } else if (t < attack + decay) {
            if (decay > 0) env = 1.0 - (1.0 - sustain) * ((t - attack) / decay);
            else env = sustain;
        } else if (t < attack + decay + sustain_time) {
            env = sustain;
        } else if (t < attack + decay + sustain_time + release) {
            if (release > 0) env = sustain * (1.0 - ((t - attack - decay - sustain_time) / release));
            else env = 0.0;
        }
        
        buffer[i] = (short)(val * env * 16000.0);
        
        if (freq == 1999) {
            int morseIdx = (int)(t / 0.035);
            if (morseIdx >= 0 && g_morseEchoSubsystem[morseIdx] != '\0') {
                if (g_morseEchoSubsystem[morseIdx] == '1') {
                    double mPhase = phase;
                    double mSin = (mPhase < 0.5) ? (4.0 * mPhase * (1.0 - mPhase)) : (-4.0 * (mPhase - 0.5) * (1.0 - (mPhase - 0.5)));
                    int combined = buffer[i] + (int)(mSin * 6500.0);
                    if (combined > 32767) combined = 32767;
                    if (combined < -32768) combined = -32768;
                    buffer[i] = (short)combined;
                }
            }
        }
        
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
    
    for (int i = total_samples; i < SAMPLE_RATE * MAX_DURATION; i++) {
        buffer[i] = 0;
    }

    static short delayBuffer[SAMPLE_RATE * MAX_DURATION];
    memset(delayBuffer, 0, sizeof(delayBuffer));
    
    if (delayMix > 0.0 && delayTime > 0.0) {
        int delay_samples = (int)(delayTime * SAMPLE_RATE);
        if (delay_samples > 0) {
            for (int i = 0; i < SAMPLE_RATE * MAX_DURATION; i++) {
                short in_val = buffer[i];
                short delayed_val = (i >= delay_samples) ? delayBuffer[i - delay_samples] : 0;
                
                int new_delay_val = in_val + (int)(delayed_val * delayFdbk);
                if (new_delay_val > 32767) new_delay_val = 32767;
                if (new_delay_val < -32768) new_delay_val = -32768;
                delayBuffer[i] = (short)new_delay_val;
                
                int out_val = (int)(in_val * (1.0 - delayMix)) + (int)(delayed_val * delayMix);
                if (out_val > 32767) out_val = 32767;
                if (out_val < -32768) out_val = -32768;
                buffer[i] = (short)out_val;
            }
            total_samples = SAMPLE_RATE * MAX_DURATION;
        }
    }

    waveHdr.dwBufferLength = total_samples * sizeof(short);

    if (hScopeWnd) InvalidateRect(hScopeWnd, NULL, TRUE);
}

void PlayTone() {
    int sel = SendMessage(hComboWave, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) sel = 0;
    
    char buf[32];
    GetWindowTextA(hFreq, buf, 32);
    int freq = 0;
    char* p = buf;
    while (*p) {
        if (*p >= '0' && *p <= '9') freq = freq * 10 + (*p - '0');
        p++;
    }
    if (freq < 20) freq = 440;
    if (freq > 20000) freq = 20000;
    
    GetWindowTextA(hAttack, buf, 32); double attack = parse_float(buf);
    GetWindowTextA(hDecay, buf, 32); double decay = parse_float(buf);
    GetWindowTextA(hSustain, buf, 32); double sustain = parse_float(buf);
    GetWindowTextA(hRelease, buf, 32); double release = parse_float(buf);
    
    if (attack < 0) attack = 0;
    if (decay < 0) decay = 0;
    if (sustain < 0) sustain = 0; if (sustain > 1) sustain = 1;
    if (release < 0) release = 0;
    
    GetWindowTextA(hDelayTime, buf, 32); double delayTime = parse_float(buf);
    GetWindowTextA(hDelayFdbk, buf, 32); double delayFdbk = parse_float(buf);
    GetWindowTextA(hDelayMix, buf, 32); double delayMix = parse_float(buf);
    
    if (delayTime < 0) delayTime = 0; if (delayTime > 2) delayTime = 2;
    if (delayFdbk < 0) delayFdbk = 0; if (delayFdbk > 0.99) delayFdbk = 0.99;
    if (delayMix < 0) delayMix = 0; if (delayMix > 1) delayMix = 1;

    GenerateWave(sel, freq, attack, decay, sustain, release, delayTime, delayFdbk, delayMix);
    
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    
    if (hWaveOut) {
        waveOutReset(hWaveOut);
        waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        hWaveOut = NULL;
    }
    
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        memset(&waveHdr, 0, sizeof(WAVEHDR));
        waveHdr.lpData = (LPSTR)buffer;
        waveHdr.dwBufferLength = (DWORD)(bufferSampleCount * sizeof(short));
        waveHdr.dwBytesRecorded = 0;
        waveHdr.dwUser = 0;
        waveHdr.dwFlags = 0;
        waveHdr.dwLoops = 0;
        
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    }
}

// Play Arpeggiated Pattern
void PlayArpeggiator() {
    int mode = SendMessage(hComboArp, CB_GETCURSEL, 0, 0);
    if (mode == CB_ERR || mode == 0) {
        PlayTone();
        return;
    }

    char buf[16];
    GetWindowTextA(hFreq, buf, 16);
    int baseFreq = parse_int(buf);
    if (baseFreq < 50) baseFreq = 440;

    int freqs[4];
    if (mode == 1) { // Up
        freqs[0] = baseFreq; freqs[1] = (int)(baseFreq * 1.25); freqs[2] = (int)(baseFreq * 1.5); freqs[3] = baseFreq * 2;
    } else if (mode == 2) { // Down
        freqs[0] = baseFreq * 2; freqs[1] = (int)(baseFreq * 1.5); freqs[2] = (int)(baseFreq * 1.25); freqs[3] = baseFreq;
    } else if (mode == 3) { // Up-Down
        freqs[0] = baseFreq; freqs[1] = (int)(baseFreq * 1.5); freqs[2] = baseFreq * 2; freqs[3] = (int)(baseFreq * 1.5);
    } else { // Random
        freqs[0] = baseFreq; freqs[1] = (int)(baseFreq * 1.18); freqs[2] = (int)(baseFreq * 1.33); freqs[3] = (int)(baseFreq * 1.68);
    }

    for (int i = 0; i < 4; i++) {
        wsprintfA(buf, "%d", freqs[i]);
        SetWindowTextA(hFreq, buf);
        PlayTone();
        Sleep(120);
    }
}

void ApplyPreset(int idx) {
    if (idx < 0 || idx >= 9) return;
    Preset p = g_presets[idx];
    SendMessage(hComboWave, CB_SETCURSEL, p.wave, 0);
    
    char buf[32];
    wsprintfA(buf, "%d", p.freq);
    SetWindowTextA(hFreq, buf);

    SetWindowTextA(hAttack, p.attack);
    SetWindowTextA(hDecay, p.decay);
    SetWindowTextA(hSustain, p.sustain);
    SetWindowTextA(hRelease, p.release);

    char stat[80];
    static const char* names[] = { "Neon Lead", "Sub Bass", "Warm Pad", "8-Bit Chiptune", "Glass Bell", "Noise Gen", "YM2612 FM Bass", "SPC700 Echo Pad", "1999 Ghost Beacon" };
    if (idx == 8) {
        wsprintfA(stat, "Preset [9]: %s (1999Hz Demodulator Active)", names[idx]);
    } else {
        wsprintfA(stat, "Preset [%d]: %s", idx + 1, names[idx]);
    }
    UpdateStatusText(stat);
}

// Custom Oscilloscope Window Procedure
LRESULT CALLBACK ScopeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            HBRUSH hBg = CreateSolidBrush(RGB(15, 18, 26));
            FillRect(memDC, &rect, hBg);
            DeleteObject(hBg);

            HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(40, 48, 64));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hGridPen);
            int stepX = Scale(30); if (stepX < 1) stepX = 1;
            int stepY = Scale(20); if (stepY < 1) stepY = 1;
            for (int x = 0; x < w; x += stepX) {
                MoveToEx(memDC, x, 0, NULL); LineTo(memDC, x, h);
            }
            for (int y = 0; y < h; y += stepY) {
                MoveToEx(memDC, 0, y, NULL); LineTo(memDC, w, y);
            }
            DeleteObject(SelectObject(memDC, hOldPen));

            HPEN hScopePen = CreatePen(PS_SOLID, Scale(2), RGB(0, 243, 255));
            hOldPen = (HPEN)SelectObject(memDC, hScopePen);

            int midY = h / 2;
            int step = bufferSampleCount / w;
            if (step < 1) step = 1;

            MoveToEx(memDC, 0, midY, NULL);
            for (int x = 0; x < w; x++) {
                int sampleIdx = x * step;
                if (sampleIdx >= bufferSampleCount) break;
                short sample = buffer[sampleIdx];
                int y = midY - (sample * (h / 2) / 32768);
                LineTo(memDC, x, y);
            }

            DeleteObject(SelectObject(memDC, hOldPen));

            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Interactive Virtual Piano Window Procedure
LRESULT CALLBACK PianoProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // 8 white keys: C(0), D(2), E(4), F(5), G(7), A(9), B(11), C5(12)
            static const int whiteIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
            static const char* whiteLabels[8] = { "C [A]", "D [S]", "E [D]", "F [F]", "G [G]", "A [H]", "B [J]", "C [K]" };

            int whiteKeyW = w / 8;
            for (int i = 0; i < 8; i++) {
                int kIdx = whiteIndices[i];
                RECT kr = { i * whiteKeyW, 0, (i == 7) ? w : (i + 1) * whiteKeyW, h };

                HBRUSH hBrush;
                if (g_activePianoKey == kIdx) {
                    hBrush = CreateSolidBrush(RGB(0, 243, 255)); // Active Cyan
                } else {
                    hBrush = CreateSolidBrush(RGB(242, 245, 250)); // Crisp White Key
                }
                FillRect(memDC, &kr, hBrush);
                DeleteObject(hBrush);

                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(175, 182, 195));
                HPEN oldPen = (HPEN)SelectObject(memDC, hPen);
                MoveToEx(memDC, kr.right - 1, 0, NULL);
                LineTo(memDC, kr.right - 1, h);
                DeleteObject(SelectObject(memDC, oldPen));

                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, (g_activePianoKey == kIdx) ? RGB(0, 0, 0) : RGB(70, 80, 95));
                RECT textR = { kr.left, h - Scale(18), kr.right, h };
                DrawTextA(memDC, whiteLabels[i], -1, &textR, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            }

            // 5 black keys: C#(1), D#(3), F#(6), G#(8), A#(10)
            static const int blackIndices[5] = { 1, 3, 6, 8, 10 };
            static const int blackSlots[5] = { 1, 2, 4, 5, 6 };
            static const char* blackLabels[5] = { "W", "E", "T", "Y", "U" };

            int blackKeyW = (whiteKeyW * 6) / 10;
            int blackKeyH = (h * 60) / 100;

            for (int i = 0; i < 5; i++) {
                int kIdx = blackIndices[i];
                int center = blackSlots[i] * whiteKeyW;
                RECT kr = { center - blackKeyW / 2, 0, center + blackKeyW / 2, blackKeyH };

                HBRUSH hBrush;
                if (g_activePianoKey == kIdx) {
                    hBrush = CreateSolidBrush(RGB(255, 0, 127)); // Active Hot Pink
                } else {
                    hBrush = CreateSolidBrush(RGB(20, 24, 32)); // Dark Key
                }
                FillRect(memDC, &kr, hBrush);
                DeleteObject(hBrush);

                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(10, 12, 16));
                HPEN oldPen = (HPEN)SelectObject(memDC, hPen);
                Rectangle(memDC, kr.left, kr.top, kr.right, kr.bottom);
                DeleteObject(SelectObject(memDC, oldPen));

                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, (g_activePianoKey == kIdx) ? RGB(0, 0, 0) : RGB(220, 225, 235));
                RECT textR = { kr.left, blackKeyH - Scale(16), kr.right, blackKeyH };
                DrawTextA(memDC, blackLabels[i], -1, &textR, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            }

            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            int whiteKeyW = w / 8;
            int blackKeyW = (whiteKeyW * 6) / 10;
            int blackKeyH = (h * 60) / 100;

            int hit = -1;
            if (y <= blackKeyH) {
                static const int blackIndices[5] = { 1, 3, 6, 8, 10 };
                static const int blackSlots[5] = { 1, 2, 4, 5, 6 };
                for (int i = 0; i < 5; i++) {
                    int center = blackSlots[i] * whiteKeyW;
                    if (x >= center - blackKeyW / 2 && x <= center + blackKeyW / 2) {
                        hit = blackIndices[i];
                        break;
                    }
                }
            }
            if (hit == -1) {
                int slot = x / whiteKeyW;
                if (slot < 0) slot = 0;
                if (slot > 7) slot = 7;
                static const int whiteIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
                hit = whiteIndices[slot];
            }

            if (hit >= 0) {
                g_activePianoKey = hit;
                InvalidateRect(hwnd, NULL, FALSE);
                static const double freqs[13] = {
                    261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
                    369.99, 392.00, 415.30, 440.00, 466.16, 493.88, 523.25
                };
                double mult = 1.0;
                if (g_octaveShift == -2) mult = 0.25;
                else if (g_octaveShift == -1) mult = 0.5;
                else if (g_octaveShift == 1) mult = 2.0;
                else if (g_octaveShift == 2) mult = 4.0;
                int finalFreq = (int)(freqs[hit] * mult + 0.5);
                char buf[32];
                wsprintfA(buf, "%d", finalFreq);
                SetWindowTextA(hFreq, buf);
                PlayTone();
                wsprintfA(buf, "Playing: %d Hz (Key %d)", finalFreq, hit);
                UpdateStatusText(buf);
                SetCapture(hwnd);
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            if (GetCapture() == hwnd) ReleaseCapture();
            g_activePianoKey = -1;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Subclass edit controls to handle Enter, Esc, F1, F5, and F9 smoothly
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            PlayTone();
            UpdateStatusText("Updated frequency/parameters [Enter]");
            return 0;
        }
        if (wParam == VK_ESCAPE) {
            SetFocus(GetParent(hwnd));
            UpdateStatusText("Keyboard controls active");
            return 0;
        }
        if (wParam == VK_F1) {
            PostMessage(GetParent(hwnd), WM_COMMAND, 3, 0);
            return 0;
        }
        if (wParam == VK_F5) {
            PostMessage(GetParent(hwnd), WM_COMMAND, 5, 0);
            return 0;
        }
        if (wParam == VK_F9) {
            PostMessage(GetParent(hwnd), WM_COMMAND, 6, 0);
            return 0;
        }
    }
    return CallWindowProc(g_oldEditProc, hwnd, msg, wParam, lParam);
}

void ExportWav(HWND hwnd) {
    if (bufferSampleCount <= 0) {
        MessageBoxA(hwnd, "No audio data synthesized yet. Play a tone first!", "Export WAV", MB_OK | MB_ICONWARNING);
        return;
    }

    HANDLE hFile = CreateFileA("ksynth_output.wav", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Could not open ksynth_output.wav for writing.", "Export Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD dataSize = (DWORD)(bufferSampleCount * sizeof(short));
    DWORD fileSize = 36 + dataSize;
    DWORD written = 0;

    unsigned char hdr[44];
    hdr[0] = 'R'; hdr[1] = 'I'; hdr[2] = 'F'; hdr[3] = 'F';
    hdr[4] = (unsigned char)(fileSize & 0xFF);
    hdr[5] = (unsigned char)((fileSize >> 8) & 0xFF);
    hdr[6] = (unsigned char)((fileSize >> 16) & 0xFF);
    hdr[7] = (unsigned char)((fileSize >> 24) & 0xFF);
    hdr[8] = 'W'; hdr[9] = 'A'; hdr[10] = 'V'; hdr[11] = 'E';
    hdr[12] = 'f'; hdr[13] = 'm'; hdr[14] = 't'; hdr[15] = ' ';
    hdr[16] = 16; hdr[17] = 0; hdr[18] = 0; hdr[19] = 0;
    hdr[20] = 1; hdr[21] = 0; // PCM
    hdr[22] = 1; hdr[23] = 0; // Mono
    DWORD sr = SAMPLE_RATE;
    hdr[24] = (unsigned char)(sr & 0xFF);
    hdr[25] = (unsigned char)((sr >> 8) & 0xFF);
    hdr[26] = (unsigned char)((sr >> 16) & 0xFF);
    hdr[27] = (unsigned char)((sr >> 24) & 0xFF);
    DWORD br = SAMPLE_RATE * 2;
    hdr[28] = (unsigned char)(br & 0xFF);
    hdr[29] = (unsigned char)((br >> 8) & 0xFF);
    hdr[30] = (unsigned char)((br >> 16) & 0xFF);
    hdr[31] = (unsigned char)((br >> 24) & 0xFF);
    hdr[32] = 2; hdr[33] = 0; // Align
    hdr[34] = 16; hdr[35] = 0; // 16-bit
    hdr[36] = 'd'; hdr[37] = 'a'; hdr[38] = 't'; hdr[39] = 'a';
    hdr[40] = (unsigned char)(dataSize & 0xFF);
    hdr[41] = (unsigned char)((dataSize >> 8) & 0xFF);
    hdr[42] = (unsigned char)((dataSize >> 16) & 0xFF);
    hdr[43] = (unsigned char)((dataSize >> 24) & 0xFF);

    WriteFile(hFile, hdr, 44, &written, NULL);
    WriteFile(hFile, buffer, dataSize, &written, NULL);
    CloseHandle(hFile);

    char msg[128];
    wsprintfA(msg, "Exported %d samples to ksynth_output.wav!", bufferSampleCount);
    UpdateStatusText("Exported ksynth_output.wav successfully");
    MessageBoxA(hwnd, msg, "KSynth WAV Export", MB_OK | MB_ICONINFORMATION);
}

void UpdateWindowTitle(HWND hwnd);

static const char* my_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return haystack;
    }
    return NULL;
}

static void extract_json_str(const char* json, const char* key, char* out, int maxLen) {
    out[0] = '\0';
    char search[64];
    wsprintfA(search, "\"%s\":", key);
    const char* p = my_strstr(json, search);
    if (!p) return;
    p += lstrlenA(search);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '\"') {
        p++;
        int i = 0;
        while (*p && *p != '\"' && i < maxLen - 1) {
            out[i++] = *p++;
        }
        out[i] = '\0';
    } else {
        int i = 0;
        while (*p && *p != ',' && *p != '}' && *p != '\r' && *p != '\n' && *p != ' ' && i < maxLen - 1) {
            out[i++] = *p++;
        }
        out[i] = '\0';
    }
}

void ExportFmPreset(HWND hwnd, const char* filename) {
    CreateDirectoryA("music", NULL);
    const char* target = (filename && filename[0]) ? filename : "music\\ksynth_patch.fm";
    HANDLE hFile = CreateFileA(target, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        target = "ksynth_patch.fm";
        hFile = CreateFileA(target, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Could not open file for writing FM preset.", "Export Error", MB_OK | MB_ICONERROR);
        return;
    }

    int wave = (int)SendMessage(hComboWave, CB_GETCURSEL, 0, 0);
    if (wave == CB_ERR) wave = 0;

    char fBuf[32], att[32], dec[32], sus[32], rel[32], dTime[32], dFdbk[32], dMix[32];
    GetWindowTextA(hFreq, fBuf, sizeof(fBuf));
    GetWindowTextA(hAttack, att, sizeof(att));
    GetWindowTextA(hDecay, dec, sizeof(dec));
    GetWindowTextA(hSustain, sus, sizeof(sus));
    GetWindowTextA(hRelease, rel, sizeof(rel));
    GetWindowTextA(hDelayTime, dTime, sizeof(dTime));
    GetWindowTextA(hDelayFdbk, dFdbk, sizeof(dFdbk));
    GetWindowTextA(hDelayMix, dMix, sizeof(dMix));

    static const char* waveNames[] = { "sine", "square", "sawtooth", "triangle", "noise" };
    const char* waveStr = (wave >= 0 && wave < 5) ? waveNames[wave] : "sine";

    char json[1024];
    wsprintfA(json,
        "{\r\n"
        "  \"format\": \"KILO_2OP_FM\",\r\n"
        "  \"version\": \"1.0\",\r\n"
        "  \"name\": \"Win32 FM Preset\",\r\n"
        "  \"engineMode\": \"fm\",\r\n"
        "  \"osc1Wave\": \"%s\",\r\n"
        "  \"freq\": %s,\r\n"
        "  \"fmRatio\": 2.0,\r\n"
        "  \"fmModDepth\": 0.40,\r\n"
        "  \"fmFeedback\": 0.20,\r\n"
        "  \"attack\": %s,\r\n"
        "  \"decay\": %s,\r\n"
        "  \"sustain\": %s,\r\n"
        "  \"release\": %s,\r\n"
        "  \"delayTime\": %s,\r\n"
        "  \"delayFdbk\": %s,\r\n"
        "  \"delayMix\": %s\r\n"
        "}\r\n",
        waveStr, fBuf, att, dec, sus, rel, dTime, dFdbk, dMix
    );

    DWORD len = (DWORD)lstrlenA(json);
    DWORD written = 0;
    WriteFile(hFile, json, len, &written, NULL);
    CloseHandle(hFile);

    char msg[160];
    wsprintfA(msg, "Exported 2-Op FM patch preset to %s!", target);
    UpdateStatusText(msg);
    MessageBoxA(hwnd, msg, "KSynth FM Patch Export", MB_OK | MB_ICONINFORMATION);
}

BOOL ImportFmPreset(HWND hwnd, const char* filename) {
    const char* target = (filename && filename[0]) ? filename : "music\\ksynth_patch.fm";
    HANDLE hFile = CreateFileA(target, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        target = "ksynth_patch.fm";
        hFile = CreateFileA(target, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile == INVALID_HANDLE_VALUE) {
        UpdateStatusText("⚠ Could not open FM patch file (music\\ksynth_patch.fm)");
        return FALSE;
    }

    char json[2048];
    DWORD readBytes = 0;
    BOOL ok = ReadFile(hFile, json, sizeof(json) - 1, &readBytes, NULL);
    CloseHandle(hFile);
    if (!ok || readBytes == 0) return FALSE;
    json[readBytes] = '\0';

    if (!my_strstr(json, "KILO_2OP_FM") && !my_strstr(json, "\"format\"")) {
        UpdateStatusText("⚠ Invalid .FM patch format");
        return FALSE;
    }

    char val[64];
    extract_json_str(json, "freq", val, sizeof(val));
    if (val[0]) SetWindowTextA(hFreq, val);

    extract_json_str(json, "attack", val, sizeof(val));
    if (val[0]) SetWindowTextA(hAttack, val);

    extract_json_str(json, "decay", val, sizeof(val));
    if (val[0]) SetWindowTextA(hDecay, val);

    extract_json_str(json, "sustain", val, sizeof(val));
    if (val[0]) SetWindowTextA(hSustain, val);

    extract_json_str(json, "release", val, sizeof(val));
    if (val[0]) SetWindowTextA(hRelease, val);

    extract_json_str(json, "delayTime", val, sizeof(val));
    if (val[0]) SetWindowTextA(hDelayTime, val);

    extract_json_str(json, "delayFdbk", val, sizeof(val));
    if (val[0]) SetWindowTextA(hDelayFdbk, val);

    extract_json_str(json, "delayMix", val, sizeof(val));
    if (val[0]) SetWindowTextA(hDelayMix, val);

    extract_json_str(json, "osc1Wave", val, sizeof(val));
    if (val[0]) {
        if (my_strstr(val, "square")) SendMessage(hComboWave, CB_SETCURSEL, 1, 0);
        else if (my_strstr(val, "saw")) SendMessage(hComboWave, CB_SETCURSEL, 2, 0);
        else if (my_strstr(val, "tri")) SendMessage(hComboWave, CB_SETCURSEL, 3, 0);
        else if (my_strstr(val, "noise")) SendMessage(hComboWave, CB_SETCURSEL, 4, 0);
        else SendMessage(hComboWave, CB_SETCURSEL, 0, 0);
    }

    SendMessage(hComboPreset, CB_SETCURSEL, -1, 0);

    char msg[160];
    wsprintfA(msg, "★ Loaded FM patch preset from %s", target);
    UpdateStatusText(msg);
    PlayTone();
    return TRUE;
}

void PlayTrackInKRadio(HWND hwnd) {
    if (bufferSampleCount <= 0) {
        PlayTone();
    }
    CreateDirectoryA("music", NULL);
    const char* outPath = "music\\ksynth_track.wav";
    HANDLE hFile = CreateFileA(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        outPath = "ksynth_track.wav";
        hFile = CreateFileA(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dataSize = (DWORD)(bufferSampleCount * sizeof(short));
        DWORD fileSize = 36 + dataSize;
        DWORD written = 0;
        unsigned char hdr[44];
        hdr[0] = 'R'; hdr[1] = 'I'; hdr[2] = 'F'; hdr[3] = 'F';
        hdr[4] = (unsigned char)(fileSize & 0xFF);
        hdr[5] = (unsigned char)((fileSize >> 8) & 0xFF);
        hdr[6] = (unsigned char)((fileSize >> 16) & 0xFF);
        hdr[7] = (unsigned char)((fileSize >> 24) & 0xFF);
        hdr[8] = 'W'; hdr[9] = 'A'; hdr[10] = 'V'; hdr[11] = 'E';
        hdr[12] = 'f'; hdr[13] = 'm'; hdr[14] = 't'; hdr[15] = ' ';
        hdr[16] = 16; hdr[17] = 0; hdr[18] = 0; hdr[19] = 0;
        hdr[20] = 1; hdr[21] = 0; // PCM
        hdr[22] = 1; hdr[23] = 0; // Mono
        DWORD sr = SAMPLE_RATE;
        hdr[24] = (unsigned char)(sr & 0xFF);
        hdr[25] = (unsigned char)((sr >> 8) & 0xFF);
        hdr[26] = (unsigned char)((sr >> 16) & 0xFF);
        hdr[27] = (unsigned char)((sr >> 24) & 0xFF);
        DWORD br = SAMPLE_RATE * 2;
        hdr[28] = (unsigned char)(br & 0xFF);
        hdr[29] = (unsigned char)((br >> 8) & 0xFF);
        hdr[30] = (unsigned char)((br >> 16) & 0xFF);
        hdr[31] = (unsigned char)((br >> 24) & 0xFF);
        hdr[32] = 2; hdr[33] = 0; // Align
        hdr[34] = 16; hdr[35] = 0; // 16-bit
        hdr[36] = 'd'; hdr[37] = 'a'; hdr[38] = 't'; hdr[39] = 'a';
        hdr[40] = (unsigned char)(dataSize & 0xFF);
        hdr[41] = (unsigned char)((dataSize >> 8) & 0xFF);
        hdr[42] = (unsigned char)((dataSize >> 16) & 0xFF);
        hdr[43] = (unsigned char)((dataSize >> 24) & 0xFF);
        WriteFile(hFile, hdr, 44, &written, NULL);
        WriteFile(hFile, buffer, dataSize, &written, NULL);
        CloseHandle(hFile);
    }
    HINSTANCE hInst = ShellExecuteA(hwnd, "open", "KRadio.exe", "music\\ksynth_track.wav", NULL, SW_SHOWNORMAL);
    if ((INT_PTR)hInst <= 32) {
        ShellExecuteA(hwnd, "open", "..\\KRadio\\KRadio.exe", "music\\ksynth_track.wav", NULL, SW_SHOWNORMAL);
    }
    UpdateStatusText("📻 Rendered track to music\\ksynth_track.wav & tuned KRadio!");
    MessageBoxA(hwnd, "Track rendered to music\\ksynth_track.wav\nReady for playback in KRadio workstation!", "KRadio Bridge", MB_OK | MB_ICONINFORMATION);
}

void ShareInTrapster(HWND hwnd) {
    ExportFmPreset(hwnd, "music\\ksynth_patch.fm");
    CreateDirectoryA("music", NULL);
    HANDLE hFile = CreateFileA("music\\trapster_shared.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        const char* entry = "FILE=ksynth_patch.fm|TYPE=2OP_FM|CH=2|BITRATE=44.1k|SHARED=TRUE|SEED=1999\r\n";
        DWORD written = 0;
        WriteFile(hFile, entry, (DWORD)lstrlenA(entry), &written, NULL);
        CloseHandle(hFile);
    }
    HINSTANCE hInst = ShellExecuteA(hwnd, "open", "knet.exe", "kweb://trapster", NULL, SW_SHOWNORMAL);
    if ((INT_PTR)hInst <= 32) {
        ShellExecuteA(hwnd, "open", "..\\knet\\knet.exe", "kweb://trapster", NULL, SW_SHOWNORMAL);
    }
    UpdateStatusText("🐱 Published 2-Op FM patch to Trapster P2P swarm catalog!");
    MessageBoxA(hwnd, "2-Op FM preset exported to music\\ksynth_patch.fm\nCataloged for Trapster P2P swarm exchange!", "Trapster P2P Swarm", MB_OK | MB_ICONINFORMATION);
}

#define KSYNTH_SAVE_MAGIC 0x4B53594E // "KSYN"
#define KSYNTH_SAVE_VER 1

typedef struct {
    DWORD magic;
    DWORD version;
    int preset;
    int wave;
    int freq;
    char attack[16];
    char decay[16];
    char sustain[16];
    char release[16];
    char delayTime[16];
    char delayFdbk[16];
    char delayMix[16];
    int arpMode;
    int octaveShift;
    DWORD checksum;
} KSynthSaveData;

static DWORD CalculateSaveChecksum(const KSynthSaveData* data) {
    DWORD sum = 0x12345678;
    const unsigned char* p = (const unsigned char*)data;
    size_t len = (size_t)((const char*)&data->checksum - (const char*)data);
    for (size_t i = 0; i < len; i++) {
        sum = (sum * 31) + p[i];
    }
    return sum;
}

BOOL SaveStateToFile(const char* filename) {
    KSynthSaveData data;
    memset(&data, 0, sizeof(data));
    data.magic = KSYNTH_SAVE_MAGIC;
    data.version = KSYNTH_SAVE_VER;

    data.preset = (int)SendMessage(hComboPreset, CB_GETCURSEL, 0, 0);
    data.wave = (int)SendMessage(hComboWave, CB_GETCURSEL, 0, 0);
    if (data.wave == CB_ERR) data.wave = 0;

    char buf[32];
    GetWindowTextA(hFreq, buf, sizeof(buf));
    data.freq = parse_int(buf);
    if (data.freq < 20) data.freq = 440;

    GetWindowTextA(hAttack, data.attack, sizeof(data.attack));
    GetWindowTextA(hDecay, data.decay, sizeof(data.decay));
    GetWindowTextA(hSustain, data.sustain, sizeof(data.sustain));
    GetWindowTextA(hRelease, data.release, sizeof(data.release));

    GetWindowTextA(hDelayTime, data.delayTime, sizeof(data.delayTime));
    GetWindowTextA(hDelayFdbk, data.delayFdbk, sizeof(data.delayFdbk));
    GetWindowTextA(hDelayMix, data.delayMix, sizeof(data.delayMix));

    data.arpMode = (int)SendMessage(hComboArp, CB_GETCURSEL, 0, 0);
    if (data.arpMode == CB_ERR) data.arpMode = 0;

    data.octaveShift = g_octaveShift;
    data.checksum = CalculateSaveChecksum(&data);

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD written = 0;
    BOOL ok = WriteFile(hFile, &data, sizeof(data), &written, NULL);
    CloseHandle(hFile);
    return (ok && written == sizeof(data));
}

BOOL LoadStateFromFile(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    KSynthSaveData data;
    DWORD read = 0;
    BOOL ok = ReadFile(hFile, &data, sizeof(data), &read, NULL);
    CloseHandle(hFile);

    if (!ok || read != sizeof(data)) return FALSE;
    if (data.magic != KSYNTH_SAVE_MAGIC || data.version != KSYNTH_SAVE_VER) return FALSE;
    if (data.checksum != CalculateSaveChecksum(&data)) return FALSE;

    if (data.preset >= 0 && data.preset < 9) {
        SendMessage(hComboPreset, CB_SETCURSEL, data.preset, 0);
    } else {
        SendMessage(hComboPreset, CB_SETCURSEL, -1, 0);
    }

    if (data.wave >= 0 && data.wave < 5) {
        SendMessage(hComboWave, CB_SETCURSEL, data.wave, 0);
    }

    char buf[32];
    if (data.freq >= 20 && data.freq <= 20000) {
        wsprintfA(buf, "%d", data.freq);
        SetWindowTextA(hFreq, buf);
    }

    if (data.attack[0]) SetWindowTextA(hAttack, data.attack);
    if (data.decay[0]) SetWindowTextA(hDecay, data.decay);
    if (data.sustain[0]) SetWindowTextA(hSustain, data.sustain);
    if (data.release[0]) SetWindowTextA(hRelease, data.release);

    if (data.delayTime[0]) SetWindowTextA(hDelayTime, data.delayTime);
    if (data.delayFdbk[0]) SetWindowTextA(hDelayFdbk, data.delayFdbk);
    if (data.delayMix[0]) SetWindowTextA(hDelayMix, data.delayMix);

    if (data.arpMode >= 0 && data.arpMode < 5) {
        SendMessage(hComboArp, CB_SETCURSEL, data.arpMode, 0);
    }

    if (data.octaveShift >= -2 && data.octaveShift <= 2) {
        g_octaveShift = data.octaveShift;
    }

    HWND hwndMain = GetParent(hFreq);
    if (hwndMain) UpdateWindowTitle(hwndMain);

    PlayTone();
    return TRUE;
}

BOOL HasSavedState(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    DWORD size = GetFileSize(hFile, NULL);
    CloseHandle(hFile);
    return (size == sizeof(KSynthSaveData));
}

BOOL HasSeenTutorial(void) {
    HANDLE hFile = CreateFileA("ksynth_tutorial.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        return TRUE;
    }
    return FALSE;
}

void MarkTutorialSeen(void) {
    HANDLE hFile = CreateFileA("ksynth_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char val = '1';
        DWORD written = 0;
        WriteFile(hFile, &val, 1, &written, NULL);
        CloseHandle(hFile);
    }
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

void ShowHelp(HWND hwnd) {
    MarkTutorialSeen();
    MessageBoxA(hwnd,
        "KSynth Workstation Pro - Keyboard & Controls Guide\n\n"
        "Interactive Piano Keyboard:\n"
        "  White Keys: [A]=C4  [S]=D4  [D]=E4  [F]=F4  [G]=G4  [H]=A4  [J]=B4  [K]=C5\n"
        "  Black Keys: [W]=C#4 [E]=D#4 [T]=F#4 [Y]=G#4 [U]=A#4\n"
        "  (Click directly on virtual piano keys with mouse!)\n\n"
        "Workstation Persistence & Shortcuts:\n"
        "  [F5]          : Quicksave entire synth workstation state (ksynth.dat)\n"
        "  [F9]          : Quickload saved workstation state\n"
        "  [Space]       : Play active synth tone\n"
        "  [P]           : Play arpeggiator pattern\n"
        "  [E] / [Ctrl+S]: Export sound to 16-bit 44.1 kHz WAV file\n"
        "  [1] - [9]     : Direct preset switch (Lead, Bass, Pad, 8-Bit, Bell, Noise, FM, SPC, Beacon)\n"
        "  [Z] / [X]     : Shift keyboard octave down / up (-2 to +2)\n"
        "  [Shift+S]     : Export 2-Op FM patch preset (.fm) to music\\ (VFS)\n"
        "  [Shift+O]     : Import 2-Op FM patch preset (.fm) from music\\ (VFS)\n"
        "  [Shift+R]     : Render audio to music\\ksynth_track.wav and tune KRadio\n"
        "  [Shift+T]     : Publish 2-Op FM preset to Trapster P2P swarm catalog\n"
        "  [Esc]         : Panic (stop sound) / Unfocus edit fields / Dismiss\n"
        "  [Enter]       : Play tone while editing frequency or ADSR fields\n"
        "  [F1] or [?]   : Display this Help guide\n\n"
        "Synthesizer Modules:\n"
        "  Preset [1-9]  : 9 sound design presets (including YM2612 FM, SPC700, 1999 Beacon)\n"
        "  Waveforms     : Sine, Square, Sawtooth, Triangle, Noise\n"
        "  ADSR Envelope : Attack, Decay, Sustain, Release curves\n"
        "  Delay Echo    : Delay time, Feedback loop, Wet/Dry mix\n"
        "  Arpeggiator   : Up, Down, Up-Down, and Random arpeggiated runs\n"
        "  Oscilloscope  : Real-time PCM waveform display",
        "KSynth Workstation Pro Help", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (msg) {
        case WM_CREATE: {
            int fontHeight = -Scale(14);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            // Preset Selection
            CreateScaledWindowEx(0, "STATIC", "Preset [1-9]:", WS_CHILD | WS_VISIBLE, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hComboPreset = CreateScaledWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 100, 12, 160, 210, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"1: Neon Lead");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"2: Sub Bass");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"3: Warm Pad");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"4: 8-Bit Chiptune");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"5: Glass Bell");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"6: Noise Generator");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"7: YM2612 FM Bass");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"8: SPC700 Echo Pad");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"9: 1999 Ghost Beacon");
            SendMessage(hComboPreset, CB_SETCURSEL, 0, 0);

            // Waveform Selection
            CreateScaledWindowEx(0, "STATIC", "Waveform:", WS_CHILD | WS_VISIBLE, 15, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hComboWave = CreateScaledWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 100, 42, 160, 150, hwnd, NULL, NULL, NULL);
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Sine");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Square");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Sawtooth");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Triangle");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Noise");
            SendMessage(hComboWave, CB_SETCURSEL, 0, 0);
            
            // Frequency
            CreateScaledWindowEx(0, "STATIC", "Freq (Hz):", WS_CHILD | WS_VISIBLE, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hFreq = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "440", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER | WS_TABSTOP, 100, 72, 80, 22, hwnd, NULL, NULL, NULL);

            // ADSR Group
            CreateScaledWindowEx(0, "STATIC", "Attack (s):", WS_CHILD | WS_VISIBLE, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hAttack = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.05", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 102, 80, 22, hwnd, NULL, NULL, NULL);

            CreateScaledWindowEx(0, "STATIC", "Decay (s):", WS_CHILD | WS_VISIBLE, 15, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hDecay = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.20", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 132, 80, 22, hwnd, NULL, NULL, NULL);

            CreateScaledWindowEx(0, "STATIC", "Sustain (0-1):", WS_CHILD | WS_VISIBLE, 15, 165, 80, 20, hwnd, NULL, NULL, NULL);
            hSustain = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.60", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 162, 80, 22, hwnd, NULL, NULL, NULL);

            CreateScaledWindowEx(0, "STATIC", "Release (s):", WS_CHILD | WS_VISIBLE, 15, 195, 80, 20, hwnd, NULL, NULL, NULL);
            hRelease = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.40", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 192, 80, 22, hwnd, NULL, NULL, NULL);

            // Delay Group
            CreateScaledWindowEx(0, "STATIC", "Delay (s):", WS_CHILD | WS_VISIBLE, 15, 225, 80, 20, hwnd, NULL, NULL, NULL);
            hDelayTime = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.30", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 222, 80, 22, hwnd, NULL, NULL, NULL);

            CreateScaledWindowEx(0, "STATIC", "Fdbk (0-1):", WS_CHILD | WS_VISIBLE, 15, 255, 80, 20, hwnd, NULL, NULL, NULL);
            hDelayFdbk = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.40", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 252, 80, 22, hwnd, NULL, NULL, NULL);

            CreateScaledWindowEx(0, "STATIC", "Mix (0-1):", WS_CHILD | WS_VISIBLE, 15, 285, 80, 20, hwnd, NULL, NULL, NULL);
            hDelayMix = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.50", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 100, 282, 80, 22, hwnd, NULL, NULL, NULL);

            // Arpeggiator Group
            CreateScaledWindowEx(0, "STATIC", "Arp Mode:", WS_CHILD | WS_VISIBLE, 15, 320, 80, 20, hwnd, NULL, NULL, NULL);
            hComboArp = CreateScaledWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 100, 317, 160, 120, hwnd, NULL, NULL, NULL);
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Off");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Up");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Down");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Up-Down");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Random");
            SendMessage(hComboArp, CB_SETCURSEL, 0, 0);

            // Play, Arp, Export WAV & Help Buttons
            hBtnPlay      = CreateScaledWindowEx(0, "BUTTON", "▶ Play", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 356, 58, 28, hwnd, (HMENU)1, NULL, NULL);
            hBtnSeq       = CreateScaledWindowEx(0, "BUTTON", "⚡ Arp", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 76, 356, 55, 28, hwnd, (HMENU)2, NULL, NULL);
            hBtnExportWav = CreateScaledWindowEx(0, "BUTTON", "🔊 WAV", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 134, 356, 58, 28, hwnd, (HMENU)4, NULL, NULL);
            hBtnHelp      = CreateScaledWindowEx(0, "BUTTON", "❓ Help", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 195, 356, 65, 28, hwnd, (HMENU)3, NULL, NULL);

            // Buttons Row 2: Quicksave [F5], Quickload [F9]
            hBtnSave      = CreateScaledWindowEx(0, "BUTTON", "💾 Quicksave [F5]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 388, 118, 26, hwnd, (HMENU)5, NULL, NULL);
            hBtnLoad      = CreateScaledWindowEx(0, "BUTTON", "📂 Quickload [F9]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 137, 388, 123, 26, hwnd, (HMENU)6, NULL, NULL);

            // Buttons Row 3: FM Patch Bank VFS: Export [Shift+S], Import [Shift+O]
            hBtnExportFm  = CreateScaledWindowEx(0, "BUTTON", "💾 .FM [Shift+S]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 418, 118, 26, hwnd, (HMENU)7, NULL, NULL);
            hBtnImportFm  = CreateScaledWindowEx(0, "BUTTON", "📂 Open .FM [Shift+O]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 137, 418, 123, 26, hwnd, (HMENU)8, NULL, NULL);

            // Buttons Row 4: Cross-App Workstation Bridges: KRadio [Shift+R], Trapster [Shift+T]
            hBtnPlayRadio = CreateScaledWindowEx(0, "BUTTON", "📻 KRadio [Shift+R]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 448, 118, 26, hwnd, (HMENU)9, NULL, NULL);
            hBtnShareTrap = CreateScaledWindowEx(0, "BUTTON", "🐱 Trapster [Shift+T]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 137, 448, 123, 26, hwnd, (HMENU)10, NULL, NULL);

            // Oscilloscope Box Window
            WNDCLASS sc = {0};
            sc.lpfnWndProc = ScopeProc;
            sc.hInstance = GetModuleHandle(NULL);
            sc.lpszClassName = "KSynthScope";
            RegisterClass(&sc);

            hScopeWnd = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "KSynthScope", "", WS_CHILD | WS_VISIBLE, 280, 12, 350, 225, hwnd, NULL, GetModuleHandle(NULL), NULL);

            // Virtual Piano Box Window
            WNDCLASS pc = {0};
            pc.lpfnWndProc = PianoProc;
            pc.hInstance = GetModuleHandle(NULL);
            pc.lpszClassName = "KSynthPiano";
            RegisterClass(&pc);

            hPianoWnd = CreateScaledWindowEx(WS_EX_CLIENTEDGE, "KSynthPiano", "", WS_CHILD | WS_VISIBLE, 280, 244, 350, 96, hwnd, NULL, GetModuleHandle(NULL), NULL);

            // Keyboard navigation hint & Status label
            CreateScaledWindowEx(0, "STATIC", "Keys: [A-K] Oct 4 | [Z/X] Octave Shift | [1-6] Presets", WS_CHILD | WS_VISIBLE, 280, 344, 350, 18, hwnd, NULL, NULL, NULL);
            hStatusText = CreateScaledWindowEx(0, "STATIC", "Ready | Press [Space] or [A-K] to play sound | [F1] Help", WS_CHILD | WS_VISIBLE, 280, 368, 350, 46, hwnd, NULL, NULL, NULL);
            CreateScaledWindowEx(0, "STATIC", "VFS Audio: /home/music/ (*.fm / *.wav) | KRadio & Trapster", WS_CHILD | WS_VISIBLE, 280, 418, 350, 20, hwnd, NULL, NULL, NULL);
            CreateScaledWindowEx(0, "STATIC", "Shortcuts: [Shift+S/O] FM Patch | [Shift+R] Radio | [Shift+T] P2P", WS_CHILD | WS_VISIBLE, 280, 444, 350, 24, hwnd, NULL, NULL, NULL);

            // Subclass edit controls
            HWND editControls[] = { hFreq, hAttack, hDecay, hSustain, hRelease, hDelayTime, hDelayFdbk, hDelayMix };
            for (int i = 0; i < 8; i++) {
                WNDPROC old = (WNDPROC)SetWindowLongPtr(editControls[i], GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
                if (!g_oldEditProc) g_oldEditProc = old;
            }

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            break;
        }
        case WM_TIMER: {
            if (wParam == 100) {
                KillTimer(hwnd, 100);
                g_activePianoKey = -1;
                if (hPianoWnd) InvalidateRect(hPianoWnd, NULL, FALSE);
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                PlayTone();
                UpdateStatusText("Playing synthesizer tone [Space]");
            } else if (LOWORD(wParam) == 2) {
                PlayArpeggiator();
                UpdateStatusText("Playing arpeggio sequence [P]");
            } else if (LOWORD(wParam) == 3) {
                ShowHelp(hwnd);
            } else if (LOWORD(wParam) == 4) {
                ExportWav(hwnd);
            } else if (LOWORD(wParam) == 5) {
                SaveStateToFile("ksynth.dat");
                UpdateStatusText("★ Workstation quicksaved to ksynth.dat [F5]");
            } else if (LOWORD(wParam) == 6) {
                if (LoadStateFromFile("ksynth.dat")) {
                    MarkTutorialSeen();
                    UpdateStatusText("★ Workstation quickloaded from ksynth.dat [F9]");
                } else {
                    UpdateStatusText("⚠ No quicksave file found (ksynth.dat)");
                }
            } else if (LOWORD(wParam) == 7) {
                ExportFmPreset(hwnd, "music\\ksynth_patch.fm");
            } else if (LOWORD(wParam) == 8) {
                ImportFmPreset(hwnd, "music\\ksynth_patch.fm");
            } else if (LOWORD(wParam) == 9) {
                PlayTrackInKRadio(hwnd);
            } else if (LOWORD(wParam) == 10) {
                ShareInTrapster(hwnd);
            } else if (HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == hComboPreset) {
                int sel = SendMessage(hComboPreset, CB_GETCURSEL, 0, 0);
                ApplyPreset(sel);
                PlayTone();
            }
            break;
        }
        case WM_DESTROY:
            SaveStateToFile("ksynth.dat");
            if (hWaveOut) {
                waveOutReset(hWaveOut);
                waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
                waveOutClose(hWaveOut);
                hWaveOut = NULL;
            }
            if (hFont) DeleteObject(hFont);
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

void UpdateWindowTitle(HWND hwnd) {
    char title[128];
    if (g_octaveShift == 0) {
        wsprintfA(title, "KSynth Workstation Pro [Space: Play | F1: Help]");
    } else {
        wsprintfA(title, "KSynth Workstation Pro [Octave: %s%d | Space: Play | F1: Help]",
                  g_octaveShift > 0 ? "+" : "", g_octaveShift);
    }
    SetWindowTextA(hwnd, title);
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSynthApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    RECT rect = { 0, 0, Scale(W), Scale(H) };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE, 0);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    HWND hwnd = CreateWindowEx(0, "KSynthApp", "KSynth Workstation Pro [Space: Play | F1: Help]", (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    if (HasSavedState("ksynth.dat")) {
        if (LoadStateFromFile("ksynth.dat")) {
            UpdateStatusText("★ Restored saved state from ksynth.dat [F9]");
        }
    } else if (!HasSeenTutorial()) {
        ShowHelp(hwnd);
        UpdateStatusText("Welcome to KSynth! Press [Space] or [A-K] to play");
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && !(msg.lParam & 0x40000000)) {
            WPARAM key = msg.wParam;
            if (key == VK_OEM_2 || key == VK_F1) {
                ShowHelp(hwnd);
                continue;
            }
            HWND hFocus = GetFocus();
            BOOL isEditing = (hFocus == hFreq || hFocus == hAttack || hFocus == hDecay ||
                              hFocus == hSustain || hFocus == hRelease || hFocus == hDelayTime ||
                              hFocus == hDelayFdbk || hFocus == hDelayMix);
            if (!isEditing) {
                if (GetKeyState(VK_SHIFT) & 0x8000) {
                    if (key == 'S') {
                        ExportFmPreset(hwnd, "music\\ksynth_patch.fm");
                        continue;
                    }
                    if (key == 'O') {
                        ImportFmPreset(hwnd, "music\\ksynth_patch.fm");
                        continue;
                    }
                    if (key == 'R') {
                        PlayTrackInKRadio(hwnd);
                        continue;
                    }
                    if (key == 'T') {
                        ShareInTrapster(hwnd);
                        continue;
                    }
                }
                if (key == VK_F5) {
                    SaveStateToFile("ksynth.dat");
                    UpdateStatusText("★ Workstation quicksaved to ksynth.dat [F5]");
                    continue;
                }
                if (key == VK_F9) {
                    if (LoadStateFromFile("ksynth.dat")) {
                        MarkTutorialSeen();
                        UpdateStatusText("★ Workstation quickloaded from ksynth.dat [F9]");
                    } else {
                        UpdateStatusText("⚠ No quicksave file found (ksynth.dat)");
                    }
                    continue;
                }
                if (key == VK_SPACE) {
                    PlayTone();
                    UpdateStatusText("Playing synthesizer tone [Space]");
                    continue;
                }
                if (key == 'P') {
                    PlayArpeggiator();
                    UpdateStatusText("Playing arpeggio sequence [P]");
                    continue;
                }
                if (key == 'E' || (key == 'S' && (GetKeyState(VK_CONTROL) & 0x8000))) {
                    ExportWav(hwnd);
                    continue;
                }
                if (key >= '1' && key <= '9') {
                    int pIdx = key - '1';
                    ApplyPreset(pIdx);
                    SendMessage(hComboPreset, CB_SETCURSEL, pIdx, 0);
                    PlayTone();
                    continue;
                }
                if (key == VK_ESCAPE) {
                    if (hWaveOut) waveOutReset(hWaveOut);
                    SetFocus(hwnd);
                    UpdateStatusText("Panic: Audio Stopped | Ready");
                    continue;
                }
                if (key == 'Z') {
                    if (g_octaveShift > -2) {
                        g_octaveShift--;
                        UpdateWindowTitle(hwnd);
                        char oBuf[48];
                        wsprintfA(oBuf, "Octave shifted to: %s%d", g_octaveShift > 0 ? "+" : "", g_octaveShift);
                        UpdateStatusText(oBuf);
                    }
                    continue;
                }
                if (key == 'X') {
                    if (g_octaveShift < 2) {
                        g_octaveShift++;
                        UpdateWindowTitle(hwnd);
                        char oBuf[48];
                        wsprintfA(oBuf, "Octave shifted to: %s%d", g_octaveShift > 0 ? "+" : "", g_octaveShift);
                        UpdateStatusText(oBuf);
                    }
                    continue;
                }
                double noteMap[256] = {0};
                noteMap['A'] = 261.63; noteMap['W'] = 277.18; noteMap['S'] = 293.66;
                noteMap['E'] = 311.13; noteMap['D'] = 329.63; noteMap['F'] = 349.23;
                noteMap['T'] = 369.99; noteMap['G'] = 392.00; noteMap['Y'] = 415.30;
                noteMap['H'] = 440.00; noteMap['U'] = 466.16; noteMap['J'] = 493.88;
                noteMap['K'] = 523.25;
                if (key < 256 && noteMap[key] > 0) {
                    double mult = 1.0;
                    if (g_octaveShift == -2) mult = 0.25;
                    else if (g_octaveShift == -1) mult = 0.5;
                    else if (g_octaveShift == 1) mult = 2.0;
                    else if (g_octaveShift == 2) mult = 4.0;
                    int finalFreq = (int)(noteMap[key] * mult + 0.5);
                    char buf[32];
                    wsprintfA(buf, "%d", finalFreq);
                    SetWindowTextA(hFreq, buf);
                    PlayTone();

                    int noteIdx = -1;
                    if (key == 'A') noteIdx = 0;
                    else if (key == 'W') noteIdx = 1;
                    else if (key == 'S') noteIdx = 2;
                    else if (key == 'E') noteIdx = 3;
                    else if (key == 'D') noteIdx = 4;
                    else if (key == 'F') noteIdx = 5;
                    else if (key == 'T') noteIdx = 6;
                    else if (key == 'G') noteIdx = 7;
                    else if (key == 'Y') noteIdx = 8;
                    else if (key == 'H') noteIdx = 9;
                    else if (key == 'U') noteIdx = 10;
                    else if (key == 'J') noteIdx = 11;
                    else if (key == 'K') noteIdx = 12;

                    if (noteIdx >= 0) {
                        g_activePianoKey = noteIdx;
                        if (hPianoWnd) InvalidateRect(hPianoWnd, NULL, FALSE);
                        SetTimer(hwnd, 100, 200, NULL);
                    }
                    wsprintfA(buf, "Playing: %d Hz", finalFreq);
                    UpdateStatusText(buf);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    ExitProcess(0);
}
