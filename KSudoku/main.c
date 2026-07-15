#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Colors
#define COLOR_BG RGB(15, 23, 42)
#define COLOR_GRID RGB(248, 250, 252)
#define COLOR_TEXT RGB(248, 250, 252)
#define COLOR_FIXED_TEXT RGB(148, 163, 184)
#define COLOR_SELECTED RGB(59, 130, 246)
#define COLOR_HIGHLIGHT RGB(30, 58, 138)
#define COLOR_ERROR RGB(239, 68, 68)

int board[9][9];
int solution[9][9];
int fixed[9][9];
int sel_r = -1, sel_c = -1;
int error_cells[9][9];
HWND hBtnNew, hBtnValidate;
HFONT hFont;

void ShuffleArray(int* arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void GenerateBoard() {
    int base[9][9] = {
        {1,2,3, 4,5,6, 7,8,9},
        {4,5,6, 7,8,9, 1,2,3},
        {7,8,9, 1,2,3, 4,5,6},
        {2,3,1, 5,6,4, 8,9,7},
        {5,6,4, 8,9,7, 2,3,1},
        {8,9,7, 2,3,1, 5,6,4},
        {3,1,2, 6,4,5, 9,7,8},
        {6,4,5, 9,7,8, 3,1,2},
        {9,7,8, 3,1,2, 6,4,5}
    };
    int nums[9] = {1,2,3,4,5,6,7,8,9};
    ShuffleArray(nums, 9);
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            solution[r][c] = nums[base[r][c]-1];
            
    for(int band=0; band<3; band++) {
        int rows[3] = {0,1,2};
        ShuffleArray(rows, 3);
        int temp[3][9];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) temp[i][c] = solution[band*3 + rows[i]][c];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) solution[band*3 + i][c] = temp[i][c];
    }
    
    for(int band=0; band<3; band++) {
        int cols[3] = {0,1,2};
        ShuffleArray(cols, 3);
        int temp[9][3];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) temp[r][i] = solution[r][band*3 + cols[i]];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) solution[r][band*3 + i] = temp[r][i];
    }
    
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            board[r][c] = solution[r][c];
            fixed[r][c] = 1;
            error_cells[r][c] = 0;
        }
    }
    
    int difficulty = 40;
    int removal = 81 - difficulty;
    while(removal > 0) {
        int r = rand() % 9;
        int c = rand() % 9;
        if(board[r][c] != 0) {
            board[r][c] = 0;
            fixed[r][c] = 0;
            removal--;
        }
    }
}

void CheckWin(HWND hwnd) {
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            if(board[r][c] != solution[r][c]) return;
    MessageBoxA(hwnd, "Congratulations! You solved the puzzle!", "KSudoku", MB_OK);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            GenerateBoard();
            hBtnNew = CreateWindowA("BUTTON", "New Game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            hBtnValidate = CreateWindowA("BUTTON", "Validate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 120, 10, 100, 30, hwnd, (HMENU)2, NULL, NULL);
            hFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // New Game
                GenerateBoard();
                sel_r = -1; sel_c = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 2) { // Validate
                for(int r=0; r<9; r++) {
                    for(int c=0; c<9; c++) {
                        if (!fixed[r][c] && board[r][c] != 0 && board[r][c] != solution[r][c]) {
                            error_cells[r][c] = 1;
                        } else {
                            error_cells[r][c] = 0;
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Set focus back to main window so keyboard works
            SetFocus(hwnd);
            break;
        }
        case WM_KEYDOWN: {
            if(sel_r == -1) sel_r = 0, sel_c = 0;
            else {
                if(wParam == VK_UP) sel_r = max(0, sel_r - 1);
                if(wParam == VK_DOWN) sel_r = min(8, sel_r + 1);
                if(wParam == VK_LEFT) sel_c = max(0, sel_c - 1);
                if(wParam == VK_RIGHT) sel_c = min(8, sel_c + 1);
            }
            if(wParam >= '1' && wParam <= '9') {
                if(!fixed[sel_r][sel_c]) {
                    board[sel_r][sel_c] = wParam - '0';
                    error_cells[sel_r][sel_c] = 0;
                    CheckWin(hwnd);
                }
            } else if(wParam == VK_BACK || wParam == VK_DELETE || wParam == '0') {
                if(!fixed[sel_r][sel_c]) {
                    board[sel_r][sel_c] = 0;
                    error_cells[sel_r][sel_c] = 0;
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int start_x = 40, start_y = 60, cell_sz = 40;
            if(x >= start_x && x < start_x + 9*cell_sz && y >= start_y && y < start_y + 9*cell_sz) {
                sel_c = (x - start_x) / cell_sz;
                sel_r = (y - start_y) / cell_sz;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Ensure focus is on main window so keyboard works
            SetFocus(hwnd);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Background
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hbg = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &clientRect, hbg);
            DeleteObject(hbg);
            
            int start_x = 40, start_y = 60, cell_sz = 40;
            
            // Determine highlight value
            int highlight_val = 0;
            if(sel_r >= 0 && sel_c >= 0 && board[sel_r][sel_c] != 0) {
                highlight_val = board[sel_r][sel_c];
            }
            
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            
            // Draw cells
            for(int r=0; r<9; r++) {
                for(int c=0; c<9; c++) {
                    RECT rc = {start_x + c*cell_sz, start_y + r*cell_sz, start_x + (c+1)*cell_sz, start_y + (r+1)*cell_sz};
                    
                    HBRUSH cellBg = NULL;
                    if(r == sel_r && c == sel_c) {
                        cellBg = CreateSolidBrush(COLOR_SELECTED);
                    } else if(sel_r >= 0 && (r == sel_r || c == sel_c || (r/3 == sel_r/3 && c/3 == sel_c/3))) {
                        cellBg = CreateSolidBrush(COLOR_HIGHLIGHT);
                    } else if(highlight_val && board[r][c] == highlight_val) {
                        cellBg = CreateSolidBrush(COLOR_HIGHLIGHT);
                    }
                    
                    if(cellBg) {
                        FillRect(hdc, &rc, cellBg);
                        DeleteObject(cellBg);
                    }
                    
                    if(board[r][c] != 0) {
                        char buf[2];
                        sprintf(buf, "%d", board[r][c]);
                        if(error_cells[r][c]) SetTextColor(hdc, COLOR_ERROR);
                        else if(fixed[r][c]) SetTextColor(hdc, COLOR_TEXT); 
                        else SetTextColor(hdc, RGB(59, 130, 246));
                        
                        DrawTextA(hdc, buf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
            
            // Draw grid lines
            for(int i=0; i<=9; i++) {
                HPEN hPen = CreatePen(PS_SOLID, (i % 3 == 0) ? 3 : 1, COLOR_GRID);
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                
                MoveToEx(hdc, start_x + i*cell_sz, start_y, NULL);
                LineTo(hdc, start_x + i*cell_sz, start_y + 9*cell_sz);
                
                MoveToEx(hdc, start_x, start_y + i*cell_sz, NULL);
                LineTo(hdc, start_x + 9*cell_sz, start_y + i*cell_sz);
                
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            
            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            DeleteObject(hFont);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "KSudokuClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);
    
    // Calculate required window size
    RECT rc = {0, 0, 440, 460}; 
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), FALSE);
    
    HWND hwnd = CreateWindowA("KSudokuClass", "KSudoku", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
        
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
