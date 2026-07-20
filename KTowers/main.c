#include <windows.h>
#include <stdio.h>

#define MAX_DISCS 8
#define NUM_PEGS 3

int pegs[NUM_PEGS][MAX_DISCS];
int counts[NUM_PEGS];
int numDiscs = 4;
int selectedPeg = -1;
int moves = 0;
int won = 0;

void InitGame() {
    for (int i = 0; i < NUM_PEGS; i++) {
        counts[i] = 0;
    }
    for (int i = numDiscs; i >= 1; i--) {
        pegs[0][counts[0]++] = i;
    }
    selectedPeg = -1;
    moves = 0;
    won = 0;
}

void HandleClick(int pegIndex, HWND hwnd) {
    if (won) {
        InitGame();
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    if (pegIndex < 0 || pegIndex >= NUM_PEGS) return;

    if (selectedPeg == -1) {
        if (counts[pegIndex] > 0) {
            selectedPeg = pegIndex;
        }
    } else if (selectedPeg == pegIndex) {
        selectedPeg = -1;
    } else {
        int fromTop = pegs[selectedPeg][counts[selectedPeg] - 1];
        if (counts[pegIndex] == 0 || pegs[pegIndex][counts[pegIndex] - 1] > fromTop) {
            counts[selectedPeg]--;
            pegs[pegIndex][counts[pegIndex]++] = fromTop;
            moves++;
            selectedPeg = -1;
            if (counts[NUM_PEGS - 1] == numDiscs) {
                won = 1;
            }
        } else {
            selectedPeg = -1;
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            InitGame();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int pegWidth = width / NUM_PEGS;
            int pegIndex = x / pegWidth;
            if (pegIndex >= 0 && pegIndex < NUM_PEGS) {
                HandleClick(pegIndex, hwnd);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            
            // Draw background
            HBRUSH hBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &clientRect, hBg);
            DeleteObject(hBg);

            // Draw status text
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            char status[128];
            if (won) {
                sprintf(status, "You won in %d moves! Click to restart.", moves);
            } else {
                sprintf(status, "Moves: %d", moves);
            }
            TextOut(hdc, 20, 20, status, strlen(status));

            // Draw pegs
            int pegWidth = width / NUM_PEGS;
            int baseHeight = height - 50;
            int pegHeight = 200;
            
            HBRUSH hPoleBrush = CreateSolidBrush(RGB(200, 200, 200));
            HBRUSH hBaseBrush = CreateSolidBrush(RGB(100, 100, 100));
            HBRUSH hSelectedBrush = CreateSolidBrush(RGB(220, 240, 255));

            COLORREF discColors[8] = {
                RGB(255, 59, 48), RGB(255, 149, 0), RGB(255, 204, 0), RGB(76, 217, 100),
                RGB(90, 200, 250), RGB(0, 122, 255), RGB(88, 86, 214), RGB(255, 45, 85)
            };

            for (int i = 0; i < NUM_PEGS; i++) {
                int cx = i * pegWidth + pegWidth / 2;
                
                // Draw selection highlight
                if (selectedPeg == i) {
                    RECT selRect = { i * pegWidth + 10, baseHeight - pegHeight - 20, (i + 1) * pegWidth - 10, baseHeight + 10 };
                    FillRect(hdc, &selRect, hSelectedBrush);
                }

                // Draw pole
                RECT poleRect = { cx - 6, baseHeight - pegHeight, cx + 6, baseHeight };
                FillRect(hdc, &poleRect, hPoleBrush);

                // Draw base
                RECT bRect = { cx - 70, baseHeight, cx + 70, baseHeight + 8 };
                FillRect(hdc, &bRect, hBaseBrush);

                // Draw discs
                for (int j = 0; j < counts[i]; j++) {
                    int size = pegs[i][j];
                    int dWidth = 40 + (size * 80) / 8;
                    int dHeight = 24;
                    int dy = baseHeight - (j + 1) * dHeight;
                    
                    RECT dRect = { cx - dWidth / 2, dy, cx + dWidth / 2, dy + dHeight - 2 };
                    HBRUSH hDiscBrush = CreateSolidBrush(discColors[(size - 1) % 8]);
                    FillRect(hdc, &dRect, hDiscBrush);
                    DeleteObject(hDiscBrush);
                    
                    // Border
                    HBRUSH hBorder = CreateSolidBrush(RGB(0, 0, 0));
                    FrameRect(hdc, &dRect, hBorder);
                    DeleteObject(hBorder);
                }
            }
            
            DeleteObject(hPoleBrush);
            DeleteObject(hBaseBrush);
            DeleteObject(hSelectedBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KTowersClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KTowers", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
