#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define W 400
#define H 200

HMIDIOUT hMidi = NULL;

const char* notes[] = { "C", "D", "E", "F", "G", "A", "B", "C2" };
int pitches[] = { 60, 62, 64, 65, 67, 69, 71, 72 };
HWND hBtns[8];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            midiOutOpen(&hMidi, (UINT)-1, 0, 0, CALLBACK_NULL);
            if (hMidi) {
                // Select acoustic grand piano
                midiOutShortMsg(hMidi, 0x000000C0);
            }
            
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            int btnW = W / 8;
            for (int i = 0; i < 8; i++) {
                hBtns[i] = CreateWindowEx(0, "BUTTON", notes[i], WS_CHILD | WS_VISIBLE, i * btnW, 0, btnW, H - 40, hwnd, (HMENU)(i + 1), NULL, NULL);
                SendMessage(hBtns[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id >= 1 && id <= 8) {
                if (hMidi) {
                    int pitch = pitches[id - 1];
                    // Note on, channel 0, velocity 100
                    DWORD msg = 0x00640090 | (pitch << 8);
                    midiOutShortMsg(hMidi, msg);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            int btnW = nw / 8;
            for (int i = 0; i < 8; i++) {
                MoveWindow(hBtns[i], i * btnW, 0, btnW, nh, TRUE);
            }
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
    wc.lpszClassName = "KSoundApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSoundApp", "KSound", WS_OVERLAPPEDWINDOW,
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
