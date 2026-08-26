#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define W 1000
#define H 800
#define NUM_KEYS 13
#define PI 3.14159265358979323846

HMIDIOUT hMidi = NULL;
int activeKeys[NUM_KEYS] = {0};
int notes[NUM_KEYS] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72};
char binds[NUM_KEYS] = {'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K'};
int is_black[NUM_KEYS] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0};
int instrument = 0;
int octaveShift = 0;

// Standalone math functions to avoid CRT dependency
static double MyFmod(double x, double y) {
    if (y == 0.0) return 0.0;
    long q = (long)(x / y);
    return x - (double)q * y;
}

static double MySin(double x) {
    x = MyFmod(x, 2.0 * PI);
    if (x < 0) x += 2.0 * PI;
    if (x > PI) return -MySin(x - PI);
    if (x > PI / 2.0) return MySin(PI - x);
    double x2 = x * x;
    return x * (1.0 - x2 / 6.0 + (x2 * x2) / 120.0 - (x2 * x2 * x2) / 5040.0);
}

// Recording & Playback State
typedef struct { int keyIndex; DWORD time; int type; } AudioEvent;
AudioEvent recordedEvents[4000];
int numEvents = 0;
int isRecording = 0;
int isPlaying = 0;
DWORD recordingStartTime = 0;
DWORD playbackStartTime = 0;
int playbackIndex = 0;
int mouseActiveKey = -1;

// Sequencer State (16 Steps)
int seqPattern[16] = {1, 0, 0, 1,  0, 1, 0, 0,  1, 0, 1, 0,  0, 0, 1, 0};
int seqPlaying = 0;
int currentStep = 0;
DWORD lastStepTime = 0;
int showHelp = 1;

// Waveform Visualizer Animation Offset
int visOffset = 0;

// WAV Header Structure
#pragma pack(push, 1)
typedef struct {
    char chunkId[4];        // "RIFF"
    DWORD chunkSize;
    char format[4];         // "WAVE"
    char subchunk1Id[4];    // "fmt "
    DWORD subchunk1Size;    // 16
    WORD audioFormat;       // 1 (PCM)
    WORD numChannels;       // 1 (Mono)
    DWORD sampleRate;       // 44100
    DWORD byteRate;         // 44100 * 2
    WORD blockAlign;        // 2
    WORD bitsPerSample;     // 16
    char subchunk2Id[4];    // "data"
    DWORD subchunk2Size;
} WavHeader;
#pragma pack(pop)

void PlayNote(int index, int on) {
    if (hMidi) {
        int actualNote = notes[index] + octaveShift * 12;
        if (actualNote < 0) actualNote = 0;
        if (actualNote > 127) actualNote = 127;
        DWORD msg = on ? (0x007F0090 | (actualNote << 8)) : (0x00000080 | (actualNote << 8));
        midiOutShortMsg(hMidi, msg);
    }
}

// Procedural Sound FX Generator via MIDI Pitch Bends & Notes
void PlaySoundFX(int preset) {
    if (!hMidi) return;
    switch (preset) {
        case 1: // Jump
            midiOutShortMsg(hMidi, 0x0060E0); // Pitch bend down
            midiOutShortMsg(hMidi, 0x007F0090 | (60 << 8)); // C4
            Sleep(40);
            midiOutShortMsg(hMidi, 0x007FFF90 | (72 << 8)); // C5 bend up
            Sleep(80);
            midiOutShortMsg(hMidi, 0x00000080 | (60 << 8));
            midiOutShortMsg(hMidi, 0x00000080 | (72 << 8));
            midiOutShortMsg(hMidi, 0x0040E0); // Reset bend
            break;
        case 2: // Laser
            midiOutShortMsg(hMidi, 0x007F0090 | (84 << 8)); // C6
            midiOutShortMsg(hMidi, 0x007FFF00 | 0xE0); // Bend high
            Sleep(30);
            midiOutShortMsg(hMidi, 0x007F0090 | (72 << 8));
            Sleep(40);
            midiOutShortMsg(hMidi, 0x00000080 | (84 << 8));
            midiOutShortMsg(hMidi, 0x00000080 | (72 << 8));
            midiOutShortMsg(hMidi, 0x0040E0);
            break;
        case 3: // Explosion (Cluster)
            midiOutShortMsg(hMidi, 0x007F0090 | (36 << 8));
            midiOutShortMsg(hMidi, 0x007F0090 | (37 << 8));
            midiOutShortMsg(hMidi, 0x007F0090 | (38 << 8));
            Sleep(120);
            midiOutShortMsg(hMidi, 0x00000080 | (36 << 8));
            midiOutShortMsg(hMidi, 0x00000080 | (37 << 8));
            midiOutShortMsg(hMidi, 0x00000080 | (38 << 8));
            break;
        case 4: // Coin (Arpeggio Chirp)
            midiOutShortMsg(hMidi, 0x007F0090 | (71 << 8)); // B5
            Sleep(60);
            midiOutShortMsg(hMidi, 0x00000080 | (71 << 8));
            midiOutShortMsg(hMidi, 0x007F0090 | (76 << 8)); // E6
            Sleep(140);
            midiOutShortMsg(hMidi, 0x00000080 | (76 << 8));
            break;
        case 5: // Powerup
            for (int n = 60; n <= 72; n += 3) {
                midiOutShortMsg(hMidi, 0x007F0090 | (n << 8));
                Sleep(40);
                midiOutShortMsg(hMidi, 0x00000080 | (n << 8));
            }
            break;
    }
}

