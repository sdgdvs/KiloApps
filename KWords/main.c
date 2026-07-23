#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MAX_GRID_SIZE 26
#define CELL_SIZE 30
#define MAX_WORDS 20
#define NUM_THEMES 11
#define THEME_DICT_SIZE 30

const char* THEMES[NUM_THEMES] = {
    "Programming", "Animals", "Countries", "Space", "Food", 
    "Sports", "Nature", "History", "Elements", "Sci-Fi", "Myth"
};

const char* DICTIONARIES[NUM_THEMES][THEME_DICT_SIZE] = {
    { // 0: Programming
        "ALGORITHM", "COMPILER", "DEBUG", "FUNCTION", "VARIABLE", 
        "POINTER", "SYNTAX", "OBJECT", "CLASS", "METHOD", 
        "ARRAY", "STRING", "BOOLEAN", "INTEGER", "FLOAT",
        "NETWORK", "SERVER", "DATABASE", "CLIENT", "PROTOCOL", 
        "ROUTER", "BROWSER", "KERNEL", "MEMORY", "THREAD", 
        "PROCESS", "SOCKET", "PACKET", "CACHE", "FRAMEWORK"
    },
    { // 1: Animals
        "ELEPHANT", "GIRAFFE", "PENGUIN", "KANGAROO", "DOLPHIN",
        "TIGER", "CHEETAH", "MONKEY", "OSTRICH", "IGUANA",
        "ZEBRA", "GORILLA", "PANTHER", "LEOPARD", "HIPPO",
        "RHINO", "CROCODILE", "ALLIGATOR", "CHIMPANZEE", "SNAIL",
        "OCTOPUS", "SHARK", "WHALE", "WALRUS", "SEAL",
        "BEAR", "WOLF", "FOX", "RABBIT", "DEER"
    },
    { // 2: Countries
        "CANADA", "BRAZIL", "JAPAN", "FRANCE", "GERMANY",
        "ITALY", "SPAIN", "INDIA", "CHINA", "RUSSIA",
        "AUSTRALIA", "MEXICO", "ARGENTINA", "CHILE", "PERU",
        "EGYPT", "MOROCCO", "NIGERIA", "KENYA", "SWEDEN",
        "NORWAY", "FINLAND", "DENMARK", "IRELAND", "POLAND",
        "GREECE", "TURKEY", "THAILAND", "VIETNAM", "INDONESIA"
    },
    { // 3: Space
        "ASTEROID", "COMET", "GALAXY", "NEBULA", "PLANET",
        "STAR", "ORBIT", "SATELLITE", "ROCKET", "GRAVITY",
        "ECLIPSE", "METEOR", "UNIVERSE", "COSMOS", "PULSAR",
        "QUASAR", "SUPERNOVA", "VACUUM", "EQUATOR", "HORIZON",
        "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT",
        "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"
    },
    { // 4: Food
        "PIZZA", "BURGER", "SALAD", "PASTA", "SUSHI",
        "STEAK", "CHEESE", "BREAD", "APPLE", "BANANA",
        "ORANGE", "GRAPE", "CHICKEN", "BACON", "TOMATO",
        "POTATO", "ONION", "GARLIC", "PEPPER", "CARROT",
        "CEREAL", "WAFFLE", "PANCAKE", "MUFFIN", "COOKIE",
        "CHOCOLATE", "VANILLA", "BUTTER", "YOGURT", "HONEY"
    },
    { // 5: Sports
        "SOCCER", "TENNIS", "BASKETBALL", "BASEBALL", "GOLF",
        "RUGBY", "CRICKET", "HOCKEY", "VOLLEYBALL", "SWIMMING",
        "BOXING", "WRESTLING", "CYCLING", "ATHLETICS", "GYMNASTICS",
        "ARCHERY", "FENCING", "BOWLING", "BILLIARDS", "SNOOKER",
        "DARTS", "KARATE", "JUDO", "TAEKWONDO", "SURFING",
        "SKATING", "SKIING", "SNOWBOARD", "ROWING", "SAILING"
    },
    { // 6: Nature
        "FOREST", "RIVER", "MOUNTAIN", "OCEAN", "DESERT",
        "VALLEY", "CANYON", "VOLCANO", "ISLAND", "JUNGLE",
        "GLACIER", "TUNDRA", "PRAIRIE", "SAVANNA", "MARSH",
        "SWAMP", "LAKE", "STREAM", "WATERFALL", "GEYSER",
        "CAVE", "CLIFF", "BEACH", "DUNE", "REEF",
        "FLOWER", "TREE", "BUSH", "GRASS", "FERN"
    },
    { // 7: History
        "EMPIRE", "PHARAOH", "PYRAMID", "CASTLE", "KNIGHT",
        "VIKING", "SAMURAI", "ROMAN", "GREEK", "SPARTAN",
        "AZTEC", "MAYAN", "INCA", "DYNASTY", "REVOLUTION",
        "WARRIOR", "GLADIATOR", "CRUSADE", "RENAISSANCE", "COLONY",
        "TREATY", "ALLIANCE", "MONARCH", "REPUBLIC", "SENATE",
        "CHIEFTAIN", "EMPEROR", "SULTAN", "TSAR", "KAISER"
    },
    { // 8: Elements
        "HYDROGEN", "HELIUM", "LITHIUM", "CARBON", "NITROGEN",
        "OXYGEN", "SODIUM", "SILICON", "IRON", "COPPER",
        "GOLD", "SILVER", "PLATINUM", "URANIUM", "TITANIUM",
        "NEON", "ARGON", "KRYPTON", "XENON", "RADON",
        "ZINC", "NICKEL", "COBALT", "TIN", "LEAD",
        "MERCURY", "CALCIUM", "POTASSIUM", "MAGNESIUM", "ALUMINUM"
    },
    { // 9: Sci-Fi
        "CYBORG", "ANDROID", "HOLOGRAM", "HYPERDRIVE", "TELEPORT",
        "STARSHIP", "WARP", "MUTANT", "FORCEFIELD", "NANITE",
        "CLONE", "ANTIMATTER", "QUANTUM", "SINGULARITY", "DYSON",
        "CYBERSPACE", "MECHA", "DYSTOPIA", "EXOPLANET", "TERRAFORM",
        "BIOMECH", "STEALTH", "LASER", "PLASMA", "BLASTER",
        "SERVO", "SYNTHETIC", "METAVERSE", "SIMULATION", "AVATAR"
    },
    { // 10: Myth
        "ZEUS", "HERCULES", "ODYSSEY", "THOR", "ODIN",
        "LOKI", "VALKYRIE", "APOLLO", "ATHENA", "POSEIDON",
        "MEDUSA", "MINOTAUR", "PEGASUS", "PHOENIX", "DRAGON",
        "HYDRA", "TITAN", "NEPTUNE", "VULCAN", "SPHINX",
        "GRIFFIN", "CENTAUR", "KRAKEN", "VALHALLA", "OLYMPUS",
        "HADES", "HERMES", "ACHILLES", "PANDORA", "NEMESIS"
    }
};

char grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool foundGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool hintedGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool frozenGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
int gridSize = 15;
int numWordsToFind = 8;
int currentDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int currentThemeIdx = 0;
int currentGameMode = 0; // 0=Classic, 1=Zen, 2=TimeAttack, 3=Campaign
int campaignStage = 1;
int magicWands = 3;    // Radar power-up charges
int freezeCharges = 3;  // Freeze timer power-up charges
int hintCharges = 3;    // Hint power-up charges
int freezeTimer = 0;    // Seconds remaining for timer freeze
int comboMultiplier = 1;
int timeSinceLastFind = 0;

char wordsToFind[MAX_WORDS][32];
bool wordsFoundStatus[MAX_WORDS];
bool wordsHintedStatus[MAX_WORDS];
int wordCount = 0;
int foundCount = 0;
int currentScore = 0;

bool isSelecting = false;
int startR = -1, startC = -1;
int curR = -1, curC = -1;

float cellAnim[MAX_GRID_SIZE][MAX_GRID_SIZE] = {0};
float strikeAnim[MAX_WORDS] = {0};

int timerSeconds = 0;
bool gameWon = false;
bool gameOver = false;

typedef struct {
    int completed;
    int bestTimes[NUM_THEMES][3];
} Stats;

Stats gameStats = {0};

void LoadStats() {
    FILE* fp = fopen("kwords_stats.dat", "rb");
    if (fp) {
        fread(&gameStats, sizeof(Stats), 1, fp);
        fclose(fp);
    } else {
        memset(&gameStats, 0, sizeof(Stats));
    }
}

void SaveStats() {
    FILE* fp = fopen("kwords_stats.dat", "wb");
    if (fp) {
        fwrite(&gameStats, sizeof(Stats), 1, fp);
        fclose(fp);
    }
}

RECT btnTheme  = {170, 10, 255, 35};
RECT btnMode   = {260, 10, 345, 35};
RECT btnEasy   = {350, 10, 395, 35};
RECT btnMed    = {400, 10, 455, 35};
RECT btnHard   = {460, 10, 505, 35};
RECT btnHint   = {510, 10, 555, 35};
RECT btnWand   = {560, 10, 615, 35};
RECT btnFreeze = {620, 10, 675, 35};
RECT btnSave   = {680, 10, 725, 35};
RECT btnLoad   = {730, 10, 775, 35};
RECT btnStats  = {780, 10, 830, 35};
RECT btnHelp   = {835, 10, 880, 35};

bool showStats = false;
bool showHelp = false;

DWORD WINAPI SoundTick(LPVOID lpParam) {
    Beep(1500, 10);
    return 0;
}

DWORD WINAPI SoundChime(LPVOID lpParam) {
    Beep(523, 100);
    Beep(659, 100);
    Beep(784, 200);
    return 0;
}

DWORD WINAPI SoundFanfare(LPVOID lpParam) {
    Beep(440, 150);
    Beep(554, 150);
    Beep(659, 150);
    Beep(880, 400);
    return 0;
}

void PlaySoundEffect(int type) {
    if (type == 0) CreateThread(NULL, 0, SoundTick, NULL, 0, NULL);
    else if (type == 1) CreateThread(NULL, 0, SoundChime, NULL, 0, NULL);
    else if (type == 2) CreateThread(NULL, 0, SoundFanfare, NULL, 0, NULL);
}

