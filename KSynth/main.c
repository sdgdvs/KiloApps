#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define W 300
#define H 200

HWND hCombo, hBtnPlay, hFreq;
HWAVEOUT hWaveOut;
WAVEHDR waveHdr;

#define SAMPLE_RATE 44100
#define DURATION 1 // 1 second
short buffer[SAMPLE_RATE * DURATION];

int _fltused = 0;

void GenerateWave(int type, int freq) {
    double phase = 0.0;
    double phaseInc = (double)freq / SAMPLE_RATE;
    
    for (int i = 0; i < SAMPLE_RATE * DURATION; i++) {
        double val = 0.0;
        
        if (type == 0) { // Sine (Parabolic approximation)
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
        }
        
        buffer[i] = (short)(val * 8000.0); // Volume scaling
        
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
}

void PlayTone() {
    int sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) sel = 0;
    
    char freqBuf[32];
    GetWindowTextA(hFreq, freqBuf, 32);
    int freq = 0;
    char* p = freqBuf;
    while (*p) {
        if (*p >= '0' && *p <= '9') freq = freq * 10 + (*p - '0');
        p++;
    }
    if (freq < 20) freq = 440;
    if (freq > 20000) freq = 20000;
    
    GenerateWave(sel, freq);
    
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        waveHdr.lpData = (LPSTR)buffer;
        waveHdr.dwBufferLength = sizeof(buffer);
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
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "Waveform:", WS_CHILD | WS_VISIBLE, 10, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hCombo = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, 12, 120, 100, hwnd, NULL, NULL, NULL);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Sine");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Square");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Sawtooth");
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
            
            CreateWindowEx(0, "STATIC", "Freq (Hz):", WS_CHILD | WS_VISIBLE, 10, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hFreq = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "440", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, 100, 42, 60, 22, hwnd, NULL, NULL, NULL);
            
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play Tone", WS_CHILD | WS_VISIBLE, 100, 80, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            
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
            }
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
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
