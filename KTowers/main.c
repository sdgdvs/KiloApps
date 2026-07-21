#include <windows.h>
#include <stdio.h>

int numDiscs = 4;
#define MAX_DISCS 8

int pegs[3][MAX_DISCS];
int pegCounts[3] = {0, 0, 0};
int selectedPeg = -1;
int moves = 0;
BOOL won = FALSE;

int historyFrom[4096];
int historyTo[4096];
int historyCount = 0;

int elapsedSeconds = 0;
BOOL timerRunning = FALSE;

int bestTime[MAX_DISCS + 1] = {0};
int fewestMoves[MAX_DISCS + 1] = {0};
BOOL hasStats[MAX_DISCS + 1] = {FALSE};

void LoadStats() {
    FILE* fp = fopen("ktowers_stats.dat", "rb");
    if (fp) {
        fread(hasStats, sizeof(BOOL), MAX_DISCS + 1, fp);
        fread(bestTime, sizeof(int), MAX_DISCS + 1, fp);
        fread(fewestMoves, sizeof(int), MAX_DISCS + 1, fp);
        fclose(fp);
    }
}

void SaveStats() {
    FILE* fp = fopen("ktowers_stats.dat", "wb");
    if (fp) {
        fwrite(hasStats, sizeof(BOOL), MAX_DISCS + 1, fp);
        fwrite(bestTime, sizeof(int), MAX_DISCS + 1, fp);
        fwrite(fewestMoves, sizeof(int), MAX_DISCS + 1, fp);
        fclose(fp);
    }
}

COLORREF colors[] = {
    RGB(255, 59, 48),  // #FF3B30
    RGB(255, 149, 0),  // #FF9500
    RGB(255, 204, 0),  // #FFCC00
    RGB(76, 217, 100), // #4CD964
    RGB(90, 200, 250), // #5AC8FA
    RGB(0, 122, 255),  // #007AFF
    RGB(88, 86, 214),  // #5856D6
    RGB(255, 45, 85)   // #FF2D55
};

HWND hRestartBtn;
HWND hDiffLabel;
HWND hDiffMinusBtn;
HWND hDiffPlusBtn;
HWND hUndoBtn;
HWND hHintBtn;
HWND hAutoBtn;
BOOL autoSolving = FALSE;

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(INT_PTR)lpParam;
    if (type == 1) { // pickup
        Beep(800, 50);
    } else if (type == 2) { // drop
        Beep(600, 50);
    } else if (type == 3) { // error
        Beep(200, 150);
    } else if (type == 4) { // win
        Beep(440, 100);
        Beep(554, 100);
        Beep(659, 100);
        Beep(880, 200);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(INT_PTR)type, 0, NULL);
}

void SaveState() {
    FILE* fp = fopen("ktowers_save.dat", "wb");
    if (fp) {
        fwrite(&numDiscs, sizeof(int), 1, fp);
        fwrite(pegCounts, sizeof(int), 3, fp);
        for (int i = 0; i < 3; i++) {
            fwrite(pegs[i], sizeof(int), MAX_DISCS, fp);
        }
        fwrite(&moves, sizeof(int), 1, fp);
        fwrite(&won, sizeof(BOOL), 1, fp);
        fwrite(&historyCount, sizeof(int), 1, fp);
        fwrite(historyFrom, sizeof(int), historyCount, fp);
        fwrite(historyTo, sizeof(int), historyCount, fp);
        fwrite(&elapsedSeconds, sizeof(int), 1, fp);
        fclose(fp);
    }
}

BOOL LoadState() {
    FILE* fp = fopen("ktowers_save.dat", "rb");
    if (fp) {
        fread(&numDiscs, sizeof(int), 1, fp);
        fread(pegCounts, sizeof(int), 3, fp);
        for (int i = 0; i < 3; i++) {
            fread(pegs[i], sizeof(int), MAX_DISCS, fp);
        }
        fread(&moves, sizeof(int), 1, fp);
        fread(&won, sizeof(BOOL), 1, fp);
        if (fread(&historyCount, sizeof(int), 1, fp) == 1) {
            fread(historyFrom, sizeof(int), historyCount, fp);
            fread(historyTo, sizeof(int), historyCount, fp);
            if (fread(&elapsedSeconds, sizeof(int), 1, fp) != 1) {
                elapsedSeconds = 0;
            }
        } else {
            historyCount = 0;
            elapsedSeconds = 0;
        }
        fclose(fp);
        return TRUE;
    }
    return FALSE;
}