// Generate PCM WAV Audio File Export
void ExportWavFile() {
    DWORD sampleRate = 44100;
    DWORD durationSec = 2;
    DWORD totalSamples = sampleRate * durationSec;
    DWORD dataSize = totalSamples * sizeof(short);

    WavHeader hdr;
    hdr.chunkId[0] = 'R'; hdr.chunkId[1] = 'I'; hdr.chunkId[2] = 'F'; hdr.chunkId[3] = 'F';
    hdr.chunkSize = 36 + dataSize;
    hdr.format[0] = 'W'; hdr.format[1] = 'A'; hdr.format[2] = 'V'; hdr.format[3] = 'E';
    hdr.subchunk1Id[0] = 'f'; hdr.subchunk1Id[1] = 'm'; hdr.subchunk1Id[2] = 't'; hdr.subchunk1Id[3] = ' ';
    hdr.subchunk1Size = 16;
    hdr.audioFormat = 1;
    hdr.numChannels = 1;
    hdr.sampleRate = sampleRate;
    hdr.byteRate = sampleRate * sizeof(short);
    hdr.blockAlign = sizeof(short);
    hdr.bitsPerSample = 16;
    hdr.subchunk2Id[0] = 'd'; hdr.subchunk2Id[1] = 'a'; hdr.subchunk2Id[2] = 't'; hdr.subchunk2Id[3] = 'a';
    hdr.subchunk2Size = dataSize;

    HANDLE hFile = CreateFileA("kaudio_export.wav", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD written = 0;
    WriteFile(hFile, &hdr, sizeof(WavHeader), &written, NULL);

    // Synthesize 2 seconds of audio (Square wave arpeggio)
    for (DWORD i = 0; i < totalSamples; i++) {
        double t = (double)i / (double)sampleRate;
        double freq = 261.63; // C4
        if (MyFmod(t, 0.4) > 0.2) freq = 329.63; // E4
        double val = (MyFmod(t * freq, 1.0) > 0.5) ? 0.3 : -0.3;
        short pcmSample = (short)(val * 32767.0);
        WriteFile(hFile, &pcmSample, sizeof(short), &written, NULL);
    }
    CloseHandle(hFile);
    MessageBoxA(NULL, "WAV Export Saved to kaudio_export.wav!", "KAudio Export", MB_OK | MB_ICONINFORMATION);
}

int GetKeyAtPoint(int x, int y) {
    int num_white = 8;
    int whiteW = W / num_white;
    int keyTop = 220;
    int keyBottom = H - 40;

    // Check black keys
    if (y >= keyTop && y <= keyTop + 80) {
        int white_idx = 0;
        for (int i = 0; i < NUM_KEYS; i++) {
            if (is_black[i]) {
                int x1 = white_idx * whiteW - whiteW / 3;
                int x2 = white_idx * whiteW + whiteW / 3;
                if (x >= x1 && x <= x2) return i;
            } else {
                white_idx++;
            }
        }
    }

    // Check white keys
    if (y >= keyTop && y <= keyBottom) {
        int white_idx = 0;
        for (int i = 0; i < NUM_KEYS; i++) {
            if (!is_black[i]) {
                int x1 = white_idx * whiteW;
                int x2 = (white_idx + 1) * whiteW;
                if (x >= x1 && x < x2) return i;
                white_idx++;
            }
        }
    }
    return -1;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            midiOutOpen(&hMidi, (UINT)-1, 0, 0, CALLBACK_NULL);
            if (hMidi) midiOutShortMsg(hMidi, 0x000000C0);
            SetTimer(hwnd, 1, 30, NULL);
            break;

        case WM_TIMER:
            visOffset = (visOffset + 4) % 360;

            // Sequencer Step Timer (120 BPM -> step every 125ms)
            if (seqPlaying) {
                DWORD now = GetTickCount();
                if (now - lastStepTime >= 125) {
                    lastStepTime = now;
                    currentStep = (currentStep + 1) % 16;
                    if (seqPattern[currentStep]) {
                        PlayNote(currentStep % NUM_KEYS, 1);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            if (isPlaying) {
                DWORD now = GetTickCount() - playbackStartTime;
                int changed = 0;
                while (playbackIndex < numEvents && recordedEvents[playbackIndex].time <= now) {
                    int idx = recordedEvents[playbackIndex].keyIndex;
                    int t = recordedEvents[playbackIndex].type;
                    activeKeys[idx] = t;
                    PlayNote(idx, t);
                    playbackIndex++;
                    changed = 1;
                }
                if (playbackIndex >= numEvents) {
                    isPlaying = 0;
                    changed = 1;
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) { PlayNote(i, 0); activeKeys[i] = 0; }
                }
                if (changed) InvalidateRect(hwnd, NULL, FALSE);
            }
            break;

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);

            // Check Preset Buttons Click (y: 35-65)
            if (y >= 35 && y <= 65) {
                if (x >= 10 && x <= 70) PlaySoundFX(1);        // Jump
                else if (x >= 80 && x <= 140) PlaySoundFX(2);  // Laser
                else if (x >= 150 && x <= 220) PlaySoundFX(3); // Explosion
                else if (x >= 230 && x <= 280) PlaySoundFX(4); // Coin
                else if (x >= 290 && x <= 360) PlaySoundFX(5); // Powerup
                else if (x >= 500 && x <= 620) ExportWavFile(); // Export WAV
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            // Check 16-Step Sequencer Click (y: 175-200)
            if (y >= 175 && y <= 200 && x >= 120 && x <= 600) {
                int stepIdx = (x - 120) / 30;
                if (stepIdx >= 0 && stepIdx < 16) {
                    seqPattern[stepIdx] = !seqPattern[stepIdx];
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }

            // Check Keyboard Keys
            int k = GetKeyAtPoint(x, y);
            if (k != -1) {
                mouseActiveKey = k;
                SetCapture(hwnd);
                if (!activeKeys[k]) {
                    activeKeys[k] = 1;
                    PlayNote(k, 1);
                    if (isRecording && numEvents < 4000) {
                        recordedEvents[numEvents].keyIndex = k;
                        recordedEvents[numEvents].time = GetTickCount() - recordingStartTime;
                        recordedEvents[numEvents].type = 1;
                        numEvents++;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }

        case WM_LBUTTONUP:
        case WM_CAPTURECHANGED: {
            if (mouseActiveKey != -1) {
                int k = mouseActiveKey;
                mouseActiveKey = -1;
                if (GetCapture() == hwnd) ReleaseCapture();
                if (activeKeys[k]) {
                    activeKeys[k] = 0;
                    PlayNote(k, 0);
                    if (isRecording && numEvents < 4000) {
                        recordedEvents[numEvents].keyIndex = k;
                        recordedEvents[numEvents].time = GetTickCount() - recordingStartTime;
                        recordedEvents[numEvents].type = 0;
                        numEvents++;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }

        case WM_KEYDOWN: {
            int isRepeat = (lParam & 0x40000000) != 0;
            if (wParam == 'P' && !isRepeat) {
                seqPlaying = !seqPlaying;
                lastStepTime = GetTickCount();
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'E' && !isRepeat) {
                ExportWavFile();
                break;
            }
            if ((wParam == 'H' || wParam == VK_F1) && !isRepeat) {
                showHelp = !showHelp;
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'Z' && !isRepeat) {
                if (isPlaying) {
                    isPlaying = 0;
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) { PlayNote(i, 0); activeKeys[i] = 0; }
                }
                isRecording = !isRecording;
                if (isRecording) {
                    numEvents = 0;
                    recordingStartTime = GetTickCount();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'X' && !isRepeat) {
                if (isRecording) isRecording = 0;
                if (numEvents == 0) break;
                isPlaying = !isPlaying;
                if (isPlaying) {
                    playbackStartTime = GetTickCount();
                    playbackIndex = 0;
                } else {
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) { PlayNote(i, 0); activeKeys[i] = 0; }
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_UP && !isRepeat) {
                instrument = (instrument + 1) % 128;
                if (hMidi) midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_DOWN && !isRepeat) {
                instrument = (instrument + 127) % 128;
                if (hMidi) midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_LEFT && !isRepeat) {
                if (octaveShift > -4) { octaveShift--; InvalidateRect(hwnd, NULL, FALSE); }
                break;
            }
            if (wParam == VK_RIGHT && !isRepeat) {
                if (octaveShift < 4) { octaveShift++; InvalidateRect(hwnd, NULL, FALSE); }
                break;
            }
            for (int i = 0; i < NUM_KEYS; i++) {
                if (wParam == binds[i] && !activeKeys[i]) {
                    activeKeys[i] = 1;
                    PlayNote(i, 1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            break;
        }

        case WM_KEYUP: {
            for (int i = 0; i < NUM_KEYS; i++) {
                if (wParam == binds[i] && activeKeys[i]) {
                    activeKeys[i] = 0;
                    PlayNote(i, 0);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBm = (HBITMAP)SelectObject(memDC, hbm);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

            // Dark Theme Background
            HBRUSH bg = CreateSolidBrush(RGB(15, 23, 42));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);

            // Title & Status Header
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(56, 189, 248));
            char title[128];
            wsprintfA(title, "KAudio Pro Native Workstation | Inst: %d | Oct: %+d | P: Seq [%s] | E: Export WAV",
                      instrument, octaveShift, seqPlaying ? "PLAYING" : "STOPPED");
            TextOutA(memDC, 10, 8, title, lstrlenA(title));
            SetTextColor(memDC, RGB(251, 191, 36));
            TextOutA(memDC, W - 160, 8, "Press F1 or 'H' for Help", 24);

            // Draw Sound FX Preset Buttons (y: 35-65)
            HBRUSH btnBrush = CreateSolidBrush(RGB(30, 41, 59));
            HBRUSH exportBrush = CreateSolidBrush(RGB(14, 165, 233));
            SetTextColor(memDC, RGB(241, 245, 249));

            RECT r1 = {10, 35, 70, 65}; FillRect(memDC, &r1, btnBrush); TextOutA(memDC, 18, 42, "Jump", 4);
            RECT r2 = {80, 35, 140, 65}; FillRect(memDC, &r2, btnBrush); TextOutA(memDC, 90, 42, "Laser", 5);
            RECT r3 = {150, 35, 220, 65}; FillRect(memDC, &r3, btnBrush); TextOutA(memDC, 155, 42, "Explode", 7);
            RECT r4 = {230, 35, 280, 65}; FillRect(memDC, &r4, btnBrush); TextOutA(memDC, 240, 42, "Coin", 4);
            RECT r5 = {290, 35, 360, 65}; FillRect(memDC, &r5, btnBrush); TextOutA(memDC, 295, 42, "Powerup", 7);
            RECT rExp = {500, 35, 620, 65}; FillRect(memDC, &rExp, exportBrush); TextOutA(memDC, 515, 42, "Export WAV", 10);

            DeleteObject(btnBrush);
            DeleteObject(exportBrush);

            // Draw Waveform Visualizer Canvas (y: 75-160)
            HBRUSH visBg = CreateSolidBrush(RGB(2, 6, 23));
            RECT visRc = {10, 75, W - 10, 160};
            FillRect(memDC, &visRc, visBg);
            DeleteObject(visBg);

            HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
            HPEN oldPen = (HPEN)SelectObject(memDC, wavePen);
            int midY = 117;
            MoveToEx(memDC, 10, midY, NULL);
            for (int x = 10; x < W - 10; x += 5) {
                int waveY = midY + (int)(25.0 * MySin((x + visOffset) * PI / 45.0));
                LineTo(memDC, x, waveY);
            }
            SelectObject(memDC, oldPen);
            DeleteObject(wavePen);

            // Draw 16-Step Pattern Sequencer Row (y: 175-200)
            SetTextColor(memDC, RGB(148, 163, 184));
            TextOutA(memDC, 10, 180, "Sequencer:", 10);
            HBRUSH stepOff = CreateSolidBrush(RGB(51, 65, 85));
            HBRUSH stepOn = CreateSolidBrush(RGB(14, 165, 233));
            HBRUSH stepCurr = CreateSolidBrush(RGB(244, 63, 94));

            for (int st = 0; st < 16; st++) {
                RECT stRc = {120 + st * 30, 175, 144 + st * 30, 198};
                if (st == currentStep && seqPlaying) {
                    FillRect(memDC, &stRc, stepCurr);
                } else {
                    FillRect(memDC, &stRc, seqPattern[st] ? stepOn : stepOff);
                }
            }
            DeleteObject(stepOff); DeleteObject(stepOn); DeleteObject(stepCurr);

            // Draw Piano Keyboard (y: 220 to H-40)
            HBRUSH white = CreateSolidBrush(RGB(226, 232, 240));
            HBRUSH activeWhite = CreateSolidBrush(RGB(14, 165, 233));
            HBRUSH activeBlack = CreateSolidBrush(RGB(56, 189, 248));
            HBRUSH black = CreateSolidBrush(RGB(30, 41, 59));

            int num_white = 8;
            int whiteW = W / num_white;
            int keyTop = 220;
            int keyBottom = H - 40;

            int white_idx = 0;
            for (int i = 0; i < NUM_KEYS; i++) {
                if (!is_black[i]) {
                    RECT r = {white_idx * whiteW + 2, keyTop, (white_idx + 1) * whiteW - 2, keyBottom};
                    FillRect(memDC, &r, activeKeys[i] ? activeWhite : white);
                    SetTextColor(memDC, activeKeys[i] ? RGB(255, 255, 255) : RGB(71, 85, 105));
                    char text[2] = {binds[i], 0};
                    TextOutA(memDC, white_idx * whiteW + whiteW / 2 - 4, keyBottom - 24, text, 1);
                    white_idx++;
                }
            }

            white_idx = 0;
            for (int i = 0; i < NUM_KEYS; i++) {
                if (is_black[i]) {
                    RECT r = {white_idx * whiteW - whiteW / 3, keyTop, white_idx * whiteW + whiteW / 3, keyTop + 80};
                    FillRect(memDC, &r, activeKeys[i] ? activeBlack : black);
                    SetTextColor(memDC, activeKeys[i] ? RGB(255, 255, 255) : RGB(148, 163, 184));
                    char text[2] = {binds[i], 0};
                    TextOutA(memDC, white_idx * whiteW - 4, keyTop + 55, text, 1);
                } else {
                    white_idx++;
                }
            }

            DeleteObject(white); DeleteObject(activeWhite);
            DeleteObject(activeBlack); DeleteObject(black);

            if (showHelp) {
                HBRUSH helpBg = CreateSolidBrush(RGB(15, 23, 42));
                RECT helpRc = {W/2 - 150, H/2 - 100, W/2 + 150, H/2 + 120};
                FillRect(memDC, &helpRc, helpBg);
                DeleteObject(helpBg);
                
                HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                HPEN oldP = (HPEN)SelectObject(memDC, borderPen);
                HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HBRUSH oldB = (HBRUSH)SelectObject(memDC, nullBrush);
                Rectangle(memDC, helpRc.left, helpRc.top, helpRc.right, helpRc.bottom);
                SelectObject(memDC, oldP);
                SelectObject(memDC, oldB);
                DeleteObject(borderPen);
                
                SetTextColor(memDC, RGB(56, 189, 248));
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 20, "HELP / INSTRUCTIONS", 19);
                SetTextColor(memDC, RGB(241, 245, 249));
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 50, "A-K: Play Piano Keys", 20);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 70, "Arrows: Octave / Instrument", 27);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 90, "Z / X: Record / Play", 20);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 110, "P: Play/Stop Sequencer", 22);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 130, "E: Export WAV", 13);
                SetTextColor(memDC, RGB(244, 63, 94));
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 170, "Press 'H' to close", 18);
            }

            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBm);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hMidi) {
                midiOutReset(hMidi);
                midiOutClose(hMidi);
                hMidi = NULL;
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KAudioApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    RECT rc = {0, 0, W, H};
    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    AdjustWindowRect(&rc, style, FALSE);

    HWND hwnd = CreateWindowEx(0, "KAudioApp", "KAudio Pro Workstation", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
