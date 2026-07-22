#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define CELL_SIZE 15
#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define TIMER_ID 1

struct Point { int x; int y; };
struct Point snake[400];
int snake_len = 3;
int dir_x = 1, dir_y = 0;
int last_dir_x = 1, last_dir_y = 0;
struct Point food = { 10, 10 };
struct Point special_food = { -1, -1 };
int special_food_timer = 0;
struct Point obstacles[50];
int num_obstacles = 0;
int game_state = 0; // 0 = Menu, 1 = Playing, 2 = Game Over
int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int score = 0;
int high_score = 0;
int current_speed = 150;
int base_speed = 150;
int score_mult = 10;
int wrap_mode = 0;
int campaign_mode = 0;
int campaign_level = 1;
int apples_eaten = 0;
struct Point ghost_food = {-1, -1};
int ghost_food_timer = 0;
int ghost_active_timer = 0;
struct Point ice_food = {-1, -1};
int ice_food_timer = 0;
int ice_active_timer = 0;
struct Point spiders[10];
int num_spiders = 0;
int spider_tick = 0;
struct Point portal1 = {-1, -1};
struct Point portal2 = {-1, -1};
struct Point trackers[5];
int num_trackers = 0;
int tracker_tick = 0;

int total_apples = 0;
int games_played = 0;

static int anim_tick = 0;

// Forward Function Declarations
int random_int(int max);
void PlaceFood();
void PlaceGhostFood();
void PlaceSpecialFood();
void PlaceIceFood();
void InitGame();

void DrawSnakeSegmentGDI(HDC hdc, int x, int y, int index, int total, int is_ghost, int d_x, int d_y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    HBRUSH oldBrush;
    HPEN oldPen;

    if (index == 0) {
        // HEAD
        HBRUSH headBrush;
        if (is_ghost) headBrush = CreateSolidBrush(RGB(72, 219, 251));
        else headBrush = CreateSolidBrush(RGB(39, 174, 96));
        HPEN headPen = CreatePen(PS_SOLID, 1, is_ghost ? RGB(0, 210, 211) : RGB(30, 130, 70));

        oldBrush = (HBRUSH)SelectObject(hdc, headBrush);
        oldPen = (HPEN)SelectObject(hdc, headPen);

        Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);

        // Eyes
        HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, eyeBrush);

        int eye1_x = cx + d_y * 3 + d_x * 2;
        int eye1_y = cy + d_x * 3 + d_y * 2;
        int eye2_x = cx - d_y * 3 + d_x * 2;
        int eye2_y = cy - d_x * 3 + d_y * 2;

        Ellipse(hdc, eye1_x - 2, eye1_y - 2, eye1_x + 2, eye1_y + 2);
        Ellipse(hdc, eye2_x - 2, eye2_y - 2, eye2_x + 2, eye2_y + 2);

        // Pupils
        HBRUSH pupilBrush = CreateSolidBrush(is_ghost ? RGB(9, 132, 227) : RGB(30, 39, 46));
        SelectObject(hdc, pupilBrush);
        Ellipse(hdc, eye1_x + d_x - 1, eye1_y + d_y - 1, eye1_x + d_x + 1, eye1_y + d_y + 1);
        Ellipse(hdc, eye2_x + d_x - 1, eye2_y + d_y - 1, eye2_x + d_x + 1, eye2_y + d_y + 1);

        DeleteObject(eyeBrush);
        DeleteObject(pupilBrush);

        // Flickering Tongue
        if ((anim_tick % 2) == 0) {
            HPEN tonguePen = CreatePen(PS_SOLID, 1, RGB(255, 56, 56));
            SelectObject(hdc, tonguePen);
            int tx = cx + d_x * 7;
            int ty = cy + d_y * 7;
            MoveToEx(hdc, cx + d_x * 4, cy + d_y * 4, NULL);
            LineTo(hdc, tx, ty);
            MoveToEx(hdc, tx, ty, NULL);
            LineTo(hdc, tx + d_x * 2 + d_y * 2, ty + d_y * 2 + d_x * 2);
            MoveToEx(hdc, tx, ty, NULL);
            LineTo(hdc, tx + d_x * 2 - d_y * 2, ty + d_y * 2 - d_x * 2);
            DeleteObject(tonguePen);
        }

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(headBrush);
        DeleteObject(headPen);
    } else {
        // BODY
        HBRUSH bodyBrush;
        if (is_ghost) {
            bodyBrush = CreateSolidBrush(index % 2 == 0 ? RGB(72, 219, 251) : RGB(0, 210, 211));
        } else {
            bodyBrush = CreateSolidBrush(index % 2 == 0 ? RGB(46, 204, 113) : RGB(33, 140, 116));
        }
        HPEN bodyPen = CreatePen(PS_SOLID, 1, is_ghost ? RGB(0, 180, 200) : RGB(25, 110, 90));

        oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
        oldPen = (HPEN)SelectObject(hdc, bodyPen);

        int inset = 1 + index / 15;
        if (inset > 4) inset = 4;
        Ellipse(hdc, px + inset, py + inset, px + CELL_SIZE - inset, py + CELL_SIZE - inset);

        // Inner highlight
        HBRUSH hlBrush = CreateSolidBrush(RGB(200, 255, 220));
        SelectObject(hdc, hlBrush);
        Ellipse(hdc, cx - 2, cy - 2, cx, cy);
        DeleteObject(hlBrush);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(bodyBrush);
        DeleteObject(bodyPen);
    }
}

void DrawAppleGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;

    HBRUSH appleBrush = CreateSolidBrush(RGB(235, 77, 75));
    HPEN applePen = CreatePen(PS_SOLID, 1, RGB(192, 57, 43));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, appleBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, applePen);

    Ellipse(hdc, px + 2, py + 3, px + CELL_SIZE - 2, py + CELL_SIZE - 1);

    HBRUSH shineBrush = CreateSolidBrush(RGB(255, 200, 200));
    SelectObject(hdc, shineBrush);
    Ellipse(hdc, px + 4, py + 5, px + 7, py + 8);
    DeleteObject(shineBrush);

    HPEN stemPen = CreatePen(PS_SOLID, 1, RGB(87, 75, 144));
    SelectObject(hdc, stemPen);
    MoveToEx(hdc, cx, py + 3, NULL);
    LineTo(hdc, cx + 2, py + 1);

    HBRUSH leafBrush = CreateSolidBrush(RGB(46, 204, 113));
    SelectObject(hdc, leafBrush);
    Ellipse(hdc, cx + 1, py, cx + 5, py + 3);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(appleBrush);
    DeleteObject(applePen);
    DeleteObject(stemPen);
    DeleteObject(leafBrush);
}

void DrawSpecialFoodGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    POINT pts[8] = {
        {cx, py + 1},
        {cx + 3, cy - 2},
        {px + CELL_SIZE - 1, cy},
        {cx + 3, cy + 2},
        {cx, py + CELL_SIZE - 1},
        {cx - 3, cy + 2},
        {px + 1, cy},
        {cx - 3, cy - 2}
    };

    HBRUSH starBrush = CreateSolidBrush(RGB(241, 196, 15));
    HPEN starPen = CreatePen(PS_SOLID, 1, RGB(243, 156, 18));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, starBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, starPen);

    Polygon(hdc, pts, 8);

    HBRUSH coreBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, coreBrush);
    Ellipse(hdc, cx - 2, cy - 2, cx + 2, cy + 2);
    DeleteObject(coreBrush);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(starBrush);
    DeleteObject(starPen);
}

void DrawGhostFoodGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    HBRUSH ghostBrush = CreateSolidBrush(RGB(72, 219, 251));
    HPEN ghostPen = CreatePen(PS_SOLID, 1, RGB(0, 210, 211));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, ghostBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, ghostPen);

    Ellipse(hdc, px + 2, py + 2, px + CELL_SIZE - 2, py + CELL_SIZE - 2);

    HBRUSH eyeBrush = CreateSolidBrush(RGB(16, 172, 132));
    SelectObject(hdc, eyeBrush);
    Ellipse(hdc, cx - 4, cy - 2, cx - 2, cy);
    Ellipse(hdc, cx + 2, cy - 2, cx + 4, cy);
    DeleteObject(eyeBrush);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(ghostBrush);
    DeleteObject(ghostPen);
}

void DrawIceFoodGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    POINT pts[4] = {
        {cx, py + 2},
        {px + CELL_SIZE - 2, cy},
        {cx, py + CELL_SIZE - 2},
        {px + 2, cy}
    };

    HBRUSH iceBrush = CreateSolidBrush(RGB(116, 185, 255));
    HPEN icePen = CreatePen(PS_SOLID, 1, RGB(162, 155, 254));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, iceBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, icePen);

    Polygon(hdc, pts, 4);

    POINT facet[3] = {
        {cx, py + 2},
        {cx, cy},
        {px + 2, cy}
    };
    HBRUSH facetBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, facetBrush);
    Polygon(hdc, facet, 3);
    DeleteObject(facetBrush);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(iceBrush);
    DeleteObject(icePen);
}

void DrawObstacleGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;

    RECT r = { px, py, px + CELL_SIZE - 1, py + CELL_SIZE - 1 };
    HBRUSH bgBrush = CreateSolidBrush(RGB(75, 101, 132));
    FillRect(hdc, &r, bgBrush);
    DeleteObject(bgBrush);

    HPEN hiPen = CreatePen(PS_SOLID, 1, RGB(119, 140, 163));
    HPEN oldPen = (HPEN)SelectObject(hdc, hiPen);
    MoveToEx(hdc, px, py + CELL_SIZE - 2, NULL);
    LineTo(hdc, px, py);
    LineTo(hdc, px + CELL_SIZE - 1, py);
    DeleteObject(hiPen);

    HPEN shPen = CreatePen(PS_SOLID, 1, RGB(45, 152, 218));
    SelectObject(hdc, shPen);
    MoveToEx(hdc, px + CELL_SIZE - 1, py + 1, NULL);
    LineTo(hdc, px + CELL_SIZE - 1, py + CELL_SIZE - 1);
    LineTo(hdc, px, py + CELL_SIZE - 1);
    DeleteObject(shPen);

    HPEN crackPen = CreatePen(PS_SOLID, 1, RGB(38, 222, 129));
    SelectObject(hdc, crackPen);
    MoveToEx(hdc, px + 3, py + 5, NULL);
    LineTo(hdc, px + 7, py + 10);
    LineTo(hdc, px + 12, py + 8);
    DeleteObject(crackPen);

    SelectObject(hdc, oldPen);
}

void DrawSpiderGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    HPEN legPen = CreatePen(PS_SOLID, 1, RGB(165, 94, 234));
    HPEN oldPen = (HPEN)SelectObject(hdc, legPen);

    int wiggle = (anim_tick % 2 == 0) ? 1 : -1;

    MoveToEx(hdc, cx, cy - 2, NULL); LineTo(hdc, cx - 6, cy - 5 + wiggle);
    MoveToEx(hdc, cx, cy - 2, NULL); LineTo(hdc, cx + 6, cy - 5 - wiggle);
    MoveToEx(hdc, cx, cy, NULL);     LineTo(hdc, cx - 7, cy + wiggle);
    MoveToEx(hdc, cx, cy, NULL);     LineTo(hdc, cx + 7, cy - wiggle);
    MoveToEx(hdc, cx, cy + 2, NULL); LineTo(hdc, cx - 6, cy + 5 + wiggle);
    MoveToEx(hdc, cx, cy + 2, NULL); LineTo(hdc, cx + 6, cy + 5 - wiggle);

    HBRUSH bodyBrush = CreateSolidBrush(RGB(75, 75, 75));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
    Ellipse(hdc, cx - 4, cy - 4, cx + 4, cy + 4);

    HBRUSH headBrush = CreateSolidBrush(RGB(45, 52, 54));
    SelectObject(hdc, headBrush);
    Ellipse(hdc, cx - 3, cy - 6, cx + 3, cy - 2);

    HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 56, 56));
    SelectObject(hdc, eyeBrush);
    Ellipse(hdc, cx - 2, cy - 5, cx - 1, cy - 4);
    Ellipse(hdc, cx + 1, cy - 5, cx + 2, cy - 4);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(legPen);
    DeleteObject(bodyBrush);
    DeleteObject(headBrush);
    DeleteObject(eyeBrush);
}