void InitGame(HWND hwnd) {
    if (autoSolving) {
        autoSolving = FALSE;
        KillTimer(hwnd, 2);
        SetWindowText(hAutoBtn, "Auto-Solve");
    }
    pegCounts[0] = 0;
    pegCounts[1] = 0;
    pegCounts[2] = 0;
    for (int i = numDiscs; i >= 1; i--) {
        pegs[0][pegCounts[0]++] = i;
    }
    selectedPeg = -1;
    moves = 0;
    won = FALSE;
    historyCount = 0;
    elapsedSeconds = 0;
    if (timerRunning) {
        KillTimer(hwnd, 1);
        timerRunning = FALSE;
    }
    SaveState();
    InvalidateRect(hwnd, NULL, TRUE);
}

BOOL CheckWin(HWND hwnd) {
    if (pegCounts[2] == numDiscs) {
        won = TRUE;
        if (timerRunning) {
            KillTimer(hwnd, 1);
            timerRunning = FALSE;
        }
        
        BOOL newBest = FALSE;
        if (!hasStats[numDiscs]) {
            hasStats[numDiscs] = TRUE;
            bestTime[numDiscs] = elapsedSeconds;
            fewestMoves[numDiscs] = moves;
            newBest = TRUE;
        } else {
            if (elapsedSeconds < bestTime[numDiscs] || 
               (elapsedSeconds == bestTime[numDiscs] && moves < fewestMoves[numDiscs])) {
                bestTime[numDiscs] = elapsedSeconds;
                fewestMoves[numDiscs] = moves;
                newBest = TRUE;
            }
        }
        if (newBest) {
            SaveStats();
        }

        PlaySoundEffect(4);
        InvalidateRect(hwnd, NULL, TRUE);
        return TRUE;
    }
    return FALSE;
}

int getNextOptimalMove(int d, int target, int* outFrom, int* outTo) {
    if (d == 0) return 0;
    int current = -1;
    for (int p = 0; p < 3; p++) {
        for (int i = 0; i < pegCounts[p]; i++) {
            if (pegs[p][i] == d) {
                current = p;
                break;
            }
        }
        if (current != -1) break;
    }
    if (current == target) {
        return getNextOptimalMove(d - 1, target, outFrom, outTo);
    } else {
        int aux = 3 - current - target;
        if (getNextOptimalMove(d - 1, aux, outFrom, outTo)) {
            return 1;
        }
        *outFrom = current;
        *outTo = target;
        return 1;
    }
}

int getHintMove(int* outFrom, int* outTo) {
    int target = 2;
    for (int d = numDiscs; d >= 1; d--) {
        int current = -1;
        for (int p = 0; p < 3; p++) {
            for (int i = 0; i < pegCounts[p]; i++) {
                if (pegs[p][i] == d) {
                    current = p;
                    break;
                }
            }
            if (current != -1) break;
        }
        if (current != target) {
            int aux = 3 - current - target;
            if (getNextOptimalMove(d - 1, aux, outFrom, outTo)) {
                return 1;
            }
            *outFrom = current;
            *outTo = target;
            return 1;
        }
    }
    return 0;
}