void InitGame() {
    srand((unsigned int)time(NULL));
    memset(foundGrid, 0, sizeof(foundGrid));
    memset(hintedGrid, 0, sizeof(hintedGrid));
    memset(frozenGrid, 0, sizeof(frozenGrid));
    memset(wordsFoundStatus, 0, sizeof(wordsFoundStatus));
    memset(wordsHintedStatus, 0, sizeof(wordsHintedStatus));
    memset(cellAnim, 0, sizeof(cellAnim));
    memset(strikeAnim, 0, sizeof(strikeAnim));
    foundCount = 0;
    if (!(currentGameMode == 3 && campaignStage > 1)) {
        currentScore = 0;
    }
    freezeTimer = 0;
    
    int numFrozen = 0;
    
    if (currentGameMode == 3) {
        // 15 Campaign Stages:
        // Stage 1: 8x8, 4 words, 120s, Theme 0 (Programming), 0 frozen
        // Stage 2: 8x8, 5 words, 120s, Theme 1 (Animals), 0 frozen
        // Stage 3: 9x9, 5 words, 130s, Theme 2 (Countries), 3 frozen
        // Stage 4: 9x9, 6 words, 130s, Theme 3 (Space), 3 frozen
        // Stage 5: 10x10, 6 words, 140s, Theme 8 (Elements), 4 frozen
        // Stage 6: 10x10, 7 words, 140s, Theme 9 (Sci-Fi), 4 frozen
        // Stage 7: 10x10, 7 words, 150s, Theme 10 (Myth), 5 frozen
        // Stage 8: 11x11, 8 words, 150s, Theme 6 (Nature), 5 frozen
        // Stage 9: 11x11, 8 words, 160s, Theme 4 (Food), 6 frozen
        // Stage 10: 11x11, 9 words, 160s, Theme 5 (Sports), 6 frozen
        // Stage 11: 12x12, 9 words, 170s, Theme 7 (History), 7 frozen
        // Stage 12: 12x12, 10 words, 170s, Theme 8 (Elements), 7 frozen
        // Stage 13: 12x12, 10 words, 180s, Theme 9 (Sci-Fi), 8 frozen
        // Stage 14: 12x12, 11 words, 180s, Theme 10 (Myth), 8 frozen
        // Stage 15: 12x12, 12 words, 200s, Theme 3 (Space), 9 frozen
        static const int stagesGrid[15]   = {8, 8, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 12};
        static const int stagesWords[15]  = {4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 12};
        static const int stagesTime[15]   = {120, 120, 130, 130, 140, 140, 150, 150, 160, 160, 170, 170, 180, 180, 200};
        static const int stagesTheme[15]  = {0, 1, 2, 3, 8, 9, 10, 6, 4, 5, 7, 8, 9, 10, 3};
        static const int stagesFrozen[15] = {0, 0, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9};

        int stg = campaignStage - 1;
        if (stg < 0) stg = 0;
        if (stg > 14) stg = 14;

        gridSize = stagesGrid[stg];
        numWordsToFind = stagesWords[stg];
        timerSeconds = stagesTime[stg];
        currentThemeIdx = stagesTheme[stg];
        numFrozen = stagesFrozen[stg];

        magicWands = 3;
        freezeCharges = 3;
        hintCharges = 3;
    } else if (currentGameMode == 2) {
        timerSeconds = (gridSize == 10) ? 120 : ((gridSize == 15) ? 180 : 300);
        numFrozen = (currentDifficulty == 0) ? 0 : ((currentDifficulty == 1) ? 3 : 6);
    } else {
        timerSeconds = 0;
        numFrozen = (currentDifficulty == 0) ? 0 : ((currentDifficulty == 1) ? 2 : 4);
    }
    comboMultiplier = 1;
    timeSinceLastFind = 0;
    
    gameWon = false;
    gameOver = false;
    
    // Fill empty
    for(int r=0; r<gridSize; r++){
        for(int c=0; c<gridSize; c++){
            grid[r][c] = ' ';
        }
    }
    
    // Pick words
    int picked[THEME_DICT_SIZE] = {0};
    wordCount = 0;
    while(wordCount < numWordsToFind) {
        int idx = rand() % THEME_DICT_SIZE;
        if(!picked[idx]) {
            picked[idx] = 1;
            strcpy(wordsToFind[wordCount], DICTIONARIES[currentThemeIdx][idx]);
            wordCount++;
        }
    }
    
    // Place words
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    for(int w=0; w<wordCount; w++) {
        bool placed = false;
        int len = strlen(wordsToFind[w]);
        int attempts = 0;
        while(!placed && attempts < 200) {
            attempts++;
            int d = rand() % 8;
            int r = rand() % gridSize;
            int c = rand() % gridSize;
            
            bool canPlace = true;
            for(int i=0; i<len; i++) {
                int nr = r + i * dirs[d][0];
                int nc = c + i * dirs[d][1];
                if(nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || (grid[nr][nc] != ' ' && grid[nr][nc] != wordsToFind[w][i])) {
                    canPlace = false;
                    break;
                }
            }
            if(canPlace) {
                for(int i=0; i<len; i++) {
                    grid[r + i * dirs[d][0]][c + i * dirs[d][1]] = wordsToFind[w][i];
                }
                placed = true;
            }
        }
    }
    
    // Fill rest
    for(int r=0; r<gridSize; r++){
        for(int c=0; c<gridSize; c++){
            if(grid[r][c] == ' '){
                grid[r][c] = 'A' + (rand() % 26);
            }
        }
    }

    // Place frozen obstacle tiles
    int frozenPlaced = 0;
    int fAttempts = 0;
    while (frozenPlaced < numFrozen && fAttempts < 300) {
        fAttempts++;
        int fr = rand() % gridSize;
        int fc = rand() % gridSize;
        if (!frozenGrid[fr][fc]) {
            frozenGrid[fr][fc] = true;
            frozenPlaced++;
        }
    }
}

int my_sign(int x) { return (x > 0) - (x < 0); }
int my_max(int a, int b) { return a > b ? a : b; }
int my_abs(int x) { return x < 0 ? -x : x; }

void GetLineCells(int r1, int c1, int r2, int c2, int* outR, int* outC, int* count) {
    *count = 0;
    int dr = my_sign(r2 - r1);
    int dc = my_sign(c2 - c1);
    int dist = my_max(my_abs(r2 - r1), my_abs(c2 - c1));
    
    if (my_abs(r2 - r1) != my_abs(c2 - c1) && r1 != r2 && c1 != c2) return;
    
    for (int i = 0; i <= dist; i++) {
        outR[*count] = r1 + i * dr;
        outC[*count] = c1 + i * dc;
        (*count)++;
    }
}

