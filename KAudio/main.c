#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define W 400
#define H 200
#define NUM_KEYS 13

HMIDIOUT hMidi = NULL;
int activeKeys[NUM_KEYS] = {0};
int notes[NUM_KEYS] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72};
char binds[NUM_KEYS] = {'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K'};
int is_black[NUM_KEYS] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0};
int instrument = 0;

void PlayNote(int index, int on) {
    if (hMidi) {
        DWORD msg = on ? (0x007F0090 | (notes[index] << 8)) : (0x00000080 | (notes[index] << 8));
        midiOutShortMsg(hMidi, msg);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            midiOutOpen(&hMidi, (UINT)-1, 0, 0, CALLBACK_NULL);
            if (hMidi) {
                // Select instrument (0 = Acoustic Grand Piano)
                midiOutShortMsg(hMidi, 0x000000C0);
            }
            break;
        case WM_KEYDOWN: {
            if (wParam == VK_UP) {
                instrument = (instrument + 1) % 128;
                midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (wParam == VK_DOWN) {
                instrument = (instrument + 127) % 128;
                midiOutShortMsg(hMidi, 0x000000C0 | (instrument << 8));
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            for (int i = 0; i < NUM_KEYS; i++) {
                if (wParam == binds[i]) {
                    if (!activeKeys[i]) {
                        activeKeys[i] = 1;
                        PlayNote(i, 1);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                }
            }
            break;
        }
        case WM_KEYUP: {
            for (int i = 0; i < NUM_KEYS; i++) {
                if (wParam == binds[i]) {
                    if (activeKeys[i]) {
                        activeKeys[i] = 0;
                        PlayNote(i, 0);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                }
            }
            break;
        }
        case WM_KILLFOCUS: {
            for (int i = 0; i < NUM_KEYS; i++) {
                if (activeKeys[i]) {
                    activeKeys[i] = 0;
                    PlayNote(i, 0);
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(15, 23, 42)); // Slate 900
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            HBRUSH white = CreateSolidBrush(RGB(226, 232, 240)); // Slate 200
            HBRUSH activeWhite = CreateSolidBrush(RGB(14, 165, 233)); // Cyan 500
            HBRUSH activeBlack = CreateSolidBrush(RGB(56, 189, 248)); // Sky 400
            HBRUSH black = CreateSolidBrush(RGB(30, 41, 59)); // Slate 800
            
            int num_white = 8;
            int whiteW = W / num_white;
            int white_idx = 0;
            for (int i = 0; i < NUM_KEYS; i++) {
                if (!is_black[i]) {
                    RECT r = {white_idx * whiteW + 2, 20, (white_idx + 1) * whiteW - 2, H - 20};
                    FillRect(memDC, &r, activeKeys[i] ? activeWhite : white);
                    SetBkMode(memDC, TRANSPARENT);
                    SetTextColor(memDC, RGB(71, 85, 105)); // Slate 600
                    if (activeKeys[i]) SetTextColor(memDC, RGB(255, 255, 255));
                    char text[2] = {binds[i], 0};
                    TextOutA(memDC, white_idx * whiteW + whiteW / 2 - 4, H - 40, text, 1);
                    white_idx++;
                }
            }
            
            white_idx = 0;
            for (int i = 0; i < NUM_KEYS; i++) {
                if (is_black[i]) {
                    RECT r = {white_idx * whiteW - whiteW / 3, 20, white_idx * whiteW + whiteW / 3, H / 2};
                    FillRect(memDC, &r, activeKeys[i] ? activeBlack : black);
                    SetBkMode(memDC, TRANSPARENT);
                    SetTextColor(memDC, RGB(148, 163, 184)); // Slate 400
                    if (activeKeys[i]) SetTextColor(memDC, RGB(255, 255, 255));
                    char text[2] = {binds[i], 0};
                    TextOutA(memDC, white_idx * whiteW - 4, H / 2 - 20, text, 1);
                } else {
                    white_idx++;
                }
            }
            
            SetTextColor(memDC, RGB(255, 255, 255));
            char instText[64];
            wsprintfA(instText, "Instrument: %d (Up/Down)", instrument);
            TextOutA(memDC, 10, 0, instText, lstrlenA(instText));
            
            DeleteObject(white);
            DeleteObject(activeWhite);
            DeleteObject(activeBlack);
            DeleteObject(black);
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (hMidi) {
                midiOutReset(hMidi);
                midiOutClose(hMidi);
            }
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
    wc.lpszClassName = "KAudioApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KAudioApp", "KAudio", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
