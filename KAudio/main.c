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
int lastSeqNote = -1;
int showHelp = 0; // Show welcome banner without blocking full screen

// Status Banner Notification State
char statusMsg[128] = "Welcome to KAudio Pro! Press [F1] for Help";
DWORD statusMsgExpire = 0;

void SetStatus(HWND hwnd, const char* msg, DWORD durationMs) {
    lstrcpynA(statusMsg, msg, sizeof(statusMsg));
    statusMsgExpire = GetTickCount() + durationMs;
    if (hwnd) InvalidateRect(hwnd, NULL, FALSE);
}

// DSP Effects & Visualizer State
int delayEnabled = 1;
int delayTimeMs = 250;
int delayFeedback = 45; // Percent
int driveEnabled = 0;
int visMode = 0; // 0 = Waveform Oscilloscope, 1 = FFT Spectrum Analyzer
float spectrumBars[32] = {0};

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

// Generate PCM WAV Audio File Export with DSP Delay & Overdrive Processing
void ExportWavFile(HWND hwnd) {
    DWORD sampleRate = 44100;
    DWORD durationSec = 3;
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
    if (hFile == INVALID_HANDLE_VALUE) {
        SetStatus(hwnd, "Error: Failed to create kaudio_export.wav", 3000);
        return;
    }

    DWORD written = 0;
    WriteFile(hFile, &hdr, sizeof(WavHeader), &written, NULL);

    // Delay ring buffer
    #define MAX_DLY_SAMPLES 44100
    static short dlyBuffer[MAX_DLY_SAMPLES];
    for (int d = 0; d < MAX_DLY_SAMPLES; d++) dlyBuffer[d] = 0;
    int dlySamples = (sampleRate * delayTimeMs) / 1000;
    if (dlySamples <= 0 || dlySamples >= MAX_DLY_SAMPLES) dlySamples = 11025;
    int dlyPos = 0;
    float fbFactor = (float)delayFeedback / 100.0f;

    // Synthesize audio with DSP Effects
    short sampleBuf[1024];
    DWORD bufIdx = 0;
    double freqsMap[13] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88, 523.25};

    for (DWORD i = 0; i < totalSamples; i++) {
        double t = (double)i / (double)sampleRate;
        int step = (int)(t / 0.125) % 16;
        double freq = 0.0;
        if (seqPattern[step]) {
            int nIdx = step % NUM_KEYS;
            freq = freqsMap[nIdx];
        } else {
            // Arpeggio fallback
            freq = (MyFmod(t, 0.4) > 0.2) ? 329.63 : 261.63;
        }

        double rawVal = (MyFmod(t * freq, 1.0) > 0.5) ? 0.35 : -0.35;

        // Apply Overdrive / Soft Clipping if enabled
        if (driveEnabled) {
            rawVal *= 2.5;
            if (rawVal > 0.75) rawVal = 0.75;
            else if (rawVal < -0.75) rawVal = -0.75;
        }

        // Apply Delay / Echo DSP
        if (delayEnabled) {
            short delayedSample = dlyBuffer[dlyPos];
            double wetVal = (double)delayedSample / 32767.0;
            double outVal = rawVal + wetVal * 0.4;
            if (outVal > 0.95) outVal = 0.95;
            if (outVal < -0.95) outVal = -0.95;

            short feedSample = (short)((rawVal + wetVal * fbFactor) * 32767.0);
            dlyBuffer[dlyPos] = feedSample;
            dlyPos = (dlyPos + 1) % dlySamples;
            rawVal = outVal;
        }

        sampleBuf[bufIdx++] = (short)(rawVal * 32767.0);
        if (bufIdx == 1024) {
            WriteFile(hFile, sampleBuf, bufIdx * sizeof(short), &written, NULL);
            bufIdx = 0;
        }
    }
    if (bufIdx > 0) {
        WriteFile(hFile, sampleBuf, bufIdx * sizeof(short), &written, NULL);
    }
    CloseHandle(hFile);
    SetStatus(hwnd, "WAV Export with DSP Effects Saved to kaudio_export.wav! [E]", 3500);
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
            SetStatus(hwnd, "Welcome to KAudio Pro! Press [F1] for Help & Shortcuts", 4500);
            break;

        case WM_KILLFOCUS:
        case WM_ACTIVATE:
            if (msg == WM_KILLFOCUS || (msg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)) {
                if (mouseActiveKey != -1) {
                    if (GetCapture() == hwnd) ReleaseCapture();
                    mouseActiveKey = -1;
                }
                for (int i = 0; i < NUM_KEYS; i++) {
                    if (activeKeys[i]) {
                        activeKeys[i] = 0;
                        PlayNote(i, 0);
                    }
                }
                if (lastSeqNote != -1) {
                    PlayNote(lastSeqNote, 0);
                    lastSeqNote = -1;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;

        case WM_TIMER:
            visOffset = (visOffset + 4) % 360;

            // Update FFT Spectrum Bins
            for (int b = 0; b < 32; b++) {
                spectrumBars[b] *= 0.82f;
                if (spectrumBars[b] < 0.04f) spectrumBars[b] = 0.04f + (float)(b % 3) * 0.015f;
            }
            for (int i = 0; i < NUM_KEYS; i++) {
                if (activeKeys[i]) {
                    int bin = (i * 28) / NUM_KEYS + 2;
                    spectrumBars[bin] = 0.95f;
                    if (bin > 0 && spectrumBars[bin - 1] < 0.65f) spectrumBars[bin - 1] = 0.65f;
                    if (bin < 31 && spectrumBars[bin + 1] < 0.65f) spectrumBars[bin + 1] = 0.65f;
                    int h1 = (bin * 2) % 32;
                    if (spectrumBars[h1] < 0.45f) spectrumBars[h1] = 0.45f;
                }
            }
            if (seqPlaying && lastSeqNote != -1) {
                int bin = (lastSeqNote * 28) / NUM_KEYS + 2;
                spectrumBars[bin] = 0.95f;
            }

            // Sequencer Step Timer (120 BPM -> step every 125ms)
            if (seqPlaying) {
                DWORD now = GetTickCount();
                if (now - lastStepTime >= 125) {
                    lastStepTime = now;
                    if (lastSeqNote != -1) {
                        PlayNote(lastSeqNote, 0);
                        lastSeqNote = -1;
                    }
                    currentStep = (currentStep + 1) % 16;
                    if (seqPattern[currentStep]) {
                        lastSeqNote = currentStep % NUM_KEYS;
                        PlayNote(lastSeqNote, 1);
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
                    SetStatus(hwnd, "Playback finished", 2000);
                }
                if (changed) InvalidateRect(hwnd, NULL, FALSE);
            }
            break;

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);

            // Check Help Dismiss Click
            if (showHelp) {
                RECT helpRc = {W/2 - 200, H/2 - 150, W/2 + 200, H/2 + 160};
                if (x >= helpRc.left && x <= helpRc.right && y >= helpRc.top && y <= helpRc.bottom) {
                    showHelp = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            // Top Help Button [F1] (x: W-130 to W-10, y: 4 to 28)
            if (x >= W - 130 && x <= W - 10 && y >= 4 && y <= 28) {
                showHelp = !showHelp;
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            // Check Preset & DSP Control Buttons (y: 35-65)
            if (y >= 35 && y <= 65) {
                if (x >= 10 && x <= 70) { PlaySoundFX(1); SetStatus(hwnd, "🦘 [1] Jump FX Triggered", 1600); }
                else if (x >= 75 && x <= 135) { PlaySoundFX(2); SetStatus(hwnd, "🔫 [2] Laser FX Triggered", 1600); }
                else if (x >= 140 && x <= 210) { PlaySoundFX(3); SetStatus(hwnd, "💥 [3] Explosion FX Triggered", 1600); }
                else if (x >= 215 && x <= 265) { PlaySoundFX(4); SetStatus(hwnd, "🪙 [4] Coin FX Triggered", 1600); }
                else if (x >= 270 && x <= 340) { PlaySoundFX(5); SetStatus(hwnd, "⚡ [5] Powerup FX Triggered", 1600); }
                else if (x >= 350 && x <= 460) { ExportWavFile(hwnd); }
                else if (x >= 470 && x <= 580) { delayEnabled = !delayEnabled; SetStatus(hwnd, delayEnabled ? "Delay / Echo Enabled [L]" : "Delay / Echo Disabled [L]", 1800); }
                else if (x >= 590 && x <= 700) { driveEnabled = !driveEnabled; SetStatus(hwnd, driveEnabled ? "Overdrive Enabled [O]" : "Overdrive Disabled [O]", 1800); }
                else if (x >= 710 && x <= 860) { visMode = !visMode; SetStatus(hwnd, visMode ? "Visualizer: FFT Spectrum [V]" : "Visualizer: Oscilloscope [V]", 1800); }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            // Check Sequencer Clear Button (x: 95-140, y: 175-198)
            if (y >= 175 && y <= 198 && x >= 95 && x <= 140) {
                for (int s = 0; s < 16; s++) seqPattern[s] = 0;
                SetStatus(hwnd, "🗑 Sequencer Pattern Cleared [C]", 2000);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            // Check 16-Step Sequencer Click (y: 175-200, x: 150 to 598)
            if (y >= 175 && y <= 200 && x >= 150 && x <= 598) {
                int stepIdx = (x - 150) / 28;
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

        case WM_MOUSEMOVE: {
            if (mouseActiveKey != -1 && (wParam & MK_LBUTTON)) {
                int x = (short)LOWORD(lParam);
                int y = (short)HIWORD(lParam);
                int k = GetKeyAtPoint(x, y);
                if (k != mouseActiveKey) {
                    if (mouseActiveKey >= 0 && mouseActiveKey < NUM_KEYS && activeKeys[mouseActiveKey]) {
                        activeKeys[mouseActiveKey] = 0;
                        PlayNote(mouseActiveKey, 0);
                        if (isRecording && numEvents < 4000) {
                            recordedEvents[numEvents].keyIndex = mouseActiveKey;
                            recordedEvents[numEvents].time = GetTickCount() - recordingStartTime;
                            recordedEvents[numEvents].type = 0;
                            numEvents++;
                        }
                    }
                    mouseActiveKey = k;
                    if (k != -1 && !activeKeys[k]) {
                        activeKeys[k] = 1;
                        PlayNote(k, 1);
                        if (isRecording && numEvents < 4000) {
                            recordedEvents[numEvents].keyIndex = k;
                            recordedEvents[numEvents].time = GetTickCount() - recordingStartTime;
                            recordedEvents[numEvents].type = 1;
                            numEvents++;
                        }
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
            if (wParam == VK_ESCAPE && !isRepeat) {
                if (showHelp) {
                    showHelp = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            // Sound FX Presets 1-5
            if (wParam >= '1' && wParam <= '5' && !isRepeat) {
                int p = (int)(wParam - '0');
                PlaySoundFX(p);
                const char* pNames[] = {"", "Jump FX", "Laser FX", "Explosion FX", "Coin FX", "Powerup FX"};
                char pmsg[64];
                wsprintfA(pmsg, "Preset [%d] %s Triggered", p, pNames[p]);
                SetStatus(hwnd, pmsg, 1600);
                break;
            }
            // Sequencer Clear
            if (wParam == 'C' && !isRepeat) {
                for (int s = 0; s < 16; s++) seqPattern[s] = 0;
                SetStatus(hwnd, "🗑 Sequencer Pattern Cleared [C]", 2000);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'P' && !isRepeat) {
                seqPlaying = !seqPlaying;
                lastStepTime = GetTickCount();
                if (!seqPlaying && lastSeqNote != -1) {
                    PlayNote(lastSeqNote, 0);
                    lastSeqNote = -1;
                }
                SetStatus(hwnd, seqPlaying ? "▶ Sequencer Started [P]" : "⏸ Sequencer Paused [P]", 1800);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'E' && !isRepeat) {
                ExportWavFile(hwnd);
                break;
            }
            if (wParam == 'V' && !isRepeat) {
                visMode = !visMode;
                SetStatus(hwnd, visMode ? "Visualizer: FFT Spectrum [V]" : "Visualizer: Oscilloscope [V]", 1800);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'L' && !isRepeat) {
                delayEnabled = !delayEnabled;
                SetStatus(hwnd, delayEnabled ? "🔄 Delay / Echo Enabled [L]" : "Delay / Echo Disabled [L]", 1800);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'O' && !isRepeat) {
                driveEnabled = !driveEnabled;
                SetStatus(hwnd, driveEnabled ? "🔥 Overdrive Enabled [O]" : "Overdrive Disabled [O]", 1800);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if ((wParam == 'H' || wParam == VK_F1 || wParam == VK_OEM_2) && !isRepeat) {
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
                    SetStatus(hwnd, "🔴 Live Recording Session Started... [Z]", 2000);
                } else {
                    char rmsg[64];
                    wsprintfA(rmsg, "⏺️ Recording Saved (%d events) [Press X to Play]", numEvents);
                    SetStatus(hwnd, rmsg, 2500);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == 'X' && !isRepeat) {
                if (isRecording) isRecording = 0;
                if (numEvents == 0) {
                    SetStatus(hwnd, "No performance recorded yet. Press [Z] to record!", 2500);
                    break;
                }
                isPlaying = !isPlaying;
                if (isPlaying) {
                    playbackStartTime = GetTickCount();
                    playbackIndex = 0;
                    SetStatus(hwnd, "▶ Playing recorded performance... [X]", 2000);
                } else {
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) { PlayNote(i, 0); activeKeys[i] = 0; }
                    SetStatus(hwnd, "⏹️ Playback stopped", 1600);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_UP && !isRepeat) {
                instrument = (instrument + 1) % 128;
                if (hMidi) midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                char imsg[64];
                wsprintfA(imsg, "MIDI Instrument #%d", instrument);
                SetStatus(hwnd, imsg, 1400);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_DOWN && !isRepeat) {
                instrument = (instrument + 127) % 128;
                if (hMidi) midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                char imsg[64];
                wsprintfA(imsg, "MIDI Instrument #%d", instrument);
                SetStatus(hwnd, imsg, 1400);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_LEFT && !isRepeat) {
                if (octaveShift > -4) {
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) PlayNote(i, 0);
                    octaveShift--;
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) PlayNote(i, 1);
                    char omsg[64];
                    wsprintfA(omsg, "Octave Shift: %+d", octaveShift);
                    SetStatus(hwnd, omsg, 1400);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                break;
            }
            if (wParam == VK_RIGHT && !isRepeat) {
                if (octaveShift < 4) {
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) PlayNote(i, 0);
                    octaveShift++;
                    for (int i = 0; i < NUM_KEYS; i++) if (activeKeys[i]) PlayNote(i, 1);
                    char omsg[64];
                    wsprintfA(omsg, "Octave Shift: %+d", octaveShift);
                    SetStatus(hwnd, omsg, 1400);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
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
            char title[160];
            wsprintfA(title, "KAudio Pro Workstation | Inst: %d | Oct: %+d | Dly: %s | Drv: %s | Vis: %s | P: Seq [%s]",
                      instrument, octaveShift, delayEnabled ? "ON" : "OFF", driveEnabled ? "ON" : "OFF",
                      visMode ? "FFT Spectrum" : "Oscilloscope", seqPlaying ? "PLAY" : "STOP");
            TextOutA(memDC, 10, 8, title, lstrlenA(title));

            // Top Help Button [F1]
            HBRUSH helpBtnBrush = CreateSolidBrush(RGB(30, 41, 59));
            RECT helpBtnRc = {W - 130, 4, W - 10, 28};
            FillRect(memDC, &helpBtnRc, helpBtnBrush);
            DeleteObject(helpBtnBrush);
            SetTextColor(memDC, RGB(251, 191, 36));
            TextOutA(memDC, W - 120, 7, "? Help [F1]", 11);

            // Draw Sound FX & DSP Preset Buttons (y: 35-65)
            HBRUSH btnBrush = CreateSolidBrush(RGB(30, 41, 59));
            HBRUSH exportBrush = CreateSolidBrush(RGB(14, 165, 233));
            HBRUSH dlyBrush = CreateSolidBrush(delayEnabled ? RGB(2, 132, 199) : RGB(30, 41, 59));
            HBRUSH drvBrush = CreateSolidBrush(driveEnabled ? RGB(225, 29, 72) : RGB(30, 41, 59));
            HBRUSH visBrush = CreateSolidBrush(visMode ? RGB(99, 102, 241) : RGB(30, 41, 59));

            SetTextColor(memDC, RGB(241, 245, 249));

            RECT r1 = {10, 35, 70, 65}; FillRect(memDC, &r1, btnBrush); TextOutA(memDC, 14, 42, "[1] Jump", 8);
            RECT r2 = {75, 35, 135, 65}; FillRect(memDC, &r2, btnBrush); TextOutA(memDC, 79, 42, "[2] Laser", 9);
            RECT r3 = {140, 35, 210, 65}; FillRect(memDC, &r3, btnBrush); TextOutA(memDC, 144, 42, "[3] Explode", 11);
            RECT r4 = {215, 35, 265, 65}; FillRect(memDC, &r4, btnBrush); TextOutA(memDC, 219, 42, "[4] Coin", 8);
            RECT r5 = {270, 35, 340, 65}; FillRect(memDC, &r5, btnBrush); TextOutA(memDC, 274, 42, "[5] Power", 9);
            RECT rExp = {350, 35, 460, 65}; FillRect(memDC, &rExp, exportBrush); TextOutA(memDC, 356, 42, "💾 Export WAV [E]", 18);

            // DSP FX Toggle Buttons
            RECT rDly = {470, 35, 580, 65}; FillRect(memDC, &rDly, dlyBrush);
            char dlyText[32]; wsprintfA(dlyText, "Delay: %s [L]", delayEnabled ? "ON" : "OFF");
            TextOutA(memDC, 478, 42, dlyText, lstrlenA(dlyText));

            RECT rDrv = {590, 35, 700, 65}; FillRect(memDC, &rDrv, drvBrush);
            char drvText[32]; wsprintfA(drvText, "Drive: %s [O]", driveEnabled ? "ON" : "OFF");
            TextOutA(memDC, 598, 42, drvText, lstrlenA(drvText));

            RECT rVis = {710, 35, 860, 65}; FillRect(memDC, &rVis, visBrush);
            char visText[32]; wsprintfA(visText, "Vis: %s [V]", visMode ? "Spectrum" : "Scope");
            TextOutA(memDC, 718, 42, visText, lstrlenA(visText));

            DeleteObject(btnBrush);
            DeleteObject(exportBrush);
            DeleteObject(dlyBrush);
            DeleteObject(drvBrush);
            DeleteObject(visBrush);

            // Draw Visualizer Canvas (y: 75-160)
            HBRUSH visBg = CreateSolidBrush(RGB(2, 6, 23));
            RECT visRc = {10, 75, W - 10, 160};
            FillRect(memDC, &visRc, visBg);
            DeleteObject(visBg);

            if (visMode == 0) {
                // Waveform Oscilloscope
                HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                HPEN oldPen = (HPEN)SelectObject(memDC, wavePen);
                int midY = 117;
                MoveToEx(memDC, 10, midY, NULL);
                for (int x = 10; x < W - 10; x += 4) {
                    double amp = (driveEnabled ? 32.0 : 25.0);
                    int waveY = midY + (int)(amp * MySin((x + visOffset) * PI / 45.0));
                    LineTo(memDC, x, waveY);
                }
                SelectObject(memDC, oldPen);
                DeleteObject(wavePen);
            } else {
                // 32-Band FFT Frequency Spectrum Analyzer
                int totalW = (W - 20);
                int barSlotW = totalW / 32;
                int barW = barSlotW - 4;
                if (barW < 4) barW = 4;

                for (int b = 0; b < 32; b++) {
                    int barH = (int)(spectrumBars[b] * 74.0f);
                    if (barH < 3) barH = 3;
                    if (barH > 80) barH = 80;

                    int bx = 12 + b * barSlotW;
                    int by = 158 - barH;

                    COLORREF barColor;
                    if (b < 10) barColor = RGB(56, 189, 248);       // Cyan (Bass)
                    else if (b < 20) barColor = RGB(129, 140, 248); // Indigo/Purple (Mids)
                    else barColor = RGB(244, 63, 94);               // Rose/Red (Highs)

                    HBRUSH barBrush = CreateSolidBrush(barColor);
                    RECT barRc = {bx, by, bx + barW, 158};
                    FillRect(memDC, &barRc, barBrush);
                    DeleteObject(barBrush);

                    // Peak cap line
                    HPEN capPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HPEN oldP = (HPEN)SelectObject(memDC, capPen);
                    MoveToEx(memDC, bx, by, NULL);
                    LineTo(memDC, bx + barW, by);
                    SelectObject(memDC, oldP);
                    DeleteObject(capPen);
                }
            }

            // Draw 16-Step Pattern Sequencer Row (y: 175-200)
            SetTextColor(memDC, RGB(148, 163, 184));
            TextOutA(memDC, 10, 180, "Sequencer [P]:", 14);

            HBRUSH clrBrush = CreateSolidBrush(RGB(225, 29, 72));
            RECT clrRc = {95, 176, 142, 198};
            FillRect(memDC, &clrRc, clrBrush);
            DeleteObject(clrBrush);
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, 99, 180, "Clr [C]", 7);

            HBRUSH stepOff = CreateSolidBrush(RGB(51, 65, 85));
            HBRUSH stepOn = CreateSolidBrush(RGB(14, 165, 233));
            HBRUSH stepCurr = CreateSolidBrush(RGB(244, 63, 94));

            for (int st = 0; st < 16; st++) {
                RECT stRc = {150 + st * 28, 175, 174 + st * 28, 198};
                if (st == currentStep && seqPlaying) {
                    FillRect(memDC, &stRc, stepCurr);
                } else {
                    FillRect(memDC, &stRc, seqPattern[st] ? stepOn : stepOff);
                }
            }
            DeleteObject(stepOff); DeleteObject(stepOn); DeleteObject(stepCurr);

            // Draw Status Banner Notification if active
            if (GetTickCount() < statusMsgExpire) {
                HBRUSH statBg = CreateSolidBrush(RGB(30, 41, 59));
                RECT statRc = {610, 175, W - 10, 198};
                FillRect(memDC, &statRc, statBg);
                DeleteObject(statBg);
                SetTextColor(memDC, RGB(56, 189, 248));
                TextOutA(memDC, 616, 180, statusMsg, lstrlenA(statusMsg));
            }

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
                RECT helpRc = {W/2 - 200, H/2 - 150, W/2 + 200, H/2 + 160};
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
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 15, "🎹 KAudio Pro Help & Shortcuts Reference", 41);
                SetTextColor(memDC, RGB(241, 245, 249));
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 45, "A-K: Play Chromatic Piano Notes", 31);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 68, "1 - 5: Sound FX Presets (Jump, Laser, Explode, Coin, Power)", 59);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 91, "Arrows Up/Dn: MIDI Instrument Select (0-127)", 44);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 114, "Arrows L/R: Transpose Octave (-4 to +4)", 39);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 137, "Z / X: Live Performance Record / Playback", 41);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 160, "P / C: Start-Stop Sequencer / Clear Grid Pattern", 48);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 183, "L / O: Toggle Stereo Delay / Overdrive Saturation", 49);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 206, "V: Toggle Oscilloscope / FFT Spectrum Analyzer", 46);
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 229, "E: Export DSP-Mastered WAV Audio File", 37);
                SetTextColor(memDC, RGB(244, 63, 94));
                TextOutA(memDC, helpRc.left + 20, helpRc.top + 265, "Press F1, 'H', [Esc], or Click to close guide", 45);
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

    HWND hwnd = CreateWindowEx(0, "KAudioApp", "KAudio Pro Workstation - Press [F1] for Help", style,
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