void EndSelection(HWND hwnd) {
    if (!isSelecting) return;
    isSelecting = false;
    
    int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
    int count;
    GetLineCells(startR, startC, curR, curC, selR, selC, &count);
    
    if (count > 0) {
        char selWord[32] = {0};
        char revWord[32] = {0};
        for(int i=0; i<count; i++) {
            selWord[i] = grid[selR[i]][selC[i]];
            revWord[count - 1 - i] = grid[selR[i]][selC[i]];
        }
        
        bool found = false;
        for(int w=0; w<wordCount; w++) {
            if(!wordsFoundStatus[w]) {
                if(strcmp(wordsToFind[w], selWord) == 0 || strcmp(wordsToFind[w], revWord) == 0) {
                    wordsFoundStatus[w] = true;
                    foundCount++;
                    found = true;
                    
                    if (currentGameMode != 1) {
                        int points = 1000 - (timerSeconds * 2);
                        if (currentGameMode == 2 || currentGameMode == 3) points = 500;
                        if (points < 100) points = 100;
                        if (timeSinceLastFind < 15) comboMultiplier++;
                        else comboMultiplier = 1;
                        currentScore += points * comboMultiplier;
                        timeSinceLastFind = 0;
                    }
                    
                    // Mark cells as found and thaw touching frozen tiles
                    for(int i=0; i<count; i++) {
                        int r = selR[i];
                        int c = selC[i];
                        foundGrid[r][c] = true;
                        
                        for (int dr = -1; dr <= 1; dr++) {
                            for (int dc = -1; dc <= 1; dc++) {
                                int nr = r + dr;
                                int nc = c + dc;
                                if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize) {
                                    if (frozenGrid[nr][nc]) {
                                        frozenGrid[nr][nc] = false;
                                        currentScore += 100; // Bonus points for thawing tiles
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
        
        if (foundCount == wordCount) {
            if (currentGameMode == 3 && campaignStage < 15) {
                campaignStage++;
                PlaySoundEffect(2);
                InitGame();
                return;
            } else {
                gameWon = true;
                PlaySoundEffect(2);
                LoadStats();
                gameStats.completed++;
                int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
                if (bTime == 0 || timerSeconds < bTime) {
                    gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
                }
                SaveStats();
            }
        } else if (found) {
            PlaySoundEffect(1);
        }
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
}

void UseWand(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (currentGameMode == 3 && magicWands <= 0) return;
    if (currentGameMode != 3 && currentScore < 200 && currentGameMode != 1) return;
    
    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w]) {
            unfound[uncount++] = w;
        }
    }
    
    if (uncount == 0) return;
    
    int targetIdx = unfound[rand() % uncount];
    
    if (currentGameMode == 3) magicWands--;
    else if (currentGameMode != 1) currentScore -= 200;
    
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    int len = strlen(wordsToFind[targetIdx]);
    bool foundInGrid = false;
    for (int r = 0; r < gridSize && !foundInGrid; r++) {
        for (int c = 0; c < gridSize && !foundInGrid; c++) {
            if (grid[r][c] == wordsToFind[targetIdx][0]) {
                for (int d = 0; d < 8; d++) {
                    bool match = true;
                    for (int i = 0; i < len; i++) {
                        int nr = r + i * dirs[d][0];
                        int nc = c + i * dirs[d][1];
                        if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || grid[nr][nc] != wordsToFind[targetIdx][i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        for (int i = 0; i < len; i++) {
                            int fr = r + i * dirs[d][0];
                            int fc = c + i * dirs[d][1];
                            foundGrid[fr][fc] = true;
                            // Thaw adjacent frozen tiles
                            for (int dr = -1; dr <= 1; dr++) {
                                for (int dc = -1; dc <= 1; dc++) {
                                    int nr = fr + dr;
                                    int nc = fc + dc;
                                    if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize) {
                                        if (frozenGrid[nr][nc]) frozenGrid[nr][nc] = false;
                                    }
                                }
                            }
                        }
                        wordsFoundStatus[targetIdx] = true;
                        foundCount++;
                        PlaySoundEffect(1);
                        foundInGrid = true;
                        break;
                    }
                }
            }
        }
    }
    
    if (foundCount == wordCount) {
        if (currentGameMode == 3 && campaignStage < 15) {
            campaignStage++;
            PlaySoundEffect(2);
            InitGame();
        } else {
            gameWon = true;
            PlaySoundEffect(2);
            LoadStats();
            gameStats.completed++;
            int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
            if (bTime == 0 || timerSeconds < bTime) {
                gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
            }
            SaveStats();
        }
    }
    
    InvalidateRect(hwnd, NULL, FALSE);
}

void UseFreeze(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (freezeTimer > 0) return;
    if (currentGameMode == 3 && freezeCharges <= 0) return;
    if (currentGameMode != 3 && currentScore < 100 && currentGameMode != 1) return;
    
    if (currentGameMode == 3) freezeCharges--;
    else if (currentGameMode != 1) currentScore -= 100;
    
    freezeTimer = 15;
    PlaySoundEffect(0);
    InvalidateRect(hwnd, NULL, FALSE);
}

void UseHint(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (currentGameMode == 3 && hintCharges <= 0) return;
    
    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w] && !wordsHintedStatus[w]) {
            unfound[uncount++] = w;
        }
    }
    
    if (uncount == 0) {
        for(int w=0; w<wordCount; w++) {
            if(!wordsFoundStatus[w]) {
                unfound[uncount++] = w;
            }
        }
    }
    
    if (uncount == 0) return;
    
    int targetIdx = unfound[rand() % uncount];
    if (currentGameMode == 3) hintCharges--;
    
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    int len = strlen(wordsToFind[targetIdx]);
    bool foundInGrid = false;
    for (int r = 0; r < gridSize && !foundInGrid; r++) {
        for (int c = 0; c < gridSize && !foundInGrid; c++) {
            if (grid[r][c] == wordsToFind[targetIdx][0]) {
                for (int d = 0; d < 8; d++) {
                    bool match = true;
                    for (int i = 0; i < len; i++) {
                        int nr = r + i * dirs[d][0];
                        int nc = c + i * dirs[d][1];
                        if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || grid[nr][nc] != wordsToFind[targetIdx][i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        hintedGrid[r][c] = true;
                        wordsHintedStatus[targetIdx] = true;
                        if (currentGameMode != 1 && currentGameMode != 3) {
                            currentScore -= 50;
                            if (currentScore < 0) currentScore = 0;
                            if (currentGameMode == 0) timerSeconds += 30;
                            else if (currentGameMode == 2) {
                                timerSeconds -= 30;
                                if (timerSeconds < 0) timerSeconds = 0;
                            }
                        }
                        foundInGrid = true;
                        PlaySoundEffect(0);
                        break;
                    }
                }
            }
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void SaveGame(HWND hwnd) {
    FILE* fp = fopen("kwords_save.dat", "wb");
    if (fp) {
        fwrite(&gridSize, sizeof(int), 1, fp);
        fwrite(&numWordsToFind, sizeof(int), 1, fp);
        fwrite(&currentDifficulty, sizeof(int), 1, fp);
        fwrite(&currentThemeIdx, sizeof(int), 1, fp);
        fwrite(&currentGameMode, sizeof(int), 1, fp);
        fwrite(&campaignStage, sizeof(int), 1, fp);
        fwrite(&magicWands, sizeof(int), 1, fp);
        fwrite(&freezeCharges, sizeof(int), 1, fp);
        fwrite(&hintCharges, sizeof(int), 1, fp);
        fwrite(&wordCount, sizeof(int), 1, fp);
        fwrite(&foundCount, sizeof(int), 1, fp);
        fwrite(&currentScore, sizeof(int), 1, fp);
        fwrite(&timerSeconds, sizeof(int), 1, fp);
        fwrite(&gameWon, sizeof(bool), 1, fp);
        fwrite(&gameOver, sizeof(bool), 1, fp);
        fwrite(grid, sizeof(char), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(foundGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(hintedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(frozenGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(wordsToFind, sizeof(char), MAX_WORDS * 32, fp);
        fwrite(wordsFoundStatus, sizeof(bool), MAX_WORDS, fp);
        fwrite(wordsHintedStatus, sizeof(bool), MAX_WORDS, fp);
        fclose(fp);
        MessageBox(hwnd, "Game Saved successfully!", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK | MB_ICONERROR);
    }
}

void LoadGame(HWND hwnd) {
    FILE* fp = fopen("kwords_save.dat", "rb");
    if (fp) {
        fread(&gridSize, sizeof(int), 1, fp);
        fread(&numWordsToFind, sizeof(int), 1, fp);
        fread(&currentDifficulty, sizeof(int), 1, fp);
        fread(&currentThemeIdx, sizeof(int), 1, fp);
        fread(&currentGameMode, sizeof(int), 1, fp);
        fread(&campaignStage, sizeof(int), 1, fp);
        fread(&magicWands, sizeof(int), 1, fp);
        fread(&freezeCharges, sizeof(int), 1, fp);
        fread(&hintCharges, sizeof(int), 1, fp);
        fread(&wordCount, sizeof(int), 1, fp);
        fread(&foundCount, sizeof(int), 1, fp);
        fread(&currentScore, sizeof(int), 1, fp);
        fread(&timerSeconds, sizeof(int), 1, fp);
        fread(&gameWon, sizeof(bool), 1, fp);
        fread(&gameOver, sizeof(bool), 1, fp);
        fread(grid, sizeof(char), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(foundGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(hintedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(frozenGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(wordsToFind, sizeof(char), MAX_WORDS * 32, fp);
        fread(wordsFoundStatus, sizeof(bool), MAX_WORDS, fp);
        fread(wordsHintedStatus, sizeof(bool), MAX_WORDS, fp);
        fclose(fp);
        
        memset(cellAnim, 0, sizeof(cellAnim));
        memset(strikeAnim, 0, sizeof(strikeAnim));
        isSelecting = false;
        
        for(int w=0; w<wordCount; w++) {
            if (wordsFoundStatus[w]) strikeAnim[w] = 1.0f;
        }
        for(int r=0; r<gridSize; r++){
            for(int c=0; c<gridSize; c++){
                if(foundGrid[r][c]) cellAnim[r][c] = 1.0f;
            }
        }
        
        InvalidateRect(hwnd, NULL, TRUE);
    } else {
        MessageBox(hwnd, "No save file found.", "Load", MB_OK | MB_ICONINFORMATION);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 1000, NULL);
            SetTimer(hwnd, 2, 30, NULL);
            break;
        case WM_TIMER:
            if(wParam == 1) {
                if(!gameWon && !gameOver) {
                    if (freezeTimer > 0) {
                        freezeTimer--;
                    } else {
                        if (currentGameMode == 2 || currentGameMode == 3) {
                            if (timerSeconds > 0) {
                                timerSeconds--;
                            } else {
                                gameOver = true;
                                PlaySoundEffect(1);
                            }
                        } else if (currentGameMode == 0) {
                            timerSeconds++;
                        }
                    }
                    timeSinceLastFind++;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 2) {
                bool needsRedraw = false;
                int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
                int selCount = 0;
                if (isSelecting) {
                    GetLineCells(startR, startC, curR, curC, selR, selC, &selCount);
                }
                for(int r=0; r<gridSize; r++) {
                    for(int c=0; c<gridSize; c++) {
                        bool isSel = false;
                        for(int i=0; i<selCount; i++) {
                            if(selR[i] == r && selC[i] == c) { isSel = true; break; }
                        }
                        bool isFound = foundGrid[r][c];
                        float target = (isSel || isFound) ? 1.0f : 0.0f;
                        if(cellAnim[r][c] < target) { cellAnim[r][c] += 0.2f; if(cellAnim[r][c]>1.0f) cellAnim[r][c]=1.0f; needsRedraw = true; }
                        else if(cellAnim[r][c] > target) { cellAnim[r][c] -= 0.2f; if(cellAnim[r][c]<0.0f) cellAnim[r][c]=0.0f; needsRedraw = true; }
                    }
                }
                for(int w=0; w<wordCount; w++) {
                    float target = wordsFoundStatus[w] ? 1.0f : 0.0f;
                    if(strikeAnim[w] < target) { strikeAnim[w] += 0.1f; if(strikeAnim[w]>1.0f) strikeAnim[w]=1.0f; needsRedraw = true; }
                }
                if(needsRedraw) InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == 'R' || wParam == 'r') UseWand(hwnd);
            else if (wParam == 'F' || wParam == 'f') UseFreeze(hwnd);
            else if (wParam == 'H' || wParam == 'h') UseHint(hwnd);
            else if (wParam == 'W' || wParam == 'w') UseWand(hwnd);
            else if (wParam == 'S' || wParam == 's') SaveGame(hwnd);
            else if (wParam == 'L' || wParam == 'l') LoadGame(hwnd);
            break;
        case WM_LBUTTONDOWN: {
            if (showStats) {
                showStats = false;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (showHelp) {
                showHelp = false;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            if (PtInRect(&btnTheme, pt)) {
                currentThemeIdx = (currentThemeIdx + 1) % NUM_THEMES; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnMode, pt)) {
                currentGameMode = (currentGameMode + 1) % 4; if (currentGameMode == 3) { campaignStage = 1; magicWands = 3; freezeCharges = 3; hintCharges = 3; currentScore = 0; } InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnEasy, pt)) {
                currentDifficulty = 0; gridSize = 10; numWordsToFind = 5; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnMed, pt)) {
                currentDifficulty = 1; gridSize = 15; numWordsToFind = 8; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnHard, pt)) {
                currentDifficulty = 2; gridSize = 20; numWordsToFind = 12; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnHint, pt)) {
                UseHint(hwnd); break;
            }
            if (PtInRect(&btnWand, pt)) {
                UseWand(hwnd); break;
            }
            if (PtInRect(&btnFreeze, pt)) {
                UseFreeze(hwnd); break;
            }
            if (PtInRect(&btnSave, pt)) {
                SaveGame(hwnd); break;
            }
            if (PtInRect(&btnLoad, pt)) {
                LoadGame(hwnd); break;
            }
            if (PtInRect(&btnStats, pt)) {
                LoadStats();
                showStats = true;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (PtInRect(&btnHelp, pt)) {
                showHelp = true;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            if(gameWon || gameOver) {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int c = (x - 20) / CELL_SIZE;
            int r = (y - 50) / CELL_SIZE;
            if(r >= 0 && r < gridSize && c >= 0 && c < gridSize) {
                isSelecting = true;
                startR = curR = r;
                startC = curC = c;
                PlaySoundEffect(0);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if(isSelecting) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                int c = (x - 20) / CELL_SIZE;
                int r = (y - 50) / CELL_SIZE;
                if(r >= 0 && r < gridSize && c >= 0 && c < gridSize) {
                    if(curR != r || curC != c) {
                        int oldR = curR, oldC = curC;
                        curR = r;
                        curC = c;
                        int dummyR[MAX_GRID_SIZE*2], dummyC[MAX_GRID_SIZE*2];
                        int oldSelCount = 0, newSelCount = 0;
                        GetLineCells(startR, startC, oldR, oldC, dummyR, dummyC, &oldSelCount);
                        GetLineCells(startR, startC, curR, curC, dummyR, dummyC, &newSelCount);
                        if (oldSelCount != newSelCount) {
                            PlaySoundEffect(0);
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_LBUTTONUP:
            EndSelection(hwnd);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 17, 26));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            char header[160];
            if (currentGameMode == 1) {
                sprintf(header, "Found: %d/%d   Score: -   Timer: --:--", foundCount, wordCount);
            } else if (currentGameMode == 3) {
                if (freezeTimer > 0) {
                    sprintf(header, "Stage: %d/15   Found: %d/%d   Score: %d   Timer: FROZEN (%ds)   R:%d F:%d H:%d", campaignStage, foundCount, wordCount, currentScore, freezeTimer, magicWands, freezeCharges, hintCharges);
                } else {
                    sprintf(header, "Stage: %d/15   Found: %d/%d   Score: %d   Timer: %02d:%02d   R:%d F:%d H:%d", campaignStage, foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60, magicWands, freezeCharges, hintCharges);
                }
            } else {
                if (freezeTimer > 0) {
                    sprintf(header, "Found: %d/%d   Score: %d   Timer: FROZEN (%ds)", foundCount, wordCount, currentScore, freezeTimer);
                } else {
                    sprintf(header, "Found: %d/%d   Score: %d   Timer: %02d:%02d", foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60);
                }
            }
            TextOut(hdc, 20, 15, header, strlen(header));
            
            // Draw top bar buttons
            HBRUSH btnBrush = CreateSolidBrush(RGB(45, 55, 72));
            HBRUSH activeBrush = CreateSolidBrush(RGB(49, 130, 206));
            
            FillRect(hdc, &btnTheme, btnBrush);
            FillRect(hdc, &btnMode, btnBrush);
            FillRect(hdc, &btnEasy, currentDifficulty == 0 ? activeBrush : btnBrush);
            FillRect(hdc, &btnMed, currentDifficulty == 1 ? activeBrush : btnBrush);
            FillRect(hdc, &btnHard, currentDifficulty == 2 ? activeBrush : btnBrush);
            
            HBRUSH hintBrush = CreateSolidBrush(RGB(214, 158, 46));
            FillRect(hdc, &btnHint, hintBrush);
            DeleteObject(hintBrush);
            
            HBRUSH wandBrush = CreateSolidBrush(RGB(159, 122, 234));
            FillRect(hdc, &btnWand, wandBrush);
            DeleteObject(wandBrush);
            
            HBRUSH freezeBtnBrush = CreateSolidBrush(RGB(56, 178, 172));
            FillRect(hdc, &btnFreeze, freezeBtnBrush);
            DeleteObject(freezeBtnBrush);
            
            HBRUSH slBrush = CreateSolidBrush(RGB(100, 100, 100));
            FillRect(hdc, &btnSave, slBrush);
            FillRect(hdc, &btnLoad, slBrush);
            FillRect(hdc, &btnStats, slBrush);
            FillRect(hdc, &btnHelp, slBrush);
            DeleteObject(slBrush);
            
            char themeStr[64];
            sprintf(themeStr, "Theme: %s", THEMES[currentThemeIdx]);
            TextOut(hdc, btnTheme.left + 4, btnTheme.top + 5, themeStr, strlen(themeStr));

            const char* MODES[4] = {"Classic", "Zen", "Time Attack", "Campaign"};
            char modeStr[64];
            if (currentGameMode == 3) sprintf(modeStr, "Stg %d", campaignStage);
            else sprintf(modeStr, "Mode: %s", MODES[currentGameMode]);
            TextOut(hdc, btnMode.left + 4, btnMode.top + 5, modeStr, strlen(modeStr));
            
            TextOut(hdc, btnEasy.left + 6, btnEasy.top + 5, "Easy", 4);
            TextOut(hdc, btnMed.left + 5, btnMed.top + 5, "Medium", 6);
            TextOut(hdc, btnHard.left + 6, btnHard.top + 5, "Hard", 4);
            
            SetTextColor(hdc, RGB(26, 32, 44));
            TextOut(hdc, btnHint.left + 6, btnHint.top + 5, "Hint", 4);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOut(hdc, btnWand.left + 5, btnWand.top + 5, "Radar", 5);
            SetTextColor(hdc, RGB(26, 32, 44));
            TextOut(hdc, btnFreeze.left + 4, btnFreeze.top + 5, "Freeze", 6);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            TextOut(hdc, btnSave.left + 5, btnSave.top + 5, "Save", 4);
            TextOut(hdc, btnLoad.left + 5, btnLoad.top + 5, "Load", 4);
            TextOut(hdc, btnStats.left + 6, btnStats.top + 5, "Stats", 5);
            TextOut(hdc, btnHelp.left + 5, btnHelp.top + 5, "Help", 4);
            
            DeleteObject(btnBrush);
            DeleteObject(activeBrush);
            
            int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
            int selCount = 0;
            if (isSelecting) {
                GetLineCells(startR, startC, curR, curC, selR, selC, &selCount);
            }
            
            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(45, 55, 72));
            HPEN hOldPen = (HPEN)SelectObject(hdc, gridPen);
            
            for(int r=0; r<gridSize; r++) {
                for(int c=0; c<gridSize; c++) {
                    RECT rc = { 20 + c*CELL_SIZE, 50 + r*CELL_SIZE, 20 + (c+1)*CELL_SIZE, 50 + (r+1)*CELL_SIZE };
                    
                    bool isSelected = false;
                    for(int i=0; i<selCount; i++) {
                        if(selR[i] == r && selC[i] == c) {
                            isSelected = true; break;
                        }
                    }
                    
                    if (cellAnim[r][c] > 0.0f) {
                        int cx = (rc.left + rc.right) / 2;
                        int cy = (rc.top + rc.bottom) / 2;
                        int cw = (rc.right - rc.left) / 2;
                        int ch = (rc.bottom - rc.top) / 2;
                        int w = (int)(cw * cellAnim[r][c]);
                        int h = (int)(ch * cellAnim[r][c]);
                        RECT animRc = { cx - w, cy - h, cx + w, cy + h };
                        COLORREF color = isSelected ? RGB(49, 130, 206) : RGB(56, 161, 105);
                        HBRUSH animBrush = CreateSolidBrush(color);
                        FillRect(hdc, &animRc, animBrush);
                        DeleteObject(animBrush);
                    } else if (frozenGrid[r][c]) {
                        HBRUSH hFrozen = CreateSolidBrush(RGB(79, 209, 197));
                        FillRect(hdc, &rc, hFrozen);
                        DeleteObject(hFrozen);
                    } else if (hintedGrid[r][c]) {
                        HBRUSH hHint = CreateSolidBrush(RGB(214, 158, 46));
                        FillRect(hdc, &rc, hHint);
                        DeleteObject(hHint);
                    }
                    
                    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                    
                    if (frozenGrid[r][c] && cellAnim[r][c] == 0.0f) SetTextColor(hdc, RGB(20, 40, 60));
                    else if (hintedGrid[r][c] && cellAnim[r][c] == 0.0f) SetTextColor(hdc, RGB(26, 32, 44));
                    else SetTextColor(hdc, RGB(226, 232, 240));
                    
                    char ch[2] = { grid[r][c], 0 };
                    DrawText(hdc, ch, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            
            int listX = 20 + gridSize*CELL_SIZE + 30;
            int listY = 50;
            TextOut(hdc, listX, listY, "Words to Find:", 14);
            listY += 25;
            
            for(int w=0; w<wordCount; w++) {
                if(wordsFoundStatus[w]) {
                    SetTextColor(hdc, RGB(100, 100, 100));
                } else {
                    SetTextColor(hdc, RGB(255, 255, 255));
                }
                TextOut(hdc, listX, listY, wordsToFind[w], strlen(wordsToFind[w]));
                
                if(strikeAnim[w] > 0.0f) {
                    HPEN strikePen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
                    HPEN oldP = (HPEN)SelectObject(hdc, strikePen);
                    MoveToEx(hdc, listX, listY + 10, NULL);
                    int strikeLen = (int)(strlen(wordsToFind[w]) * 9 * strikeAnim[w]);
                    LineTo(hdc, listX + strikeLen, listY + 10);
                    SelectObject(hdc, oldP);
                    DeleteObject(strikePen);
                }
                
                listY += 25;
            }
            
            if(gameWon) {
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOut(hdc, listX, listY + 20, "YOU WIN!", 8);
                SetTextColor(hdc, RGB(200, 200, 200));
                char scoreStr[64];
                sprintf(scoreStr, "Score: %d", currentScore);
                TextOut(hdc, listX, listY + 50, scoreStr, strlen(scoreStr));
                TextOut(hdc, listX, listY + 75, "Click anywhere", 14);
                TextOut(hdc, listX, listY + 95, "to play again", 13);
            } else if (gameOver) {
                SetTextColor(hdc, RGB(255, 100, 100));
                TextOut(hdc, listX, listY + 20, "GAME OVER!", 10);
                SetTextColor(hdc, RGB(200, 200, 200));
                TextOut(hdc, listX, listY + 50, "Time's up!", 10);
                TextOut(hdc, listX, listY + 75, "Click anywhere", 14);
                TextOut(hdc, listX, listY + 95, "to try again", 12);
            }
            
            if(showStats) {
                RECT modalRc = { 100, 100, 700, 700 };
                HBRUSH modalBrush = CreateSolidBrush(RGB(30, 33, 43));
                FillRect(hdc, &modalRc, modalBrush);
                DeleteObject(modalBrush);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 120, 120, "Statistics", 10);
                char statStr[128];
                sprintf(statStr, "Puzzles Completed: %d", gameStats.completed);
                TextOut(hdc, 120, 160, statStr, strlen(statStr));
                
                int yPos = 200;
                TextOut(hdc, 120, yPos, "Best Times:", 11);
                yPos += 30;
                const char* diffNames[] = {"Easy", "Medium", "Hard"};
                for(int t=0; t<NUM_THEMES; t++) {
                    for(int d=0; d<3; d++) {
                        int bTime = gameStats.bestTimes[t][d];
                        if(bTime > 0) {
                            sprintf(statStr, "%s - %s: %02d:%02d", THEMES[t], diffNames[d], bTime / 60, bTime % 60);
                            TextOut(hdc, 140, yPos, statStr, strlen(statStr));
                            yPos += 25;
                        }
                    }
                }
                TextOut(hdc, 120, 650, "Click anywhere to close stats", 29);
            }
            
            if(showHelp) {
                RECT modalRc = { 100, 100, 700, 700 };
                HBRUSH modalBrush = CreateSolidBrush(RGB(30, 33, 43));
                FillRect(hdc, &modalRc, modalBrush);
                DeleteObject(modalBrush);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 120, 120, "How to Play KWords", 18);
                TextOut(hdc, 120, 160, "Goal: Find all the hidden words in the grid!", 44);
                TextOut(hdc, 140, 200, "- Words can be horizontal, vertical, or diagonal.", 49);
                TextOut(hdc, 140, 230, "- They can be spelled forwards or backwards.", 44);
                TextOut(hdc, 140, 260, "- Click and drag to select a word in the grid.", 46);
                
                TextOut(hdc, 120, 300, "Modes & Difficulties", 20);
                TextOut(hdc, 140, 330, "- Campaign: 15 stages with frozen tiles & power-ups.", 52);
                TextOut(hdc, 140, 360, "- Time Attack / Zen / Classic modes available.", 46);
                
                SetTextColor(hdc, RGB(214, 158, 46));
                TextOut(hdc, 120, 400, "Power-Ups (Hotkeys H, R/W, F):", 30);
                TextOut(hdc, 140, 430, "H: Hint / First Letter - flashes starting letter.", 49);
                TextOut(hdc, 140, 460, "R/W: Radar / Reveal Word - solves a word.", 41);
                TextOut(hdc, 140, 490, "F: Freeze Timer - pauses timer for 15s.", 39);
                
                SetTextColor(hdc, RGB(79, 209, 197));
                TextOut(hdc, 120, 540, "Frozen Tiles:", 13);
                TextOut(hdc, 140, 570, "Thaw locked tiles by finding words touching them!", 50);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 120, 650, "Click anywhere to close help", 28);
            }
            
            SelectObject(hdc, hOldPen);
            DeleteObject(gridPen);
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
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
    wc.lpszClassName = "KWordsClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 0;
    
    HWND hwnd = CreateWindow("KWordsClass", "KWords", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT, 920, 850,
                             NULL, NULL, hInstance, NULL);
                             
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}