void PerformPegClick(HWND hwnd, int clickedPeg) {
    if (won) return;
    if (clickedPeg < 0 || clickedPeg > 2) return;

    if (selectedPeg == -1) {
        if (pegCounts[clickedPeg] > 0) {
            selectedPeg = clickedPeg;
            PlaySoundEffect(1);
            InvalidateRect(hwnd, NULL, TRUE);
        } else {
            PlaySoundEffect(3);
        }
    } else if (selectedPeg == clickedPeg) {
        selectedPeg = -1;
        PlaySoundEffect(2);
        InvalidateRect(hwnd, NULL, TRUE);
    } else {
        int fromTop = pegs[selectedPeg][pegCounts[selectedPeg] - 1];
        BOOL canMove = FALSE;
        if (pegCounts[clickedPeg] == 0) {
            canMove = TRUE;
        } else {
            int toTop = pegs[clickedPeg][pegCounts[clickedPeg] - 1];
            if (fromTop < toTop) {
                canMove = TRUE;
            }
        }

        if (canMove) {
            historyFrom[historyCount] = selectedPeg;
            historyTo[historyCount] = clickedPeg;
            historyCount++;
            pegCounts[selectedPeg]--;
            pegs[clickedPeg][pegCounts[clickedPeg]++] = fromTop;
            moves++;
            selectedPeg = -1;
            if (!timerRunning && !won) {
                timerRunning = TRUE;
                SetTimer(hwnd, 1, 1000, NULL);
            }
            if (!CheckWin(hwnd)) {
                PlaySoundEffect(2);
            }
            SaveState();
        } else {
            selectedPeg = -1;
            PlaySoundEffect(3);
        }
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void ApplyHint(HWND hwnd) {
    if (won) return;
    int from, to;
    if (getHintMove(&from, &to)) {
        selectedPeg = -1;
        PerformPegClick(hwnd, from);
        PerformPegClick(hwnd, to);
    }
}

void HandleClick(HWND hwnd, int x, int y) {
    if (won) return;

    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int pegWidth = width / 3;

    int clickedPeg = x / pegWidth;
    PerformPegClick(hwnd, clickedPeg);
}

void UndoMove(HWND hwnd) {
    if (historyCount == 0 || won) return;
    historyCount--;
    int from = historyFrom[historyCount];
    int to = historyTo[historyCount];
    
    int disc = pegs[to][pegCounts[to] - 1];
    pegCounts[to]--;
    pegs[from][pegCounts[from]++] = disc;
    moves--;
    selectedPeg = -1;
    
    PlaySoundEffect(2);
    SaveState();
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hRestartBtn = CreateWindow("BUTTON", "Restart Game",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     10, 10, 120, 30,
                                     hwnd, (HMENU) 1,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hDiffMinusBtn = CreateWindow("BUTTON", "-",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     140, 10, 30, 30,
                                     hwnd, (HMENU) 2,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hDiffLabel = CreateWindow("STATIC", "Discs: 4",
                                     WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_CENTER,
                                     180, 10, 80, 30,
                                     hwnd, (HMENU) 3,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hDiffPlusBtn = CreateWindow("BUTTON", "+",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     270, 10, 30, 30,
                                     hwnd, (HMENU) 4,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hUndoBtn = CreateWindow("BUTTON", "Undo",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     310, 10, 60, 30,
                                     hwnd, (HMENU) 5,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hHintBtn = CreateWindow("BUTTON", "Hint",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     380, 10, 60, 30,
                                     hwnd, (HMENU) 6,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hAutoBtn = CreateWindow("BUTTON", "Auto-Solve",
                                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                     450, 10, 100, 30,
                                     hwnd, (HMENU) 7,
                                     (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            LoadStats();
            if (LoadState()) {
                char buf[32];
                sprintf(buf, "Discs: %d", numDiscs);
                SetWindowText(hDiffLabel, buf);
                if (moves > 0 && !won && !timerRunning) {
                    timerRunning = TRUE;
                    SetTimer(hwnd, 1, 1000, NULL);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else {
                InitGame(hwnd);
            }
            break;
        case WM_TIMER:
            if (wParam == 1) {
                elapsedSeconds++;
                SaveState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 2) {
                if (won) {
                    autoSolving = FALSE;
                    KillTimer(hwnd, 2);
                    SetWindowText(hAutoBtn, "Auto-Solve");
                } else {
                    ApplyHint(hwnd);
                }
            }
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                InitGame(hwnd);
            } else if (LOWORD(wParam) == 2) {
                if (numDiscs > 3) {
                    numDiscs--;
                    char buf[32];
                    sprintf(buf, "Discs: %d", numDiscs);
                    SetWindowText(hDiffLabel, buf);
                    InitGame(hwnd);
                }
            } else if (LOWORD(wParam) == 4) {
                if (numDiscs < MAX_DISCS) {
                    numDiscs++;
                    char buf[32];
                    sprintf(buf, "Discs: %d", numDiscs);
                    SetWindowText(hDiffLabel, buf);
                    InitGame(hwnd);
                }
            } else if (LOWORD(wParam) == 5) {
                UndoMove(hwnd);
            } else if (LOWORD(wParam) == 6) {
                ApplyHint(hwnd);
            } else if (LOWORD(wParam) == 7) {
                if (autoSolving) {
                    autoSolving = FALSE;
                    KillTimer(hwnd, 2);
                    SetWindowText(hAutoBtn, "Auto-Solve");
                } else {
                    if (!won) {
                        autoSolving = TRUE;
                        SetTimer(hwnd, 2, 500, NULL);
                        SetWindowText(hAutoBtn, "Stop Auto");
                    }
                }
            }
            break;
        case WM_LBUTTONDOWN:
            HandleClick(hwnd, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_PAINT: {
            EnableWindow(hUndoBtn, historyCount > 0 && !won);
            EnableWindow(hHintBtn, !won && !autoSolving);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &rc, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            
            char textBuf[256];
            int optimalMoves = (1 << numDiscs) - 1;
            int m = elapsedSeconds / 60;
            int s = elapsedSeconds % 60;

            char bestBuf[64];
            if (hasStats[numDiscs]) {
                sprintf(bestBuf, "%02d:%02d (%d moves)", bestTime[numDiscs]/60, bestTime[numDiscs]%60, fewestMoves[numDiscs]);
            } else {
                strcpy(bestBuf, "--:-- (-- moves)");
            }

            sprintf(textBuf, "Time: %02d:%02d | Moves: %d / Optimal: %d | Best: %s", m, s, moves, optimalMoves, bestBuf);
            TextOut(hdc, rc.right / 2 - 180, 20, textBuf, strlen(textBuf));

            if (won) {
                SetTextColor(hdc, RGB(74, 222, 128));
                sprintf(textBuf, "You won in %d moves!", moves);
                TextOut(hdc, rc.right / 2 - 70, 50, textBuf, strlen(textBuf));
            }

            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            int pegAreaWidth = width / 3;
            int baseX[3] = { pegAreaWidth / 2, pegAreaWidth + pegAreaWidth / 2, 2 * pegAreaWidth + pegAreaWidth / 2 };
            int groundY = height - 100;
            
            // Draw poles
            HBRUSH poleBrush = CreateSolidBrush(RGB(85, 85, 85));
            for (int i = 0; i < 3; i++) {
                if (selectedPeg == i) {
                    // Draw highlight
                    HBRUSH hlBrush = CreateSolidBrush(RGB(30, 58, 95));
                    RECT hlRect = { baseX[i] - 70, groundY - 200, baseX[i] + 70, groundY + 10 };
                    FillRect(hdc, &hlRect, hlBrush);
                    DeleteObject(hlBrush);
                    
                    // Draw outline
                    HPEN hlPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 215));
                    HGDIOBJ oldPen = SelectObject(hdc, hlPen);
                    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, hlRect.left, hlRect.top, hlRect.right, hlRect.bottom);
                    SelectObject(hdc, oldBrush);
                    SelectObject(hdc, oldPen);
                    DeleteObject(hlPen);
                }
                
                RECT poleRect = { baseX[i] - 6, groundY - 180, baseX[i] + 6, groundY };
                FillRect(hdc, &poleRect, poleBrush);
                
                // Draw base
                RECT baseRect = { baseX[i] - 70, groundY, baseX[i] + 70, groundY + 8 };
                HBRUSH baseBrush = CreateSolidBrush(RGB(136, 136, 136));
                FillRect(hdc, &baseRect, baseBrush);
                DeleteObject(baseBrush);
            }
            DeleteObject(poleBrush);
            
            // Draw discs
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < pegCounts[i]; j++) {
                    int discSize = pegs[i][j];
                    int discWidth = 40 + discSize * 10;
                    int discHeight = 24;
                    int rectY = groundY - (j + 1) * (discHeight + 2);
                    
                    RECT dRect = { baseX[i] - discWidth / 2, rectY, baseX[i] + discWidth / 2, rectY + discHeight };
                    
                    HBRUSH dBrush = CreateSolidBrush(colors[(discSize - 1) % 8]);
                    FillRect(hdc, &dRect, dBrush);
                    
                    HPEN dPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                    HGDIOBJ oldPen = SelectObject(hdc, dPen);
                    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    
                    // Draw rounded rect equivalent (or just regular rectangle for simplicity)
                    // We can use RoundRect
                    RoundRect(hdc, dRect.left, dRect.top, dRect.right, dRect.bottom, 12, 12);
                    
                    SelectObject(hdc, oldBrush);
                    SelectObject(hdc, oldPen);
                    
                    DeleteObject(dPen);
                    DeleteObject(dBrush);
                }
            }

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));
    wc.lpszClassName = "KTowersClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("KTowersClass", "KTowers", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                             NULL, NULL, hInstance, NULL);
                             
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
