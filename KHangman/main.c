#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define W 500
#define H 700

#define NUM_CATEGORIES 10
#define TOTAL_CAT_COUNT 11 // 10 predefined + Custom

const char* CAT_NAMES[TOTAL_CAT_COUNT] = {
    "Animals", "Science", "Countries", "Movies", "Tech", 
    "Myth", "Food", "Literature", "Space", "Sports", "Custom"
};

const char* CAT_WORDS[NUM_CATEGORIES][30] = {
    // 0: Animals
    {"BEAR", "DEER", "LION", "PUMA", "WOLF", "TIGER", "ZEBRA", "MONKEY", "EAGLE", "RABBIT", "FALCON", "JAGUAR", "WALRUS", "ELEPHANT", "GIRAFFE", "KANGAROO", "PENGUIN", "DOLPHIN", "CHEETAH", "GORILLA", "OCTOPUS", "PANTHER", "LEOPARD", "ALLIGATOR", "PLATYPUS", "CHIMPANZEE", "HIPPOPOTAMUS", "RHINOCEROS", "SALAMANDER", "RATTLESNAKE"},
    // 1: Science
    {"ATOM", "CELL", "ACID", "WAVE", "MASS", "GENE", "PROTON", "PHYSICS", "BIOLOGY", "GEOLOGY", "GRAVITY", "QUANTUM", "CHEMISTRY", "ASTRONOMY", "MOLECULE", "RADIATION", "ECOLOGY", "GENETICS", "EVOLUTION", "VELOCITY", "PARTICLE", "NEUROLOGY", "BACTERIA", "TELESCOPE", "THERMODYNAMICS", "PHOTOSYNTHESIS", "ASTROPHYSICS", "CHROMOSOME", "NEUROSCIENCE", "SPECTROSCOPY"},
    // 2: Countries
    {"EGYPT", "CHILE", "INDIA", "ITALY", "JAPAN", "MEXICO", "SPAIN", "DENMARK", "FRANCE", "GREECE", "NORWAY", "SWEDEN", "AUSTRALIA", "BRAZIL", "CANADA", "GERMANY", "ARGENTINA", "BELGIUM", "PORTUGAL", "THAILAND", "VIETNAM", "COLOMBIA", "SWITZERLAND", "PHILIPPINES", "NETHERLANDS", "MADAGASCAR", "AFGHANISTAN", "LIECHTENSTEIN", "AZERBAIJAN", "LUXEMBOURG"},
    // 3: Movies
    {"JAWS", "ALIEN", "ROCKY", "AVATAR", "MATRIX", "TITANIC", "BATMAN", "PSYCHO", "GLADIATOR", "INCEPTION", "GODFATHER", "TERMINATOR", "JURASSIC", "SUPERMAN", "SPIDERMAN", "AVENGERS", "PREDATOR", "CASABLANCA", "GHOSTBUSTERS", "HALLOWEEN", "INTERSTELLAR", "GOODFELLAS", "BRAVEHEART", "RATATOUILLE", "OPPENHEIMER", "BEETLEJUICE", "TRANSFORMERS", "POLTERGEIST", "UNBREAKABLE", "ARMAGEDDON"},
    // 4: Tech
    {"BYTE", "DATA", "CODE", "WIRE", "CHIP", "ROUTER", "SERVER", "MEMORY", "BROWSER", "NETWORK", "COMPUTER", "SOFTWARE", "INTERNET", "HARDWARE", "DATABASE", "KEYBOARD", "MONITOR", "COMPILER", "PROCESSOR", "VARIABLE", "FUNCTION", "APPLICATION", "DEVELOPER", "ALGORITHM", "MICROPROCESSOR", "CYBERSECURITY", "MOTHERBOARD", "SUPERCOMPUTER", "SUPERCONDUCTOR", "CRYPTOGRAPHY"},
    // 5: Myth
    {"ZEUS", "THOR", "LOKI", "HADES", "TITAN", "HYDRA", "SPHINX", "MEDUSA", "HERCULES", "ODYSSEY", "VALHALLA", "PEGASUS", "MINOTAUR", "POSEIDON", "ATHENA", "ANUBIS", "PHOENIX", "CENTAUR", "CHIMERA", "CERBERUS", "KRAKEN", "LEVIATHAN", "OUROBOROS", "BEHEMOTH", "VALKYRIE", "EXCALIBUR", "PROMETHEUS", "NEPTUNE", "YGGDRASIL", "GILGAMESH"},
    // 6: Food
    {"TACO", "SOUP", "RICE", "MEAT", "PIZZA", "BURGER", "SUSHI", "PASTA", "STEAK", "SALAD", "WAFFLE", "CHEESE", "BREAD", "SANDWICH", "NOODLES", "PANCAKE", "CHICKEN", "POTATO", "BANANA", "CHOCOLATE", "SPAGHETTI", "BURRITO", "CROISSANT", "GUACAMOLE", "CHEESECAKE", "ENCHILADA", "MAYONNAISE", "CAPPUCCINO", "RATATOUILLE", "MARGHERITA"},
    // 7: Literature
    {"NOVEL", "POEM", "BARD", "ILIAD", "HOMER", "UTOPIA", "HAMLET", "MACBETH", "ORWELL", "GATSBY", "POETRY", "MONTAGUE", "INFERNO", "BEOWULF", "DICKENS", "TOLKIEN", "DRACULA", "SHERLOCK", "SHAKESPEARE", "FRANKENSTEIN", "ODYSSEY", "MYTHOLOGY", "BIOGRAPHY", "ENCYCLOPEDIA", "SILMARILLION", "DOSTOEVSKY", "GULLIVER", "PINOCCHIO", "METAMORPHOSIS", "PYGMALION"},
    // 8: Space
    {"MOON", "STAR", "MARS", "ORBIT", "COMET", "NEBULA", "METEOR", "COSMOS", "ASTEROID", "EXOPLANET", "ANDROMEDA", "GALAXY", "SATELLITE", "ECLIPSE", "TACHYON", "HUBBLE", "QUASAR", "PULSAR", "PARSEC", "GRAVITY", "SUPERNOVA", "METEORITE", "CONSTELLATION", "ASTROPHYSICS", "EXOSPHERE", "PLANETARIUM", "INTERSTELLAR", "UNIVERSE", "SINGULARITY", "MICROGRAVITY"},
    // 9: Sports
    {"GOLF", "BOXING", "RUGBY", "TENNIS", "SOCCER", "HOCKEY", "RACING", "SKIING", "BASEBALL", "FOOTBALL", "CRICKET", "SWIMMING", "WRESTLING", "CYCLING", "ARCHERY", "FENCING", "SURFING", "MARATHON", "BASKETBALL", "VOLLEYBALL", "BADMINTON", "GYMNASTICS", "SKATEBOARDING", "SNOWBOARDING", "WEIGHTLIFTING", "BOBSLEDDING", "EQUESTRIAN", "DECATHLON", "BODYBUILDING", "TRIATHLON"}
};
const int NUM_WORDS_PER_CAT = 30;

typedef struct {
    int stage_num;
    int cat_idx; // 0..9, or -1 for Polymath
    const char* stage_name;
    const char* cat_name;
    int min_len;
    int max_len;
    int max_errors;
} CampaignStage;

const CampaignStage CAMPAIGN_STAGES[20] = {
    {1,  0, "Stage 1: Safari Starter", "Animals", 4, 6, 6},
    {2,  1, "Stage 2: Lab Foundations", "Science", 4, 6, 6},
    {3,  2, "Stage 3: World Explorer", "Countries", 5, 7, 6},
    {4,  3, "Stage 4: Cinema Classic", "Movies", 5, 7, 6},
    {5,  4, "Stage 5: Digital Age", "Tech", 5, 8, 6},
    {6,  5, "Stage 6: Mythic Lore", "Myth", 5, 8, 6},
    {7,  6, "Stage 7: Gourmet Bistro", "Food", 6, 9, 6},
    {8,  7, "Stage 8: Literary Master", "Literature", 6, 9, 6},
    {9,  8, "Stage 9: Cosmic Voyage", "Space", 6, 9, 5},
    {10, 9, "Stage 10: Arena Champion", "Sports", 6, 9, 5},
    {11, 0, "Stage 11: Wild Fauna", "Animals", 7, 10, 5},
    {12, 1, "Stage 12: Quantum Realm", "Science", 8, 11, 5},
    {13, 2, "Stage 13: Global Frontier", "Countries", 8, 11, 5},
    {14, 3, "Stage 14: Blockbuster Vault", "Movies", 8, 12, 5},
    {15, 4, "Stage 15: Cyber Core", "Tech", 9, 12, 4},
    {16, 5, "Stage 16: Olympian Pantheon", "Myth", 9, 13, 4},
    {17, 6, "Stage 17: Culinary Perfection", "Food", 9, 13, 4},
    {18, 7, "Stage 18: Great Classics", "Literature", 10, 14, 4},
    {19, 8, "Stage 19: Deep Galaxy", "Space", 10, 14, 4},
    {20, -1, "Stage 20: Polymath Grandmaster", "Polymath", 10, 14, 4}
};

