#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>

#define W 320
#define H 340

HWND hCombo, hBtnPlay, hFreq, hAttack, hDecay, hSustain, hRelease;
HWAVEOUT hWaveOut;
WAVEHDR waveHdr;

#define SAMPLE_RATE 44100
#define MAX_DURATION 5
short buffer[SAMPLE_RATE * MAX_DURATION];

int _fltused = 0;

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

void GenerateWave(int type, int freq, double attack, double decay, double sustain, double release) {
    double phase = 0.0;
    double phaseInc = (double)freq / SAMPLE_RATE;
    
    double sustain_time = 0.5;
    double total_time = attack + decay + sustain_time + release;
    if (total_time > 5.0) total_time = 5.0;
    
    int total_samples = (int)(total_time * SAMPLE_RATE);
    if (total_samples < 1) total_samples = 1;
    
    for (int i = 0; i < total_samples; i++) {
        double val = 0.0;
        
        if (type == 0) { // Sine
            double t = phase;
            if (t < 0.5) {
                double x = t * 2.0;
                val = 4.0 * x * (1.0 - x);
            } else {
                double x = (t - 0.5) * 2.0;
                val = -4.0 * x * (1.0 - x);
            }
        } else if (type == 1) { // Square
            val = (phase < 0.5) ? 0.5 : -0.5;
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
}

void PlayTone() {
    int sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
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

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "Waveform:", WS_CHILD | WS_VISIBLE, 10, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hCombo = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, 12, 120, 100, hwnd, NULL, NULL, NULL);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Sine");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Square");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Sawtooth");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Triangle");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Noise");
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
            
            CreateWindowEx(0, "STATIC", "Freq (Hz):", WS_CHILD | WS_VISIBLE, 10, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hFreq = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "440", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, 100, 42, 60, 22, hwnd, NULL, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Attack(s):", WS_CHILD | WS_VISIBLE, 10, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hAttack = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 72, 60, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Decay(s):", WS_CHILD | WS_VISIBLE, 10, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hDecay = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.2", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 102, 60, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sustain(0-1):", WS_CHILD | WS_VISIBLE, 10, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hSustain = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.5", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 132, 60, 22, hwnd, NULL, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Release(s):", WS_CHILD | WS_VISIBLE, 10, 165, 80, 20, hwnd, NULL, NULL, NULL);
            hRelease = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0.5", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 100, 162, 60, 22, hwnd, NULL, NULL, NULL);
            
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play Tone", WS_CHILD | WS_VISIBLE, 100, 200, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Play Keyboard: A=C4, W=C#4, S=D4, E=D#4,\nD=E4, F=F4, T=F#4, G=G4, Y=G#4, H=A4, U=A#4, J=B4, K=C5", WS_CHILD | WS_VISIBLE, 10, 240, 300, 40, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                PlayTone();
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

    HWND hwnd = CreateWindowEx(0, "KSynthApp", "KSynth", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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