void DrawPortalGDI(HDC hdc, int x, int y, int is_secondary) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    COLORREF pCol = is_secondary ? RGB(230, 126, 34) : RGB(243, 156, 18);

    HPEN pPen = CreatePen(PS_SOLID, 1, pCol);
    HPEN oldPen = (HPEN)SelectObject(hdc, pPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, (HGDIOBJ)GetStockObject(NULL_BRUSH));

    Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);
    Ellipse(hdc, px + 3, py + 3, px + CELL_SIZE - 3, py + CELL_SIZE - 3);

    HBRUSH coreBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, coreBrush);
    Ellipse(hdc, cx - 2, cy - 2, cx + 2, cy + 2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pPen);
    DeleteObject(coreBrush);
}

void DrawTrackerGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2;
    int cy = py + CELL_SIZE / 2;

    HBRUSH bodyBrush = CreateSolidBrush(RGB(44, 62, 80));
    HPEN bodyPen = CreatePen(PS_SOLID, 1, RGB(255, 56, 56));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, bodyPen);

    Ellipse(hdc, px + 2, py + 2, px + CELL_SIZE - 2, py + CELL_SIZE - 2);

    HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 56, 56));
    SelectObject(hdc, eyeBrush);
    Ellipse(hdc, cx - 3, cy - 3, cx + 3, cy + 3);

    HBRUSH dotBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, dotBrush);
    Ellipse(hdc, cx - 1, cy - 1, cx + 1, cy + 1);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bodyBrush);
    DeleteObject(bodyPen);
    DeleteObject(eyeBrush);
    DeleteObject(dotBrush);
}