// Particle Struct
#define MAX_PARTICLES 400
typedef struct {
    float x, y;
    float vx, vy;
    int size;
    COLORREF color;
    int life;
    int maxLife;
    int type; // 0: confetti, 1: rain, 2: spark, 3: shooting star, 4: firework, 5: debris
} Particle;

Particle particles[MAX_PARTICLES];
int particle_count = 0;
int anim_ticks = 0;

// Game state variables
int current_category = 0;
int game_mode = 0; // 0: Freeplay, 1: Campaign (20 stages), 2: Time Attack Blitz
int campaign_level = 1;
int max_errors = 6;

// Active Skills & Power-ups
int vowel_reveals = 2;
int consonant_radars = 2;
int shields = 2;
int freezes = 2;
int bombs = 2;
int freeze_timer_seconds = 0;

int blitz_time = 60;
int blitz_words = 0;

HWND hCustomEdit = NULL;
int errors = 0;
char target_word[32];
int guessed[26] = {0};
int game_over = 0;
int won = 0;
int initialized = 0;
int is_muted = 0;
float shake_mag = 0.0f;
float shake_angle = 0.0f;
float lightning_flash = 0.0f;
int win_pulse_phase = 0;
int blitz_timer_counter = 0;
int loss_anim_timer = 0;

float ropeAngle = 0.0f;
float ropeAngularVelocity = 0.0f;
float windForce = 0.0f;

void SpawnSparkParticles() {
    COLORREF colors[] = {RGB(0, 255, 255), RGB(255, 0, 255), RGB(255, 255, 0), RGB(0, 255, 0), RGB(255, 128, 0)};
    for (int i = 0; i < 30 && particle_count < MAX_PARTICLES; i++) {
        particles[particle_count].x = (float)(W / 2 + (CustomRand() % 160 - 80));
        particles[particle_count].y = (float)(H / 2 - 100 + (CustomRand() % 80 - 40));
        particles[particle_count].vx = (float)((CustomRand() % 100 - 50) / 5.0f);
        particles[particle_count].vy = (float)((CustomRand() % 100 - 50) / 5.0f);
        particles[particle_count].size = CustomRand() % 4 + 2;
        particles[particle_count].color = colors[CustomRand() % 5];
        particles[particle_count].life = 0;
        particles[particle_count].maxLife = 20 + CustomRand() % 20;
        particles[particle_count].type = 2; // spark
        particle_count++;
    }
}

void PlaySoundEffect(int type) {
    if (is_muted) return;
    if (type == 1) { // valid
        Beep(800, 80);
    } else if (type == 2) { // invalid
        Beep(200, 120);
    } else if (type == 3) { // win
        Beep(400, 80);
        Beep(600, 80);
        Beep(800, 150);
    } else if (type == 4) { // lose
        Beep(300, 150);
        Beep(150, 250);
    } else if (type == 5) { // shield absorbed
        Beep(1000, 100);
        Beep(1200, 150);
    } else if (type == 6) { // bomb used
        Beep(150, 180);
    } else if (type == 7) { // vowel reveal
        Beep(900, 100);
        Beep(1100, 100);
    } else if (type == 8) { // freeze timer
        Beep(600, 100);
        Beep(600, 150);
    }
}

typedef struct {
    int wins;
    int losses;
    int streak;
    int best;
    int blitz_best;
} Stats;

Stats stats = {0, 0, 0, 0, 0};

void LoadStats() {
    FILE* f = fopen("khangman_stats.dat", "rb");
    if (f) {
        fread(&stats, sizeof(Stats), 1, f);
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("khangman_stats.dat", "wb");
    if (f) {
        fwrite(&stats, sizeof(Stats), 1, f);
        fclose(f);
    }
}

typedef struct {
    char target_word[32];
    int current_category;
    int guessed[26];
    int errors;
    int game_over;
    int won;
    int game_mode;
    int campaign_level;
    int max_errors;
    int vowel_reveals;
    int consonant_radars;
    int shields;
    int freezes;
    int bombs;
    int freeze_timer_seconds;
    int blitz_time;
    int blitz_words;
} GameState;

void SaveGame(HWND hwnd) {
    if (game_over) {
        MessageBoxA(hwnd, "Cannot save a finished game.", "Save", MB_OK);
        return;
    }
    GameState st;
    strcpy(st.target_word, target_word);
    st.current_category = current_category;
    for (int i = 0; i < 26; i++) st.guessed[i] = guessed[i];
    st.errors = errors;
    st.game_over = game_over;
    st.won = won;
    st.game_mode = game_mode;
    st.campaign_level = campaign_level;
    st.max_errors = max_errors;
    st.vowel_reveals = vowel_reveals;
    st.consonant_radars = consonant_radars;
    st.shields = shields;
    st.freezes = freezes;
    st.bombs = bombs;
    st.freeze_timer_seconds = freeze_timer_seconds;
    st.blitz_time = blitz_time;
    st.blitz_words = blitz_words;
    
    FILE* f = fopen("khangman_save.dat", "wb");
    if (f) {
        fwrite(&st, sizeof(GameState), 1, f);
        fclose(f);
        MessageBoxA(hwnd, "Game saved successfully.", "Save", MB_OK);
    }
}

void LoadGame(HWND hwnd) {
    FILE* f = fopen("khangman_save.dat", "rb");
    if (f) {
        GameState st;
        if (fread(&st, sizeof(GameState), 1, f) == 1) {
            strcpy(target_word, st.target_word);
            current_category = st.current_category;
            for (int i = 0; i < 26; i++) guessed[i] = st.guessed[i];
            errors = st.errors;
            game_over = st.game_over;
            won = st.won;
            game_mode = st.game_mode;
            campaign_level = st.campaign_level;
            max_errors = st.max_errors;
            vowel_reveals = st.vowel_reveals;
            consonant_radars = st.consonant_radars;
            shields = st.shields;
            freezes = st.freezes;
            bombs = st.bombs;
            freeze_timer_seconds = st.freeze_timer_seconds;
            blitz_time = st.blitz_time;
            blitz_words = st.blitz_words;
            MessageBoxA(hwnd, "Game loaded successfully.", "Load", MB_OK);
        }
        fclose(f);
    } else {
        MessageBoxA(hwnd, "No saved game found.", "Load", MB_OK);
    }
}

unsigned int seed = 0;

int CustomRand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

void CustomSrand(unsigned int s) {
    seed = s;
}

void SpawnWinParticles() {
    COLORREF colors[] = {RGB(255, 64, 129), RGB(0, 230, 118), RGB(255, 235, 59), RGB(0, 176, 255), RGB(220, 64, 251), RGB(255, 255, 255)};
    int basex = W / 2 + (CustomRand() % 100 - 50);
    int basey = 150 + (CustomRand() % 100 - 50);
    for (int i = 0; i < 60 && particle_count < MAX_PARTICLES; i++) {
        particles[particle_count].x = (float)basex;
        particles[particle_count].y = (float)basey;
        float angle = (float)(CustomRand() % 360) * 3.14159f / 180.0f;
        float speed = (float)(CustomRand() % 100) / 10.0f;
        particles[particle_count].vx = cosf(angle) * speed;
        particles[particle_count].vy = sinf(angle) * speed;
        particles[particle_count].size = CustomRand() % 4 + 2;
        particles[particle_count].color = colors[CustomRand() % 6];
        particles[particle_count].life = 0;
        particles[particle_count].maxLife = 50 + CustomRand() % 40;
        particles[particle_count].type = 4; // firework
        particle_count++;
    }
    for (int i = 0; i < 30 && particle_count < MAX_PARTICLES; i++) {
        particles[particle_count].x = (float)(CustomRand() % W);
        particles[particle_count].y = -10.0f;
        particles[particle_count].vx = (float)((CustomRand() % 100 - 50) / 15.0f);
        particles[particle_count].vy = (float)((CustomRand() % 50 + 20) / 10.0f);
        particles[particle_count].size = CustomRand() % 5 + 4;
        particles[particle_count].color = colors[CustomRand() % 5];
        particles[particle_count].life = 0;
        particles[particle_count].maxLife = 100 + CustomRand() % 50;
        particles[particle_count].type = 0; // confetti
        particle_count++;
    }
}

void SpawnBombParticles() {
    COLORREF colors[] = {RGB(255, 100, 0), RGB(200, 50, 0), RGB(128, 128, 128), RGB(64, 64, 64), RGB(255, 200, 0)};
    for (int i = 0; i < 60 && particle_count < MAX_PARTICLES; i++) {
        particles[particle_count].x = (float)(W / 2 + (CustomRand() % 100 - 50));
        particles[particle_count].y = (float)(H / 2 - 50 + (CustomRand() % 100 - 50));
        float angle = (float)(CustomRand() % 360) * 3.14159f / 180.0f;
        float speed = (float)(CustomRand() % 120 + 40) / 10.0f;
        particles[particle_count].vx = cosf(angle) * speed;
        particles[particle_count].vy = sinf(angle) * speed;
        particles[particle_count].size = CustomRand() % 6 + 3;
        particles[particle_count].color = colors[CustomRand() % 5];
        particles[particle_count].life = 0;
        particles[particle_count].maxLife = 50 + CustomRand() % 30;
        particles[particle_count].type = 5; // debris
        particle_count++;
    }
    shake_mag = 25.0f;
    lightning_flash = 1.0f;
}

void SpawnLossParticles() {
    for (int i = 0; i < 30 && particle_count < MAX_PARTICLES; i++) {
        particles[particle_count].x = (float)(CustomRand() % W);
        particles[particle_count].y = (float)(CustomRand() % 200);
        particles[particle_count].vx = -0.5f;
        particles[particle_count].vy = (float)(CustomRand() % 40 + 30) / 10.0f;
        particles[particle_count].size = CustomRand() % 3 + 2;
        particles[particle_count].color = RGB(100, 180, 255);
        particles[particle_count].life = 0;
        particles[particle_count].maxLife = 50;
        particles[particle_count].type = 1; // rain
        particle_count++;
    }
}

void UpdateParticles() {
    for (int i = particle_count - 1; i >= 0; i--) {
        particles[i].life++;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if (particles[i].type == 0) particles[i].vy += 0.15f; // gravity
        else if (particles[i].type == 2) particles[i].vy += 0.2f; // spark gravity
        else if (particles[i].type == 4) {
            particles[i].vx *= 0.92f;
            particles[i].vy *= 0.92f;
            particles[i].vy += 0.08f;
        } else if (particles[i].type == 5) {
            particles[i].vy += 0.5f; // heavy gravity
            particles[i].vx *= 0.96f; // air friction
            if (particles[i].y > H - 150) { // bounce near bottom
                particles[i].y = (float)(H - 150);
                particles[i].vy = -particles[i].vy * 0.4f;
                particles[i].vx *= 0.6f;
            }
        }
        
        if (particles[i].life >= particles[i].maxLife || particles[i].y > H) {
            particles[i] = particles[particle_count - 1];
            particle_count--;
        }
    }
}

void SelectNewWord() {
    if (game_mode == 1) { // Campaign stage selection based on length
        int stage_idx = (campaign_level - 1) % 20;
        CampaignStage st = CAMPAIGN_STAGES[stage_idx];
        max_errors = st.max_errors;
        
        char candidates[60][32];
        int cand_count = 0;
        
        if (st.cat_idx >= 0) {
            int cat = st.cat_idx;
            for (int w = 0; w < NUM_WORDS_PER_CAT; w++) {
                int len = (int)strlen(CAT_WORDS[cat][w]);
                if (len >= st.min_len && len <= st.max_len) {
                    strcpy(candidates[cand_count++], CAT_WORDS[cat][w]);
                }
            }
        } else { // Polymath Grandmaster (Stage 20)
            for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
                for (int w = 0; w < NUM_WORDS_PER_CAT; w++) {
                    int len = (int)strlen(CAT_WORDS[cat][w]);
                    if (len >= st.min_len && len <= st.max_len && cand_count < 60) {
                        strcpy(candidates[cand_count++], CAT_WORDS[cat][w]);
                    }
                }
            }
        }
        
        if (cand_count > 0) {
            int idx = CustomRand() % cand_count;
            strcpy(target_word, candidates[idx]);
        } else {
            // Fallback
            int cat = (st.cat_idx >= 0) ? st.cat_idx : 0;
            strcpy(target_word, CAT_WORDS[cat][CustomRand() % NUM_WORDS_PER_CAT]);
        }
    } else if (current_category == TOTAL_CAT_COUNT - 1) { // Custom
        char buf[1024] = {0};
        if (hCustomEdit) {
            GetWindowTextA(hCustomEdit, buf, sizeof(buf));
        } else {
            strcpy(buf, "APPLE, BANANA");
        }
        
        char words[50][32];
        int w_count = 0;
        char *p = buf;
        while (*p && w_count < 50) {
            while (*p == ' ' || *p == ',') p++;
            if (!*p) break;
            int i = 0;
            while (*p && *p != ',' && i < 31) {
                if (*p >= 'a' && *p <= 'z') words[w_count][i++] = *p - 32;
                else if (*p >= 'A' && *p <= 'Z') words[w_count][i++] = *p;
                p++;
            }
            words[w_count][i] = '\0';
            if (i > 0) w_count++;
            while (*p && *p != ',') p++;
        }
        if (w_count == 0) {
            strcpy(target_word, "CUSTOM");
        } else {
            int w_idx = CustomRand() % w_count;
            strcpy(target_word, words[w_idx]);
        }
    } else { // Freeplay / Blitz category selection
        int cat_idx = current_category % NUM_CATEGORIES;
        int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
        strcpy(target_word, CAT_WORDS[cat_idx][w_idx]);
    }

    for (int i = 0; i < 26; i++) guessed[i] = 0;
    errors = 0;
    particle_count = 0;
}

