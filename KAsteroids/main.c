#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TIMER_ID 1
#define WIDTH 800
#define HEIGHT 600

const char g_szClassName[] = "KAsteroidsClass";

typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    bool active;
    float radius;
    int hyperdrive_cooldown;
    float anim_frame;
} Ship;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    int level;
    int hp;
    bool is_armored;
    bool active;
    float points[10];
    float rot;
    float rot_speed;
    int type; // 0 = Basalt Slate, 1 = Iron Rust, 2 = Cryo Ice
} Asteroid;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    bool active;
    bool is_enemy;
    bool is_laser;
} Bullet;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    bool active;
    int shoot_timer;
    int type; // 0 = Normal, 1 = Mother Boss
    int hp;
    int max_hp;
    float anim_frame;
} Ufo;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    bool active;
    float anim_frame;
} Mine;

typedef struct {
    float x, y;
    float vx, vy;
    int type; // 1 = Shield, 2 = Spread, 3 = EMP, 4 = Laser
    int life;
    bool active;
    float anim_frame;
} PowerUp;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    int max_life;
    COLORREF color;
    bool active;
    float size;
    int type; // 0 = spark, 1 = smoke
} Particle;

typedef struct {
    float x, y;
    float radius;
    float max_radius;
    int life;
    int max_life;
    COLORREF color;
    bool active;
} Shockwave;

typedef struct {
    float x, y;
    float size;
    float phase;
    float speed;
    int layer;
} Star;

typedef struct {
    int high_score_classic;
    int high_score_time_attack;
    int high_score_hardcore;
    int high_score_campaign;
    int games_played;
    int asteroids_destroyed;
    int shots_fired;
    int shots_hit;
    int best_accuracy;
    int campaign_wins;
} Stats;

Ship ship;
Asteroid asteroids[100];
int num_asteroids = 0;
Bullet bullets[100];
int num_bullets = 0;
Ufo ufos[10];
int num_ufos = 0;
Mine mines[30];
int num_mines = 0;
PowerUp powerups[30];
int num_powerups = 0;
Particle particles[600];
int num_particles = 0;
Shockwave shockwaves[30];
int num_shockwaves = 0;
Star stars[80];

int shield_timer = 0;
int spread_timer = 0;
int laser_timer = 0;
int ufo_spawn_timer = 0;

Stats stats = {0};

int score = 0;
int wave = 1;
bool game_over = true;
bool campaign_victory = false;
bool keys[256] = {0};
int game_mode = 0; // 0=Classic, 1=TimeAttack, 2=Hardcore, 3=Campaign
int time_left = 0;
long long last_tick_time = 0;
long long last_shoot_time = 0;
bool showing_stats = false;
bool showing_help = false;
int current_shots_fired = 0;
int current_shots_hit = 0;

