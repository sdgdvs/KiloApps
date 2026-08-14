#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAX_GRID_SIZE 26
#define MAX_WORDS 25
#define NUM_THEMES 12
#define THEME_DICT_SIZE 30
#define NUM_CAMPAIGN_STAGES 20

const char* THEMES[NUM_THEMES] = {
    "Elements", "Sci-Fi", "Myth", "Animals", "Science", 
    "Geography", "History", "Literature", "Tech", "Food", "Astronomy", "Sports"
};

const char* DICTIONARIES[NUM_THEMES][THEME_DICT_SIZE] = {
    { // 0: Elements
        "HYDROGEN", "HELIUM", "LITHIUM", "CARBON", "NITROGEN",
        "OXYGEN", "SODIUM", "SILICON", "IRON", "COPPER",
        "GOLD", "SILVER", "PLATINUM", "URANIUM", "TITANIUM",
        "NEON", "ARGON", "KRYPTON", "XENON", "RADON",
        "ZINC", "NICKEL", "COBALT", "TIN", "LEAD",
        "MERCURY", "CALCIUM", "POTASSIUM", "MAGNESIUM", "ALUMINUM"
    },
    { // 1: Sci-Fi
        "CYBORG", "ANDROID", "HOLOGRAM", "HYPERDRIVE", "TELEPORT",
        "STARSHIP", "WARP", "MUTANT", "FORCEFIELD", "NANITE",
        "CLONE", "ANTIMATTER", "QUANTUM", "SINGULARITY", "DYSON",
        "CYBERSPACE", "MECHA", "DYSTOPIA", "EXOPLANET", "TERRAFORM",
        "BIOMECH", "STEALTH", "LASER", "PLASMA", "BLASTER",
        "SERVO", "SYNTHETIC", "METAVERSE", "SIMULATION", "AVATAR"
    },
    { // 2: Myth
        "ZEUS", "HERCULES", "ODYSSEY", "THOR", "ODIN",
        "LOKI", "VALKYRIE", "APOLLO", "ATHENA", "POSEIDON",
        "MEDUSA", "MINOTAUR", "PEGASUS", "PHOENIX", "DRAGON",
        "HYDRA", "TITAN", "NEPTUNE", "VULCAN", "SPHINX",
        "GRIFFIN", "CENTAUR", "KRAKEN", "VALHALLA", "OLYMPUS",
        "HADES", "HERMES", "ACHILLES", "PANDORA", "NEMESIS"
    },
    { // 3: Animals
        "ELEPHANT", "GIRAFFE", "PENGUIN", "KANGAROO", "DOLPHIN",
        "TIGER", "CHEETAH", "MONKEY", "OSTRICH", "IGUANA",
        "ZEBRA", "GORILLA", "PANTHER", "LEOPARD", "HIPPO",
        "RHINO", "CROCODILE", "ALLIGATOR", "CHIMPANZEE", "SNAIL",
        "OCTOPUS", "SHARK", "WHALE", "WALRUS", "SEAL",
        "BEAR", "WOLF", "FOX", "RABBIT", "DEER"
    },
    { // 4: Science
        "PHYSICS", "CHEMISTRY", "BIOLOGY", "MOLECULE", "GENETICS",
        "QUANTUM", "GRAVITY", "MAGNET", "ENERGY", "ELECTRON",
        "PROTON", "NEUTRON", "NUCLEUS", "FOSSIL", "MICROBE",
        "VIRUS", "BACTERIA", "GENOME", "ENZYME", "COMPOUND",
        "PHOSPHATE", "REACTION", "THERMAL", "OPTICS", "ISOTOPE",
        "LABORATORY", "KINETIC", "CATALYST", "MUTATION", "SPECTRUM"
    },
    { // 5: Geography
        "CONTINENT", "EQUATOR", "MOUNTAIN", "ARCHIPELAGO", "PENINSULA",
        "CAPITAL", "GLACIER", "VOLCANO", "ISTHMUS", "LATITUDE",
        "LONGITUDE", "MERIDIAN", "ISLAND", "TUNDRA", "SAVANNA",
        "TROPIC", "DELTA", "CANYON", "REEF", "FJORD",
        "PLATEAU", "VALLEY", "DESERT", "CHANNEL", "HARBOR",
        "BASIN", "ESTUARY", "ATMOSPHERE", "TOPOGRAPHY", "OCEANIC"
    },
    { // 6: History
        "EMPIRE", "PHARAOH", "PYRAMID", "CASTLE", "KNIGHT",
        "VIKING", "SAMURAI", "ROMAN", "GREEK", "SPARTAN",
        "AZTEC", "MAYAN", "INCA", "DYNASTY", "REVOLUTION",
        "WARRIOR", "GLADIATOR", "CRUSADE", "RENAISSANCE", "COLONY",
        "TREATY", "ALLIANCE", "MONARCH", "REPUBLIC", "SENATE",
        "CHIEFTAIN", "EMPEROR", "SULTAN", "TSAR", "KAISER"
    },
    { // 7: Literature
        "NOVEL", "POETRY", "FICTION", "CHAPTER", "AUTHOR",
        "SHAKESPEARE", "TRAGEDY", "COMEDY", "DRAMA", "PROSE",
        "SONNET", "STANZA", "EPIC", "FOIL", "METAPHOR",
        "SYMBOL", "NARRATIVE", "DIALOGUE", "HARDBACK", "FOLKLORE",
        "PARABLE", "MONOLOGUE", "ANTHOLOGY", "MEMOIR", "ESSAY",
        "ALLEGORY", "SATIRE", "PROTAGONIST", "GENRE", "SOLILOQUY"
    },
    { // 8: Tech
        "ALGORITHM", "COMPILER", "DEBUG", "FUNCTION", "VARIABLE",
        "POINTER", "SYNTAX", "OBJECT", "CLASS", "METHOD",
        "ARRAY", "STRING", "BOOLEAN", "INTEGER", "FLOAT",
        "NETWORK", "SERVER", "DATABASE", "CLIENT", "PROTOCOL",
        "ROUTER", "BROWSER", "KERNEL", "MEMORY", "THREAD",
        "PROCESS", "SOCKET", "PACKET", "CACHE", "FRAMEWORK"
    },
    { // 9: Food
        "PIZZA", "BURGER", "SALAD", "PASTA", "SUSHI",
        "STEAK", "CHEESE", "BREAD", "APPLE", "BANANA",
        "ORANGE", "GRAPE", "CHICKEN", "BACON", "TOMATO",
        "POTATO", "ONION", "GARLIC", "PEPPER", "CARROT",
        "CEREAL", "WAFFLE", "PANCAKE", "MUFFIN", "COOKIE",
        "CHOCOLATE", "VANILLA", "BUTTER", "YOGURT", "HONEY"
    },
    { // 10: Astronomy
        "ASTEROID", "COMET", "GALAXY", "NEBULA", "PLANET",
        "STAR", "ORBIT", "SATELLITE", "ROCKET", "GRAVITY",
        "ECLIPSE", "METEOR", "UNIVERSE", "COSMOS", "PULSAR",
        "QUASAR", "SUPERNOVA", "VACUUM", "EQUATOR", "HORIZON",
        "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT",
        "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"
    },
    { // 11: Sports
        "SOCCER", "TENNIS", "BASKETBALL", "BASEBALL", "GOLF",
        "RUGBY", "CRICKET", "HOCKEY", "VOLLEYBALL", "SWIMMING",
        "BOXING", "WRESTLING", "CYCLING", "ATHLETICS", "GYMNASTICS",
        "ARCHERY", "FENCING", "BOWLING", "BILLIARDS", "SNOOKER",
        "DARTS", "KARATE", "JUDO", "TAEKWONDO", "SURFING",
        "SKATING", "SKIING", "SNOWBOARD", "ROWING", "SAILING"
    }
};