void LoadStats() {
    HANDLE hFile = CreateFileA("ksnake.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &high_score, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &total_apples, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &games_played, sizeof(int), &bytesRead, NULL);
        CloseHandle(hFile);
    }
}
void SaveStats() {
    HANDLE hFile = CreateFileA("ksnake.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, &high_score, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &total_apples, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &games_played, sizeof(int), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void InitGame() {
    games_played++;
    SaveStats();
    apples_eaten = 0;
    if(campaign_mode) {
        difficulty = 1;
    }
    snake_len = 3;
    snake[0].x = 5; snake[0].y = 5;
    snake[1].x = 4; snake[1].y = 5;
    snake[2].x = 3; snake[2].y = 5;
    dir_x = 1; dir_y = 0;
    last_dir_x = 1; last_dir_y = 0;
    
    if (difficulty == 0) { base_speed = 200; score_mult = 5; }
    else if (difficulty == 1) { base_speed = 150; score_mult = 10; }
    else { base_speed = 100; score_mult = 20; }
    
    current_speed = base_speed;
    game_state = 1;
    score = 0;
    special_food.x = -1;
    special_food.y = -1;
    special_food_timer = 0;
    ghost_food.x = -1; ghost_food.y = -1;
    ghost_food_timer = 0;
    ghost_active_timer = 0;
    ice_food.x = -1; ice_food.y = -1;
    ice_food_timer = 0;
    ice_active_timer = 0;
    spider_tick = 0;
    tracker_tick = 0;
    portal1.x = -1; portal1.y = -1;
    portal2.x = -1; portal2.y = -1;
    
    if (campaign_mode) {
        num_obstacles = campaign_level * 4;
        if (num_obstacles > 45) num_obstacles = 45;
    } else {
        num_obstacles = difficulty * 10;
    }
    for(int i=0; i<num_obstacles; i++) {
        int ok = 0;
        while(!ok) {
            obstacles[i].x = random_int(GRID_WIDTH);
            obstacles[i].y = random_int(GRID_HEIGHT);
            ok = 1;
            if (obstacles[i].y == 5 && (obstacles[i].x >= 2 && obstacles[i].x <= 6)) ok = 0;
        }
    }
    
    num_spiders = difficulty;
    if (campaign_mode) {
        num_spiders = campaign_level / 2;
    }
    if (num_spiders > 10) num_spiders = 10;
    
    for(int i=0; i<num_spiders; i++) {
        spiders[i].x = random_int(GRID_WIDTH);
        spiders[i].y = random_int(GRID_HEIGHT);
    }
    
    if (campaign_mode && campaign_level >= 6) {
        int ok = 0;
        while(!ok) {
            portal1.x = random_int(GRID_WIDTH); portal1.y = random_int(GRID_HEIGHT);
            portal2.x = random_int(GRID_WIDTH); portal2.y = random_int(GRID_HEIGHT);
            ok = 1;
            if (portal1.x == portal2.x && portal1.y == portal2.y) ok = 0;
            for(int i=0; i<num_obstacles; i++) {
                if (portal1.x == obstacles[i].x && portal1.y == obstacles[i].y) ok = 0;
                if (portal2.x == obstacles[i].x && portal2.y == obstacles[i].y) ok = 0;
            }
            if (portal1.y == 5 && (portal1.x >= 2 && portal1.x <= 6)) ok = 0;
            if (portal2.y == 5 && (portal2.x >= 2 && portal2.x <= 6)) ok = 0;
        }
    }
    
    num_trackers = 0;
    if (campaign_mode && campaign_level >= 11) {
        num_trackers = campaign_level - 10;
    } else if (!campaign_mode && difficulty == 2) {
        num_trackers = 1;
    }
    if (num_trackers > 5) num_trackers = 5;
    
    for(int i=0; i<num_trackers; i++) {
        trackers[i].x = random_int(GRID_WIDTH);
        trackers[i].y = random_int(GRID_HEIGHT);
    }

    PlaceFood();
}

unsigned int rng_state = 12345;
int random_int(int max) {
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

void PlaceFood() {
    int ok = 0;
    while(!ok) {
        food.x = random_int(GRID_WIDTH);
        food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) {
            if (food.x == obstacles[i].x && food.y == obstacles[i].y) ok = 0;
        }
        for(int i=0; i<snake_len; i++) {
            if (food.x == snake[i].x && food.y == snake[i].y) ok = 0;
        }
        if (portal1.x != -1 && food.x == portal1.x && food.y == portal1.y) ok = 0;
        if (portal2.x != -1 && food.x == portal2.x && food.y == portal2.y) ok = 0;
    }
}

void PlaceGhostFood() {
    int ok = 0;
    while(!ok) {
        ghost_food.x = random_int(GRID_WIDTH); ghost_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if(ghost_food.x==obstacles[i].x && ghost_food.y==obstacles[i].y) ok=0;
        for(int i=0; i<snake_len; i++) if(ghost_food.x==snake[i].x && ghost_food.y==snake[i].y) ok=0;
        if(ghost_food.x == food.x && ghost_food.y == food.y) ok=0;
        if(ghost_food.x == special_food.x && ghost_food.y == special_food.y) ok=0;
        if (portal1.x != -1 && ghost_food.x == portal1.x && ghost_food.y == portal1.y) ok = 0;
        if (portal2.x != -1 && ghost_food.x == portal2.x && ghost_food.y == portal2.y) ok = 0;
    }
    ghost_food_timer = 40;
}

void PlaceSpecialFood() {
    int ok = 0;
    while(!ok) {
        special_food.x = random_int(GRID_WIDTH);
        special_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) {
            if (special_food.x == obstacles[i].x && special_food.y == obstacles[i].y) ok = 0;
        }
        for(int i=0; i<snake_len; i++) {
            if (special_food.x == snake[i].x && special_food.y == snake[i].y) ok = 0;
        }
        if (special_food.x == food.x && special_food.y == food.y) ok = 0;
        if (portal1.x != -1 && special_food.x == portal1.x && special_food.y == portal1.y) ok = 0;
        if (portal2.x != -1 && special_food.x == portal2.x && special_food.y == portal2.y) ok = 0;
    }
    special_food_timer = 40;
}