void InitGame() {
    if (!initialized) {
        LoadStats();
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    if (game_mode == 1) { // Campaign (20 Stages)
        if (campaign_level < 1) campaign_level = 1;
        if (campaign_level > 20) campaign_level = 20;
        
        vowel_reveals = 2;
        consonant_radars = 2;
        shields = 2;
        freezes = 2;
        bombs = 2;
    } else if (game_mode == 2) { // Time Attack Blitz
        max_errors = 6;
        vowel_reveals = 3;
        consonant_radars = 3;
        shields = 2;
        freezes = 2;
        bombs = 2;
        if (blitz_time <= 0 || game_over) {
            blitz_time = 60;
            blitz_words = 0;
        }
    } else { // Freeplay
        max_errors = 6;
        vowel_reveals = 2;
        consonant_radars = 2;
        shields = 2;
        freezes = 2;
        bombs = 2;
    }
    
    freeze_timer_seconds = 0;
    SelectNewWord();
    game_over = 0;
    won = 0;
    shake_mag = 0.0f;
    lightning_flash = 0.0f;
    win_pulse_phase = 0;
    blitz_timer_counter = 0;
    loss_anim_timer = 0;
}

void Guess(char c) {
    if (game_over) return;
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 'A' || c > 'Z') return;
    
    int idx = c - 'A';
    if (guessed[idx]) return;
    
    guessed[idx] = 1;
    
    int found = 0;
    int all_guessed = 1;
    for (int i = 0; target_word[i] != '\0'; i++) {
        if (target_word[i] == c) found = 1;
        if (!guessed[target_word[i] - 'A']) all_guessed = 0;
    }
    
    if (!found) {
        if (shields > 0) {
            shields--;
            PlaySoundEffect(5); // Shield absorbed
        } else {
            errors++;
            ropeAngularVelocity += ((CustomRand() % 2 == 0) ? 0.25f : -0.25f);
            shake_mag = 12.0f;
            lightning_flash = 1.0f;
            if (errors >= max_errors) {
                game_over = 1;
                won = 0;
                PlaySoundEffect(4); // lose
                SpawnLossParticles();
                stats.losses++;
                stats.streak = 0;
                if (game_mode == 1) {
                    game_mode = 0;
                    campaign_level = 1;
                }
                SaveStats();
            } else {
                PlaySoundEffect(2); // invalid guess
            }
        }
    } else {
        if (game_mode == 2) {
            blitz_time += 2; // +2s per correct letter in Blitz
        }
        all_guessed = 1;
        for (int i = 0; target_word[i] != '\0'; i++) {
            if (!guessed[target_word[i] - 'A']) {
                all_guessed = 0;
                break;
            }
        }
        if (all_guessed) {
            if (game_mode == 2) {
                blitz_words++;
                blitz_time += 15; // +15s bonus per word
                if (blitz_words > stats.blitz_best) {
                    stats.blitz_best = blitz_words;
                }
                PlaySoundEffect(3);
                SpawnWinParticles();
                stats.wins++;
                stats.streak++;
                if (stats.streak > stats.best) stats.best = stats.streak;
                SaveStats();
                SelectNewWord(); // Immediately advance to next word
            } else {
                game_over = 1;
                won = 1;
                PlaySoundEffect(3); // win
                SpawnWinParticles();
                stats.wins++;
                stats.streak++;
                if (stats.streak > stats.best) stats.best = stats.streak;
                SaveStats();
            }
        } else {
            PlaySoundEffect(1); // valid
            SpawnSparkParticles();
        }
    }
}