const char* SECRET_WORDS_BANK[] = {
    "BONUS", "SECRET", "HIDDEN", "JEWEL", "MAGIC",
    "KILO", "SUPER", "HERO", "LUCKY", "PRIZE"
};
#define NUM_SECRET_BANK 10

typedef struct {
    int grid;
    int words;
    int time;
    int theme; // -1 for Polyglot (mixed)
    int frozen;
    int fog; // 0 = false, 1 = true
    const char* title;
} CampaignStageDef;

static const CampaignStageDef CAMPAIGN_STAGES_DEF[NUM_CAMPAIGN_STAGES] = {
    {10,  5, 120,  9, 0, 0, "Stage 1: Culinary Starter"},        // Food
    {10,  6, 120,  3, 0, 0, "Stage 2: Wildlife Search"},        // Animals
    {11,  6, 130, 11, 2, 0, "Stage 3: Sports Arena"},           // Sports
    {11,  7, 130,  5, 2, 0, "Stage 4: Geographic Expedition"},  // Geography
    {12,  7, 140,  8, 3, 0, "Stage 5: Tech Horizon"},           // Tech
    {12,  8, 140,  0, 3, 1, "Stage 6: Element Shroud"},         // Elements (Fog)
    {13,  8, 150,  4, 4, 0, "Stage 7: Scientific Discovery"},   // Science
    {13,  9, 150,  6, 4, 1, "Stage 8: Ancient History"},        // History (Fog)
    {14,  9, 160,  7, 5, 0, "Stage 9: Literary Classics"},      // Literature
    {14, 10, 160,  1, 5, 1, "Stage 10: Cyber Sci-Fi"},          // Sci-Fi (Fog)
    {15, 10, 170,  2, 6, 0, "Stage 11: Mythic Legends"},        // Myth
    {15, 11, 170, 10, 6, 1, "Stage 12: Celestial Astronomy"},   // Astronomy (Fog)
    {16, 11, 180,  9, 7, 0, "Stage 13: Gourmet Feast"},          // Food
    {16, 12, 180,  3, 7, 1, "Stage 14: Deep Jungle Safari"},    // Animals (Fog)
    {17, 12, 190,  8, 8, 0, "Stage 15: Quantum Computing"},     // Tech
    {17, 13, 190,  4, 8, 1, "Stage 16: Genetic Frontier"},      // Science (Fog)
    {18, 13, 200,  0, 9, 0, "Stage 17: Periodic Master"},       // Elements
    {18, 14, 210,  1, 10, 1, "Stage 18: Interstellar Warp"},    // Sci-Fi (Fog)
    {19, 15, 220,  2, 11, 1, "Stage 19: Olympus Ascendant"},     // Myth (Fog)
    {20, 16, 240, -1, 12, 1, "Stage 20: Polyglot Grandmaster"}  // Mixed themes (Fog)
};

char grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool foundGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool hintedGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
int  frozenGrid[MAX_GRID_SIZE][MAX_GRID_SIZE]; // 0=none, 1=1 layer, 2=2 layers
bool unfoggedGrid[MAX_GRID_SIZE][MAX_GRID_SIZE]; // Fog of War
bool radarGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool pathfinderGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool secretGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
int premiumGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
float scanAnim[MAX_GRID_SIZE][MAX_GRID_SIZE];

int gridSize = 15;
int numWordsToFind = 8;
int currentDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int currentThemeIdx = 0;
int currentGameMode = 0; // 0=Classic, 1=Zen, 2=TimeAttack, 3=Campaign
int campaignStage = 1;
bool isFogStage = false;

int magicWands = 3;        // Radar (R)
int pathfinderCharges = 3; // Pathfinder (P)
int freezeCharges = 3;     // Freeze (F)
int hintCharges = 3;       // Hint (H)
int freezeTimer = 0;       // Seconds remaining for timer freeze
int comboMultiplier = 1;
int timeSinceLastFind = 0;

char wordsToFind[MAX_WORDS][32];
bool wordsFoundStatus[MAX_WORDS];
bool wordsHintedStatus[MAX_WORDS];
int wordCount = 0;
int foundCount = 0;
int currentScore = 0;

char secretWords[2][32];
bool secretFoundStatus[2];
int secretCount = 0;
int secretBannerTimer = 0;
char secretBannerMsg[64] = "";

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

int GetLetterScore(char ch) {
    if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
    if (ch < 'A' || ch > 'Z') return 1;
    static const int scores[26] = {
        1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
    };
    return scores[ch - 'A'];
}

int GetCellPx() {
    if (gridSize <= 9) return 40;
    if (gridSize <= 12) return 34;
    if (gridSize <= 15) return 28;
    if (gridSize <= 18) return 24;
    return 21;
}

typedef struct {
    float x, y;
    float vx, vy;
    float angle, vRot;
    COLORREF color;
    int size;
    int shape; // 0=rect, 1=star
    float life;
    float decay;
    int type; // 0=confetti, 1=ice, 2=spark
} Particle;

#define MAX_PARTICLES 300
Particle particles[MAX_PARTICLES];
bool particlesInit = false;