DWORD WINAPI SoundThread(LPVOID param) {
    int type = (int)(intptr_t)param;
    if (type == 1) { // Laser
        Beep(880, 40);
    } else if (type == 2) { // Explosion
        Beep(120, 120);
    } else if (type == 3) { // Thrust
        Beep(60, 30);
    } else if (type == 4) { // Powerup
        Beep(600, 60);
        Beep(1200, 60);
    } else if (type == 5) { // EMP Blast
        Beep(1500, 80);
        Beep(400, 120);
    } else if (type == 6) { // Hyperdrive
        Beep(300, 50);
        Beep(900, 50);
        Beep(1800, 50);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

void InitStarfield() {
    for (int i = 0; i < 80; i++) {
        stars[i].x = (float)(rand() % WIDTH);
        stars[i].y = (float)(rand() % HEIGHT);
        stars[i].size = 1.0f + (rand() % 15) / 10.0f;
        stars[i].phase = ((rand() % 360) * 3.14159f) / 180.0f;
        stars[i].speed = 0.02f + (rand() % 50) / 1000.0f;
        stars[i].layer = rand() % 3;
    }
}

void LoadStats() {
    FILE* f = fopen("kasteroids_stats.txt", "r");
    if (f) {
        int r = fscanf(f, "%d %d %d %d %d %d %d %d %d %d", 
               &stats.high_score_classic, 
               &stats.high_score_time_attack, 
               &stats.high_score_hardcore, 
               &stats.high_score_campaign, 
               &stats.games_played, 
               &stats.asteroids_destroyed, 
               &stats.shots_fired, 
               &stats.shots_hit,
               &stats.best_accuracy,
               &stats.campaign_wins);
        if (r < 9) stats.best_accuracy = 0;
        if (r < 10) stats.campaign_wins = 0;
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("kasteroids_stats.txt", "w");
    if (f) {
        fprintf(f, "%d %d %d %d %d %d %d %d %d %d", 
                stats.high_score_classic, 
                stats.high_score_time_attack, 
                stats.high_score_hardcore, 
                stats.high_score_campaign, 
                stats.games_played, 
                stats.asteroids_destroyed, 
                stats.shots_fired, 
                stats.shots_hit,
                stats.best_accuracy,
                stats.campaign_wins);
        fclose(f);
    }
}

int GetHighScore() {
    if (game_mode == 1) return stats.high_score_time_attack;
    if (game_mode == 2) return stats.high_score_hardcore;
    if (game_mode == 3) return stats.high_score_campaign;
    return stats.high_score_classic;
}

void UpdateHighScore() {
    int current_high = GetHighScore();
    if (score > current_high) {
        if (game_mode == 1) stats.high_score_time_attack = score;
        else if (game_mode == 2) stats.high_score_hardcore = score;
        else if (game_mode == 3) stats.high_score_campaign = score;
        else stats.high_score_classic = score;
        SaveStats();
    }
}

long long GetTimeMs() {
    return GetTickCount64();
}

void CreateExplosion(float x, float y, COLORREF color, int count, float max_radius) {
    if (num_shockwaves < 30) {
        Shockwave* sw = &shockwaves[num_shockwaves++];
        sw->x = x; sw->y = y;
        sw->radius = 2.0f; sw->max_radius = max_radius;
        sw->life = 25; sw->max_life = 25;
        sw->color = color; sw->active = true;
    }

    for (int p = 0; p < count; p++) {
        if (num_particles >= 600) break;
        Particle* pt = &particles[num_particles++];
        pt->x = x; pt->y = y;
        float ang = (rand() % 360) * 3.14159f / 180.0f;
        float spd = 1.0f + (rand() % 500) / 80.0f;
        pt->vx = cos(ang) * spd;
        pt->vy = sin(ang) * spd;
        pt->life = 25 + (rand() % 25);
        pt->max_life = pt->life;
        pt->color = color;
        pt->active = true;
        pt->type = (p % 3 == 0) ? 1 : 0;
        pt->size = (pt->type == 1) ? 4.0f : 2.5f;
    }
}

void SpawnAsteroids(int count, int armored_count) {
    float speedMult = 1.0f + (wave - 1) * 0.15f;
    if (game_mode == 2) speedMult *= 1.4f;

    for (int i = 0; i < count; i++) {
        if (num_asteroids >= 100) break;
        float x, y;
        do {
            x = (float)(rand() % WIDTH);
            y = (float)(rand() % HEIGHT);
        } while (sqrt(pow(x - ship.x, 2) + pow(y - ship.y, 2)) < 120);

        Asteroid* a = &asteroids[num_asteroids++];
        a->x = x;
        a->y = y;
        a->radius = 40.0f;
        a->level = 3;
        a->is_armored = (i < armored_count);
        a->hp = a->is_armored ? 2 : 1;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = ((rand() % 200 + 100) / 100.0f) * speedMult;
        a->vx = cos(angle) * speed;
        a->vy = sin(angle) * speed;
        a->rot = (rand() % 360) * 3.14159f / 180.0f;
        a->rot_speed = ((rand() % 100) - 50) / 1500.0f;
        a->type = rand() % 3;
        a->active = true;
        for (int j = 0; j < 10; j++) {
            a->points[j] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
        }
    }
}

void SpawnBossUfo() {
    if (num_ufos >= 10) return;
    Ufo* u = &ufos[num_ufos++];
    u->y = HEIGHT / 2.0f - 100.0f;
    u->x = (rand() % 2 == 0) ? 0.0f : (float)WIDTH;
    u->vx = (u->x == 0.0f) ? 1.5f : -1.5f;
    u->vy = ((rand() % 200) - 100) / 100.0f * 1.0f;
    u->radius = 28.0f;
    u->active = true;
    u->shoot_timer = 0;
    u->type = 1; // Mother Boss
    u->max_hp = (game_mode == 3 && wave == 15) ? 30 : (10 + wave * 2);
    u->hp = u->max_hp;
    u->anim_frame = 0;
}

void SetupWave() {
    num_asteroids = 0;
    num_ufos = 0;
    num_mines = 0;

    if (game_mode == 3) { // Campaign
        if (wave == 5 || wave == 10 || wave == 15) {
            SpawnBossUfo();
            SpawnAsteroids(2 + wave / 3, wave / 3);
        } else {
            int total = 3 + wave / 2;
            int armored = (wave >= 3) ? (wave / 3) : 0;
            SpawnAsteroids(total, armored);
            if (wave >= 4) {
                int mine_count = (wave / 4);
                for (int m = 0; m < mine_count && num_mines < 30; m++) {
                    Mine* mine = &mines[num_mines++];
                    mine->x = (rand() % 2 == 0) ? 0.0f : (float)WIDTH;
                    mine->y = (float)(rand() % HEIGHT);
                    mine->vx = 0;
                    mine->vy = 0;
                    mine->radius = 12.0f;
                    mine->active = true;
                    mine->anim_frame = 0;
                }
            }
        }
    } else {
        int armored = (wave >= 3) ? (wave / 3) : 0;
        SpawnAsteroids(3 + wave, armored);
    }
}

void InitGame(int mode) {
    game_mode = mode;
    score = 0;
    wave = 1;
    campaign_victory = false;
    time_left = (game_mode == 1) ? 120 : 0;
    last_tick_time = GetTimeMs();
    current_shots_fired = 0;
    current_shots_hit = 0;
    showing_stats = false;
    showing_help = false;

    ship.x = WIDTH / 2.0f;
    ship.y = HEIGHT / 2.0f;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = -3.14159f / 2.0f;
    ship.active = true;
    ship.radius = 10.0f;
    ship.hyperdrive_cooldown = 0;
    ship.anim_frame = 0;

    num_asteroids = 0;
    num_bullets = 0;
    num_ufos = 0;
    num_powerups = 0;
    num_mines = 0;
    num_particles = 0;
    num_shockwaves = 0;

    shield_timer = 0;
    spread_timer = 0;
    laser_timer = 0;
    ufo_spawn_timer = 0;
    game_over = false;

    stats.games_played++;
    SaveStats();

    SetupWave();
}

void TriggerEmp() {
    PlaySoundEffect(5);
    for (int i = 0; i < num_bullets; i++) {
        if (bullets[i].active && bullets[i].is_enemy) {
            bullets[i].active = false;
        }
    }
    for (int i = 0; i < num_ufos; i++) {
        if (ufos[i].active) {
            ufos[i].hp -= 5;
            if (ufos[i].hp <= 0) {
                ufos[i].active = false;
                score += (ufos[i].type == 1) ? 500 : 200;
                CreateExplosion(ufos[i].x, ufos[i].y, RGB(217, 70, 239), 40, 60.0f);
            }
        }
    }
    for (int i = 0; i < num_asteroids; i++) {
        if (asteroids[i].active && asteroids[i].is_armored) {
            asteroids[i].hp--;
            if (asteroids[i].hp <= 0) {
                asteroids[i].is_armored = false;
            }
        }
    }
    CreateExplosion(ship.x, ship.y, RGB(217, 70, 239), 60, 100.0f);
    UpdateHighScore();
}

void TriggerHyperdrive() {
    if (ship.hyperdrive_cooldown > 0 || !ship.active) return;

    PlaySoundEffect(6);
    CreateExplosion(ship.x, ship.y, RGB(56, 189, 248), 25, 40.0f);

    float nx, ny;
    bool safe = false;
    for (int attempts = 0; attempts < 50; attempts++) {
        nx = 50.0f + (rand() % (WIDTH - 100));
        ny = 50.0f + (rand() % (HEIGHT - 100));
        bool collide = false;
        for (int a = 0; a < num_asteroids; a++) {
            if (asteroids[a].active && sqrt(pow(nx - asteroids[a].x, 2) + pow(ny - asteroids[a].y, 2)) < 100) {
                collide = true; break;
            }
        }
        for (int u = 0; u < num_ufos; u++) {
            if (ufos[u].active && sqrt(pow(nx - ufos[u].x, 2) + pow(ny - ufos[u].y, 2)) < 100) {
                collide = true; break;
            }
        }
        if (!collide) { safe = true; break; }
    }

    if (safe) {
        ship.x = nx;
        ship.y = ny;
    }
    ship.vx = 0;
    ship.vy = 0;
    ship.hyperdrive_cooldown = 600; // 10s cooldown

    CreateExplosion(ship.x, ship.y, RGB(56, 189, 248), 25, 40.0f);
}

void KillShip() {
    if (shield_timer > 0) return;

    ship.active = false;
    game_over = true;
    PlaySoundEffect(2);
    CreateExplosion(ship.x, ship.y, RGB(239, 68, 68), 50, 70.0f);
}

void CheckCollisions() {
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;

        if (bullets[i].is_enemy && ship.active) {
            float dist = sqrt(pow(bullets[i].x - ship.x, 2) + pow(bullets[i].y - ship.y, 2));
            if (dist < ship.radius) {
                if (shield_timer > 0) {
                    bullets[i].active = false;
                    PlaySoundEffect(2);
                    CreateExplosion(bullets[i].x, bullets[i].y, RGB(56, 189, 248), 10, 20.0f);
                } else {
                    bullets[i].active = false;
                    KillShip();
                }
            }
        }

        if (!bullets[i].active) continue;

        if (!bullets[i].is_enemy) {
            for (int m = 0; m < num_mines; m++) {
                if (!mines[m].active) continue;
                float dist = sqrt(pow(bullets[i].x - mines[m].x, 2) + pow(bullets[i].y - mines[m].y, 2));
                if (dist < mines[m].radius) {
                    if (!bullets[i].is_laser) bullets[i].active = false;
                    mines[m].active = false;
                    PlaySoundEffect(2);
                    CreateExplosion(mines[m].x, mines[m].y, RGB(168, 85, 247), 25, 45.0f);
                    score += 150;
                    stats.shots_hit++;
                    current_shots_hit++;
                    UpdateHighScore();
                    break;
                }
            }
        }

        if (!bullets[i].active) continue;

        if (!bullets[i].is_enemy) {
            for (int k = 0; k < num_ufos; k++) {
                if (!ufos[k].active) continue;
                float dist = sqrt(pow(bullets[i].x - ufos[k].x, 2) + pow(bullets[i].y - ufos[k].y, 2));
                if (dist < ufos[k].radius) {
                    if (!bullets[i].is_laser) bullets[i].active = false;
                    ufos[k].hp--;
                    stats.shots_hit++;
                    current_shots_hit++;
                    SaveStats();

                    CreateExplosion(bullets[i].x, bullets[i].y, RGB(239, 68, 68), 8, 15.0f);

                    if (ufos[k].hp <= 0) {
                        ufos[k].active = false;
                        PlaySoundEffect(2);
                        CreateExplosion(ufos[k].x, ufos[k].y, (ufos[k].type == 1 ? RGB(192, 132, 252) : RGB(239, 68, 68)), (ufos[k].type == 1 ? 50 : 25), (ufos[k].type == 1 ? 75.0f : 40.0f));
                        score += (ufos[k].type == 1) ? 500 : 200;
                        UpdateHighScore();

                        if (rand() % 100 < 35 && num_powerups < 30) {
                            PowerUp* pu = &powerups[num_powerups++];
                            pu->x = ufos[k].x; pu->y = ufos[k].y;
                            pu->vx = ((rand() % 100) - 50) / 50.0f; pu->vy = ((rand() % 100) - 50) / 50.0f;
                            pu->type = (rand() % 4) + 1;
                            pu->life = 60 * 10; pu->active = true; pu->anim_frame = 0;
                        }
                    }
                    break;
                }
            }
        }

        if (!bullets[i].active) continue;

        if (!bullets[i].is_enemy) {
            for (int j = 0; j < num_asteroids; j++) {
                if (!asteroids[j].active) continue;
                float dist = sqrt(pow(bullets[i].x - asteroids[j].x, 2) + pow(bullets[i].y - asteroids[j].y, 2));
                if (dist < asteroids[j].radius) {
                    if (!bullets[i].is_laser) bullets[i].active = false;
                    asteroids[j].hp--;

                    stats.shots_hit++;
                    current_shots_hit++;
                    SaveStats();

                    CreateExplosion(bullets[i].x, bullets[i].y, asteroids[j].is_armored ? RGB(249, 115, 22) : RGB(148, 163, 184), 10, 20.0f);

                    if (asteroids[j].hp <= 0) {
                        asteroids[j].active = false;
                        PlaySoundEffect(2);
                        CreateExplosion(asteroids[j].x, asteroids[j].y, asteroids[j].is_armored ? RGB(249, 115, 22) : RGB(148, 163, 184), 25, 45.0f);
                        stats.asteroids_destroyed++;
                        SaveStats();
                        score += (4 - asteroids[j].level) * 10 + (asteroids[j].is_armored ? 20 : 0);
                        UpdateHighScore();

                        if (rand() % 100 < 18 && num_powerups < 30) {
                            PowerUp* pu = &powerups[num_powerups++];
                            pu->x = asteroids[j].x; pu->y = asteroids[j].y;
                            pu->vx = ((rand() % 100) - 50) / 50.0f; pu->vy = ((rand() % 100) - 50) / 50.0f;
                            pu->type = (rand() % 4) + 1;
                            pu->life = 60 * 10; pu->active = true; pu->anim_frame = 0;
                        }

                        if (asteroids[j].level > 1 && num_asteroids + 2 <= 100) {
                            for (int k = 0; k < 2; k++) {
                                Asteroid* a = &asteroids[num_asteroids++];
                                a->x = asteroids[j].x;
                                a->y = asteroids[j].y;
                                a->radius = asteroids[j].radius / 2.0f;
                                a->level = asteroids[j].level - 1;
                                a->is_armored = false;
                                a->hp = 1;
                                float angle = (rand() % 360) * 3.14159f / 180.0f;
                                float speedMult = 1.0f + (wave - 1) * 0.15f;
                                float speed = ((rand() % 200 + 100) / 100.0f) * speedMult;
                                a->vx = cos(angle) * speed;
                                a->vy = sin(angle) * speed;
                                a->rot = (rand() % 360) * 3.14159f / 180.0f;
                                a->rot_speed = ((rand() % 100) - 50) / 1500.0f;
                                a->type = rand() % 3;
                                a->active = true;
                                for (int p = 0; p < 10; p++) {
                                    a->points[p] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (ship.active) {
        for (int m = 0; m < num_mines; m++) {
            if (!mines[m].active) continue;
            float dist = sqrt(pow(ship.x - mines[m].x, 2) + pow(ship.y - mines[m].y, 2));
            if (dist < mines[m].radius + ship.radius) {
                if (shield_timer > 0) {
                    mines[m].active = false;
                    PlaySoundEffect(2);
                    CreateExplosion(mines[m].x, mines[m].y, RGB(168, 85, 247), 25, 45.0f);
                } else {
                    KillShip();
                }
            }
        }

        for (int j = 0; j < num_asteroids; j++) {
            if (!asteroids[j].active) continue;
            float dist = sqrt(pow(ship.x - asteroids[j].x, 2) + pow(ship.y - asteroids[j].y, 2));
            if (dist < asteroids[j].radius + ship.radius) {
                if (shield_timer > 0) {
                    asteroids[j].active = false;
                    PlaySoundEffect(2);
                    CreateExplosion(asteroids[j].x, asteroids[j].y, asteroids[j].is_armored ? RGB(249, 115, 22) : RGB(148, 163, 184), 25, 45.0f);
                } else {
                    KillShip();
                }
            }
        }

        for (int k = 0; k < num_ufos; k++) {
            if (!ufos[k].active) continue;
            float dist = sqrt(pow(ship.x - ufos[k].x, 2) + pow(ship.y - ufos[k].y, 2));
            if (dist < ufos[k].radius + ship.radius) {
                if (shield_timer > 0) {
                    ufos[k].active = false;
                    PlaySoundEffect(2);
                    CreateExplosion(ufos[k].x, ufos[k].y, RGB(239, 68, 68), 25, 45.0f);
                } else {
                    KillShip();
                }
            }
        }
    }
}

void CompactArrays() {
    int ab = 0;
    for (int i = 0; i < num_bullets; i++) if (bullets[i].active) bullets[ab++] = bullets[i];
    num_bullets = ab;

    int aa = 0;
    for (int i = 0; i < num_asteroids; i++) if (asteroids[i].active) asteroids[aa++] = asteroids[i];
    num_asteroids = aa;

    int au = 0;
    for (int i = 0; i < num_ufos; i++) if (ufos[i].active) ufos[au++] = ufos[i];
    num_ufos = au;

    int ap = 0;
    for (int i = 0; i < num_particles; i++) if (particles[i].active) particles[ap++] = particles[i];
    num_particles = ap;

    int as = 0;
    for (int i = 0; i < num_shockwaves; i++) if (shockwaves[i].active) shockwaves[as++] = shockwaves[i];
    num_shockwaves = as;

    int apw = 0;
    for (int i = 0; i < num_powerups; i++) if (powerups[i].active) powerups[apw++] = powerups[i];
    num_powerups = apw;

    int am = 0;
    for (int i = 0; i < num_mines; i++) if (mines[i].active) mines[am++] = mines[i];
    num_mines = am;
}

void Update() {
    if (game_over) {
        if (keys['S']) { showing_stats = true; showing_help = false; }
        if (keys['H']) { showing_help = true; showing_stats = false; }
        if (keys['B']) { showing_stats = false; showing_help = false; }
        if (keys['1']) InitGame(0);
        if (keys['2']) InitGame(1);
        if (keys['3']) InitGame(2);
        if (keys['4']) InitGame(3);
        return;
    }

    bool was_game_over = game_over;

    if (game_mode == 1) { // Time Attack
        long long now = GetTimeMs();
        if (now - last_tick_time >= 1000) {
            time_left--;
            last_tick_time = now;
            if (time_left <= 0) {
                KillShip();
            }
        }
    }

    if (ship.hyperdrive_cooldown > 0) ship.hyperdrive_cooldown--;

    if (keys['C'] || keys[VK_SHIFT]) {
        TriggerHyperdrive();
    }

    if (keys[VK_LEFT]) ship.angle -= 0.05f;
    if (keys[VK_RIGHT]) ship.angle += 0.05f;
    if (keys[VK_UP]) {
        ship.vx += cos(ship.angle) * 0.12f;
        ship.vy += sin(ship.angle) * 0.12f;
        static int thrust_timer = 0;
        thrust_timer++;
        if (thrust_timer > 3) {
            PlaySoundEffect(3);
            thrust_timer = 0;
        }
        if (num_particles < 600 && ship.active) {
            Particle* p = &particles[num_particles++];
            p->x = ship.x - cos(ship.angle) * 15.0f;
            p->y = ship.y - sin(ship.angle) * 15.0f;
            p->vx = -cos(ship.angle) * 3.0f + ((rand() % 100) - 50) / 33.0f;
            p->vy = -sin(ship.angle) * 3.0f + ((rand() % 100) - 50) / 33.0f;
            p->life = 15 + (rand() % 10); p->max_life = p->life;
            p->color = (rand() % 2 == 0) ? RGB(249, 115, 22) : RGB(239, 68, 68);
            p->active = true; p->type = 0; p->size = 2.5f;
        }
    }

    ship.anim_frame += 0.2f;
    ship.vx *= 0.99f;
    ship.vy *= 0.99f;
    ship.x += ship.vx;
    ship.y += ship.vy;

    if (ship.x < 0) ship.x = WIDTH;
    if (ship.x > WIDTH) ship.x = 0;
    if (ship.y < 0) ship.y = HEIGHT;
    if (ship.y > HEIGHT) ship.y = 0;

    if (keys[VK_SPACE]) {
        long long now = GetTimeMs();
        int cooldown = (laser_timer > 0) ? 100 : 200;
        if (now - last_shoot_time > cooldown && num_bullets < 100) {
            PlaySoundEffect(1);
            stats.shots_fired++;
            current_shots_fired++;
            SaveStats();

            if (spread_timer > 0) {
                for (int k = -1; k <= 1; k++) {
                    if (num_bullets < 100) {
                        Bullet* b = &bullets[num_bullets++];
                        b->x = ship.x + cos(ship.angle) * 15.0f;
                        b->y = ship.y + sin(ship.angle) * 15.0f;
                        float ang = ship.angle + k * 0.2f;
                        b->vx = cos(ang) * 8.5f;
                        b->vy = sin(ang) * 8.5f;
                        b->life = 60;
                        b->active = true;
                        b->is_enemy = false;
                        b->is_laser = (laser_timer > 0);
                    }
                }
            } else {
                Bullet* b = &bullets[num_bullets++];
                b->x = ship.x + cos(ship.angle) * 15.0f;
                b->y = ship.y + sin(ship.angle) * 15.0f;
                b->vx = cos(ship.angle) * 8.5f;
                b->vy = sin(ship.angle) * 8.5f;
                b->life = 60;
                b->active = true;
                b->is_enemy = false;
                b->is_laser = (laser_timer > 0);
            }
            last_shoot_time = now;
        }
    }

    // Update bullets
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        bullets[i].life--;
        if (bullets[i].life <= 0) bullets[i].active = false;

        if (bullets[i].x < 0) bullets[i].x = WIDTH;
        if (bullets[i].x > WIDTH) bullets[i].x = 0;
        if (bullets[i].y < 0) bullets[i].y = HEIGHT;
        if (bullets[i].y > HEIGHT) bullets[i].y = 0;
    }

    // Update asteroids
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;
        asteroids[i].x += asteroids[i].vx;
        asteroids[i].y += asteroids[i].vy;
        asteroids[i].rot += asteroids[i].rot_speed;

        if (asteroids[i].x < -asteroids[i].radius) asteroids[i].x = WIDTH + asteroids[i].radius;
        if (asteroids[i].x > WIDTH + asteroids[i].radius) asteroids[i].x = -asteroids[i].radius;
        if (asteroids[i].y < -asteroids[i].radius) asteroids[i].y = HEIGHT + asteroids[i].radius;
        if (asteroids[i].y > HEIGHT + asteroids[i].radius) asteroids[i].y = -asteroids[i].radius;
    }

    // UFO Spawning
    if (game_mode != 3 || (wave != 5 && wave != 10 && wave != 15)) {
        ufo_spawn_timer++;
        int ufo_threshold = (game_mode == 2) ? 300 : 550;
        if (ufo_spawn_timer > ufo_threshold) {
            ufo_spawn_timer = 0;
            if ((rand() % 100) < (int)((0.4f + (wave * 0.05f)) * 100) && num_ufos < 10) {
                Ufo* u = &ufos[num_ufos++];
                u->y = 50.0f + (rand() % (HEIGHT - 100));
                u->x = (rand() % 2 == 0) ? 0.0f : (float)WIDTH;
                u->vx = (u->x == 0.0f) ? 2.0f : -2.0f;
                u->vy = ((rand() % 200) - 100) / 100.0f * 1.5f;
                u->radius = 15.0f;
                u->active = true;
                u->shoot_timer = 0;
                u->type = 0; // Normal
                u->hp = 1;
                u->max_hp = 1;
                u->anim_frame = 0;
            }
        }
    }

    // Update UFOs
    for (int i = 0; i < num_ufos; i++) {
        if (!ufos[i].active) continue;
        ufos[i].x += ufos[i].vx;
        ufos[i].y += ufos[i].vy;
        ufos[i].anim_frame += 0.15f;

        if (ufos[i].y < 0) ufos[i].y = HEIGHT;
        if (ufos[i].y > HEIGHT) ufos[i].y = 0;

        if ((ufos[i].vx > 0 && ufos[i].x > WIDTH + 50) || (ufos[i].vx < 0 && ufos[i].x < -50)) {
            if (ufos[i].type == 0) ufos[i].active = false;
            else ufos[i].vx = -ufos[i].vx;
        }

        ufos[i].shoot_timer++;
        int shoot_interval = (ufos[i].type == 1) ? 70 : 100;
        if (ufos[i].shoot_timer > shoot_interval && ship.active && num_bullets < 100) {
            ufos[i].shoot_timer = 0;
            float angle = atan2(ship.y - ufos[i].y, ship.x - ufos[i].x);

            if (ufos[i].type == 1) { // Boss 3-shot burst
                for (int b = -1; b <= 1; b++) {
                    if (num_bullets < 100) {
                        Bullet* blt = &bullets[num_bullets++];
                        blt->x = ufos[i].x; blt->y = ufos[i].y;
                        blt->vx = cos(angle + b * 0.25f) * 6.0f;
                        blt->vy = sin(angle + b * 0.25f) * 6.0f;
                        blt->life = 70; blt->active = true;
                        blt->is_enemy = true; blt->is_laser = false;
                    }
                }
            } else {
                angle += (((rand() % 100) - 50) / 100.0f) * 0.2f;
                Bullet* blt = &bullets[num_bullets++];
                blt->x = ufos[i].x; blt->y = ufos[i].y;
                blt->vx = cos(angle) * 7.0f; blt->vy = sin(angle) * 7.0f;
                blt->life = 60; blt->active = true;
                blt->is_enemy = true; blt->is_laser = false;
            }
            PlaySoundEffect(1);
        }
    }

    if (shield_timer > 0) shield_timer--;
    if (spread_timer > 0) spread_timer--;
    if (laser_timer > 0) laser_timer--;

    // Update PowerUps
    for (int i = 0; i < num_powerups; i++) {
        if (!powerups[i].active) continue;
        powerups[i].x += powerups[i].vx;
        powerups[i].y += powerups[i].vy;
        powerups[i].anim_frame += 0.1f;
        powerups[i].life--;
        if (powerups[i].life <= 0) powerups[i].active = false;

        if (powerups[i].x < 0) powerups[i].x = WIDTH;
        if (powerups[i].x > WIDTH) powerups[i].x = 0;
        if (powerups[i].y < 0) powerups[i].y = HEIGHT;
        if (powerups[i].y > HEIGHT) powerups[i].y = 0;

        if (ship.active) {
            float dist = sqrt(pow(ship.x - powerups[i].x, 2) + pow(ship.y - powerups[i].y, 2));
            if (dist < ship.radius + 15) {
                powerups[i].active = false;
                PlaySoundEffect(4);
                if (powerups[i].type == 1) shield_timer = 60 * 10;
                else if (powerups[i].type == 2) spread_timer = 60 * 10;
                else if (powerups[i].type == 3) TriggerEmp();
                else if (powerups[i].type == 4) laser_timer = 60 * 10;
                score += 50;
                UpdateHighScore();
            }
        }
    }

    // Spawn Mines in non-boss waves
    if (wave >= 3 && game_mode != 3) {
        if (rand() % 1000 < 5 && num_mines < 30) {
            Mine* m = &mines[num_mines++];
            m->x = (rand() % 2 == 0) ? 0.0f : (float)WIDTH;
            m->y = (float)(rand() % HEIGHT);
            m->vx = 0; m->vy = 0;
            m->radius = 12.0f; m->active = true; m->anim_frame = 0;
        }
    }

    // Update Mines
    for (int i = 0; i < num_mines; i++) {
        if (!mines[i].active) continue;
        mines[i].anim_frame += 0.1f;
        if (ship.active) {
            float ang = atan2(ship.y - mines[i].y, ship.x - mines[i].x);
            mines[i].vx += cos(ang) * 0.035f;
            mines[i].vy += sin(ang) * 0.035f;
        }
        float speed = sqrt(mines[i].vx * mines[i].vx + mines[i].vy * mines[i].vy);
        if (speed > 2.5f) {
            mines[i].vx = (mines[i].vx / speed) * 2.5f;
            mines[i].vy = (mines[i].vy / speed) * 2.5f;
        }
        mines[i].x += mines[i].vx;
        mines[i].y += mines[i].vy;

        if (mines[i].x < 0) mines[i].x = WIDTH;
        if (mines[i].x > WIDTH) mines[i].x = 0;
        if (mines[i].y < 0) mines[i].y = HEIGHT;
        if (mines[i].y > HEIGHT) mines[i].y = 0;
    }

    CheckCollisions();

    // Update particles
    for (int i = 0; i < num_particles; i++) {
        if (!particles[i].active) continue;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if (particles[i].type == 1) {
            particles[i].size += 0.15f;
            particles[i].vx *= 0.95f;
            particles[i].vy *= 0.95f;
        }
        particles[i].life--;
        if (particles[i].life <= 0) particles[i].active = false;
    }

    // Update shockwaves
    for (int i = 0; i < num_shockwaves; i++) {
        if (!shockwaves[i].active) continue;
        shockwaves[i].radius += (shockwaves[i].max_radius - shockwaves[i].radius) * 0.15f;
        shockwaves[i].life--;
        if (shockwaves[i].life <= 0) shockwaves[i].active = false;
    }

    CompactArrays();

    // Check Sector / Wave clear
    if (num_asteroids == 0 && num_ufos == 0) {
        if (game_mode == 3) {
            if (wave >= 15) {
                campaign_victory = true;
                game_over = true;
                stats.campaign_wins++;
                SaveStats();
                UpdateHighScore();
            } else {
                wave++;
                SetupWave();
            }
        } else {
            wave++;
            SetupWave();
        }
    }

    if (!was_game_over && game_over) {
        showing_stats = false;
        showing_help = false;
        if (current_shots_fired >= 10) {
            int acc = (int)round(((double)current_shots_hit / current_shots_fired) * 100.0);
            if (acc > stats.best_accuracy) {
                stats.best_accuracy = acc;
                SaveStats();
            }
        }
    }
}

void Draw(HDC hdc) {
    // Deep Space Background
    HBRUSH bgBrush = CreateSolidBrush(RGB(9, 13, 22));
    RECT rect = {0, 0, WIDTH, HEIGHT};
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Starfield Background Parallax & Twinkle
    for (int i = 0; i < 80; i++) {
        stars[i].phase += stars[i].speed;
        int alpha = (int)(150 + sin(stars[i].phase) * 90);
        if (alpha < 40) alpha = 40; if (alpha > 255) alpha = 255;
        COLORREF starColor = (stars[i].layer == 2) ? RGB(186, 230, 253) : ((stars[i].layer == 1) ? RGB(226, 232, 240) : RGB(100, 116, 139));
        HBRUSH sb = CreateSolidBrush(starColor);
        HPEN sp = CreatePen(PS_SOLID, 1, starColor);
        SelectObject(hdc, sb); SelectObject(hdc, sp);
        int sz = (int)stars[i].size;
        Ellipse(hdc, (int)stars[i].x - sz, (int)stars[i].y - sz, (int)stars[i].x + sz, (int)stars[i].y + sz);
        DeleteObject(sb); DeleteObject(sp);
    }

    // Draw Ship
    if (ship.active) {
        // Thruster flame animation
        if (keys[VK_UP]) {
            float flameLen = 15.0f + sin(ship.anim_frame * 0.5f) * 5.0f + (rand() % 6);
            HPEN flamePen = CreatePen(PS_SOLID, 2, RGB(249, 115, 22));
            HBRUSH flameBrush = CreateSolidBrush(RGB(239, 68, 68));
            SelectObject(hdc, flamePen); SelectObject(hdc, flameBrush);

            POINT fpts[3];
            fpts[0].x = (LONG)(ship.x - cos(ship.angle) * (10.0f + flameLen));
            fpts[0].y = (LONG)(ship.y - sin(ship.angle) * (10.0f + flameLen));
            fpts[1].x = (LONG)(ship.x + cos(ship.angle + 2.7f) * 10.0f);
            fpts[1].y = (LONG)(ship.y + sin(ship.angle + 2.7f) * 10.0f);
            fpts[2].x = (LONG)(ship.x + cos(ship.angle - 2.7f) * 10.0f);
            fpts[2].y = (LONG)(ship.y + sin(ship.angle - 2.7f) * 10.0f);
            Polygon(hdc, fpts, 3);
            DeleteObject(flamePen); DeleteObject(flameBrush);
        }

        // Detailed Space Fighter Ship Hull
        HPEN hullPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        HBRUSH hullBrush = CreateSolidBrush(RGB(30, 41, 59));
        SelectObject(hdc, hullPen); SelectObject(hdc, hullBrush);

        POINT pts[8];
        float c = cos(ship.angle), s = sin(ship.angle);
        // Helper inline rotation: x' = x*c - y*s, y' = x*s + y*c
        pts[0].x = (LONG)(ship.x + (18 * c - 0 * s));   pts[0].y = (LONG)(ship.y + (18 * s + 0 * c));
        pts[1].x = (LONG)(ship.x + (8 * c - (-6) * s)); pts[1].y = (LONG)(ship.y + (8 * s + (-6) * c));
        pts[2].x = (LONG)(ship.x + (-12 * c - (-14) * s)); pts[2].y = (LONG)(ship.y + (-12 * s + (-14) * c));
        pts[3].x = (LONG)(ship.x + (-7 * c - (-4) * s));  pts[3].y = (LONG)(ship.y + (-7 * s + (-4) * c));
        pts[4].x = (LONG)(ship.x + (-12 * c - 0 * s));   pts[4].y = (LONG)(ship.y + (-12 * s + 0 * c));
        pts[5].x = (LONG)(ship.x + (-7 * c - 4 * s));    pts[5].y = (LONG)(ship.y + (-7 * s + 4 * c));
        pts[6].x = (LONG)(ship.x + (-12 * c - 14 * s));  pts[6].y = (LONG)(ship.y + (-12 * s + 14 * c));
        pts[7].x = (LONG)(ship.x + (8 * c - 6 * s));     pts[7].y = (LONG)(ship.y + (8 * s + 6 * c));
        Polygon(hdc, pts, 8);
        DeleteObject(hullPen); DeleteObject(hullBrush);

        // Cockpit Glass Canopy
        HBRUSH glassBrush = CreateSolidBrush(RGB(56, 189, 248));
        HPEN glassPen = CreatePen(PS_SOLID, 1, RGB(186, 230, 253));
        SelectObject(hdc, glassBrush); SelectObject(hdc, glassPen);
        int cx = (int)(ship.x + 4 * c); int cy = (int)(ship.y + 4 * s);
        Ellipse(hdc, cx - 5, cy - 5, cx + 5, cy + 5);
        DeleteObject(glassBrush); DeleteObject(glassPen);

        // Shield Bubble Aura
        if (shield_timer > 0) {
            HPEN shieldPen = CreatePen(PS_SOLID, 2, RGB(59, 130, 246));
            SelectObject(hdc, shieldPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int pulse = (int)(sin(ship.anim_frame * 0.3f) * 2);
            Ellipse(hdc, (int)(ship.x - (25 + pulse)), (int)(ship.y - (25 + pulse)), (int)(ship.x + (25 + pulse)), (int)(ship.y + (25 + pulse)));
            DeleteObject(shieldPen);
        }
    }

    // Draw Asteroids with Rotation & Texture Facets
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;

        COLORREF fillC, strokeC;
        if (asteroids[i].is_armored) {
            fillC = RGB(69, 26, 3); strokeC = RGB(249, 115, 22);
        } else if (asteroids[i].type == 1) { // Iron Rust
            fillC = RGB(39, 25, 23); strokeC = RGB(180, 83, 9);
        } else if (asteroids[i].type == 2) { // Cryo Ice
            fillC = RGB(12, 42, 58); strokeC = RGB(56, 189, 248);
        } else { // Basalt Slate
            fillC = RGB(24, 24, 27); strokeC = RGB(148, 163, 184);
        }

        HPEN aPen = CreatePen(PS_SOLID, asteroids[i].is_armored ? 3 : 2, strokeC);
        HBRUSH aBrush = CreateSolidBrush(fillC);
        SelectObject(hdc, aPen); SelectObject(hdc, aBrush);

        POINT pts[10];
        for (int j = 0; j < 10; j++) {
            float a = (j / 10.0f) * 3.14159f * 2.0f + asteroids[i].rot;
            pts[j].x = (LONG)(asteroids[i].x + cos(a) * asteroids[i].points[j]);
            pts[j].y = (LONG)(asteroids[i].y + sin(a) * asteroids[i].points[j]);
        }
        Polygon(hdc, pts, 10);

        // Inner crater line facet detail
        HPEN facetPen = CreatePen(PS_SOLID, 1, strokeC);
        SelectObject(hdc, facetPen);
        MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
        LineTo(hdc, (LONG)asteroids[i].x, (LONG)asteroids[i].y);
        LineTo(hdc, pts[4].x, pts[4].y);
        DeleteObject(facetPen);

        DeleteObject(aPen); DeleteObject(aBrush);
    }

    // Draw UFO Saucer Sprites
    for (int i = 0; i < num_ufos; i++) {
        if (!ufos[i].active) continue;
        float r = ufos[i].radius;

        if (ufos[i].type == 1) { // Mother Boss UFO
            HPEN bossPen = CreatePen(PS_SOLID, 3, RGB(168, 85, 247));
            HBRUSH bossBrush = CreateSolidBrush(RGB(88, 28, 135));
            SelectObject(hdc, bossPen); SelectObject(hdc, bossBrush);
            Ellipse(hdc, (int)(ufos[i].x - r), (int)(ufos[i].y - r * 0.4f), (int)(ufos[i].x + r), (int)(ufos[i].y + r * 0.4f));

            // Purple Plasma Dome
            HBRUSH domeB = CreateSolidBrush(RGB(192, 132, 252));
            SelectObject(hdc, domeB);
            Pie(hdc, (int)(ufos[i].x - r * 0.6f), (int)(ufos[i].y - r * 0.8f), (int)(ufos[i].x + r * 0.6f), (int)(ufos[i].y + r * 0.2f), (int)(ufos[i].x + r * 0.6f), (int)ufos[i].y, (int)(ufos[i].x - r * 0.6f), (int)ufos[i].y);
            DeleteObject(bossPen); DeleteObject(bossBrush); DeleteObject(domeB);

            // Boss HP Bar
            HBRUSH hpBg = CreateSolidBrush(RGB(24, 24, 27));
            HBRUSH hpFg = CreateSolidBrush(RGB(34, 197, 94));
            RECT barBg = {(LONG)(ufos[i].x - 30), (LONG)(ufos[i].y - 40), (LONG)(ufos[i].x + 30), (LONG)(ufos[i].y - 33)};
            FillRect(hdc, &barBg, hpBg);
            float pct = (float)ufos[i].hp / ufos[i].max_hp;
            RECT barFg = {(LONG)(ufos[i].x - 30), (LONG)(ufos[i].y - 40), (LONG)(ufos[i].x - 30 + 60 * pct), (LONG)(ufos[i].y - 33)};
            FillRect(hdc, &barFg, hpFg);
            DeleteObject(hpBg); DeleteObject(hpFg);
        } else { // Scout UFO
            HPEN ufoPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
            HBRUSH ufoBrush = CreateSolidBrush(RGB(30, 27, 75));
            SelectObject(hdc, ufoPen); SelectObject(hdc, ufoBrush);
            Ellipse(hdc, (int)(ufos[i].x - r), (int)(ufos[i].y - r * 0.4f), (int)(ufos[i].x + r), (int)(ufos[i].y + r * 0.4f));

            // Green Dome
            HBRUSH domeB = CreateSolidBrush(RGB(74, 222, 128));
            SelectObject(hdc, domeB);
            Pie(hdc, (int)(ufos[i].x - r * 0.5f), (int)(ufos[i].y - r * 0.7f), (int)(ufos[i].x + r * 0.5f), (int)(ufos[i].y + r * 0.1f), (int)(ufos[i].x + r * 0.5f), (int)ufos[i].y, (int)(ufos[i].x - r * 0.5f), (int)ufos[i].y);
            DeleteObject(ufoPen); DeleteObject(ufoBrush); DeleteObject(domeB);
        }
    }

    // Draw Bullets (Glowing Plasma & Laser)
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        if (bullets[i].is_enemy) {
            HPEN ep = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
            HBRUSH eb = CreateSolidBrush(RGB(254, 240, 138));
            SelectObject(hdc, ep); SelectObject(hdc, eb);
            Ellipse(hdc, (int)(bullets[i].x - 4), (int)(bullets[i].y - 4), (int)(bullets[i].x + 4), (int)(bullets[i].y + 4));
            DeleteObject(ep); DeleteObject(eb);
        } else if (bullets[i].is_laser) {
            HPEN lp = CreatePen(PS_SOLID, 4, RGB(239, 68, 68));
            SelectObject(hdc, lp);
            MoveToEx(hdc, (int)(bullets[i].x - bullets[i].vx * 1.5f), (int)(bullets[i].y - bullets[i].vy * 1.5f), NULL);
            LineTo(hdc, (int)bullets[i].x, (int)bullets[i].y);
            DeleteObject(lp);
        } else {
            HPEN bp = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
            HBRUSH bb = CreateSolidBrush(RGB(253, 224, 71));
            SelectObject(hdc, bp); SelectObject(hdc, bb);
            Ellipse(hdc, (int)(bullets[i].x - 3), (int)(bullets[i].y - 3), (int)(bullets[i].x + 3), (int)(bullets[i].y + 3));
            DeleteObject(bp); DeleteObject(bb);
        }
    }

    // Draw Shockwaves
    for (int i = 0; i < num_shockwaves; i++) {
        if (!shockwaves[i].active) continue;
        HPEN swp = CreatePen(PS_SOLID, 2, shockwaves[i].color);
        SelectObject(hdc, swp); SelectObject(hdc, GetStockObject(NULL_BRUSH));
        int r = (int)shockwaves[i].radius;
        Ellipse(hdc, (int)shockwaves[i].x - r, (int)shockwaves[i].y - r, (int)shockwaves[i].x + r, (int)shockwaves[i].y + r);
        DeleteObject(swp);
    }

    // Draw Particles
    for (int i = 0; i < num_particles; i++) {
        if (!particles[i].active) continue;
        HBRUSH pb = CreateSolidBrush(particles[i].color);
        HPEN pp = CreatePen(PS_SOLID, 1, particles[i].color);
        SelectObject(hdc, pb); SelectObject(hdc, pp);
        int sz = (int)particles[i].size;
        Ellipse(hdc, (int)(particles[i].x - sz), (int)(particles[i].y - sz), (int)(particles[i].x + sz), (int)(particles[i].y + sz));
        DeleteObject(pb); DeleteObject(pp);
    }

    // Draw PowerUps
    SetBkMode(hdc, TRANSPARENT);
    for (int i = 0; i < num_powerups; i++) {
        if (!powerups[i].active) continue;
        COLORREF color = RGB(59, 130, 246);
        const char* label = "S";
        if (powerups[i].type == 2) { color = RGB(34, 197, 94); label = "P"; }
        else if (powerups[i].type == 3) { color = RGB(217, 70, 239); label = "E"; }
        else if (powerups[i].type == 4) { color = RGB(234, 179, 8); label = "L"; }

        HBRUSH b = CreateSolidBrush(color);
        SelectObject(hdc, b);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Rectangle(hdc, (int)powerups[i].x - 9, (int)powerups[i].y - 9, (int)powerups[i].x + 9, (int)powerups[i].y + 9);
        DeleteObject(b);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, (int)powerups[i].x - 4, (int)powerups[i].y - 8, label, 1);
    }

    // Draw Mines
    HPEN minePen = CreatePen(PS_SOLID, 2, RGB(168, 85, 247));
    SelectObject(hdc, minePen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    for (int m = 0; m < num_mines; m++) {
        if (!mines[m].active) continue;
        Ellipse(hdc, (int)(mines[m].x - mines[m].radius), (int)(mines[m].y - mines[m].radius), (int)(mines[m].x + mines[m].radius), (int)(mines[m].y + mines[m].radius));
        MoveToEx(hdc, (int)(mines[m].x - mines[m].radius - 3), (int)mines[m].y, NULL);
        LineTo(hdc, (int)(mines[m].x + mines[m].radius + 3), (int)mines[m].y);
        MoveToEx(hdc, (int)mines[m].x, (int)(mines[m].y - mines[m].radius - 3), NULL);
        LineTo(hdc, (int)mines[m].x, (int)(mines[m].y + mines[m].radius + 3));
    }
    DeleteObject(minePen);

    // Draw HUD
    SetTextColor(hdc, RGB(56, 189, 248));
    char scoreStr[160];
    const char* modeStr = (game_mode == 0) ? "Classic" : ((game_mode == 1) ? "Time Attack" : ((game_mode == 2) ? "Hardcore" : "Campaign"));
    int current_high = GetHighScore();

    if (game_mode == 1) {
        sprintf(scoreStr, "Score: %d   High: %d   Time: %ds   Mode: %s", score, current_high, time_left, modeStr);
    } else if (game_mode == 3) {
        sprintf(scoreStr, "Score: %d   High: %d   Sector: %d/15   Mode: %s", score, current_high, wave, modeStr);
    } else {
        sprintf(scoreStr, "Score: %d   High: %d   Wave: %d   Mode: %s", score, current_high, wave, modeStr);
    }
    TextOutA(hdc, 10, 10, scoreStr, strlen(scoreStr));

    // Hyperdrive indicator
    if (ship.active) {
        char hdStr[64];
        if (ship.hyperdrive_cooldown <= 0) {
            SetTextColor(hdc, RGB(56, 189, 248));
            sprintf(hdStr, "[C / SHIFT] Hyperdrive READY");
        } else {
            SetTextColor(hdc, RGB(113, 113, 122));
            sprintf(hdStr, "Hyperdrive: %ds", ship.hyperdrive_cooldown / 60 + 1);
        }
        TextOutA(hdc, 10, 30, hdStr, strlen(hdStr));
    }

    // Draw Game Over / Menu / Stats / Victory
    if (game_over) {
        if (campaign_victory) {
            SetTextColor(hdc, RGB(34, 197, 94));
            TextOutA(hdc, WIDTH / 2 - 110, HEIGHT / 2 - 80, "CAMPAIGN VICTORY!", 17);
            SetTextColor(hdc, RGB(250, 204, 21));
            char vicStr[64];
            sprintf(vicStr, "Final Campaign Score: %d", score);
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 - 50, vicStr, strlen(vicStr));
        } else if (game_mode == 1 && time_left <= 0) {
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, WIDTH / 2 - 40, HEIGHT / 2 - 60, "TIME UP!", 8);
        } else {
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, WIDTH / 2 - 50, HEIGHT / 2 - 60, "K-ASTEROIDS", 11);
        }

        if (showing_stats) {
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, WIDTH / 2 - 40, HEIGHT / 2 - 20, "Statistics", 10);
            SetTextColor(hdc, RGB(161, 161, 170));
            char statsStr[64];
            sprintf(statsStr, "Games Played: %d", stats.games_played);
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 10, statsStr, strlen(statsStr));
            sprintf(statsStr, "Asteroids Destroyed: %d", stats.asteroids_destroyed);
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 30, statsStr, strlen(statsStr));
            sprintf(statsStr, "Best Accuracy: %d%%", stats.best_accuracy);
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 50, statsStr, strlen(statsStr));
            sprintf(statsStr, "Campaign Wins: %d", stats.campaign_wins);
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 70, statsStr, strlen(statsStr));

            SetTextColor(hdc, RGB(250, 204, 21));
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 100, "[B] Back to Menu", 16);
        } else if (showing_help) {
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, WIDTH / 2 - 45, HEIGHT / 2 - 20, "How to Play", 11);
            SetTextColor(hdc, RGB(161, 161, 170));
            TextOutA(hdc, WIDTH / 2 - 95, HEIGHT / 2 + 10, "Arrows: Rotate & Thrust", 23);
            TextOutA(hdc, WIDTH / 2 - 95, HEIGHT / 2 + 30, "Space: Shoot", 12);
            TextOutA(hdc, WIDTH / 2 - 95, HEIGHT / 2 + 50, "C / Shift: Emergency Hyperdrive", 31);
            TextOutA(hdc, WIDTH / 2 - 95, HEIGHT / 2 + 70, "Powerups: [S]hield, [P]spread, [E]mp, [L]aser", 44);

            SetTextColor(hdc, RGB(250, 204, 21));
            TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 105, "[B] Back to Menu", 16);
        } else {
            SetTextColor(hdc, RGB(161, 161, 170));
            TextOutA(hdc, WIDTH / 2 - 70, HEIGHT / 2 - 20, "Select Mode to Play:", 20);
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 5, "[1] Classic - Infinite Waves", 28);
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 25, "[2] Time Attack - 2 Min Limit", 29);
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 45, "[3] Hardcore - Fast & Deadly", 28);
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 65, "[4] Sector Campaign (15 Sectors)", 32);

            SetTextColor(hdc, RGB(163, 230, 53));
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 95, "[S] View Statistics", 19);
            SetTextColor(hdc, RGB(110, 231, 183));
            TextOutA(hdc, WIDTH / 2 - 90, HEIGHT / 2 + 115, "[H] How to Play", 15);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            InitStarfield();
            LoadStats();
            SetTimer(hwnd, TIMER_ID, 16, NULL); // ~60fps
            break;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_KEYDOWN:
            if (wParam < 256) keys[wParam] = true;
            break;
        case WM_KEYUP:
            if (wParam < 256) keys[wParam] = false;
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
            SelectObject(memDC, memBitmap);

            Draw(memDC);

            BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memDC, 0, 0, SRCCOPY);

            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
        }
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT clientRect = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "KAsteroids",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