void PlaceIceFood() {
    int ok = 0;
    while(!ok) {
        ice_food.x = random_int(GRID_WIDTH);
        ice_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if (ice_food.x == obstacles[i].x && ice_food.y == obstacles[i].y) ok = 0;
        for(int i=0; i<snake_len; i++) if (ice_food.x == snake[i].x && ice_food.y == snake[i].y) ok = 0;
        if (ice_food.x == food.x && ice_food.y == food.y) ok = 0;
        if (portal1.x != -1 && ice_food.x == portal1.x && ice_food.y == portal1.y) ok = 0;
        if (portal2.x != -1 && ice_food.x == portal2.x && ice_food.y == portal2.y) ok = 0;
    }
    ice_food_timer = 40;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            // don't start timer yet, wait in menu
            break;
        case WM_TIMER: {
            if (game_state != 1) break;
            anim_tick++;
            
            last_dir_x = dir_x;
            last_dir_y = dir_y;
            
            // Move body
            for(int i = snake_len - 1; i > 0; i--) {
                snake[i] = snake[i-1];
            }
            // Move head
            snake[0].x += dir_x;
            snake[0].y += dir_y;

            // Portals
            if (portal1.x != -1) {
                if (snake[0].x == portal1.x && snake[0].y == portal1.y) {
                    snake[0].x = portal2.x; snake[0].y = portal2.y;
                } else if (snake[0].x == portal2.x && snake[0].y == portal2.y) {
                    snake[0].x = portal1.x; snake[0].y = portal1.y;
                }
            }

            // Collision with walls
            if (wrap_mode) {
                if (snake[0].x < 0) snake[0].x = GRID_WIDTH - 1;
                else if (snake[0].x >= GRID_WIDTH) snake[0].x = 0;
                
                if (snake[0].y < 0) snake[0].y = GRID_HEIGHT - 1;
                else if (snake[0].y >= GRID_HEIGHT) snake[0].y = 0;
            } else {
                if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || 
                    snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                    game_state = 2;
                }
            }

            // Spider Movement
            spider_tick++;
            if (spider_tick >= 2) {
                spider_tick = 0;
                for(int i=0; i<num_spiders; i++) {
                    int dx = 0, dy = 0;
                    int r = random_int(4);
                    if (r==0) dx = 1; else if (r==1) dx = -1; else if (r==2) dy = 1; else if (r==3) dy = -1;
                    int nx = spiders[i].x + dx;
                    int ny = spiders[i].y + dy;
                    if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                        int obs_hit = 0;
                        for(int j=0; j<num_obstacles; j++) if (nx == obstacles[j].x && ny == obstacles[j].y) obs_hit = 1;
                        if (!obs_hit) { spiders[i].x = nx; spiders[i].y = ny; }
                    }
                }
            }

            // Tracker Movement
            tracker_tick++;
            if (tracker_tick >= 3) {
                tracker_tick = 0;
                for(int i=0; i<num_trackers; i++) {
                    int dx = 0, dy = 0;
                    if (snake[0].x > trackers[i].x) dx = 1;
                    else if (snake[0].x < trackers[i].x) dx = -1;
                    if (snake[0].y > trackers[i].y) dy = 1;
                    else if (snake[0].y < trackers[i].y) dy = -1;
                    
                    int nx = trackers[i].x + dx;
                    int ny = trackers[i].y;
                    int blocked = 0;
                    for(int j=0; j<num_obstacles; j++) if(nx == obstacles[j].x && ny == obstacles[j].y) blocked=1;
                    if (!blocked && dx != 0) { trackers[i].x = nx; }
                    else {
                        nx = trackers[i].x; ny = trackers[i].y + dy; blocked = 0;
                        for(int j=0; j<num_obstacles; j++) if(nx == obstacles[j].x && ny == obstacles[j].y) blocked=1;
                        if (!blocked && dy != 0) { trackers[i].y = ny; }
                    }
                }
            }

            // Collision with self and obstacles
            if(ghost_active_timer == 0) {
                for(int i = 1; i < snake_len; i++) {
                    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) game_state = 2;
                }
                for(int i = 0; i < num_obstacles; i++) {
                    if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) game_state = 2;
                }
                for(int i = 0; i < num_spiders; i++) {
                    if (snake[0].x == spiders[i].x && snake[0].y == spiders[i].y) game_state = 2;
                }
                for(int i = 0; i < num_trackers; i++) {
                    if (snake[0].x == trackers[i].x && snake[0].y == trackers[i].y) game_state = 2;
                }
            }
            
            if (game_state == 2) {
                MessageBeep(MB_ICONHAND);
            }

            // Eat food
            if (snake[0].x == food.x && snake[0].y == food.y) {
                MessageBeep(MB_OK);
                if (snake_len < 400) {
                    snake[snake_len] = snake[snake_len-1]; // grow
                    snake_len++;
                }
                score += score_mult;
                total_apples++;
                if (score > high_score) { high_score = score; }
                SaveStats();
                if (current_speed > 30) current_speed -= 2;
                SetTimer(hwnd, TIMER_ID, ice_active_timer > 0 ? current_speed + 100 : current_speed, NULL);
                PlaceFood();
                
                if (special_food.x == -1 && random_int(100) < 20) {
                    PlaceSpecialFood();
                }
                if (ghost_food.x == -1 && random_int(100) < 15) {
                    PlaceGhostFood();
                }
                if (ice_food.x == -1 && random_int(100) < 15) {
                    PlaceIceFood();
                }
                
                apples_eaten++;
                if (campaign_mode && apples_eaten >= 10) {
                    campaign_level++;
                    if (campaign_level > 15) {
                        game_state = 3;
                    } else {
                        base_speed -= 10;
                        if(base_speed < 40) base_speed = 40;
                        InitGame();
                    }
                }
            }

            if (ice_active_timer > 0) {
                ice_active_timer--;
                if (ice_active_timer == 0) SetTimer(hwnd, TIMER_ID, current_speed, NULL);
            }
            if (ice_food_timer > 0) {
                ice_food_timer--;
                if (ice_food_timer == 0) { ice_food.x = -1; ice_food.y = -1; }
            }
            if (ice_food.x != -1 && snake[0].x == ice_food.x && snake[0].y == ice_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 2;
                if (score > high_score) { high_score = score; SaveStats(); }
                ice_active_timer = 50;
                ice_food.x = -1; ice_food.y = -1;
                SetTimer(hwnd, TIMER_ID, current_speed + 100, NULL);
            }

            if (ghost_active_timer > 0) ghost_active_timer--;
            
            if (ghost_food_timer > 0) {
                ghost_food_timer--;
                if (ghost_food_timer == 0) { ghost_food.x = -1; ghost_food.y = -1; }
            }
            if (ghost_food.x != -1 && snake[0].x == ghost_food.x && snake[0].y == ghost_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 2;
                if (score > high_score) { high_score = score; SaveStats(); }
                ghost_active_timer = 50; // ghost for 50 ticks
                ghost_food.x = -1; ghost_food.y = -1;
            }

            if (special_food_timer > 0) {
                special_food_timer--;
                if (special_food_timer == 0) {
                    special_food.x = -1;
                    special_food.y = -1;
                }
            }

            if (special_food.x != -1 && snake[0].x == special_food.x && snake[0].y == special_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 5;
                if (score > high_score) { high_score = score; SaveStats(); }
                snake_len -= 3;
                if (snake_len < 3) snake_len = 3;
                special_food.x = -1;
                special_food.y = -1;
            }

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_KEYDOWN: {
            if (game_state == 0) {
                if (wParam == '1') { difficulty = 0; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '2') { difficulty = 1; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '3') { difficulty = 2; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '4') { campaign_mode = 1; campaign_level = 1; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == 'W') { wrap_mode = !wrap_mode; InvalidateRect(hwnd, NULL, TRUE); }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if ((game_state == 2 || game_state == 3) && wParam == VK_RETURN) {
                game_state = 0;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 1) {
                if (wParam == VK_UP && last_dir_y != 1) { dir_x = 0; dir_y = -1; }
                if (wParam == VK_DOWN && last_dir_y != -1) { dir_x = 0; dir_y = 1; }
                if (wParam == VK_LEFT && last_dir_x != 1) { dir_x = -1; dir_y = 0; }
                if (wParam == VK_RIGHT && last_dir_x != -1) { dir_x = 1; dir_y = 0; }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &ps.rcPaint, bg);
            DeleteObject(bg);

            if (game_state == 0) {
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 80, 50, "KSNAKE", 6);
                TextOutA(hdc, 40, 100, "Select Difficulty:", 18);
                TextOutA(hdc, 40, 130, "1 - Easy", 8);
                TextOutA(hdc, 40, 150, "2 - Medium", 10);
                TextOutA(hdc, 40, 170, "3 - Hard", 8);
                TextOutA(hdc, 40, 190, "4 - Campaign Mode", 17);
                char wrap_text[32];
                wsprintf(wrap_text, "W - Toggle Wrap: %s", wrap_mode ? "ON" : "OFF");
                TextOutA(hdc, 40, 210, wrap_text, lstrlenA(wrap_text));
                
                char stats_text[64];
                wsprintf(stats_text, "Games: %d  Total Apples: %d", games_played, total_apples);
                TextOutA(hdc, 40, 230, stats_text, lstrlenA(stats_text));
            } else if (game_state == 2) {
                SetTextColor(hdc, RGB(255, 0, 0));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 100, 100, "GAME OVER", 9);
                TextOutA(hdc, 60, 130, "Press ENTER to return", 21);
            } else if (game_state == 3) {
                SetTextColor(hdc, RGB(0, 255, 0));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 100, 100, "YOU WIN!", 8);
                TextOutA(hdc, 60, 130, "Press ENTER to return", 21);
            } else {
                for(int i = 0; i < num_obstacles; i++) {
                    DrawObstacleGDI(hdc, obstacles[i].x, obstacles[i].y);
                }

                if (portal1.x != -1) {
                    DrawPortalGDI(hdc, portal1.x, portal1.y, 0);
                    DrawPortalGDI(hdc, portal2.x, portal2.y, 1);
                }

                DrawAppleGDI(hdc, food.x, food.y);
                if (ghost_food.x != -1) DrawGhostFoodGDI(hdc, ghost_food.x, ghost_food.y);
                if (special_food.x != -1) DrawSpecialFoodGDI(hdc, special_food.x, special_food.y);
                if (ice_food.x != -1) DrawIceFoodGDI(hdc, ice_food.x, ice_food.y);

                for(int i = 0; i < num_spiders; i++) {
                    DrawSpiderGDI(hdc, spiders[i].x, spiders[i].y);
                }

                for(int i = 0; i < num_trackers; i++) {
                    DrawTrackerGDI(hdc, trackers[i].x, trackers[i].y);
                }

                for(int i = snake_len - 1; i >= 0; i--) {
                    DrawSnakeSegmentGDI(hdc, snake[i].x, snake[i].y, i, snake_len, ghost_active_timer > 0, dir_x, dir_y);
                }
            }

            if (game_state != 0) {
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                HBRUSH borderBrush = CreateSolidBrush(RGB(102, 102, 102));
                FrameRect(hdc, &clientRect, borderBrush);
                InflateRect(&clientRect, -1, -1);
                FrameRect(hdc, &clientRect, borderBrush);
                DeleteObject(borderBrush);
                
                char score_text[64];
                if (campaign_mode) {
                    wsprintf(score_text, "Lvl: %d Score: %d High: %d", campaign_level, score, high_score);
                } else {
                    wsprintf(score_text, "Score: %d  High: %d", score, high_score);
                }
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 5, 5, score_text, lstrlenA(score_text));
            }

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() { LoadStats();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSnakeApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    
    // Try to load custom icon, ignore if fails
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    RegisterClass(&wc);

    int winWidth = GRID_WIDTH * CELL_SIZE + 16;
    int winHeight = GRID_HEIGHT * CELL_SIZE + 39;

    HWND hwnd = CreateWindowEx(0, "KSnakeApp", "KSnake", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