void SpawnParticles(int px, int py, int type) {
    if (!particlesInit) { memset(particles, 0, sizeof(particles)); particlesInit = true; }
    int numSpawn = (type == 1) ? 15 : 20;
    if (type == 0) numSpawn = 80;
    if (type == 3) numSpawn = 50; // Dust
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < numSpawn; i++) {
        if (type == 0 || (particles[i].type != 0 && particles[i].life <= 0)) {
            particles[i].type = type;
            particles[i].life = 1.0f;
            particles[i].decay = 0.02f + (rand() % 50) / 1000.0f;
            particles[i].angle = (float)((rand() % 360) * 0.0174533);
            particles[i].vRot = ((rand() % 100) - 50) / 100.0f;
            if (type == 0) {
                particles[i].x = (float)(50 + (rand() % 750));
                particles[i].y = -10.0f - (float)(rand() % 300);
                particles[i].vx = ((rand() % 100) - 50) / 20.0f;
                particles[i].vy = 2.5f + (rand() % 100) / 20.0f;
                COLORREF colors[] = { RGB(255, 224, 102), RGB(104, 211, 145), RGB(99, 179, 237), RGB(246, 173, 85), RGB(252, 129, 129), RGB(183, 148, 244) };
                particles[i].color = colors[rand() % 6];
                particles[i].size = 6 + rand() % 7;
                particles[i].shape = rand() % 2;
                particles[i].decay = 0;
            } else if (type == 3) { // Dust motes
                particles[i].x = (float)(rand() % 1000);
                particles[i].y = (float)(rand() % 900);
                particles[i].vx = ((rand() % 100) - 50) / 150.0f;
                particles[i].vy = -0.2f - (rand() % 100) / 200.0f;
                particles[i].color = RGB(255, 230, 180);
                particles[i].size = 1 + rand() % 3;
                particles[i].shape = 0;
                particles[i].decay = 0;
            } else {
                particles[i].x = (float)px;
                particles[i].y = (float)py;
                particles[i].vx = ((rand() % 100) - 50) / 10.0f;
                particles[i].vy = ((rand() % 100) - 50) / 10.0f;
                if (type == 1) {
                    particles[i].color = rand()%2 ? RGB(255, 255, 255) : RGB(179, 242, 255);
                    particles[i].size = 4 + rand() % 4;
                    particles[i].shape = 0;
                    particles[i].vy -= 2.0f;
                } else {
                    COLORREF sc[] = { RGB(255, 215, 0), RGB(255, 140, 0), RGB(255, 0, 255) };
                    particles[i].color = sc[rand() % 3];
                    particles[i].size = 3 + rand() % 4;
                    particles[i].shape = 1;
                }
            }
            spawned++;
        }
    }
}

void ResetConfetti() { SpawnParticles(0, 0, 0); }

void TriggerIceShatter(int r, int c) {
    int cellPx = GetCellPx();
    int px = 20 + 6 + c * cellPx + cellPx / 2;
    int py = 50 + 6 + r * cellPx + cellPx / 2;
    SpawnParticles(px, py, 1);
}

void TriggerMagicSpark(int r, int c) {
    int cellPx = GetCellPx();
    int px = 20 + 6 + c * cellPx + cellPx / 2;
    int py = 50 + 6 + r * cellPx + cellPx / 2;
    SpawnParticles(px, py, 2);
}

void DrawConfettiFX(HDC hdc, int width, int height) {
    if (!particlesInit) return;
    for(int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].type != 0 && particles[i].life <= 0) continue;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].angle += particles[i].vRot;
        if (particles[i].type == 0) {
            if (particles[i].y > height + 20) {
                particles[i].y = -10.0f;
                particles[i].x = (float)(rand() % width);
            }
        } else if (particles[i].type == 3) {
            particles[i].x += (float)(sin((double)particles[i].y * 0.05) * 0.2);
            if (particles[i].y < -10.0f || particles[i].x < -10.0f || particles[i].x > width + 10.0f) {
                particles[i].y = height + 10.0f;
                particles[i].x = (float)(rand() % width);
            }
        } else {
            particles[i].vy += 0.2f;
            particles[i].life -= particles[i].decay;
        }
        HBRUSH cBrush = CreateSolidBrush(particles[i].color);
        HPEN cPen = CreatePen(PS_SOLID, 1, particles[i].color);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, cBrush);
        HPEN oldP = (HPEN)SelectObject(hdc, cPen);
        int px = (int)particles[i].x, py = (int)particles[i].y, sz = particles[i].size;
        if (particles[i].type != 0) {
            sz = (int)(sz * particles[i].life);
            if (sz < 1) sz = 1;
        }
        int hsz = (int)(sz * cos((double)particles[i].angle));
        if (hsz == 0) hsz = 1;
        if (particles[i].shape == 0) {
            RECT cRc = { px - sz/2, py - abs(hsz)/2, px + sz/2, py + abs(hsz)/2 };
            FillRect(hdc, &cRc, cBrush);
        } else {
            POINT pts[5];
            for (int k = 0; k < 5; k++) {
                double a = particles[i].angle + k * 1.25664;
                pts[k].x = px + (long)(sz * cos(a));
                pts[k].y = py + (long)(sz * sin(a));
            }
            Polygon(hdc, pts, 5);
        }
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(cBrush);
        DeleteObject(cPen);
    }
}

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

RECT btnTheme  = {170, 10, 245, 35};
RECT btnMode   = {250, 10, 325, 35};
RECT btnEasy   = {330, 10, 375, 35};
RECT btnMed    = {380, 10, 435, 35};
RECT btnHard   = {440, 10, 485, 35};
RECT btnHint   = {490, 10, 535, 35};
RECT btnWand   = {540, 10, 590, 35};
RECT btnPath   = {595, 10, 645, 35};
RECT btnFreeze = {650, 10, 700, 35};
RECT btnSave   = {705, 10, 745, 35};
RECT btnLoad   = {750, 10, 790, 35};
RECT btnStats  = {795, 10, 840, 35};
RECT btnHelp   = {845, 10, 885, 35};

bool showStats = false;
bool showHelp = false;

DWORD WINAPI SoundTick(LPVOID lpParam) { Beep(1500, 10); return 0; }
DWORD WINAPI SoundChime(LPVOID lpParam) { Beep(523, 100); Beep(659, 100); Beep(784, 200); return 0; }
DWORD WINAPI SoundFanfare(LPVOID lpParam) { Beep(440, 150); Beep(554, 150); Beep(659, 150); Beep(880, 400); return 0; }

void PlaySoundEffect(int type) {
    if (type == 0) CreateThread(NULL, 0, SoundTick, NULL, 0, NULL);
    else if (type == 1) CreateThread(NULL, 0, SoundChime, NULL, 0, NULL);
    else if (type == 2) CreateThread(NULL, 0, SoundFanfare, NULL, 0, NULL);
}

void UnfogArea(int centerR, int centerC, int radius) {
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            int nr = centerR + dr;
            int nc = centerC + dc;
            if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize) {
                unfoggedGrid[nr][nc] = true;
            }
        }
    }
}