// Active Skill Implementations
void UseVowelReveal() {
    if (vowel_reveals <= 0 || game_over) return;
    char unvowels[32];
    int count = 0;
    for (int i = 0; target_word[i] != '\0'; i++) {
        char ch = target_word[i];
        if ((ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U') && !guessed[ch - 'A']) {
            int dup = 0;
            for (int j = 0; j < count; j++) { if (unvowels[j] == ch) dup = 1; }
            if (!dup) unvowels[count++] = ch;
        }
    }
    if (count > 0) {
        vowel_reveals--;
        PlaySoundEffect(7);
        Guess(unvowels[CustomRand() % count]);
    }
}

void UseConsonantRadar() {
    if (consonant_radars <= 0 || game_over) return;
    char unconsonants[32];
    int count = 0;
    for (int i = 0; target_word[i] != '\0'; i++) {
        char ch = target_word[i];
        if (ch != 'A' && ch != 'E' && ch != 'I' && ch != 'O' && ch != 'U' && !guessed[ch - 'A']) {
            int dup = 0;
            for (int j = 0; j < count; j++) { if (unconsonants[j] == ch) dup = 1; }
            if (!dup) unconsonants[count++] = ch;
        }
    }
    if (count > 0) {
        consonant_radars--;
        PlaySoundEffect(1);
        Guess(unconsonants[CustomRand() % count]);
    }
}

void UseShield() {
    if (game_over) return;
    MessageBoxA(NULL, "Strike Shield absorbs 1 wrong letter guess automatically!", "Shield Active", MB_OK | MB_ICONINFORMATION);
}

void UseFreezeTimer() {
    if (freezes <= 0 || game_over) return;
    freezes--;
    freeze_timer_seconds = 15;
    PlaySoundEffect(8);
}

void UseBomb() {
    if (bombs <= 0 || game_over) return;
    bombs--;
    PlaySoundEffect(6);
    SpawnBombParticles();
    int elims = 0;
    for (int k = 0; k < 50 && elims < 3; k++) {
        int r = CustomRand() % 26;
        if (!guessed[r]) {
            int in_word = 0;
            for (int w = 0; target_word[w] != '\0'; w++) {
                if (target_word[w] == 'A' + r) { in_word = 1; break; }
            }
            if (!in_word) {
                guessed[r] = 1;
                elims++;
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hCustomEdit = CreateWindowEx(0, "EDIT", "APPLE, BANANA", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, 100, 68, 300, 20, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            if (current_category == TOTAL_CAT_COUNT - 1) ShowWindow(hCustomEdit, SW_SHOW);
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;

        case WM_TIMER:
            if (wParam == 1) {
                anim_ticks++;
                UpdateParticles();

                if (CustomRand() % 100 < 2) {
                    windForce = (float)(CustomRand() % 100 - 50) / 10000.0f; // -0.005 to 0.005
                }
                float ropeAngularAcceleration = -0.05f * (float)sin(ropeAngle) + windForce;
                ropeAngularVelocity += ropeAngularAcceleration;
                ropeAngularVelocity *= 0.95f; // damping
                ropeAngle += ropeAngularVelocity;

                if (CustomRand() % 100 < 2) {
                    particles[particle_count].x = (float)(CustomRand() % W);
                    particles[particle_count].y = (float)(CustomRand() % (H / 3));
                    particles[particle_count].vx = (float)(-(CustomRand() % 100) / 10.0f - 5.0f);
                    particles[particle_count].vy = (float)((CustomRand() % 50) / 10.0f + 2.0f);
                    particles[particle_count].size = CustomRand() % 2 + 1;
                    particles[particle_count].color = RGB(255, 255, 255);
                    particles[particle_count].life = 0;
                    particles[particle_count].maxLife = 20;
                    particles[particle_count].type = 3; // shooting star
                    if (particle_count < MAX_PARTICLES) particle_count++;
                }

                if (game_over && won && (anim_ticks % 3 == 0)) {
                    SpawnWinParticles();
                } else if (game_over && !won) {
                    if (anim_ticks % 4 == 0) SpawnLossParticles();
                    loss_anim_timer++;
                }

                if (shake_mag > 0.0f) {
                    shake_mag *= 0.85f;
                    shake_angle += 1.5f;
                    if (shake_mag < 0.5f) shake_mag = 0.0f;
                }
                if (lightning_flash > 0.0f) {
                    lightning_flash -= 0.1f;
                }
                if (game_over && won) {
                    win_pulse_phase += 10;
                    if (win_pulse_phase > 360) win_pulse_phase -= 360;
                }

                blitz_timer_counter++;
                if (blitz_timer_counter >= 33) { // approx 1 second
                    blitz_timer_counter = 0;
                    if (freeze_timer_seconds > 0) {
                        freeze_timer_seconds--;
                    } else if (game_mode == 2 && !game_over) {
                        if (blitz_time > 0) {
                            blitz_time--;
                            if (blitz_time <= 0) {
                                game_over = 1;
                                won = 0;
                                PlaySoundEffect(4);
                                SpawnLossParticles();
                                stats.losses++;
                                stats.streak = 0;
                                if (blitz_words > stats.blitz_best) stats.blitz_best = blitz_words;
                                SaveStats();
                            }
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;

        case WM_KEYDOWN: {
            if (wParam == 'V' && vowel_reveals > 0 && !game_over) {
                UseVowelReveal();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'H' && consonant_radars > 0 && !game_over) {
                UseConsonantRadar();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'S' && shields > 0 && !game_over) {
                UseShield();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'F' && freezes > 0 && !game_over) {
                UseFreezeTimer();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'B' && bombs > 0 && !game_over) {
                UseBomb();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam >= 'A' && wParam <= 'Z') {
                Guess((char)wParam);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);

            // Row 1 Skill Buttons (y: 515..545)
            // Vowel Reveal (V) [10..100]
            if (x >= 10 && x <= 100 && y >= 515 && y <= 545) {
                UseVowelReveal();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Consonant Radar (H) [105..195]
            if (x >= 105 && x <= 195 && y >= 515 && y <= 545) {
                UseConsonantRadar();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Strike Shield (S) [200..290]
            if (x >= 200 && x <= 290 && y >= 515 && y <= 545) {
                UseShield();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Freeze Timer (F) [295..385]
            if (x >= 295 && x <= 385 && y >= 515 && y <= 545) {
                UseFreezeTimer();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Bomb (B) [390..480]
            if (x >= 390 && x <= 480 && y >= 515 && y <= 545) {
                UseBomb();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Row 2 Mode Buttons (y: 555..585)
            // Campaign [15..160]
            if (x >= 15 && x <= 160 && y >= 555 && y <= 585) {
                SetFocus(hwnd);
                if (game_over && won && game_mode == 1 && campaign_level < 20) {
                    campaign_level++;
                } else {
                    game_mode = 1;
                    campaign_level = 1;
                }
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Time Attack Blitz [170..315]
            if (x >= 170 && x <= 315 && y >= 555 && y <= 585) {
                SetFocus(hwnd);
                game_mode = 2;
                blitz_time = 60;
                blitz_words = 0;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Freeplay [325..470]
            if (x >= 325 && x <= 470 && y >= 555 && y <= 585) {
                SetFocus(hwnd);
                game_mode = 0;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Row 3 Action Buttons (y: 595..625)
            // Save (15..95)
            if (x >= 15 && x <= 95 && y >= 595 && y <= 625) {
                SaveGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Load (105..185)
            if (x >= 105 && x <= 185 && y >= 595 && y <= 625) {
                LoadGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Help (195..275)
            if (x >= 195 && x <= 275 && y >= 595 && y <= 625) {
                MessageBoxA(hwnd,
                    "KHangman - Rules & Game Modes\n\n"
                    "Rules: Guess the word letter by letter.\n\n"
                    "Modes:\n"
                    " • Freeplay: Classic Hangman mode.\n"
                    " • Campaign: 20 Stages across 10 categories ending in Stage 20 Grandmaster Polymath Challenge.\n"
                    " • Time Attack Blitz: 60s timer! Solve as many words as you can!\n\n"
                    "Active Skills & Power-ups:\n"
                    " • Vowel Reveal [V]: Reveals 1 hidden vowel in target word.\n"
                    " • Consonant Radar [H]: Reveals 1 hidden consonant.\n"
                    " • Strike Shield [S]: Absorbs 1 wrong letter penalty.\n"
                    " • Freeze Timer [F]: Freezes stage/blitz timer for 15s.\n"
                    " • Bomb Nuke [B]: Eliminates 3 incorrect letters.",
                    "Help / How to Play", MB_OK | MB_ICONINFORMATION);
                break;
            }
            // Sound (285..365)
            if (x >= 285 && x <= 365 && y >= 595 && y <= 625) {
                is_muted = !is_muted;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            // Reset Stats (375..475)
            if (x >= 375 && x <= 475 && y >= 595 && y <= 625) {
                if (MessageBoxA(hwnd, "Reset all lifetime statistics?", "Reset Stats", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    stats.wins = 0;
                    stats.losses = 0;
                    stats.streak = 0;
                    stats.best = 0;
                    stats.blitz_best = 0;
                    SaveStats();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }

            // Check categories (2 rows of 6)
            int cy = 25;
            for (int r = 0; r < 2; r++) {
                int cx = 10;
                for (int c = 0; c < 6; c++) {
                    int cat_idx = r * 6 + c;
                    if (cat_idx < TOTAL_CAT_COUNT) {
                        if (x >= cx && x <= cx + 75 && y >= cy && y <= cy + 18) {
                            if (current_category != cat_idx) {
                                current_category = cat_idx;
                                game_mode = 0; // switch to freeplay on category click
                                if (current_category == TOTAL_CAT_COUNT - 1) ShowWindow(hCustomEdit, SW_SHOW);
                                else ShowWindow(hCustomEdit, SW_HIDE);
                                SetFocus(hwnd);
                                InitGame();
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                            break;
                        }
                    }
                    cx += 80;
                }
                cy += 20;
            }

            // Check keyboard clicks
            int kx = 110;
            int ky = 350;
            for (int i = 0; i < 26; i++) {
                int col = i % 7;
                int row = i / 7;
                int bx = kx + col * 40;
                int by = ky + row * 40;
                if (x >= bx && x <= bx + 35 && y >= by && y <= by + 35) {
                    Guess('A' + i);
                    InvalidateRect(hwnd, NULL, TRUE);
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
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);

            int vpX = (int)(cosf(shake_angle) * shake_mag);
            int vpY = (int)(sinf(shake_angle * 1.2f) * shake_mag);
            SetViewportOrgEx(memDC, vpX, vpY, NULL);

            // Spooky Environmental Art Background
            // Night Sky
            HBRUSH bgBrush = CreateSolidBrush(lightning_flash > 0.0f ? RGB(40, 50, 70) : RGB(11, 16, 33));
            RECT fullRc = {-50, -50, W + 50, H + 50};
            FillRect(memDC, &fullRc, bgBrush);
            DeleteObject(bgBrush);

            // Stars
            for (int i = 0; i < 40; i++) {
                int sx = (i * 997) % W;
                int sy = (i * 541) % (H / 2);
                SetPixel(memDC, sx, sy, RGB(255, 255, 255));
                if (i % 3 == 0) SetPixel(memDC, sx + 1, sy, RGB(255, 255, 255));
            }

            // Aurora
            float auroraPhase = anim_ticks * 0.05f;
            HPEN auroraPen = CreatePen(PS_SOLID, 16, RGB(0, 100 + (int)(50*sin(auroraPhase)), 50 + (int)(30*cos(auroraPhase))));
            SelectObject(memDC, auroraPen);
            POINT pts[4];
            pts[0].x = 0; pts[0].y = 50 + (int)(20*sin(auroraPhase));
            pts[1].x = W/3; pts[1].y = 20 + (int)(30*cos(auroraPhase));
            pts[2].x = 2*W/3; pts[2].y = 80 + (int)(20*sin(auroraPhase));
            pts[3].x = W; pts[3].y = 40 + (int)(30*cos(auroraPhase));
            PolyBezier(memDC, pts, 4);
            DeleteObject(auroraPen);

            HPEN auroraPen2 = CreatePen(PS_SOLID, 12, RGB(50 + (int)(30*cos(auroraPhase)), 0, 100 + (int)(50*sin(auroraPhase))));
            SelectObject(memDC, auroraPen2);
            pts[0].y -= 20; pts[1].y += 10; pts[2].y -= 10; pts[3].y += 20;
            PolyBezier(memDC, pts, 4);
            DeleteObject(auroraPen2);

            // Moon
            HBRUSH moonBrush = CreateSolidBrush(RGB(240, 240, 230));
            HPEN noPen = CreatePen(PS_NULL, 0, RGB(0,0,0));
            HPEN oldPen = (HPEN)SelectObject(memDC, noPen);
            HBRUSH oldB = (HBRUSH)SelectObject(memDC, moonBrush);
            Ellipse(memDC, W - 120, 40, W - 40, 120);

            // Moon Craters
            HBRUSH craterBrush = CreateSolidBrush(RGB(200, 200, 190));
            SelectObject(memDC, craterBrush);
            Ellipse(memDC, W - 100, 60, W - 80, 80);
            Ellipse(memDC, W - 70, 80, W - 55, 95);
            Ellipse(memDC, W - 85, 50, W - 75, 60);

            // Terrain Hills (Back)
            HBRUSH hill1Brush = CreateSolidBrush(RGB(17, 10, 18));
            SelectObject(memDC, hill1Brush);
            POINT hill1[5] = {{0, H}, {0, H - 200}, {W / 2, H - 350}, {W, H - 250}, {W, H}};
            Polygon(memDC, hill1, 5);

            // Terrain Hills (Front)
            HBRUSH hill2Brush = CreateSolidBrush(RGB(10, 5, 11));
            SelectObject(memDC, hill2Brush);
            POINT hill2[5] = {{0, H}, {0, H - 150}, {W / 3, H - 220}, {W, H - 100}, {W, H}};
            Polygon(memDC, hill2, 5);

            // Dead tree silhouette
            HPEN treePen = CreatePen(PS_SOLID, 4, RGB(5, 2, 5));
            SelectObject(memDC, treePen);
            MoveToEx(memDC, W - 80, H - 150, NULL); LineTo(memDC, W - 80, H - 300);
            MoveToEx(memDC, W - 80, H - 250, NULL); LineTo(memDC, W - 130, H - 330);
            MoveToEx(memDC, W - 80, H - 220, NULL); LineTo(memDC, W - 40, H - 280);

            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldB);
            DeleteObject(noPen);
            DeleteObject(moonBrush);
            DeleteObject(craterBrush);
            DeleteObject(hill1Brush);
            DeleteObject(hill2Brush);
            DeleteObject(treePen);

            SetBkMode(memDC, TRANSPARENT);
            
            HFONT hFontMain;
            if (game_over && won) {
                int size = 22 + (4 * abs(win_pulse_phase - 180) / 180);
                hFontMain = CreateFontA(size, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            } else {
                hFontMain = CreateFontA(22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            }
            HFONT hFontMono = CreateFontA(15, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
            HFONT hFontSmall = CreateFontA(12, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            
            // Title
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMain);
            TextOutA(memDC, 195, 2, "KHangman", 8);

            // Categories rendering (2 rows of 6)
            SelectObject(memDC, hFontSmall);
            int catY = 25;
            for (int r = 0; r < 2; r++) {
                int cx = 10;
                for (int c = 0; c < 6; c++) {
                    int cat_idx = r * 6 + c;
                    if (cat_idx < TOTAL_CAT_COUNT) {
                        RECT cRect = {cx, catY, cx + 75, catY + 18};
                        HBRUSH cBg = CreateSolidBrush((cat_idx == current_category && game_mode == 0) ? RGB(0, 137, 204) : RGB(35, 44, 58));
                        FillRect(memDC, &cRect, cBg);
                        DeleteObject(cBg);
                        SetTextColor(memDC, RGB(240, 244, 248));
                        DrawTextA(memDC, CAT_NAMES[cat_idx], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                    cx += 80;
                }
                catY += 20;
            }

            // Dramatic Lightning Bolt
            if (lightning_flash > 0.5f) {
                HPEN lightPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
                HPEN oldLP = (HPEN)SelectObject(memDC, lightPen);
                POINT bolt[6] = {{W/2 - 20, 0}, {W/2 + 10, 80}, {W/2 - 10, 90}, {W/2 + 30, 180}, {W/2, 190}, {W/2 + 20, 280}};
                Polyline(memDC, bolt, 6);
                SelectObject(memDC, oldLP);
                DeleteObject(lightPen);
            }

            int ox = 0; // Handled by SetViewportOrgEx

            int dark = errors * 15;
            #define G_COLOR(r, g, b) RGB((r - dark < 0) ? 0 : r - dark, (g - dark < 0) ? 0 : g - dark, (b - dark < 0) ? 0 : b - dark)

            // 3D Wooden Gallows Rendering
            RECT baseRc = {180 + ox, 260, 320 + ox, 270};
            HBRUSH baseBrush = CreateSolidBrush(G_COLOR(110, 65, 30));
            FillRect(memDC, &baseRc, baseBrush);
            DeleteObject(baseBrush);

            HPEN hHighlightPen = CreatePen(PS_SOLID, 1, G_COLOR(170, 105, 50));
            HPEN hShadowPen = CreatePen(PS_SOLID, 1, G_COLOR(50, 30, 15));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hHighlightPen);
            MoveToEx(memDC, 180 + ox, 260, NULL); LineTo(memDC, 320 + ox, 260);
            SelectObject(memDC, hShadowPen);
            MoveToEx(memDC, 180 + ox, 270, NULL); LineTo(memDC, 320 + ox, 270);

            // Vertical Post
            RECT postRc = {205 + ox, 115, 217 + ox, 260};
            HBRUSH postBrush = CreateSolidBrush(G_COLOR(125, 75, 35));
            FillRect(memDC, &postRc, postBrush);
            DeleteObject(postBrush);

            SelectObject(memDC, hHighlightPen);
            MoveToEx(memDC, 205 + ox, 115, NULL); LineTo(memDC, 205 + ox, 260);
            SelectObject(memDC, hShadowPen);
            MoveToEx(memDC, 217 + ox, 115, NULL); LineTo(memDC, 217 + ox, 260);

            // Beam
            RECT beamRc = {205 + ox, 115, 280 + ox, 127};
            HBRUSH beamBrush = CreateSolidBrush(G_COLOR(135, 80, 40));
            FillRect(memDC, &beamRc, beamBrush);
            DeleteObject(beamBrush);

            SelectObject(memDC, hHighlightPen);
            MoveToEx(memDC, 205 + ox, 115, NULL); LineTo(memDC, 280 + ox, 115);
            SelectObject(memDC, hShadowPen);
            MoveToEx(memDC, 205 + ox, 127, NULL); LineTo(memDC, 280 + ox, 127);

            // Support Brace
            POINT bracePts[4] = {{205 + ox, 150}, {240 + ox, 127}, {247 + ox, 127}, {205 + ox, 160}};
            HBRUSH braceBrush = CreateSolidBrush(G_COLOR(105, 60, 28));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, braceBrush);
            Polygon(memDC, bracePts, 4);
            SelectObject(memDC, hOldBrush);
            DeleteObject(braceBrush);

            // Splinters
            if (errors >= 3) {
                HPEN hSplinterPen = CreatePen(PS_SOLID, 1, G_COLOR(30, 15, 5));
                SelectObject(memDC, hSplinterPen);
                MoveToEx(memDC, 210 + ox, 180, NULL); LineTo(memDC, 214 + ox, 185); LineTo(memDC, 212 + ox, 192);
                MoveToEx(memDC, 215 + ox, 220, NULL); LineTo(memDC, 208 + ox, 225); LineTo(memDC, 212 + ox, 230);
                MoveToEx(memDC, 250 + ox, 120, NULL); LineTo(memDC, 255 + ox, 123); LineTo(memDC, 260 + ox, 118);
                if (errors >= 5) {
                    MoveToEx(memDC, 206 + ox, 140, NULL); LineTo(memDC, 215 + ox, 144); LineTo(memDC, 210 + ox, 150);
                    MoveToEx(memDC, 230 + ox, 125, NULL); LineTo(memDC, 235 + ox, 120); LineTo(memDC, 240 + ox, 125);
                }
                DeleteObject(hSplinterPen);
            }

            DeleteObject(hHighlightPen);
            DeleteObject(hShadowPen);
            #undef G_COLOR

            // Rope Physics
            float nooseSwayAngle = ropeAngle;
            if (game_over && won) nooseSwayAngle = (float)sin(anim_ticks * 0.3) * 0.08f;

            int ropeX1 = 270 + ox;
            int ropeY1 = 124;
            int ropeLen = 22;
            int hangX = ropeX1 + (int)(sin(nooseSwayAngle) * ropeLen);
            int hangY = ropeY1 + (int)(cos(nooseSwayAngle) * ropeLen);

            HPEN hRopePen = CreatePen(PS_SOLID, 3, RGB(210, 180, 140));
            SelectObject(memDC, hRopePen);
            MoveToEx(memDC, ropeX1, ropeY1, NULL);
            LineTo(memDC, hangX, hangY);
            DeleteObject(hRopePen);

            // Character Sprite
            int cx = hangX;
            int cy = hangY;
            int idleBounce = (game_over && won) ? abs((int)(sin(anim_ticks * 0.4) * 8)) : (int)(sin(anim_ticks * 0.15) * 2);

            if (errors > 0) {
                // Head (Error 1)
                int headY = cy + 12 - idleBounce;
                int headR = 12;

                int faceR = 255;
                int faceG = 220;
                int faceB = 180;
                float faceHealth = (float)(max_errors - errors) / (float)max_errors;
                if (faceHealth < 0) faceHealth = 0;
                float shadowIntensity = (1.0f - faceHealth) * 0.8f;
                faceR = (int)(faceR * (1.0f - shadowIntensity * 0.5f));
                faceG = (int)(faceG * (1.0f - shadowIntensity * 0.6f));
                faceB = (int)(faceB * (1.0f - shadowIntensity * 0.6f));

                HBRUSH headBrush = CreateSolidBrush(RGB(faceR, faceG, faceB));
                HPEN charPen = CreatePen(PS_SOLID, 2, RGB(45, 55, 70));
                SelectObject(memDC, headBrush);
                SelectObject(memDC, charPen);
                Ellipse(memDC, cx - headR, headY - headR, cx + headR, headY + headR);
                DeleteObject(headBrush);

                // Cap
                HBRUSH capBrush = CreateSolidBrush(RGB(2, 136, 209));
                SelectObject(memDC, capBrush);
                Chord(memDC, cx - headR, headY - headR, cx + headR, headY + 2, cx + headR, headY - 1, cx - headR, headY - 1);
                RECT brimRc = {cx - headR - 2, headY - 3, cx + headR + 2, headY + 1};
                FillRect(memDC, &brimRc, capBrush);
                DeleteObject(capBrush);

                // Expressions
                SelectObject(memDC, charPen);
                if (game_over && won) {
                    MoveToEx(memDC, cx - 7, headY - 1, NULL); LineTo(memDC, cx - 4, headY - 4); LineTo(memDC, cx - 1, headY - 1);
                    MoveToEx(memDC, cx + 1, headY - 1, NULL); LineTo(memDC, cx + 4, headY - 4); LineTo(memDC, cx + 7, headY - 1);
                } else if (game_over && !won) {
                    MoveToEx(memDC, cx - 7, headY - 4, NULL); LineTo(memDC, cx - 3, headY);
                    MoveToEx(memDC, cx - 3, headY - 4, NULL); LineTo(memDC, cx - 7, headY);
                    MoveToEx(memDC, cx + 3, headY - 4, NULL); LineTo(memDC, cx + 7, headY);
                    MoveToEx(memDC, cx + 7, headY - 4, NULL); LineTo(memDC, cx + 3, headY);
                } else if (shake_mag > 0.0f) {
                    HBRUSH eyeBrush = CreateSolidBrush(RGB(20, 20, 50));
                    SelectObject(memDC, eyeBrush);
                    Ellipse(memDC, cx - 7, headY - 5, cx - 1, headY + 1);
                    Ellipse(memDC, cx + 1, headY - 5, cx + 7, headY + 1);
                    DeleteObject(eyeBrush);
                    HBRUSH pupil = CreateSolidBrush(RGB(255, 255, 255));
                    SelectObject(memDC, pupil);
                    Ellipse(memDC, cx - 5, headY - 3, cx - 3, headY - 1);
                    Ellipse(memDC, cx + 3, headY - 3, cx + 5, headY - 1);
                    DeleteObject(pupil);
                } else {
                    HBRUSH eyeBrush = CreateSolidBrush(RGB(20, 20, 50));
                    SelectObject(memDC, eyeBrush);
                    Ellipse(memDC, cx - 6, headY - 4, cx - 2, headY);
                    Ellipse(memDC, cx + 2, headY - 4, cx + 6, headY);
                    DeleteObject(eyeBrush);
                }

                // Torso & Limbs
                if (errors > 1) {
                    int bodyY1 = headY + headR;
                    int bodyY2 = bodyY1 + 28;
                    RECT bodyRc = {cx - 7, bodyY1, cx + 7, bodyY2};
                    HBRUSH bodyBrush = CreateSolidBrush(RGB(0, 137, 123));
                    FillRect(memDC, &bodyRc, bodyBrush);
                    FrameRect(memDC, &bodyRc, (HBRUSH)GetStockObject(BLACK_BRUSH));
                    DeleteObject(bodyBrush);

                    // Clothing Details (Folds and Buttons)
                    HPEN foldPen = CreatePen(PS_SOLID, 1, RGB(0, 77, 64));
                    SelectObject(memDC, foldPen);
                    MoveToEx(memDC, cx - 4, bodyY1 + 5, NULL); LineTo(memDC, cx - 2, bodyY1 + 12);
                    MoveToEx(memDC, cx + 5, bodyY1 + 6, NULL); LineTo(memDC, cx + 3, bodyY1 + 15);
                    MoveToEx(memDC, cx - 3, bodyY1 + 20, NULL); LineTo(memDC, cx - 1, bodyY1 + 25);
                    DeleteObject(foldPen);

                    HBRUSH btnBrush = CreateSolidBrush(RGB(255, 235, 59));
                    HPEN btnPen = CreatePen(PS_SOLID, 1, RGB(188, 155, 0));
                    SelectObject(memDC, btnBrush);
                    SelectObject(memDC, btnPen);
                    Ellipse(memDC, cx - 2, bodyY1 + 5, cx + 2, bodyY1 + 9);
                    Ellipse(memDC, cx - 2, bodyY1 + 13, cx + 2, bodyY1 + 17);
                    Ellipse(memDC, cx - 2, bodyY1 + 21, cx + 2, bodyY1 + 25);
                    DeleteObject(btnBrush);
                    DeleteObject(btnPen);

                    if (errors > 2) {
                        SelectObject(memDC, charPen);
                        MoveToEx(memDC, cx - 7, bodyY1 + 5, NULL);
                        if (game_over && won) LineTo(memDC, cx - 18, bodyY1 - 10);
                        else if (shake_mag > 0.0f) LineTo(memDC, cx - 20, bodyY1 + 5 + (int)(sinf(anim_ticks * 1.5f) * 10));
                        else LineTo(memDC, cx - 18, bodyY1 + 18);
                    }
                    if (errors > 3) {
                        SelectObject(memDC, charPen);
                        MoveToEx(memDC, cx + 7, bodyY1 + 5, NULL);
                        if (game_over && won) LineTo(memDC, cx + 18, bodyY1 - 10);
                        else if (shake_mag > 0.0f) LineTo(memDC, cx + 20, bodyY1 + 5 - (int)(sinf(anim_ticks * 1.5f) * 10));
                        else LineTo(memDC, cx + 18, bodyY1 + 18);
                    }
                    if (errors > 4) {
                        HPEN legPen = CreatePen(PS_SOLID, 3, RGB(55, 71, 79));
                        SelectObject(memDC, legPen);
                        MoveToEx(memDC, cx - 4, bodyY2, NULL);
                        LineTo(memDC, cx - 8, bodyY2 + 18);
                        DeleteObject(legPen);
                    }
                    if (errors > 5) {
                        HPEN legPen = CreatePen(PS_SOLID, 3, RGB(55, 71, 79));
                        SelectObject(memDC, legPen);
                        MoveToEx(memDC, cx + 4, bodyY2, NULL);
                        LineTo(memDC, cx + 8, bodyY2 + 18);
                        DeleteObject(legPen);
                    }
                }
                DeleteObject(charPen);
            }

            // 4.5. Ghost floating up on loss (Loop 2)
            if (game_over && !won) {
                int ghostY = (int)(hangY - 20 - loss_anim_timer * 1.5);
                if (ghostY > -50) {
                    HPEN noPen = CreatePen(PS_NULL, 0, RGB(0,0,0));
                    HBRUSH ghostBrush = CreateSolidBrush(RGB(224, 247, 250));
                    HPEN oldP = (HPEN)SelectObject(memDC, noPen);
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, ghostBrush);
                    
                    int sway = (int)(sin(loss_anim_timer * 0.15) * 4.0);
                    Ellipse(memDC, cx - 14 + sway, ghostY - 14, cx + 14 + sway, ghostY + 14);
                    POINT tail[3];
                    tail[0].x = cx - 14 + sway; tail[0].y = ghostY + 5;
                    tail[1].x = cx + 14 + sway; tail[1].y = ghostY + 5;
                    tail[2].x = cx + sway;      tail[2].y = ghostY + 22;
                    Polygon(memDC, tail, 3);
                    
                    HBRUSH eyeBrush = CreateSolidBrush(RGB(0, 96, 100));
                    SelectObject(memDC, eyeBrush);
                    Ellipse(memDC, cx - 7 + sway, ghostY - 6, cx - 3 + sway, ghostY - 2);
                    Ellipse(memDC, cx + 3 + sway, ghostY - 6, cx + 7 + sway, ghostY - 2);
                    
                    Ellipse(memDC, cx - 2 + sway, ghostY + 2, cx + 2 + sway, ghostY + 8);
                    
                    SelectObject(memDC, oldP);
                    SelectObject(memDC, oldB);
                    DeleteObject(noPen);
                    DeleteObject(ghostBrush);
                    DeleteObject(eyeBrush);
                }
            }

            // Word display
            COLORREF wordColor = RGB(240, 244, 248);
            if (game_over && won) {
                int g = 175 + (50 * abs(win_pulse_phase - 180) / 180);
                int r = 76 + (50 * abs(win_pulse_phase - 180) / 180);
                int b = 80 + (50 * abs(win_pulse_phase - 180) / 180);
                wordColor = RGB(r, g, b);
            }
            SetTextColor(memDC, wordColor);
            SelectObject(memDC, hFontMain);
            char disp[100] = {0};
            int len = 0;
            for (int i = 0; target_word[i] != '\0'; i++) {
                if (guessed[target_word[i] - 'A']) {
                    disp[len++] = target_word[i];
                } else {
                    disp[len++] = '_';
                }
                disp[len++] = ' ';
            }
            disp[len] = '\0';
            
            SIZE tSize;
            GetTextExtentPoint32A(memDC, disp, len, &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 275, disp, len);

            // Status message
            char msgBuf[128] = "Guess a letter to start";
            COLORREF msgColor = RGB(240, 244, 248);
            if (game_mode == 1) {
                int s_idx = (campaign_level - 1) % 20;
                if (freeze_timer_seconds > 0) {
                    wsprintfA(msgBuf, "%s | ❄️ FROZEN (%ds)", CAMPAIGN_STAGES[s_idx].stage_name, freeze_timer_seconds);
                } else {
                    wsprintfA(msgBuf, "%s (Max Strikes: %d)", CAMPAIGN_STAGES[s_idx].stage_name, max_errors);
                }
                msgColor = RGB(255, 193, 7);
            } else if (game_mode == 2) {
                if (freeze_timer_seconds > 0) {
                    wsprintfA(msgBuf, "⚡ Blitz Time: %ds | Words: %d | ❄️ FROZEN (%ds)", blitz_time, blitz_words, freeze_timer_seconds);
                } else {
                    wsprintfA(msgBuf, "⚡ Blitz Time: %ds | Words Solved: %d", blitz_time, blitz_words);
                }
                msgColor = (blitz_time <= 10) ? RGB(244, 67, 54) : RGB(79, 172, 254);
            }
            if (game_over) {
                if (won) {
                    if (game_mode == 1 && campaign_level == 20) strcpy(msgBuf, "Stage 20 Polymath Complete! Champion!");
                    else strcpy(msgBuf, "Stage Clear! You Win!");
                    msgColor = RGB(76, 175, 80);
                } else {
                    wsprintfA(msgBuf, "Game Over! Word was: %s", target_word);
                    msgColor = RGB(244, 67, 54);
                }
            }
            SetTextColor(memDC, msgColor);
            GetTextExtentPoint32A(memDC, msgBuf, lstrlenA(msgBuf), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 315, msgBuf, lstrlenA(msgBuf));

            // On-screen Keyboard (y: 350..490)
            SelectObject(memDC, hFontMono);
            int kx = 110;
            int ky = 350;
            for (int i = 0; i < 26; i++) {
                int col = i % 7;
                int row = i / 7;
                int bx = kx + col * 40;
                int by = ky + row * 40;
                
                RECT btnRect = {bx, by, bx + 35, by + 35};
                COLORREF keyFill, highlightCol, shadowCol, textCol;
                const char* badgeStr = NULL;

                if (guessed[i]) {
                    int is_correct = 0;
                    for (int w = 0; target_word[w] != '\0'; w++) {
                        if (target_word[w] == 'A' + i) { is_correct = 1; break; }
                    }
                    if (is_correct) {
                        keyFill = RGB(46, 125, 50); highlightCol = RGB(76, 175, 80); shadowCol = RGB(20, 70, 25); textCol = RGB(255, 255, 255); badgeStr = "v";
                    } else {
                        keyFill = RGB(150, 35, 35); highlightCol = RGB(220, 70, 70); shadowCol = RGB(60, 10, 10); textCol = RGB(255, 190, 190); badgeStr = "x";
                    }
                } else {
                    keyFill = RGB(40, 48, 62); highlightCol = RGB(75, 90, 115); shadowCol = RGB(15, 20, 28); textCol = RGB(240, 244, 248);
                }

                HBRUSH btnBg = CreateSolidBrush(keyFill);
                FillRect(memDC, &btnRect, btnBg);
                DeleteObject(btnBg);

                HPEN hH = CreatePen(PS_SOLID, 1, highlightCol);
                HPEN hS = CreatePen(PS_SOLID, 1, shadowCol);
                SelectObject(memDC, hH);
                MoveToEx(memDC, bx, by + 34, NULL); LineTo(memDC, bx, by); LineTo(memDC, bx + 34, by);
                SelectObject(memDC, hS);
                MoveToEx(memDC, bx + 34, by, NULL); LineTo(memDC, bx + 34, by + 34); LineTo(memDC, bx, by + 34);
                DeleteObject(hH);
                DeleteObject(hS);

                SetTextColor(memDC, textCol);
                char l[2] = {(char)('A' + i), 0};
                DrawTextA(memDC, l, 1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                if (badgeStr) {
                    SelectObject(memDC, hFontSmall);
                    SetTextColor(memDC, (badgeStr[0] == 'v') ? RGB(165, 214, 167) : RGB(239, 154, 154));
                    RECT badgeRc = {bx + 22, by + 1, bx + 33, by + 12};
                    DrawTextA(memDC, badgeStr, 1, &badgeRc, DT_RIGHT | DT_TOP | DT_SINGLELINE);
                    SelectObject(memDC, hFontMono);
                }
            }

            SelectObject(memDC, hFontSmall);

            // Row 1 Active Skill Buttons (y: 515..545)
            // Vowel [V] (10..100)
            RECT vRc = {10, 515, 100, 545};
            HBRUSH vBg = CreateSolidBrush((vowel_reveals <= 0 || game_over) ? RGB(45, 52, 65) : RGB(0, 150, 136));
            FillRect(memDC, &vRc, vBg); DeleteObject(vBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            char vBuf[32]; wsprintfA(vBuf, "Vowel[V](%d)", vowel_reveals);
            DrawTextA(memDC, vBuf, -1, &vRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Radar [H] (105..195)
            RECT hRc = {105, 515, 195, 545};
            HBRUSH hBg = CreateSolidBrush((consonant_radars <= 0 || game_over) ? RGB(45, 52, 65) : RGB(251, 140, 0));
            FillRect(memDC, &hRc, hBg); DeleteObject(hBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            char hBuf[32]; wsprintfA(hBuf, "Radar[H](%d)", consonant_radars);
            DrawTextA(memDC, hBuf, -1, &hRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Shield [S] (200..290)
            RECT sRc = {200, 515, 290, 545};
            HBRUSH sBg = CreateSolidBrush((shields <= 0 || game_over) ? RGB(45, 52, 65) : RGB(142, 36, 170));
            FillRect(memDC, &sRc, sBg); DeleteObject(sBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sBuf[32]; wsprintfA(sBuf, "Shield[S](%d)", shields);
            DrawTextA(memDC, sBuf, -1, &sRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Freeze [F] (295..385)
            RECT fRc = {295, 515, 385, 545};
            HBRUSH fBg = CreateSolidBrush((freezes <= 0 || game_over) ? RGB(45, 52, 65) : RGB(3, 169, 244));
            FillRect(memDC, &fRc, fBg); DeleteObject(fBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            char fBuf[32]; wsprintfA(fBuf, "Freeze[F](%d)", freezes);
            DrawTextA(memDC, fBuf, -1, &fRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Bomb [B] (390..480)
            RECT bRc = {390, 515, 480, 545};
            HBRUSH bBg = CreateSolidBrush((bombs <= 0 || game_over) ? RGB(45, 52, 65) : RGB(229, 57, 53));
            FillRect(memDC, &bRc, bBg); DeleteObject(bBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            char bBuf[32]; wsprintfA(bBuf, "Bomb[B](%d)", bombs);
            DrawTextA(memDC, bBuf, -1, &bRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Row 2 Game Modes (y: 555..585)
            RECT campRect = {15, 555, 160, 585};
            HBRUSH campBg = CreateSolidBrush(game_mode == 1 ? RGB(0, 137, 123) : RGB(50, 60, 75));
            FillRect(memDC, &campRect, campBg); DeleteObject(campBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            if (game_over && won && game_mode == 1 && campaign_level < 20) {
                DrawTextA(memDC, "Next Stage", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextA(memDC, "Campaign (20)", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            RECT blitzRect = {170, 555, 315, 585};
            HBRUSH blitzBg = CreateSolidBrush(game_mode == 2 ? RGB(216, 27, 96) : RGB(50, 60, 75));
            FillRect(memDC, &blitzRect, blitzBg); DeleteObject(blitzBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Blitz Mode", -1, &blitzRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT resRect = {325, 555, 470, 585};
            HBRUSH resBg = CreateSolidBrush(game_mode == 0 ? RGB(2, 136, 209) : RGB(50, 60, 75));
            FillRect(memDC, &resRect, resBg); DeleteObject(resBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Freeplay", -1, &resRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Row 3 Actions (y: 595..625)
            RECT saveRect = {15, 595, 95, 625};
            HBRUSH saveBg = CreateSolidBrush(RGB(67, 160, 71));
            FillRect(memDC, &saveRect, saveBg); DeleteObject(saveBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Save", -1, &saveRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT loadRect = {105, 595, 185, 625};
            HBRUSH loadBg = CreateSolidBrush(RGB(106, 27, 154));
            FillRect(memDC, &loadRect, loadBg); DeleteObject(loadBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Load", -1, &loadRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT helpRect = {195, 595, 275, 625};
            HBRUSH helpBg = CreateSolidBrush(RGB(30, 136, 229));
            FillRect(memDC, &helpRect, helpBg); DeleteObject(helpBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Help", -1, &helpRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT muteRect = {285, 595, 365, 625};
            HBRUSH muteBg = CreateSolidBrush(RGB(69, 90, 100));
            FillRect(memDC, &muteRect, muteBg); DeleteObject(muteBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, is_muted ? "Muted" : "Sound", -1, &muteRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT rstRect = {375, 595, 475, 625};
            HBRUSH rstBg = CreateSolidBrush(RGB(84, 110, 122));
            FillRect(memDC, &rstRect, rstBg); DeleteObject(rstBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Reset Stats", -1, &rstRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            DeleteObject(hFontSmall);

            // Stats footer (y: 645)
            char statText[160];
            wsprintfA(statText, "Wins: %d | Loss: %d | Streak: %d (Best: %d) | Blitz Best: %d", stats.wins, stats.losses, stats.streak, stats.best, stats.blitz_best);
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMono);
            GetTextExtentPoint32A(memDC, statText, lstrlenA(statText), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 645, statText, lstrlenA(statText));

            // Render particles
            for (int i = 0; i < particle_count; i++) {
                if (particles[i].type == 4) { // firework
                    HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                    int s = particles[i].size;
                    RECT pRc = {(int)particles[i].x - s, (int)particles[i].y - s, (int)particles[i].x + s, (int)particles[i].y + s};
                    FillRect(memDC, &pRc, pBrush);
                    DeleteObject(pBrush);
                } else if (particles[i].type == 2) { // spark
                    HBRUSH pBrush2 = CreateSolidBrush(particles[i].color);
                    RECT pRc2 = {(int)particles[i].x - 1, (int)particles[i].y - 1, (int)particles[i].x + particles[i].size + 1, (int)particles[i].y + particles[i].size + 1};
                    FillRect(memDC, &pRc2, pBrush2);
                    DeleteObject(pBrush2);
                    HBRUSH pBrush = CreateSolidBrush(RGB(255, 255, 255));
                    RECT pRc = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + particles[i].size, (int)particles[i].y + particles[i].size};
                    FillRect(memDC, &pRc, pBrush);
                    DeleteObject(pBrush);
                } else if (particles[i].type == 3) { // shooting star
                    HPEN ssPen = CreatePen(PS_SOLID, particles[i].size, particles[i].color);
                    HPEN oldPen = (HPEN)SelectObject(memDC, ssPen);
                    MoveToEx(memDC, (int)particles[i].x, (int)particles[i].y, NULL);
                    LineTo(memDC, (int)(particles[i].x - particles[i].vx * 3), (int)(particles[i].y - particles[i].vy * 3));
                    SelectObject(memDC, oldPen);
                    DeleteObject(ssPen);
                } else if (particles[i].type == 5) { // debris
                    HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                    int s = particles[i].size;
                    RECT pRc = {(int)particles[i].x - s, (int)particles[i].y - s, (int)particles[i].x + s, (int)particles[i].y + s};
                    FillRect(memDC, &pRc, pBrush);
                    DeleteObject(pBrush);
                    if (s > 4) {
                        HBRUSH darkBrush = CreateSolidBrush(RGB(0, 0, 0));
                        RECT dRc = {(int)particles[i].x - s/2, (int)particles[i].y - s/2, (int)particles[i].x + s/2, (int)particles[i].y + s/2};
                        FillRect(memDC, &dRc, darkBrush);
                        DeleteObject(darkBrush);
                    }
                } else {
                    HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                    RECT pRc = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + particles[i].size, (int)particles[i].y + particles[i].size};
                    FillRect(memDC, &pRc, pBrush);
                    DeleteObject(pBrush);
                }
            }

            DeleteObject(hFontMain);
            DeleteObject(hFontMono);

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
            DeleteDC(memDC);
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

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KHangmanApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KHangmanApp", "KHangman", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
