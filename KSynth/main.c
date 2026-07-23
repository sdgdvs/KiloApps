#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>

#define W 640
#define H 540

HWND hComboPreset, hComboWave, hBtnPlay, hBtnSeq, hFreq, hAttack, hDecay, hSustain, hRelease;
HWND hComboArp, hArpBpm, hArpOct;
HWND hScopeWnd;

HWAVEOUT hWaveOut;
WAVEHDR waveHdr;

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

Preset g_presets[] = {
    { 0, 440, "0.05", "0.20", "0.60", "0.40" }, // Neon Lead (Sine)
    { 1, 110, "0.01", "0.25", "0.40", "0.15" }, // Sub Bass (Square)
    { 2, 330, "0.40", "0.60", "0.80", "1.00" }, // Warm Pad (Sawtooth)
    { 1, 523, "0.001","0.10", "0.30", "0.05" }, // 8-Bit Chiptune
    { 3, 880, "0.005","0.60", "0.10", "0.80" }, // Glass Bell (Triangle)
    { 4, 440, "0.01", "0.15", "0.10", "0.10" }  // Noise Generator
};

void GenerateWave(int type, int freq, double attack, double decay, double sustain, double release) {
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
        
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
    
    for (int i = total_samples; i < SAMPLE_RATE * MAX_DURATION; i++) {
        buffer[i] = 0;
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
    
    GenerateWave(sel, freq, attack, decay, sustain, release);
    
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
        waveHdr.dwBufferLength = waveHdr.dwBufferLength ? waveHdr.dwBufferLength : (DWORD)(SAMPLE_RATE * sizeof(short));
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
    if (idx < 0 || idx >= 6) return;
    Preset p = g_presets[idx];
    SendMessage(hComboWave, CB_SETCURSEL, p.wave, 0);
    
    char buf[32];
    wsprintfA(buf, "%d", p.freq);
    SetWindowTextA(hFreq, buf);

    SetWindowTextA(hAttack, p.attack);
    SetWindowTextA(hDecay, p.decay);
    SetWindowTextA(hSustain, p.sustain);
    SetWindowTextA(hRelease, p.release);
}

// Custom Oscilloscope Window Procedure
LRESULT CALLBACK ScopeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;

            HBRUSH hBg = CreateSolidBrush(RGB(15, 18, 26));
            FillRect(hdc, &rect, hBg);
            DeleteObject(hBg);

            HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(40, 48, 64));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
            for (int x = 0; x < w; x += 30) {
                MoveToEx(hdc, x, 0, NULL); LineTo(hdc, x, h);
            }
            for (int y = 0; y < h; y += 20) {
                MoveToEx(hdc, 0, y, NULL); LineTo(hdc, w, y);
            }
            DeleteObject(SelectObject(hdc, hOldPen));

            HPEN hScopePen = CreatePen(PS_SOLID, 2, RGB(0, 243, 255));
            hOldPen = (HPEN)SelectObject(hdc, hScopePen);

            int midY = h / 2;
            int step = bufferSampleCount / w;
            if (step < 1) step = 1;

            MoveToEx(hdc, 0, midY, NULL);
            for (int x = 0; x < w; x++) {
                int sampleIdx = x * step;
                if (sampleIdx >= bufferSampleCount) break;
                short sample = buffer[sampleIdx];
                int y = midY - (sample * (h / 2) / 32768);
                LineTo(hdc, x, y);
            }

            DeleteObject(SelectObject(hdc, hOldPen));
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            // Preset Selection
            CreateWindowEx(0, "STATIC", "Preset:", WS_CHILD | WS_VISIBLE, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hComboPreset = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, 12, 160, 150, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"0: Neon Lead");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"1: Sub Bass");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"2: Warm Pad");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"3: 8-Bit Chiptune");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"4: Glass Bell");
            SendMessage(hComboPreset, CB_ADDSTRING, 0, (LPARAM)"5: Noise Generator");
            SendMessage(hComboPreset, CB_SETCURSEL, 0, 0);

            // Waveform Selection
            CreateWindowEx(0, "STATIC", "Waveform:", WS_CHILD | WS_VISIBLE, 15, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hComboWave = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, 42, 160, 150, hwnd, NULL, NULL, NULL);
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Sine");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Square");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Sawtooth");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Triangle");
            SendMessage(hComboWave, CB_ADDSTRING, 0, (LPARAM)"Noise");
            SendMessage(hComboWave, CB_SETCURSEL, 0, 0);
            
            // Frequency
            CreateWindowEx(0, "STATIC", "Freq (Hz):", WS_CHILD | WS_VISIBLE, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hFreq = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "440", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, 100, 72, 80, 22, hwnd, NULL, NULL, NULL);

            // ADSR Group
            CreateWindowEx(0, "STATIC", "Attack (s):", WS_CHILD | WS_VISIBLE, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hAttack = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.05", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 102, 80, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Decay (s):", WS_CHILD | WS_VISIBLE, 15, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hDecay = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.20", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 132, 80, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sustain (0-1):", WS_CHILD | WS_VISIBLE, 15, 165, 80, 20, hwnd, NULL, NULL, NULL);
            hSustain = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.60", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 162, 80, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Release (s):", WS_CHILD | WS_VISIBLE, 15, 195, 80, 20, hwnd, NULL, NULL, NULL);
            hRelease = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.40", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 192, 80, 22, hwnd, NULL, NULL, NULL);

            // Arpeggiator Group
            CreateWindowEx(0, "STATIC", "Arp Mode:", WS_CHILD | WS_VISIBLE, 15, 230, 80, 20, hwnd, NULL, NULL, NULL);
            hComboArp = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, 227, 160, 120, hwnd, NULL, NULL, NULL);
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Off");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Up");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Down");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Up-Down");
            SendMessage(hComboArp, CB_ADDSTRING, 0, (LPARAM)"Random");
            SendMessage(hComboArp, CB_SETCURSEL, 0, 0);

            // Play Buttons
            hBtnPlay = CreateWindowEx(0, "BUTTON", "▶ Play Tone", WS_CHILD | WS_VISIBLE, 15, 270, 110, 32, hwnd, (HMENU)1, NULL, NULL);
            hBtnSeq  = CreateWindowEx(0, "BUTTON", "⚡ Run Arp", WS_CHILD | WS_VISIBLE, 140, 270, 120, 32, hwnd, (HMENU)2, NULL, NULL);

            // Oscilloscope Box Window
            WNDCLASS sc = {0};
            sc.lpfnWndProc = ScopeProc;
            sc.hInstance = GetModuleHandle(NULL);
            sc.lpszClassName = "KSynthScope";
            RegisterClass(&sc);

            hScopeWnd = CreateWindowEx(WS_EX_CLIENTEDGE, "KSynthScope", "", WS_CHILD | WS_VISIBLE, 280, 12, 330, 240, hwnd, NULL, GetModuleHandle(NULL), NULL);

            CreateWindowEx(0, "STATIC", "Keyboard mapping:\nKeys [A, W, S, E, D, F, T, G, Y, H, U, J, K]\nTrigger notes C4 to C5 in real-time.", WS_CHILD | WS_VISIBLE, 280, 260, 330, 50, hwnd, NULL, NULL, NULL);

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                PlayTone();
            } else if (LOWORD(wParam) == 2) {
                PlayArpeggiator();
            } else if (HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == hComboPreset) {
                int sel = SendMessage(hComboPreset, CB_GETCURSEL, 0, 0);
                ApplyPreset(sel);
            }
            break;
        }
        case WM_DESTROY:
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

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSynthApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSynthApp", "KSynth Workstation Pro", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && !(msg.lParam & 0x40000000)) {
            HWND hFocus = GetFocus();
            if (hFocus != hFreq && hFocus != hAttack && hFocus != hDecay && hFocus != hSustain && hFocus != hRelease) {
                double noteMap[256] = {0};
                noteMap['A'] = 261.63; noteMap['W'] = 277.18; noteMap['S'] = 293.66;
                noteMap['E'] = 311.13; noteMap['D'] = 329.63; noteMap['F'] = 349.23;
                noteMap['T'] = 369.99; noteMap['G'] = 392.00; noteMap['Y'] = 415.30;
                noteMap['H'] = 440.00; noteMap['U'] = 466.16; noteMap['J'] = 493.88;
                noteMap['K'] = 523.25;
                WPARAM key = msg.wParam;
                if (key < 256 && noteMap[key] > 0) {
                    char buf[32];
                    wsprintfA(buf, "%d", (int)(noteMap[key] + 0.5));
                    SetWindowTextA(hFreq, buf);
                    PlayTone();
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