bool PlaceSingleWord(const char* word) {
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    int len = strlen(word);
    int attempts = 0;
    while(attempts < 5000) {
        attempts++;
        int d = rand() % 8;
        int r = rand() % gridSize;
        int c = rand() % gridSize;
        
        bool canPlace = true;
        for(int i=0; i<len; i++) {
            int nr = r + i * dirs[d][0];
            int nc = c + i * dirs[d][1];
            if(nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || (grid[nr][nc] != ' ' && grid[nr][nc] != word[i])) {
                canPlace = false;
                break;
            }
        }
        if(canPlace) {
            for(int i=0; i<len; i++) {
                grid[r + i * dirs[d][0]][c + i * dirs[d][1]] = word[i];
            }
            return true;
        }
    }
    return false;
}

void InitGame() {
    srand((unsigned int)time(NULL));
    memset(foundGrid, 0, sizeof(foundGrid));
    memset(hintedGrid, 0, sizeof(hintedGrid));
    memset(frozenGrid, 0, sizeof(frozenGrid));
    memset(unfoggedGrid, 0, sizeof(unfoggedGrid));
    memset(radarGrid, 0, sizeof(radarGrid));
    memset(pathfinderGrid, 0, sizeof(pathfinderGrid));
    memset(secretGrid, 0, sizeof(secretGrid));
    memset(premiumGrid, 0, sizeof(premiumGrid));
    memset(wordsFoundStatus, 0, sizeof(wordsFoundStatus));
    memset(wordsHintedStatus, 0, sizeof(wordsHintedStatus));
    memset(cellAnim, 0, sizeof(cellAnim));
    memset(scanAnim, 0, sizeof(scanAnim));
    memset(strikeAnim, 0, sizeof(strikeAnim));
    memset(secretFoundStatus, 0, sizeof(secretFoundStatus));
    
    foundCount = 0;
    if (!(currentGameMode == 3 && campaignStage > 1)) {
        currentScore = 0;
    }
    freezeTimer = 0;
    secretBannerTimer = 0;
    
    int numFrozen = 0;
    isFogStage = false;
    
    if (currentGameMode == 3) {
        int stg = campaignStage - 1;
        if (stg < 0) stg = 0;
        if (stg >= NUM_CAMPAIGN_STAGES) stg = NUM_CAMPAIGN_STAGES - 1;

        gridSize = CAMPAIGN_STAGES_DEF[stg].grid;
        numWordsToFind = CAMPAIGN_STAGES_DEF[stg].words;
        timerSeconds = CAMPAIGN_STAGES_DEF[stg].time;
        int tIdx = CAMPAIGN_STAGES_DEF[stg].theme;
        if (tIdx >= 0) currentThemeIdx = tIdx;
        numFrozen = CAMPAIGN_STAGES_DEF[stg].frozen;
        isFogStage = (CAMPAIGN_STAGES_DEF[stg].fog == 1);

        magicWands = 3;
        pathfinderCharges = 3;
        freezeCharges = 3;
        hintCharges = 3;
    } else if (currentGameMode == 2) {
        timerSeconds = (gridSize == 10) ? 120 : ((gridSize == 15) ? 180 : 300);
        numFrozen = (currentDifficulty == 0) ? 0 : ((currentDifficulty == 1) ? 3 : 6);
        isFogStage = (currentDifficulty == 2);
    } else {
        timerSeconds = 0;
        numFrozen = (currentDifficulty == 0) ? 0 : ((currentDifficulty == 1) ? 2 : 4);
        isFogStage = false;
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
    
    // Pick target words
    wordCount = 0;
    if (currentGameMode == 3 && campaignStage == 20) {
        // Polyglot Grandmaster: Pick words from ALL themes
        while(wordCount < numWordsToFind) {
            int t = rand() % NUM_THEMES;
            int idx = rand() % THEME_DICT_SIZE;
            const char* candidate = DICTIONARIES[t][idx];
            bool dup = false;
            for(int k=0; k<wordCount; k++) {
                if(strcmp(wordsToFind[k], candidate) == 0) { dup = true; break; }
            }
            if(!dup) {
                strcpy(wordsToFind[wordCount++], candidate);
            }
        }
    } else {
        int picked[THEME_DICT_SIZE] = {0};
        while(wordCount < numWordsToFind) {
            int idx = rand() % THEME_DICT_SIZE;
            if(!picked[idx]) {
                picked[idx] = 1;
                strcpy(wordsToFind[wordCount++], DICTIONARIES[currentThemeIdx][idx]);
            }
        }
    }
    
    // Place target words
    for(int w=0; w<wordCount; w++) {
        PlaceSingleWord(wordsToFind[w]);
    }
    
    // Place secret bonus words
    secretCount = 0;
    int sPicked = rand() % NUM_SECRET_BANK;
    if (PlaceSingleWord(SECRET_WORDS_BANK[sPicked])) {
        strcpy(secretWords[secretCount++], SECRET_WORDS_BANK[sPicked]);
    }
    int sPicked2 = (sPicked + 1 + (rand() % (NUM_SECRET_BANK - 1))) % NUM_SECRET_BANK;
    if (PlaceSingleWord(SECRET_WORDS_BANK[sPicked2])) {
        strcpy(secretWords[secretCount++], SECRET_WORDS_BANK[sPicked2]);
    }

    // Fill rest of grid with random letters
    for(int r=0; r<gridSize; r++){
        for(int c=0; c<gridSize; c++){
            if(grid[r][c] == ' '){
                grid[r][c] = 'A' + (rand() % 26);
            }
        }
    }

    // Place frozen tiles (2 layers of ice each)
    int frozenPlaced = 0;
    int fAttempts = 0;
    while (frozenPlaced < numFrozen && fAttempts < 300) {
        fAttempts++;
        int fr = rand() % gridSize;
        int fc = rand() % gridSize;
        if (frozenGrid[fr][fc] == 0) {
            frozenGrid[fr][fc] = 2; // 2 layers of ice
            frozenPlaced++;
        }
    }

    // Place premium tiles
    int numPremium = (gridSize * gridSize) / 20;
    for (int i = 0; i < numPremium; i++) {
        int pr = rand() % gridSize;
        int pc = rand() % gridSize;
        if (premiumGrid[pr][pc] == 0) premiumGrid[pr][pc] = (rand() % 4) + 1;
    }

    // Initialize Fog of War
    if (!isFogStage) {
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                unfoggedGrid[r][c] = true;
            }
        }
    } else {
        // Unfog center starting area
        UnfogArea(gridSize / 2, gridSize / 2, 2);
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
        for (int i=0; i<count; i++) scanAnim[selR[i]][selC[i]] = 1.0f;

        char selWord[32] = {0};
        char revWord[32] = {0};
        for(int i=0; i<count; i++) {
            selWord[i] = grid[selR[i]][selC[i]];
            revWord[count - 1 - i] = grid[selR[i]][selC[i]];
        }
        
        bool found = false;
        // Check target words
        for(int w=0; w<wordCount; w++) {
            if(!wordsFoundStatus[w]) {
                if(strcmp(wordsToFind[w], selWord) == 0 || strcmp(wordsToFind[w], revWord) == 0) {
                    wordsFoundStatus[w] = true;
                    foundCount++;
                    found = true;
                    
                    if (currentGameMode != 1) {
                        int wordMult = 1;
                        int wordScore = 0;
                        for (int k = 0; k < count; k++) {
                            int r = selR[k];
                            int c = selC[k];
                            int ls = GetLetterScore(grid[r][c]);
                            if (premiumGrid[r][c] == 1) ls *= 2;
                            else if (premiumGrid[r][c] == 2) ls *= 3;
                            else if (premiumGrid[r][c] == 3) wordMult *= 2;
                            else if (premiumGrid[r][c] == 4) wordMult *= 3;
                            wordScore += ls;
                        }
                        int points = 1000 - (timerSeconds * 2);
                        if (currentGameMode == 2 || currentGameMode == 3) points = 500;
                        points += wordScore * wordMult * 10;
                        if (points < 100) points = 100;
                        if (timeSinceLastFind < 15) comboMultiplier++;
                        else comboMultiplier = 1;
                        currentScore += points * comboMultiplier;
                        timeSinceLastFind = 0;
                    }
                    
                    // Mark cells as found & thaw adjacent frozen tiles & clear fog
                    for(int i=0; i<count; i++) {
                        int r = selR[i];
                        int c = selC[i];
                        foundGrid[r][c] = true;
                        if (isFogStage) UnfogArea(r, c, 2);
                        
                        for (int dr = -1; dr <= 1; dr++) {
                            for (int dc = -1; dc <= 1; dc++) {
                                int nr = r + dr;
                                int nc = c + dc;
                                if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize) {
                                    if (frozenGrid[nr][nc] > 0) {
                                        frozenGrid[nr][nc] = 0;
                                        currentScore += 100;
                                        TriggerIceShatter(nr, nc);
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }

        // Check secret words
        if (!found) {
            for(int s=0; s<secretCount; s++) {
                if(!secretFoundStatus[s]) {
                    if(strcmp(secretWords[s], selWord) == 0 || strcmp(secretWords[s], revWord) == 0) {
                        secretFoundStatus[s] = true;
                        currentScore += 500;
                        secretBannerTimer = 4;
                        sprintf(secretBannerMsg, "SECRET WORD FOUND: %s! (+500 PTS)", secretWords[s]);
                        PlaySoundEffect(2);
                        for(int i=0; i<count; i++) {
                            int r = selR[i];
                            int c = selC[i];
                            secretGrid[r][c] = true;
                            if (isFogStage) UnfogArea(r, c, 2);
                        }
                        found = true;
                        break;
                    }
                }
            }
        }
        
        if (foundCount == wordCount) {
            if (currentGameMode == 3 && campaignStage < NUM_CAMPAIGN_STAGES) {
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

void UseRadar(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (currentGameMode == 3 && magicWands <= 0) return;
    if (currentGameMode != 3 && currentScore < 200 && currentGameMode != 1) return;
    
    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w]) unfound[uncount++] = w;
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
                        radarGrid[r][c] = true;
                        if (isFogStage) UnfogArea(r, c, 2);
                        foundInGrid = true;
                        PlaySoundEffect(1);
                        TriggerMagicSpark(r, c);
                        break;
                    }
                }
            }
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void UsePathfinder(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (currentGameMode == 3 && pathfinderCharges <= 0) return;
    if (currentGameMode != 3 && currentScore < 300 && currentGameMode != 1) return;

    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w]) unfound[uncount++] = w;
    }
    if (uncount == 0) return;

    int targetIdx = unfound[rand() % uncount];

    if (currentGameMode == 3) pathfinderCharges--;
    else if (currentGameMode != 1) currentScore -= 300;

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
                            pathfinderGrid[fr][fc] = true;
                            if (isFogStage) UnfogArea(fr, fc, 2);
                            TriggerMagicSpark(fr, fc);
                        }
                        foundInGrid = true;
                        PlaySoundEffect(1);
                        break;
                    }
                }
            }
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
        if(!wordsFoundStatus[w]) unfound[uncount++] = w;
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
                        int randChar = rand() % len;
                        int hr = r + randChar * dirs[d][0];
                        int hc = c + randChar * dirs[d][1];
                        hintedGrid[hr][hc] = true;
                        if (isFogStage) UnfogArea(hr, hc, 2);
                        foundInGrid = true;
                        PlaySoundEffect(0);
                        TriggerMagicSpark(hr, hc);
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
        fwrite(&pathfinderCharges, sizeof(int), 1, fp);
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
        fwrite(frozenGrid, sizeof(int), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(unfoggedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
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
        fread(&pathfinderCharges, sizeof(int), 1, fp);
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
        fread(frozenGrid, sizeof(int), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(unfoggedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
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
            SpawnParticles(0, 0, 3); // Initial dust
            SetTimer(hwnd, 1, 1000, NULL);
            SetTimer(hwnd, 2, 30, NULL);
            break;
        case WM_TIMER:
            if(wParam == 1) {
                if(!gameWon && !gameOver) {
                    if (secretBannerTimer > 0) secretBannerTimer--;
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
                        if (scanAnim[r][c] > 0.0f) {
                            scanAnim[r][c] -= 0.1f;
                            if (scanAnim[r][c] < 0.0f) scanAnim[r][c] = 0.0f;
                            needsRedraw = true;
                        }
                    }
                }
                for(int w=0; w<wordCount; w++) {
                    float target = wordsFoundStatus[w] ? 1.0f : 0.0f;
                    if(strikeAnim[w] < target) { strikeAnim[w] += 0.1f; if(strikeAnim[w]>1.0f) strikeAnim[w]=1.0f; needsRedraw = true; }
                }
                for(int i=0; i<MAX_PARTICLES; i++) {
                    if (particles[i].type != 0 && particles[i].life > 0) {
                        needsRedraw = true;
                        break;
                    }
                }
                if(gameWon && particlesInit) needsRedraw = true;
                if(needsRedraw) InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == 'R' || wParam == 'r' || wParam == 'W' || wParam == 'w') UseRadar(hwnd);
            else if (wParam == 'P' || wParam == 'p') UsePathfinder(hwnd);
            else if (wParam == 'F' || wParam == 'f') UseFreeze(hwnd);
            else if (wParam == 'H' || wParam == 'h') UseHint(hwnd);
            else if (wParam == 'S' || wParam == 's') SaveGame(hwnd);
            else if (wParam == 'L' || wParam == 'l') LoadGame(hwnd);
            break;
        case WM_LBUTTONDOWN: {
            if (showStats) { showStats = false; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (showHelp) { showHelp = false; InvalidateRect(hwnd, NULL, TRUE); break; }
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            if (PtInRect(&btnTheme, pt)) {
                currentThemeIdx = (currentThemeIdx + 1) % NUM_THEMES; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnMode, pt)) {
                currentGameMode = (currentGameMode + 1) % 4;
                if (currentGameMode == 3) { campaignStage = 1; magicWands = 3; pathfinderCharges = 3; freezeCharges = 3; hintCharges = 3; currentScore = 0; }
                InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
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
            if (PtInRect(&btnHint, pt)) { UseHint(hwnd); break; }
            if (PtInRect(&btnWand, pt)) { UseRadar(hwnd); break; }
            if (PtInRect(&btnPath, pt)) { UsePathfinder(hwnd); break; }
            if (PtInRect(&btnFreeze, pt)) { UseFreeze(hwnd); break; }
            if (PtInRect(&btnSave, pt)) { SaveGame(hwnd); break; }
            if (PtInRect(&btnLoad, pt)) { LoadGame(hwnd); break; }
            if (PtInRect(&btnStats, pt)) { LoadStats(); showStats = true; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (PtInRect(&btnHelp, pt)) { showHelp = true; InvalidateRect(hwnd, NULL, TRUE); break; }

            if(gameWon || gameOver) {
                InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }

            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int cellPx = GetCellPx();
            int c = (x - 20 - 6) / cellPx;
            int r = (y - 50 - 6) / cellPx;
            if(r >= 0 && r < gridSize && c >= 0 && c < gridSize) {
                // If tile is frozen, clicking it thaws 1 layer (requires 2 clicks to thaw fully)
                if (frozenGrid[r][c] > 0) {
                    frozenGrid[r][c]--;
                    PlaySoundEffect(0);
                    TriggerIceShatter(r, c);
                    if (frozenGrid[r][c] == 0) {
                        currentScore += 100;
                        PlaySoundEffect(1);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                
                // If fog stage, unfog cell and 5x5 surrounding
                if (isFogStage) UnfogArea(r, c, 2);

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
                int cellPx = GetCellPx();
                int c = (x - 20 - 6) / cellPx;
                int r = (y - 50 - 6) / cellPx;
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
                        if (isFogStage) UnfogArea(r, c, 2);
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
            
            // Detailed wooden study desk texture with ambient lighting
            for (int y = ps.rcPaint.top; y <= ps.rcPaint.bottom; y++) {
                int dy = abs(y - 425);
                int glow = 300 - dy;
                if (glow < 0) glow = 0;
                glow = glow / 4; 
                
                int wave = (int)(sin(y * 0.03) * 15 + sin(y * 0.1) * 5);
                int grain = ((y + wave) % 25 < 6) ? -12 : 0;
                int microGrain = (y % 3 == 0) ? -5 : 0;
                
                int cr = 55 + glow + grain + microGrain;
                int cg = 30 + (glow * 2 / 3) + grain + microGrain;
                int cb = 15 + (glow / 3) + grain + microGrain;
                if (cr < 0) cr = 0; if (cr > 255) cr = 255;
                if (cg < 0) cg = 0; if (cg > 255) cg = 255;
                if (cb < 0) cb = 0; if (cb > 255) cb = 255;
                
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(cr, cg, cb));
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                MoveToEx(hdc, ps.rcPaint.left, y, NULL);
                LineTo(hdc, ps.rcPaint.right, y);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            char header[160];
            if (currentGameMode == 1) {
                sprintf(header, "Found: %d/%d   Score: -   Timer: --:--", foundCount, wordCount);
            } else if (currentGameMode == 3) {
                if (freezeTimer > 0) {
                    sprintf(header, "Stage: %d/%d   Found: %d/%d   Score: %d   Timer: FROZEN (%ds)   R:%d P:%d F:%d H:%d",
                            campaignStage, NUM_CAMPAIGN_STAGES, foundCount, wordCount, currentScore, freezeTimer,
                            magicWands, pathfinderCharges, freezeCharges, hintCharges);
                } else {
                    sprintf(header, "Stage: %d/%d   Found: %d/%d   Score: %d   Timer: %02d:%02d   R:%d P:%d F:%d H:%d",
                            campaignStage, NUM_CAMPAIGN_STAGES, foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60,
                            magicWands, pathfinderCharges, freezeCharges, hintCharges);
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

            HBRUSH pathBrush = CreateSolidBrush(RGB(72, 187, 120));
            FillRect(hdc, &btnPath, pathBrush);
            DeleteObject(pathBrush);
            
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
            TextOut(hdc, btnPath.left + 4, btnPath.top + 5, "Path", 4);
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
            
            HFONT hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            int cellPx = GetCellPx();
            int boardLeft = 20;
            int boardTop = 50;
            int gridW = gridSize * cellPx;
            int gridH = gridSize * cellPx;
            int boardRight = boardLeft + gridW + 12;
            int boardBottom = boardTop + gridH + 12;

            // Draw Mahogany Wood Frame
            HBRUSH woodOuter = CreateSolidBrush(RGB(74, 38, 16));
            HBRUSH woodInner = CreateSolidBrush(RGB(36, 19, 11));
            HBRUSH socketBg  = CreateSolidBrush(RGB(24, 13, 7));

            RECT outerRc = { boardLeft, boardTop, boardRight, boardBottom };
            FillRect(hdc, &outerRc, woodOuter);

            RECT innerRc = { boardLeft + 6, boardTop + 6, boardRight - 6, boardBottom - 6 };
            FillRect(hdc, &innerRc, woodInner);

            HPEN woodHilite = CreatePen(PS_SOLID, 2, RGB(110, 56, 24));
            HPEN woodShadow = CreatePen(PS_SOLID, 2, RGB(20, 10, 5));
            HPEN oldP = (HPEN)SelectObject(hdc, woodHilite);

            MoveToEx(hdc, boardLeft, boardBottom, NULL);
            LineTo(hdc, boardLeft, boardTop);
            LineTo(hdc, boardRight, boardTop);
            SelectObject(hdc, woodShadow);
            LineTo(hdc, boardRight, boardBottom);
            LineTo(hdc, boardLeft, boardBottom);
            SelectObject(hdc, oldP);

            DeleteObject(woodOuter);
            DeleteObject(woodInner);
            DeleteObject(woodHilite);
            DeleteObject(woodShadow);

            for(int r=0; r<gridSize; r++) {
                for(int c=0; c<gridSize; c++) {
                    int tileX = boardLeft + 6 + c * cellPx;
                    int tileY = boardTop + 6 + r * cellPx;
                    RECT socketRc = { tileX, tileY, tileX + cellPx, tileY + cellPx };
                    FillRect(hdc, &socketRc, socketBg);
                    
                    bool isSelected = false;
                    for(int i=0; i<selCount; i++) {
                        if(selR[i] == r && selC[i] == c) {
                            isSelected = true; break;
                        }
                    }

                    bool isFogged = (isFogStage && !unfoggedGrid[r][c] && !isSelected);
                    
                    COLORREF bgTop, bgBot, borderHi, borderLo, textColor;
                    if (isFogged) {
                        bgTop = RGB(35, 40, 55); bgBot = RGB(20, 24, 35);
                        borderHi = RGB(55, 60, 80); borderLo = RGB(10, 12, 20);
                        textColor = RGB(70, 80, 105);
                    } else if (isSelected) {
                        bgTop = RGB(99, 179, 237); bgBot = RGB(49, 130, 206);
                        borderHi = RGB(190, 227, 249); borderLo = RGB(26, 54, 93);
                        textColor = RGB(255, 255, 255);
                    } else if (secretGrid[r][c]) {
                        bgTop = RGB(255, 215, 0); bgBot = RGB(218, 165, 32);
                        borderHi = RGB(255, 250, 205); borderLo = RGB(139, 101, 8);
                        textColor = RGB(60, 40, 0);
                    } else if (foundGrid[r][c] || cellAnim[r][c] > 0.0f) {
                        bgTop = RGB(104, 211, 145); bgBot = RGB(56, 161, 105);
                        borderHi = RGB(198, 246, 213); borderLo = RGB(28, 69, 50);
                        textColor = RGB(255, 255, 255);
                    } else if (radarGrid[r][c]) {
                        bgTop = RGB(214, 158, 255); bgBot = RGB(159, 122, 234);
                        borderHi = RGB(243, 225, 255); borderLo = RGB(90, 40, 160);
                        textColor = RGB(255, 255, 255);
                    } else if (pathfinderGrid[r][c]) {
                        bgTop = RGB(129, 230, 217); bgBot = RGB(49, 151, 149);
                        borderHi = RGB(220, 255, 250); borderLo = RGB(20, 80, 80);
                        textColor = RGB(255, 255, 255);
                    } else if (hintedGrid[r][c]) {
                        bgTop = RGB(255, 224, 102); bgBot = RGB(214, 158, 46);
                        borderHi = RGB(254, 235, 200); borderLo = RGB(116, 66, 16);
                        textColor = RGB(45, 25, 0);
                    } else if (frozenGrid[r][c] > 0) {
                        if (frozenGrid[r][c] == 2) {
                            bgTop = RGB(179, 242, 255); bgBot = RGB(49, 180, 206);
                            borderHi = RGB(230, 255, 255); borderLo = RGB(20, 100, 120);
                        } else {
                            bgTop = RGB(210, 250, 255); bgBot = RGB(100, 210, 225);
                            borderHi = RGB(240, 255, 255); borderLo = RGB(30, 120, 140);
                        }
                        textColor = RGB(13, 56, 56);
                    } else if (premiumGrid[r][c] > 0) {
                        if (premiumGrid[r][c] == 1) { // DL
                            bgTop = RGB(176, 212, 255); bgBot = RGB(120, 168, 255);
                            borderHi = RGB(255, 255, 255); borderLo = RGB(66, 114, 196);
                        } else if (premiumGrid[r][c] == 2) { // TL
                            bgTop = RGB(104, 168, 255); bgBot = RGB(43, 108, 176);
                            borderHi = RGB(255, 255, 255); borderLo = RGB(26, 54, 93);
                        } else if (premiumGrid[r][c] == 3) { // DW
                            bgTop = RGB(255, 192, 176); bgBot = RGB(255, 140, 120);
                            borderHi = RGB(255, 255, 255); borderLo = RGB(196, 89, 66);
                        } else { // TW
                            bgTop = RGB(255, 140, 104); bgBot = RGB(197, 48, 48);
                            borderHi = RGB(255, 255, 255); borderLo = RGB(116, 27, 27);
                        }
                        textColor = RGB(255, 255, 255);
                    } else { // Ivory / Oak Scrabble Keycap
                        bgTop = RGB(247, 241, 227); bgBot = RGB(212, 196, 168);
                        borderHi = RGB(255, 255, 255); borderLo = RGB(158, 122, 74);
                        textColor = RGB(58, 35, 18);
                    }

                    if (scanAnim[r][c] > 0.0f) {
                        int val = (int)(scanAnim[r][c] * 100);
                        bgBot = RGB(min(255, GetRValue(bgBot) + val), min(255, GetGValue(bgBot) + val), min(255, GetBValue(bgBot) + val));
                        bgTop = RGB(min(255, GetRValue(bgTop) + val), min(255, GetGValue(bgTop) + val), min(255, GetBValue(bgTop) + val));
                    }

                    int pad = 2;
                    int tW = cellPx - pad * 2;
                    int tH = cellPx - pad * 2;
                    int curW = tW;
                    int curH = tH;
                    if (cellAnim[r][c] > 0.0f && !foundGrid[r][c]) {
                        curW = (int)(tW * (0.3f + 0.7f * cellAnim[r][c]));
                        curH = (int)(tH * (0.3f + 0.7f * cellAnim[r][c]));
                    }
                    int offsetX = pad + (tW - curW) / 2;
                    int offsetY = pad + (tH - curH) / 2;
                    if (isSelected) {
                        offsetY -= 3;
                    }
                    RECT tileRc = { tileX + offsetX, tileY + offsetY, tileX + offsetX + curW, tileY + offsetY + curH };

                    HBRUSH shadowBrush = CreateSolidBrush(borderLo);
                    RECT shadowRc = { tileRc.left, tileRc.top + (isSelected ? 6 : 3), tileRc.right, tileRc.bottom + (isSelected ? 6 : 3) };
                    FillRect(hdc, &shadowRc, shadowBrush);
                    DeleteObject(shadowBrush);

                    HBRUSH faceBrush = CreateSolidBrush(bgBot);
                    FillRect(hdc, &tileRc, faceBrush);
                    DeleteObject(faceBrush);

                    HPEN hiPen = CreatePen(PS_SOLID, 1, borderHi);
                    HPEN oldP2 = (HPEN)SelectObject(hdc, hiPen);
                    MoveToEx(hdc, tileRc.left, tileRc.bottom - 1, NULL);
                    LineTo(hdc, tileRc.left, tileRc.top);
                    LineTo(hdc, tileRc.right - 1, tileRc.top);
                    SelectObject(hdc, oldP2);
                    DeleteObject(hiPen);

                    HFONT letterFont = CreateFont(curH * 3 / 5, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Georgia");
                    HFONT oldF1 = (HFONT)SelectObject(hdc, letterFont);
                    SetTextColor(hdc, textColor);
                    
                    char letterStr[2] = { isFogged ? '?' : grid[r][c], 0 };
                    RECT textRc = { tileRc.left, tileRc.top + 1, tileRc.right, tileRc.bottom - 4 };
                    DrawText(hdc, letterStr, 1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    SelectObject(hdc, oldF1);
                    DeleteObject(letterFont);

                    if (!isFogged) {
                        if (!foundGrid[r][c] && premiumGrid[r][c] > 0) {
                            const char* pLbl = (premiumGrid[r][c]==1)?"DL":((premiumGrid[r][c]==2)?"TL":((premiumGrid[r][c]==3)?"DW":"TW"));
                            HFONT pFont = CreateFont(curH / 4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
                            HFONT oldPF = (HFONT)SelectObject(hdc, pFont);
                            SetTextColor(hdc, textColor);
                            RECT pRc = { tileRc.left + 2, tileRc.top + 1, tileRc.right, tileRc.bottom };
                            DrawText(hdc, pLbl, 2, &pRc, DT_LEFT | DT_TOP | DT_SINGLELINE);
                            SelectObject(hdc, oldPF);
                            DeleteObject(pFont);
                        }

                        int pts = GetLetterScore(grid[r][c]);
                        char ptsStr[8];
                        sprintf(ptsStr, "%d", pts);
                        HFONT ptsFont = CreateFont(curH / 3, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
                        HFONT oldF2 = (HFONT)SelectObject(hdc, ptsFont);
                        SetTextColor(hdc, textColor);
                        RECT ptsRc = { tileRc.right - (curW * 4 / 9), tileRc.bottom - (curH * 2 / 5), tileRc.right - 2, tileRc.bottom - 1 };
                        DrawText(hdc, ptsStr, strlen(ptsStr), &ptsRc, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
                        SelectObject(hdc, oldF2);
                        DeleteObject(ptsFont);
                    }
                }
            }
            DeleteObject(socketBg);
            
            int listX = boardRight + 20;
            int listY = 50;

            if (currentGameMode == 3) {
                SetTextColor(hdc, RGB(236, 201, 75));
                TextOut(hdc, listX, listY, CAMPAIGN_STAGES_DEF[campaignStage - 1].title, strlen(CAMPAIGN_STAGES_DEF[campaignStage - 1].title));
                listY += 22;
            }

            SetTextColor(hdc, RGB(226, 232, 240));
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
                
                listY += 22;
            }

            if (secretBannerTimer > 0) {
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOut(hdc, listX, listY + 10, secretBannerMsg, strlen(secretBannerMsg));
            }
            
            DrawConfettiFX(hdc, 920, 850);
            
            if(gameWon) {
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOut(hdc, listX, listY + 30, "YOU WIN!", 8);
                SetTextColor(hdc, RGB(200, 200, 200));
                char scoreStr[64];
                sprintf(scoreStr, "Score: %d", currentScore);
                TextOut(hdc, listX, listY + 55, scoreStr, strlen(scoreStr));
                TextOut(hdc, listX, listY + 80, "Click anywhere", 14);
                TextOut(hdc, listX, listY + 100, "to play again", 13);
            } else if (gameOver) {
                SetTextColor(hdc, RGB(255, 100, 100));
                TextOut(hdc, listX, listY + 30, "GAME OVER!", 10);
                SetTextColor(hdc, RGB(200, 200, 200));
                TextOut(hdc, listX, listY + 55, "Time's up!", 10);
                TextOut(hdc, listX, listY + 80, "Click anywhere", 14);
                TextOut(hdc, listX, listY + 100, "to try again", 12);
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
                RECT modalRc = { 80, 80, 720, 720 };
                HBRUSH modalBrush = CreateSolidBrush(RGB(30, 33, 43));
                FillRect(hdc, &modalRc, modalBrush);
                DeleteObject(modalBrush);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 100, 100, "How to Play KWords - Loop 7 Edition", 35);
                TextOut(hdc, 100, 135, "Goal: Find all hidden target words and secret bonus words!", 58);
                TextOut(hdc, 120, 165, "- Words placed horizontally, vertically, or diagonally.", 55);
                TextOut(hdc, 120, 190, "- Words can be spelled forwards or backwards.", 44);
                TextOut(hdc, 120, 215, "- Click & drag to select words on the grid.", 43);
                
                TextOut(hdc, 100, 250, "Campaign Mode (20 Stages)", 25);
                TextOut(hdc, 120, 275, "- 20 Progressive Stages with dynamic grid sizes (10x10 to 20x20).", 64);
                TextOut(hdc, 120, 300, "- 12 Themes + Stage 20 Polyglot Grandmaster Challenge!", 54);
                
                SetTextColor(hdc, RGB(214, 158, 46));
                TextOut(hdc, 100, 335, "Active Skills & Assistance (Hotkeys):", 37);
                TextOut(hdc, 120, 360, "R / W: Word Radar - Sonar pulse highlights start of target word.", 64);
                TextOut(hdc, 120, 385, "P: Word Pathfinder - Outlines full exact path of target word.", 61);
                TextOut(hdc, 120, 410, "F: Freeze Timer - Pauses timer countdown for 15 seconds.", 55);
                TextOut(hdc, 120, 435, "H: Hint - Flashes a random hidden letter in target word.", 55);
                
                SetTextColor(hdc, RGB(79, 209, 197));
                TextOut(hdc, 100, 470, "Obstacles & Secret Features:", 28);
                TextOut(hdc, 120, 495, "- Frozen Tiles: Cyan ice cells require adjacent solves or 2 clicks to thaw.", 74);
                TextOut(hdc, 120, 520, "- Fog of War: Dark cells reveal letters when searched or scanned.", 65);
                TextOut(hdc, 120, 545, "- Bonus Secret Words: Hidden bonus words award +500 PTS!", 55);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 100, 660, "Click anywhere to close help", 28);
            }
            
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
