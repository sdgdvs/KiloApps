#include <windows.h>
#include <stdio.h>

HWND hLogEdit;
HWND hBtnExplore, hBtnTown, hBtnInventory, hBtnRest, hBtnNewGame;
HWND hStatusText;

typedef struct {
    char name[32];
    char heroClass[16];
    int level;
    int hp, maxHp;
    int mp, maxMp;
    int gold;
    int xp, nextXp;
    int dungeonFloor;
} Hero;

Hero player = { "Hero", "Warrior", 1, 50, 50, 20, 20, 50, 0, 100, 1 };

void LogMessage(const char* msg) {
    if (!hLogEdit) return;
    int len = GetWindowTextLength(hLogEdit);
    SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void UpdateUI() {
    char statusBuf[256];
    sprintf(statusBuf, "Hero: %s (%s) | Level: %d | HP: %d/%d | MP: %d/%d | Gold: %d | XP: %d/%d | Floor: %d",
            player.name, player.heroClass, player.level,
            player.hp, player.maxHp, player.mp, player.maxMp,
            player.gold, player.xp, player.nextXp, player.dungeonFloor);
    if (hStatusText) {
        SetWindowText(hStatusText, statusBuf);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hLogEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                20, 60, 740, 360, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);

            hStatusText = CreateWindow("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, 20, 740, 30, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);

            hBtnExplore = CreateWindow("BUTTON", "Explore Dungeon",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 440, 130, 40, hwnd, (HMENU)201, GetModuleHandle(NULL), NULL);

            hBtnTown = CreateWindow("BUTTON", "Visit Town",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                160, 440, 130, 40, hwnd, (HMENU)202, GetModuleHandle(NULL), NULL);

            hBtnInventory = CreateWindow("BUTTON", "Inventory",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                300, 440, 130, 40, hwnd, (HMENU)203, GetModuleHandle(NULL), NULL);

            hBtnRest = CreateWindow("BUTTON", "Rest at Inn",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                440, 440, 130, 40, hwnd, (HMENU)204, GetModuleHandle(NULL), NULL);

            hBtnNewGame = CreateWindow("BUTTON", "New Game",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                580, 440, 130, 40, hwnd, (HMENU)205, GetModuleHandle(NULL), NULL);

            UpdateUI();
            LogMessage("=== Welcome to KQuest: Fantasy Dungeon RPG ===");
            LogMessage("Choose an action below to begin your adventure!");
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 201: // Explore
                    LogMessage("You venture into the dark dungeon depths... Floor 1.");
                    break;
                case 202: // Visit Town
                    LogMessage("You enter the town hub. Shops and quest boards await.");
                    break;
                case 203: // Inventory
                    LogMessage("Inventory: Iron Sword, Health Potion x2, Torch.");
                    break;
                case 204: // Rest
                    player.hp = player.maxHp;
                    player.mp = player.maxMp;
                    UpdateUI();
                    LogMessage("You rested at the inn and recovered full HP and MP!");
                    break;
                case 205: // New Game
                    player.level = 1;
                    player.hp = 50; player.maxHp = 50;
                    player.mp = 20; player.maxMp = 20;
                    player.gold = 50;
                    player.xp = 0; player.dungeonFloor = 1;
                    UpdateUI();
                    LogMessage("--- New Adventure Started ---");
                    break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(220, 220, 240));
            SetBkColor(hdcStatic, RGB(30, 30, 45));
            static HBRUSH hbrBkgnd = NULL;
            if (!hbrBkgnd) hbrBkgnd = CreateSolidBrush(RGB(30, 30, 45));
            return (INT_PTR)hbrBkgnd;
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
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 30));
    wc.lpszClassName = "KQuestClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("KQuestClass", "KQuest - Fantasy RPG", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 540,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    int ret = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
