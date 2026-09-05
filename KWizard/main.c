#include <windows.h>
#include <math.h>

#define BTN_DRAW 101
#define BTN_RESET 102
#define BTN_END_TURN 103
#define CMB_DIFFICULTY 104
#define BTN_CAMPAIGN 105
#define BTN_DECK 106
#define LST_AVAIL 107
#define LST_DECK 108
#define BTN_DECK_CLOSE 109
#define BTN_HELP 110
#define LST_HELP 111
#define BTN_HELP_CLOSE 112
#define IDT_CAMPAIGN_NEXT 1001
#define IDT_ANIM 1002

typedef struct {
    char name[32];
    int cost;
    char effect[64];
    int damage;
    int heal;
    int burn;
    int freeze;
    int shield;
    int regen;
    int poison;
    int type; // 0: fire, 1: ice, 2: arcane, 3: nature, 4: poison
} CardDef;

CardDef sampleCards[] = {
    {"Fireball", 3, "Deals 4 Fire damage", 4, 0, 0, 0, 0, 0, 0, 0},
    {"Scorch", 1, "Deals 1 Fire damage", 1, 0, 0, 0, 0, 0, 0, 0},
    {"Flame Strike", 5, "Deals 5 AoE Fire damage", 5, 0, 0, 0, 0, 0, 0, 0},
    {"Ember", 1, "Burns for 1 damage", 0, 0, 2, 0, 0, 0, 0, 0},
    {"Pyroblast", 6, "Deals 8 Fire damage", 8, 0, 0, 0, 0, 0, 0, 0},
    {"Wall of Fire", 4, "Creates a fiery barrier", 0, 0, 0, 0, 5, 0, 0, 0},
    {"Meteor", 8, "Deals 10 Fire damage", 10, 0, 0, 0, 0, 0, 0, 0},
    {"Ignite", 2, "Deals 2 Fire dmg over time", 0, 0, 3, 0, 0, 0, 0, 0},
    {"Ice Shard", 2, "Deals 2 Ice damage", 2, 0, 0, 0, 0, 0, 0, 1},
    {"Frostbolt", 3, "Deals 3 Ice dmg, slows", 3, 0, 0, 1, 0, 0, 0, 1},
    {"Blizzard", 6, "Deals 4 AoE Ice damage", 4, 0, 0, 1, 0, 0, 0, 1},
    {"Frost Nova", 4, "Freezes enemies in place", 0, 0, 0, 1, 0, 0, 0, 1},
    {"Ice Lance", 1, "Deals 1 Ice dmg (3 if frozen)", 1, 0, 0, 0, 0, 0, 0, 1},
    {"Glacial Spike", 7, "Deals 9 Ice damage", 9, 0, 0, 0, 0, 0, 0, 1},
    {"Cold Snap", 5, "Resets cooldowns (Ice)", 0, 0, 0, 0, 0, 0, 0, 1},
    {"Arcane Missiles", 1, "Fires 3 arcane bolts", 3, 0, 0, 0, 0, 0, 0, 2},
    {"Arcane Intellect", 3, "Draw 2 cards", 0, 0, 0, 0, 0, 0, 0, 2},
    {"Counterspell", 3, "Interrupts a spell", 0, 0, 0, 0, 0, 0, 0, 2},
    {"Magic Missile", 2, "Deals 2 Arcane damage", 2, 0, 0, 0, 0, 0, 0, 2},
    {"Arcane Blast", 4, "Deals 5 Arcane damage", 5, 0, 0, 0, 0, 0, 0, 2},
    {"Time Warp", 8, "Take an extra turn", 0, 0, 0, 0, 0, 0, 0, 2},
    {"Polymorph", 4, "Turns target into a sheep", 0, 0, 0, 1, 0, 0, 0, 2},
    {"Mana Shield", 2, "Absorbs damage using mana", 0, 0, 0, 0, 4, 0, 0, 2},
    {"Healing Touch", 2, "Heals 3 Life points", 0, 3, 0, 0, 0, 0, 0, 3},
    {"Rejuvenation", 3, "Heals 4 over time", 0, 0, 0, 0, 0, 4, 0, 3},
    {"Regrowth", 4, "Heals 2 + 2 over time", 0, 2, 0, 0, 0, 2, 0, 3},
    {"Swiftmend", 1, "Instantly heals 2", 0, 2, 0, 0, 0, 0, 0, 3},
    {"Tranquility", 8, "Heals 10 to all allies", 0, 10, 0, 0, 0, 0, 0, 3},
    {"Nourish", 3, "Heals 4", 0, 4, 0, 0, 0, 0, 0, 3},
    {"Nature's Grasp", 2, "Roots attackers", 0, 0, 0, 1, 0, 0, 0, 3},
    {"Lifebloom", 2, "Heals 1, blooms for 3", 0, 1, 0, 0, 0, 3, 0, 3},
    {"Flash Heal", 2, "Fast heal for 3", 0, 3, 0, 0, 0, 0, 0, 3},
    {"Greater Heal", 5, "Heals 7", 0, 7, 0, 0, 0, 0, 0, 3},
    {"Renew", 1, "Heals 2 over time", 0, 0, 0, 0, 0, 3, 0, 3},
    {"Poison Bolt", 4, "Deals 2 dmg, poisons for 3", 2, 0, 0, 0, 0, 0, 3, 4},
    {"Venom Strike", 2, "Poisons for 2", 0, 0, 0, 0, 0, 0, 2, 4}
};
#define NUM_SAMPLE_CARDS (sizeof(sampleCards)/sizeof(CardDef))

int playerHand[7];
int opponentHand[7];
int playerCount = 0;
int opponentCount = 0;

int playerDeck[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
int playerDeckCount = 20;

int playerHp = 30;
int opponentHp = 30;
int gameState = 0; // 0 = playing, 1 = player win, 2 = opponent win, 3 = deck builder, 4 = help

int campaignLevel = 0;
typedef struct {
    char name[32];
    int diff;
    int hp;
    int deckSize;
    int deck[36];
    COLORREF robeColor;
    COLORREF staffColor;
} MageDef;

MageDef mages[] = {
    {"Novice Pyromancer", 0, 20, 4, {0, 1, 3, 7}, RGB(180, 40, 30), RGB(255, 120, 0)},
    {"Apprentice Cryomancer", 0, 25, 4, {8, 9, 12, 14}, RGB(10, 120, 180), RGB(100, 220, 255)},
    {"Arcane Scholar", 1, 30, 5, {15, 16, 18, 19, 22}, RGB(110, 50, 160), RGB(200, 100, 255)},
    {"Forest Druid", 1, 35, 6, {23, 24, 25, 28, 29, 30}, RGB(20, 120, 50), RGB(100, 255, 120)},
    {"Venomancer", 1, 40, 4, {34, 35, 23, 30}, RGB(15, 100, 60), RGB(80, 240, 120)},
    {"Master Pyromancer", 2, 45, 8, {0, 1, 2, 3, 4, 5, 6, 7}, RGB(220, 30, 20), RGB(255, 160, 20)},
    {"Master Cryomancer", 2, 50, 7, {8, 9, 10, 11, 12, 13, 14}, RGB(10, 90, 190), RGB(120, 240, 255)},
    {"Arcane Archon", 2, 55, 8, {15, 16, 17, 18, 19, 20, 21, 22}, RGB(130, 30, 180), RGB(240, 120, 255)},
    {"High Priest", 2, 60, 8, {16, 22, 23, 26, 27, 28, 31, 32}, RGB(180, 140, 20), RGB(255, 240, 100)},
    {"Grand Magus", 2, 70, 36, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35}, RGB(220, 160, 20), RGB(255, 215, 0)}
};

int playerMana = 1;
int playerMaxMana = 1;
int opponentMana = 1;
int opponentMaxMana = 1;

int playerBurn = 0, playerFreeze = 0, playerShield = 0, playerRegen = 0, playerPoison = 0;
int opponentBurn = 0, opponentFreeze = 0, opponentShield = 0, opponentRegen = 0, opponentPoison = 0;

char arenaMsg[128] = "Spells and effects go here";

// Particle & FX structures
#define MAX_PARTICLES 160
typedef struct {
    float x, y, vx, vy;
    float size;
    float life;
    float decay;
    float gravity;
    COLORREF color;
    int type; // 0: spark, 1: smoke, 2: shard, 3: star
} Particle;
Particle particles[MAX_PARTICLES];
int particleCount = 0;

#define MAX_SHOCKWAVES 12
typedef struct {
    float x, y, r, maxR;
    float alpha;
    COLORREF color;
} Shockwave;
Shockwave shockwaves[MAX_SHOCKWAVES];
int shockwaveCount = 0;

#define MAX_FLOATERS 16
typedef struct {
    float x, y, vy;
    char text[32];
    COLORREF color;
    float life;
} Floater;
Floater floaters[MAX_FLOATERS];
int floaterCount = 0;

#define MAX_PROJECTILES 8
typedef struct {
    float x, y, startX, startY, targetX, targetY;
    float progress, speed;
    int type;
    int fromPlayer;
    int active;
} Projectile;
Projectile projectiles[MAX_PROJECTILES];

float screenShake = 0.0f;
float runicAngle = 0.0f;

HWND hwndDraw, hwndReset, hwndCombo, hwndDeckBtn, hwndHelpBtn, hwndAvail, hwndDeck, hwndDeckClose, hwndHelp, hwndHelpClose;

unsigned int seed = 0;
int my_rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed >> 16) & 0x7FFF;
}

void TriggerShake(float amt) {
    screenShake += amt;
    if (screenShake > 20.0f) screenShake = 20.0f;
}

void SpawnShockwave(float x, float y, COLORREF color) {
    if (shockwaveCount < MAX_SHOCKWAVES) {
        shockwaves[shockwaveCount].x = x;
        shockwaves[shockwaveCount].y = y;
        shockwaves[shockwaveCount].r = 6.0f;
        shockwaves[shockwaveCount].maxR = 75.0f;
        shockwaves[shockwaveCount].alpha = 1.0f;
        shockwaves[shockwaveCount].color = color;
        shockwaveCount++;
    }
}

void SpawnFloater(float x, float y, const char* text, COLORREF color) {
    if (floaterCount < MAX_FLOATERS) {
        floaters[floaterCount].x = x;
        floaters[floaterCount].y = y;
        floaters[floaterCount].vy = -2.0f;
        floaters[floaterCount].life = 1.0f;
        floaters[floaterCount].color = color;
        lstrcpynA(floaters[floaterCount].text, text, 32);
        floaterCount++;
    }
}

void SpawnExplosion(float x, float y, int type) {
    TriggerShake(7.0f);
    COLORREF pColor = RGB(168, 85, 247);
    if (type == 0) pColor = RGB(255, 69, 0);
    else if (type == 1) pColor = RGB(0, 220, 255);
    else if (type == 3) pColor = RGB(34, 197, 94);
    else if (type == 4) pColor = RGB(16, 185, 129);

    SpawnShockwave(x, y, pColor);

    // Layer 1: Needle Sparks
    for (int i = 0; i < 14 && particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 628)) / 100.0f;
        float spd = 3.0f + (float)(my_rand() % 60) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * spd;
        particles[particleCount].vy = sinf(angle) * spd;
        particles[particleCount].size = 2.0f;
        particles[particleCount].life = 1.0f;
        particles[particleCount].decay = 0.04f;
        particles[particleCount].gravity = 0.0f;
        particles[particleCount].color = RGB(255, 255, 255);
        particles[particleCount].type = 0;
        particleCount++;
    }

    // Layer 2: Expanding Smoke Puffs
    for (int i = 0; i < 10 && particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 628)) / 100.0f;
        float spd = 1.5f + (float)(my_rand() % 30) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * spd;
        particles[particleCount].vy = sinf(angle) * spd - 0.6f;
        particles[particleCount].size = 5.0f;
        particles[particleCount].life = 1.0f;
        particles[particleCount].decay = 0.03f;
        particles[particleCount].gravity = -0.05f;
        particles[particleCount].color = pColor;
        particles[particleCount].type = 1;
        particleCount++;
    }

    // Layer 3: Kinematic Crystal Debris Shards
    for (int i = 0; i < 8 && particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 628)) / 100.0f;
        float spd = 2.5f + (float)(my_rand() % 40) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * spd;
        particles[particleCount].vy = sinf(angle) * spd - 2.0f;
        particles[particleCount].size = 4.0f;
        particles[particleCount].life = 1.0f;
        particles[particleCount].decay = 0.025f;
        particles[particleCount].gravity = 0.18f;
        particles[particleCount].color = pColor;
        particles[particleCount].type = 2;
        particleCount++;
    }
}

void SpawnCelebrationStars(float x, float y) {
    TriggerShake(12.0f);
    for (int i = 0; i < 28 && particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 628)) / 100.0f;
        float spd = 3.0f + (float)(my_rand() % 60) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * spd;
        particles[particleCount].vy = sinf(angle) * spd - 2.5f;
        particles[particleCount].size = 4.5f;
        particles[particleCount].life = 1.0f;
        particles[particleCount].decay = 0.018f;
        particles[particleCount].gravity = 0.12f;
        particles[particleCount].color = (i % 2 == 0) ? RGB(255, 215, 0) : RGB(255, 255, 255);
        particles[particleCount].type = 3;
        particleCount++;
    }
}

void LaunchSpellVisual(int fromPlayer, CardDef* cd, int cw, int ch) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) {
            projectiles[i].active = 1;
            projectiles[i].fromPlayer = fromPlayer;
            projectiles[i].type = cd->type;
            projectiles[i].progress = 0.0f;
            projectiles[i].speed = 0.05f;
            if (fromPlayer) {
                projectiles[i].startX = (float)cw * 0.25f;
                projectiles[i].startY = (float)ch * 0.48f;
                projectiles[i].targetX = (float)cw * 0.75f;
                projectiles[i].targetY = (float)ch * 0.48f;
            } else {
                projectiles[i].startX = (float)cw * 0.75f;
                projectiles[i].startY = (float)ch * 0.48f;
                projectiles[i].targetX = (float)cw * 0.25f;
                projectiles[i].targetY = (float)ch * 0.48f;
            }
            break;
        }
    }
}

void DealDamageToPlayer(int dmg, int cw, int ch) {
    if (dmg <= 0) return;
    if (playerShield > 0) {
        if (playerShield >= dmg) {
            playerShield -= dmg;
            char b[32]; wsprintf(b, "ABSORBED %d", dmg);
            SpawnFloater((float)cw * 0.25f, (float)ch * 0.40f, b, RGB(96, 165, 250));
            dmg = 0;
        } else {
            dmg -= playerShield;
            SpawnFloater((float)cw * 0.25f, (float)ch * 0.40f, "SHIELD BROKE", RGB(96, 165, 250));
            playerShield = 0;
        }
    }
    if (dmg > 0) {
        playerHp -= dmg;
        char b[32]; wsprintf(b, "-%d HP", dmg);
        SpawnFloater((float)cw * 0.25f, (float)ch * 0.40f, b, RGB(239, 68, 68));
    }
    if (playerHp < 0) playerHp = 0;
}

void DealDamageToOpponent(int dmg, int cw, int ch) {
    if (dmg <= 0) return;
    if (opponentShield > 0) {
        if (opponentShield >= dmg) {
            opponentShield -= dmg;
            char b[32]; wsprintf(b, "ABSORBED %d", dmg);
            SpawnFloater((float)cw * 0.75f, (float)ch * 0.40f, b, RGB(96, 165, 250));
            dmg = 0;
        } else {
            dmg -= opponentShield;
            SpawnFloater((float)cw * 0.75f, (float)ch * 0.40f, "SHIELD BROKE", RGB(96, 165, 250));
            opponentShield = 0;
        }
    }
    if (dmg > 0) {
        opponentHp -= dmg;
        char b[32]; wsprintf(b, "-%d HP", dmg);
        SpawnFloater((float)cw * 0.75f, (float)ch * 0.40f, b, RGB(239, 68, 68));
    }
    if (opponentHp < 0) opponentHp = 0;
}

const char* GetSoundType(CardDef* cd) {
    if (cd->damage > 0 && strstr(cd->effect, "Fire") != NULL) return "fire";
    if (cd->damage > 0 && strstr(cd->effect, "Ice") != NULL) return "ice";
    if (cd->damage > 0 && strstr(cd->effect, "Arcane") != NULL) return "arcane";
    if (cd->heal > 0 || cd->regen > 0) return "heal";
    if (strcmp(cd->name, "Ice Lance") == 0 || strstr(cd->name, "Frost") != NULL || strstr(cd->name, "Cold") != NULL || strstr(cd->name, "Blizzard") != NULL) return "ice";
    if (strstr(cd->name, "Fire") != NULL || strstr(cd->name, "Flame") != NULL || strstr(cd->name, "Pyro") != NULL || strstr(cd->name, "Ignite") != NULL || strstr(cd->name, "Ember") != NULL || strstr(cd->name, "Scorch") != NULL || strstr(cd->name, "Meteor") != NULL) return "fire";
    return "arcane";
}

void PlaySoundEffect(const char* type) {
    if (strcmp(type, "fire") == 0) {
        Beep(150, 40);
        Beep(100, 40);
    } else if (strcmp(type, "ice") == 0) {
        Beep(800, 30);
        Beep(1200, 30);
    } else if (strcmp(type, "arcane") == 0 || strcmp(type, "heal") == 0) {
        Beep(500, 40);
        Beep(800, 50);
    } else if (strcmp(type, "damage") == 0) {
        Beep(120, 50);
    } else if (strcmp(type, "win") == 0) {
        Beep(554, 80);
        Beep(880, 120);
    } else if (strcmp(type, "lose") == 0) {
        Beep(250, 100);
        Beep(120, 150);
    }
}

void DrawCard(int isOpponent) {
    if (isOpponent) {
        if (opponentCount < 7) {
            if (campaignLevel > 0) {
                MageDef m = mages[campaignLevel - 1];
                opponentHand[opponentCount++] = m.deck[my_rand() % m.deckSize];
            } else {
                opponentHand[opponentCount++] = my_rand() % NUM_SAMPLE_CARDS;
            }
        }
    } else {
        if (playerCount < 7) {
            playerHand[playerCount++] = playerDeck[my_rand() % playerDeckCount];
        }
    }
}

void InitGame(int oppHp) {
    playerCount = 0;
    opponentCount = 0;
    playerHp = 30;
    opponentHp = oppHp;
    gameState = 0;
    playerMaxMana = 1;
    playerMana = 1;
    opponentMaxMana = 1;
    opponentMana = 1;
    playerBurn = 0; playerFreeze = 0; playerShield = 0; playerRegen = 0; playerPoison = 0;
    opponentBurn = 0; opponentFreeze = 0; opponentShield = 0; opponentRegen = 0; opponentPoison = 0;
    particleCount = 0;
    shockwaveCount = 0;
    floaterCount = 0;
    for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = 0;

    if (campaignLevel > 0) {
        wsprintf(arenaMsg, "Battle %d: vs %s!", campaignLevel, mages[campaignLevel-1].name);
    } else {
        lstrcpyA(arenaMsg, "Arcane Duel Begins!");
    }
    for (int i = 0; i < 3; i++) {
        DrawCard(0);
        DrawCard(1);
    }
}

void ResetGame() {
    campaignLevel = 0;
    InitGame(30);
}

int EvaluateCard(CardDef* cd) {
    int score = cd->cost * 2;
    if (opponentHp < 15 && (cd->heal > 0 || cd->shield > 0 || cd->regen > 0)) {
        score += 20 + cd->heal * 3 + cd->shield * 2 + cd->regen * 2;
    }
    if (strcmp(cd->name, "Ice Lance") == 0 && playerFreeze > 0) {
        score += 30;
    }
    if (playerHp <= 10 && cd->damage > 0) {
        score += cd->damage * 4;
    }
    if (strcmp(cd->name, "Time Warp") == 0) score += 25;
    if (strcmp(cd->name, "Arcane Intellect") == 0 && opponentCount < 3) score += 15;
    return score;
}

void PlayOpponentTurn(int cw, int ch) {
    if (gameState != 0) return;
    
    int diff = (int)SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
    if (campaignLevel > 0) {
        diff = mages[campaignLevel-1].diff;
    }
    if (diff == 2) {
        for (int x = 0; x < opponentCount - 1; x++) {
            for (int y = x + 1; y < opponentCount; y++) {
                if (EvaluateCard(&sampleCards[opponentHand[x]]) < EvaluateCard(&sampleCards[opponentHand[y]])) {
                    int temp = opponentHand[x];
                    opponentHand[x] = opponentHand[y];
                    opponentHand[y] = temp;
                }
            }
        }
    } else if (diff == 1) {
        for (int x = 0; x < opponentCount - 1; x++) {
            for (int y = x + 1; y < opponentCount; y++) {
                if (sampleCards[opponentHand[x]].cost < sampleCards[opponentHand[y]].cost) {
                    int temp = opponentHand[x];
                    opponentHand[x] = opponentHand[y];
                    opponentHand[y] = temp;
                }
            }
        }
    }

    int i = 0;
    char playedStr[256] = "Opponent cast: ";
    int playedAny = 0;
    while (i < opponentCount) {
        CardDef cd = sampleCards[opponentHand[i]];
        if (opponentMana >= cd.cost) {
            opponentMana -= cd.cost;
            
            int dmg = cd.damage;
            if (strcmp(cd.name, "Ice Lance") == 0 && playerFreeze > 0) dmg = 3;
            
            DealDamageToPlayer(dmg, cw, ch);
            if (cd.heal > 0) {
                opponentHp += cd.heal;
                char b[32]; wsprintf(b, "+%d HP", cd.heal);
                SpawnFloater((float)cw * 0.75f, (float)ch * 0.40f, b, RGB(74, 222, 128));
            }
            
            playerBurn += cd.burn;
            playerFreeze += cd.freeze;
            opponentShield += cd.shield;
            opponentRegen += cd.regen;
            playerPoison += cd.poison;
            
            LaunchSpellVisual(0, &cd, cw, ch);
            PlaySoundEffect(GetSoundType(&cd));
            
            if (strcmp(cd.name, "Arcane Intellect") == 0) {
                DrawCard(1); DrawCard(1);
            }

            if (playerHp <= 0) {
                playerHp = 0;
                if (gameState != 2) PlaySoundEffect("lose");
                gameState = 2; // opponent win
            }
            if (opponentHp > 30) opponentHp = 30;

            if (playedAny) {
                lstrcatA(playedStr, ", ");
            }
            lstrcatA(playedStr, cd.name);
            playedAny = 1;
            
            for (int j = i; j < opponentCount - 1; j++) {
                opponentHand[j] = opponentHand[j + 1];
            }
            opponentCount--;
            
            if (gameState != 0) break;
        } else {
            i++;
        }
    }
    if (playedAny) {
        lstrcpyA(arenaMsg, playedStr);
    } else {
        lstrcpyA(arenaMsg, "Opponent ends turn without casting.");
    }
}

void DrawWizardSpriteGDI(HDC hdc, int cx, int cy, int isPlayer, COLORREF robeColor, COLORREF staffColor, float time) {
    int bobY = (int)(sinf(time * 4.0f + (isPlayer ? 0.0f : 3.14f)) * 4.0f);
    int y = cy + bobY;

    // Pedestal shadow
    HBRUSH shadowBrush = CreateSolidBrush(RGB(15, 6, 28));
    HPEN shadowPen = CreatePen(PS_SOLID, 1, isPlayer ? RGB(59, 130, 246) : RGB(239, 68, 68));
    HGDIOBJ oldB = SelectObject(hdc, shadowBrush);
    HGDIOBJ oldP = SelectObject(hdc, shadowPen);
    Ellipse(hdc, cx - 26, y + 30, cx + 26, y + 42);

    // Robe Polygon
    HBRUSH robeBrush = CreateSolidBrush(robeColor);
    HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    SelectObject(hdc, robeBrush);
    SelectObject(hdc, goldPen);

    POINT robePts[4] = {
        {cx - 18, y + 32},
        {cx - 12, y - 6},
        {cx + 12, y - 6},
        {cx + 18, y + 32}
    };
    Polygon(hdc, robePts, 4);

    // Cowl / Head
    HBRUSH headBrush = CreateSolidBrush(RGB(30, 16, 48));
    SelectObject(hdc, headBrush);
    Ellipse(hdc, cx - 11, y - 18, cx + 11, y - 2);

    // Glowing Eyes
    COLORREF eyeCol = isPlayer ? RGB(96, 165, 250) : RGB(248, 113, 113);
    SetPixel(hdc, cx - 4, y - 12, eyeCol);
    SetPixel(hdc, cx - 3, y - 12, eyeCol);
    SetPixel(hdc, cx + 3, y - 12, eyeCol);
    SetPixel(hdc, cx + 4, y - 12, eyeCol);

    // Wizard Hat Brim
    Ellipse(hdc, cx - 16, y - 22, cx + 16, y - 14);

    // Hat Cone
    POINT hatPts[3] = {
        {cx - 12, y - 18},
        {cx + (isPlayer ? 4 : -4), y - 38},
        {cx + 12, y - 18}
    };
    Polygon(hdc, hatPts, 3);

    // Hat brooch
    HBRUSH goldBrush = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, goldBrush);
    Ellipse(hdc, cx - 3, y - 22, cx + 3, y - 16);

    // Staff
    int staffX = isPlayer ? cx + 24 : cx - 24;
    HPEN staffPen = CreatePen(PS_SOLID, 3, RGB(139, 90, 43));
    SelectObject(hdc, staffPen);
    MoveToEx(hdc, staffX, y + 36, NULL);
    LineTo(hdc, staffX, y - 28);

    // Crystal Orb
    HBRUSH orbBrush = CreateSolidBrush(staffColor);
    HPEN orbPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    SelectObject(hdc, orbBrush);
    SelectObject(hdc, orbPen);
    Ellipse(hdc, staffX - 6, y - 38, staffX + 6, y - 26);

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(shadowBrush);
    DeleteObject(shadowPen);
    DeleteObject(robeBrush);
    DeleteObject(headBrush);
    DeleteObject(goldBrush);
    DeleteObject(goldPen);
    DeleteObject(staffPen);
    DeleteObject(orbBrush);
    DeleteObject(orbPen);
}

void DrawSpellIconGDI(HDC hdc, int x, int y, int type) {
    if (type == 0) { // Fire
        HBRUSH fireBrush = CreateSolidBrush(RGB(255, 69, 0));
        HPEN firePen = CreatePen(PS_SOLID, 1, RGB(255, 200, 0));
        HGDIOBJ ob = SelectObject(hdc, fireBrush);
        HGDIOBJ op = SelectObject(hdc, firePen);
        Ellipse(hdc, x - 10, y - 10, x + 10, y + 10);
        HBRUSH inner = CreateSolidBrush(RGB(255, 220, 50));
        SelectObject(hdc, inner);
        Ellipse(hdc, x - 5, y - 4, x + 5, y + 6);
        SelectObject(hdc, ob); SelectObject(hdc, op);
        DeleteObject(fireBrush); DeleteObject(firePen); DeleteObject(inner);
    } else if (type == 1) { // Ice
        HPEN icePen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        HGDIOBJ op = SelectObject(hdc, icePen);
        MoveToEx(hdc, x, y - 12, NULL); LineTo(hdc, x, y + 12);
        MoveToEx(hdc, x - 10, y - 6, NULL); LineTo(hdc, x + 10, y + 6);
        MoveToEx(hdc, x - 10, y + 6, NULL); LineTo(hdc, x + 10, y - 6);
        SelectObject(hdc, op); DeleteObject(icePen);
    } else if (type == 2) { // Arcane
        HBRUSH arcBrush = CreateSolidBrush(RGB(192, 132, 252));
        HPEN arcPen = CreatePen(PS_SOLID, 1, RGB(233, 213, 255));
        HGDIOBJ ob = SelectObject(hdc, arcBrush);
        HGDIOBJ op = SelectObject(hdc, arcPen);
        Ellipse(hdc, x - 8, y - 8, x + 8, y + 8);
        SelectObject(hdc, ob); SelectObject(hdc, op);
        DeleteObject(arcBrush); DeleteObject(arcPen);
    } else if (type == 3) { // Nature
        HBRUSH leafBrush = CreateSolidBrush(RGB(34, 197, 94));
        HPEN leafPen = CreatePen(PS_SOLID, 1, RGB(134, 239, 172));
        HGDIOBJ ob = SelectObject(hdc, leafBrush);
        HGDIOBJ op = SelectObject(hdc, leafPen);
        POINT pts[4] = {{x, y - 10}, {x + 8, y}, {x, y + 10}, {x - 8, y}};
        Polygon(hdc, pts, 4);
        SelectObject(hdc, ob); SelectObject(hdc, op);
        DeleteObject(leafBrush); DeleteObject(leafPen);
    } else if (type == 4) { // Poison
        HBRUSH pBrush = CreateSolidBrush(RGB(16, 185, 129));
        HPEN pPen = CreatePen(PS_SOLID, 1, RGB(163, 230, 53));
        HGDIOBJ ob = SelectObject(hdc, pBrush);
        HGDIOBJ op = SelectObject(hdc, pPen);
        POINT pts[3] = {{x, y - 10}, {x + 8, y + 8}, {x - 8, y + 8}};
        Polygon(hdc, pts, 3);
        SelectObject(hdc, ob); SelectObject(hdc, op);
        DeleteObject(pBrush); DeleteObject(pPen);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndCombo = CreateWindow("COMBOBOX", "", 
                                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
                                     130, 10, 100, 100,
                                     hwnd, (HMENU)CMB_DIFFICULTY, NULL, NULL);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessage(hwndCombo, CB_SETCURSEL, 1, 0); // Default to Medium
            hwndDraw = CreateWindow("BUTTON", "Draw Card",
                                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                    240, 10, 95, 30,
                                    hwnd, (HMENU)BTN_DRAW, NULL, NULL);
            CreateWindow("BUTTON", "End Turn",
                         WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                         345, 10, 95, 30,
                         hwnd, (HMENU)BTN_END_TURN, NULL, NULL);
            hwndReset = CreateWindow("BUTTON", "Reset Game",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     450, 10, 95, 30,
                                     hwnd, (HMENU)BTN_RESET, NULL, NULL);
            CreateWindow("BUTTON", "Campaign",
                         WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                         555, 10, 95, 30,
                         hwnd, (HMENU)BTN_CAMPAIGN, NULL, NULL);
            hwndDeckBtn = CreateWindow("BUTTON", "Deck",
                                       WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                       660, 10, 50, 30,
                                       hwnd, (HMENU)BTN_DECK, NULL, NULL);
            hwndHelpBtn = CreateWindow("BUTTON", "Help",
                                       WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                       720, 10, 50, 30,
                                       hwnd, (HMENU)BTN_HELP, NULL, NULL);

            hwndAvail = CreateWindow("LISTBOX", "",
                                     WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                     50, 50, 300, 400,
                                     hwnd, (HMENU)LST_AVAIL, NULL, NULL);
            hwndDeck = CreateWindow("LISTBOX", "",
                                    WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                     450, 50, 300, 400,
                                     hwnd, (HMENU)LST_DECK, NULL, NULL);
            hwndDeckClose = CreateWindow("BUTTON", "Save & Close",
                                         WS_CHILD | BS_PUSHBUTTON,
                                         350, 470, 100, 30,
                                         hwnd, (HMENU)BTN_DECK_CLOSE, NULL, NULL);
                                         
            hwndHelp = CreateWindow("LISTBOX", "",
                                    WS_CHILD | WS_BORDER | WS_VSCROLL,
                                    50, 50, 700, 400,
                                    hwnd, (HMENU)LST_HELP, NULL, NULL);
            hwndHelpClose = CreateWindow("BUTTON", "Close Grimoire",
                                         WS_CHILD | BS_PUSHBUTTON,
                                         350, 470, 120, 30,
                                         hwnd, (HMENU)BTN_HELP_CLOSE, NULL, NULL);

            for (int i = 0; i < NUM_SAMPLE_CARDS; i++) {
                char buf[64];
                wsprintf(buf, "%s (Mana: %d)", sampleCards[i].name, sampleCards[i].cost);
                SendMessage(hwndAvail, LB_ADDSTRING, 0, (LPARAM)buf);
            }
            seed = GetTickCount();
            SetTimer(hwnd, IDT_ANIM, 33, NULL);
            ResetGame();
            return 0;

        case WM_TIMER:
            if (wParam == IDT_CAMPAIGN_NEXT) {
                KillTimer(hwnd, IDT_CAMPAIGN_NEXT);
                if (gameState == 1 && campaignLevel > 0 && campaignLevel < 10) {
                    campaignLevel++;
                    InitGame(mages[campaignLevel-1].hp);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == IDT_ANIM) {
                RECT cr; GetClientRect(hwnd, &cr);
                int cw = cr.right - cr.left;
                int ch = cr.bottom - cr.top;

                runicAngle += 0.04f;
                if (screenShake > 0.1f) screenShake *= 0.90f;
                else screenShake = 0.0f;

                // Update Projectiles
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (projectiles[i].active) {
                        projectiles[i].progress += projectiles[i].speed;
                        if (projectiles[i].progress >= 1.0f) {
                            projectiles[i].active = 0;
                            SpawnExplosion(projectiles[i].targetX, projectiles[i].targetY, projectiles[i].type);
                        }
                    }
                }

                // Update Shockwaves
                for (int i = 0; i < shockwaveCount; i++) {
                    shockwaves[i].r += 3.0f;
                    shockwaves[i].alpha -= 0.04f;
                    if (shockwaves[i].alpha <= 0.0f) {
                        shockwaves[i] = shockwaves[--shockwaveCount];
                        i--;
                    }
                }

                // Update Particles
                for (int i = 0; i < particleCount; i++) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].vy += particles[i].gravity;
                    particles[i].life -= particles[i].decay;
                    if (particles[i].life <= 0.0f) {
                        particles[i] = particles[--particleCount];
                        i--;
                    }
                }

                // Update Floaters
                for (int i = 0; i < floaterCount; i++) {
                    floaters[i].y += floaters[i].vy;
                    floaters[i].life -= 0.025f;
                    if (floaters[i].life <= 0.0f) {
                        floaters[i] = floaters[--floaterCount];
                        i--;
                    }
                }

                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_DRAW) {
                if (gameState == 0) {
                    DrawCard(0);
                    DrawCard(1);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (LOWORD(wParam) == BTN_END_TURN) {
                RECT cr; GetClientRect(hwnd, &cr);
                int cw = cr.right - cr.left, ch = cr.bottom - cr.top;

                if (gameState == 0) {
                    if (playerFreeze > 0) playerFreeze--;

                    if (opponentMaxMana < 10) opponentMaxMana++;
                    opponentMana = opponentMaxMana;
                    DrawCard(1);

                    if (opponentBurn > 0) { DealDamageToOpponent(opponentBurn, cw, ch); opponentBurn--; }
                    if (opponentPoison > 0) { DealDamageToOpponent(opponentPoison, cw, ch); opponentPoison--; }
                    if (opponentRegen > 0) { opponentHp += opponentRegen; opponentRegen--; }
                    if (opponentHp > 30) opponentHp = 30;
                    
                    if (opponentHp <= 0) {
                        if (gameState != 1) PlaySoundEffect("win");
                        gameState = 1; // player win by dots
                        SpawnCelebrationStars((float)cw * 0.5f, (float)ch * 0.48f);
                        if (campaignLevel > 0) {
                            if (campaignLevel < 10) {
                                wsprintf(arenaMsg, "VICTORY! %s defeated! Next battle in 3s...", mages[campaignLevel-1].name);
                                SetTimer(hwnd, IDT_CAMPAIGN_NEXT, 3000, NULL);
                            } else {
                                lstrcpyA(arenaMsg, "CAMPAIGN COMPLETE! You are the Grand Magus!");
                                campaignLevel = 0;
                            }
                        }
                    } else {
                        if (opponentFreeze > 0) {
                            lstrcpyA(arenaMsg, "Opponent is frozen and skips turn!");
                            opponentFreeze--;
                        } else {
                            PlayOpponentTurn(cw, ch);
                        }
                    }

                    if (gameState == 0) {
                        if (playerMaxMana < 10) playerMaxMana++;
                        playerMana = playerMaxMana;
                        DrawCard(0);
                        
                        if (playerBurn > 0) { DealDamageToPlayer(playerBurn, cw, ch); playerBurn--; }
                        if (playerPoison > 0) { DealDamageToPlayer(playerPoison, cw, ch); playerPoison--; }
                        if (playerRegen > 0) { playerHp += playerRegen; playerRegen--; }
                        if (playerHp > 30) playerHp = 30;
                        if (playerHp <= 0) { playerHp = 0; if (gameState != 2) PlaySoundEffect("lose"); gameState = 2; }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }

            } else if (LOWORD(wParam) == BTN_RESET) {
                ResetGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == BTN_CAMPAIGN) {
                campaignLevel = 1;
                InitGame(mages[campaignLevel-1].hp);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == BTN_DECK) {
                gameState = 3;
                ShowWindow(hwndHelp, SW_HIDE);
                ShowWindow(hwndHelpClose, SW_HIDE);
                SendMessage(hwndDeck, LB_RESETCONTENT, 0, 0);
                for (int i = 0; i < playerDeckCount; i++) {
                    char buf[64];
                    wsprintf(buf, "%s (Mana: %d)", sampleCards[playerDeck[i]].name, sampleCards[playerDeck[i]].cost);
                    SendMessage(hwndDeck, LB_ADDSTRING, 0, (LPARAM)buf);
                }
                ShowWindow(hwndAvail, SW_SHOW);
                ShowWindow(hwndDeck, SW_SHOW);
                ShowWindow(hwndDeckClose, SW_SHOW);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == BTN_DECK_CLOSE) {
                if (playerDeckCount != 20) {
                    MessageBox(hwnd, "You must have exactly 20 cards in your deck.", "Deck Builder", MB_OK | MB_ICONWARNING);
                } else {
                    gameState = 0;
                    ShowWindow(hwndAvail, SW_HIDE);
                    ShowWindow(hwndDeck, SW_HIDE);
                    ShowWindow(hwndDeckClose, SW_HIDE);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (LOWORD(wParam) == BTN_HELP) {
                gameState = 4;
                ShowWindow(hwndAvail, SW_HIDE);
                ShowWindow(hwndDeck, SW_HIDE);
                ShowWindow(hwndDeckClose, SW_HIDE);
                SendMessage(hwndHelp, LB_RESETCONTENT, 0, 0);
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== HOW TO PLAY ===");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"You and your opponent take turns casting spells.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"You start with 1 Max Mana, gaining 1 per turn (up to 10).");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Defeat the opponent by reducing their HP to 0.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== STATUS EFFECTS ===");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Shield: Absorbs incoming damage.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Burn/Poison: Take damage at the start of your turn.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Frozen: Skip your next turn or cannot cast spells.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Regen: Heal HP at the start of your turn.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== SPELL INDEX ===");
                for (int i = 0; i < NUM_SAMPLE_CARDS; i++) {
                    char buf[128];
                    wsprintf(buf, "%s (Cost: %d) - %s", sampleCards[i].name, sampleCards[i].cost, sampleCards[i].effect);
                    SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)buf);
                }
                ShowWindow(hwndHelp, SW_SHOW);
                ShowWindow(hwndHelpClose, SW_SHOW);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == BTN_HELP_CLOSE) {
                gameState = 0;
                ShowWindow(hwndHelp, SW_HIDE);
                ShowWindow(hwndHelpClose, SW_HIDE);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == LST_AVAIL && HIWORD(wParam) == LBN_DBLCLK) {
                if (gameState == 3 && playerDeckCount < 20) {
                    int sel = (int)SendMessage(hwndAvail, LB_GETCURSEL, 0, 0);
                    if (sel != LB_ERR) {
                        playerDeck[playerDeckCount++] = sel;
                        char buf[64];
                        wsprintf(buf, "%s (Mana: %d)", sampleCards[sel].name, sampleCards[sel].cost);
                        SendMessage(hwndDeck, LB_ADDSTRING, 0, (LPARAM)buf);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            } else if (LOWORD(wParam) == LST_DECK && HIWORD(wParam) == LBN_DBLCLK) {
                if (gameState == 3) {
                    int sel = (int)SendMessage(hwndDeck, LB_GETCURSEL, 0, 0);
                    if (sel != LB_ERR) {
                        SendMessage(hwndDeck, LB_DELETESTRING, sel, 0);
                        for (int i = sel; i < playerDeckCount - 1; i++) {
                            playerDeck[i] = playerDeck[i + 1];
                        }
                        playerDeckCount--;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            return 0;

        case WM_LBUTTONDOWN: {
            int xPos = (short)LOWORD(lParam);
            int yPos = (short)HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;
            
            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;

            if (gameState != 0) return 0;
            if (playerFreeze > 0) {
                lstrcpyA(arenaMsg, "You are frozen and cannot cast spells!");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            
            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                if (xPos >= cx && xPos <= cx + cardW && yPos >= playerY && yPos <= playerY + cardH) {
                    CardDef cd = sampleCards[playerHand[i]];
                    if (playerMana >= cd.cost) {
                        playerMana -= cd.cost;
                        
                        int dmg = cd.damage;
                        if (strcmp(cd.name, "Ice Lance") == 0 && opponentFreeze > 0) dmg = 3;
                        DealDamageToOpponent(dmg, cw, ch);
                        
                        if (cd.heal > 0) {
                            playerHp += cd.heal;
                            char b[32]; wsprintf(b, "+%d HP", cd.heal);
                            SpawnFloater((float)cw * 0.25f, (float)ch * 0.40f, b, RGB(74, 222, 128));
                        }
                        opponentBurn += cd.burn;
                        opponentFreeze += cd.freeze;
                        playerShield += cd.shield;
                        playerRegen += cd.regen;
                        opponentPoison += cd.poison;
                        
                        LaunchSpellVisual(1, &cd, cw, ch);
                        PlaySoundEffect(GetSoundType(&cd));
                        
                        if (strcmp(cd.name, "Arcane Intellect") == 0) {
                            DrawCard(0); DrawCard(0);
                        }

                        if (opponentHp <= 0) {
                            opponentHp = 0;
                            if (gameState != 1) PlaySoundEffect("win");
                            gameState = 1; // player win
                            SpawnCelebrationStars((float)cw * 0.5f, (float)ch * 0.48f);
                        }
                        if (playerHp > 30) playerHp = 30;

                        if (gameState == 1 && campaignLevel > 0) {
                            if (campaignLevel < 10) {
                                wsprintf(arenaMsg, "VICTORY! %s defeated! Next battle in 3s...", mages[campaignLevel-1].name);
                                SetTimer(hwnd, IDT_CAMPAIGN_NEXT, 3000, NULL);
                            } else {
                                lstrcpyA(arenaMsg, "CAMPAIGN COMPLETE! You are the Grand Magus!");
                                campaignLevel = 0;
                            }
                        } else {
                            wsprintf(arenaMsg, "Cast %s: %s", cd.name, cd.effect);
                        }
                        
                        for (int j = i; j < playerCount - 1; j++) {
                            playerHand[j] = playerHand[j + 1];
                        }
                        playerCount--;
                    } else {
                        wsprintf(arenaMsg, "Not enough mana for %s!", cd.name);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;

            // Double Buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, cw, ch);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);

            HFONT hFont = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_ROMAN, "Georgia");
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

            if (gameState == 3 || gameState == 4) {
                HBRUSH bgBrush = CreateSolidBrush(RGB(10, 5, 20));
                FillRect(memDC, &clientRect, bgBrush);
                DeleteObject(bgBrush);
                
                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, RGB(232, 216, 183));
                
                if (gameState == 3) {
                    RECT lblA = {50, 20, 350, 50};
                    DrawText(memDC, "Available Spells (Double-click to add)", -1, &lblA, DT_CENTER | DT_SINGLELINE);
                    
                    char lblDStr[64];
                    wsprintf(lblDStr, "Your Deck (%d/20) (Double-click to remove)", playerDeckCount);
                    RECT lblD = {450, 20, 750, 50};
                    DrawText(memDC, lblDStr, -1, &lblD, DT_CENTER | DT_SINGLELINE);
                } else if (gameState == 4) {
                    RECT lblH = {0, 20, cw, 50};
                    DrawText(memDC, "Grimoire & How to Play", -1, &lblH, DT_CENTER | DT_SINGLELINE);
                }

                SelectObject(memDC, oldFont);
                DeleteObject(hFont);
                BitBlt(hdc, 0, 0, cw, ch, memDC, 0, 0, SRCCOPY);
                SelectObject(memDC, oldBM);
                DeleteObject(memBM);
                DeleteDC(memDC);
                EndPaint(hwnd, &ps);
                return 0;
            }

            // Chamber Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(13, 4, 22));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            // Arena Viewport Frame
            RECT arenaRect = {20, 210, cw - 20, ch - 200};
            HBRUSH arenaBrush = CreateSolidBrush(RGB(20, 9, 36));
            FillRect(memDC, &arenaRect, arenaBrush);
            DeleteObject(arenaBrush);
            
            // Central Rotating Arcane Runic Transmutation Circle
            int arenaCX = (arenaRect.left + arenaRect.right) / 2;
            int arenaCY = (arenaRect.top + arenaRect.bottom) / 2;
            
            HPEN runePen1 = CreatePen(PS_SOLID, 1, RGB(184, 153, 71));
            HGDIOBJ oldP = SelectObject(memDC, runePen1);
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldB = SelectObject(memDC, nullBrush);
            
            Ellipse(memDC, arenaCX - 80, arenaCY - 35, arenaCX + 80, arenaCY + 35);
            Ellipse(memDC, arenaCX - 60, arenaCY - 25, arenaCX + 60, arenaCY + 25);

            for (int i = 0; i < 8; i++) {
                float a = runicAngle + (float)i * 0.785f;
                int rx = arenaCX + (int)(cosf(a) * 70.0f);
                int ry = arenaCY + (int)(sinf(a) * 30.0f);
                SetPixel(memDC, rx, ry, RGB(255, 215, 0));
                SetPixel(memDC, rx+1, ry, RGB(255, 215, 0));
            }

            // Draw Wizards
            float curTime = (float)GetTickCount() * 0.001f;
            COLORREF oppRobe = (campaignLevel > 0) ? mages[campaignLevel-1].robeColor : RGB(160, 30, 40);
            COLORREF oppStaff = (campaignLevel > 0) ? mages[campaignLevel-1].staffColor : RGB(255, 100, 0);

            DrawWizardSpriteGDI(memDC, (int)((float)cw * 0.25f), arenaCY, 1, RGB(55, 25, 90), RGB(96, 165, 250), curTime);
            DrawWizardSpriteGDI(memDC, (int)((float)cw * 0.75f), arenaCY, 0, oppRobe, oppStaff, curTime);

            // Projectiles
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (projectiles[i].active) {
                    float px = projectiles[i].startX + (projectiles[i].targetX - projectiles[i].startX) * projectiles[i].progress;
                    float py = projectiles[i].startY + (projectiles[i].targetY - projectiles[i].startY) * projectiles[i].progress - sinf(projectiles[i].progress * 3.1415f) * 40.0f;
                    
                    COLORREF projCol = RGB(168, 85, 247);
                    if (projectiles[i].type == 0) projCol = RGB(255, 69, 0);
                    else if (projectiles[i].type == 1) projCol = RGB(0, 220, 255);
                    else if (projectiles[i].type == 3) projCol = RGB(34, 197, 94);
                    else if (projectiles[i].type == 4) projCol = RGB(16, 185, 129);

                    HBRUSH pBr = CreateSolidBrush(projCol);
                    SelectObject(memDC, pBr);
                    Ellipse(memDC, (int)px - 6, (int)py - 6, (int)px + 6, (int)py + 6);
                    DeleteObject(pBr);
                }
            }

            // Shockwaves
            for (int i = 0; i < shockwaveCount; i++) {
                HPEN swPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                SelectObject(memDC, swPen);
                int ir = (int)shockwaves[i].r;
                Ellipse(memDC, (int)shockwaves[i].x - ir, (int)shockwaves[i].y - ir/2, (int)shockwaves[i].x + ir, (int)shockwaves[i].y + ir/2);
                DeleteObject(swPen);
            }

            // Particles
            for (int i = 0; i < particleCount; i++) {
                HBRUSH ptBr = CreateSolidBrush(particles[i].color);
                SelectObject(memDC, ptBr);
                int sz = (int)particles[i].size;
                if (sz < 1) sz = 1;
                if (particles[i].type == 2) {
                    Rectangle(memDC, (int)particles[i].x - sz/2, (int)particles[i].y - sz/2, (int)particles[i].x + sz/2, (int)particles[i].y + sz/2);
                } else {
                    Ellipse(memDC, (int)particles[i].x - sz, (int)particles[i].y - sz, (int)particles[i].x + sz, (int)particles[i].y + sz);
                }
                DeleteObject(ptBr);
            }

            // Arena Border
            HPEN borderPen = CreatePen(PS_SOLID, 3, RGB(184, 153, 71));
            SelectObject(memDC, borderPen);
            Rectangle(memDC, arenaRect.left, arenaRect.top, arenaRect.right, arenaRect.bottom);
            DeleteObject(borderPen);

            // Ornate Gold Corner Filigree L-Brackets
            HPEN filigreePen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
            SelectObject(memDC, filigreePen);
            int bLen = 16;
            // TL
            MoveToEx(memDC, arenaRect.left + 6, arenaRect.top + 6 + bLen, NULL);
            LineTo(memDC, arenaRect.left + 6, arenaRect.top + 6);
            LineTo(memDC, arenaRect.left + 6 + bLen, arenaRect.top + 6);
            // TR
            MoveToEx(memDC, arenaRect.right - 6 - bLen, arenaRect.top + 6, NULL);
            LineTo(memDC, arenaRect.right - 6, arenaRect.top + 6);
            LineTo(memDC, arenaRect.right - 6, arenaRect.top + 6 + bLen);
            // BL
            MoveToEx(memDC, arenaRect.left + 6, arenaRect.bottom - 6 - bLen, NULL);
            LineTo(memDC, arenaRect.left + 6, arenaRect.bottom - 6);
            LineTo(memDC, arenaRect.left + 6 + bLen, arenaRect.bottom - 6);
            // BR
            MoveToEx(memDC, arenaRect.right - 6 - bLen, arenaRect.bottom - 6, NULL);
            LineTo(memDC, arenaRect.right - 6, arenaRect.bottom - 6);
            LineTo(memDC, arenaRect.right - 6, arenaRect.bottom - 6 - bLen);
            DeleteObject(filigreePen);

            // Floaters
            SetBkMode(memDC, TRANSPARENT);
            HFONT hFloatFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                          DEFAULT_PITCH | FF_ROMAN, "Georgia");
            SelectObject(memDC, hFloatFont);
            for (int i = 0; i < floaterCount; i++) {
                SetTextColor(memDC, floaters[i].color);
                RECT fR = {(int)floaters[i].x - 60, (int)floaters[i].y - 10, (int)floaters[i].x + 60, (int)floaters[i].y + 10};
                DrawText(memDC, floaters[i].text, -1, &fR, DT_CENTER | DT_SINGLELINE);
            }
            SelectObject(memDC, hFont);
            DeleteObject(hFloatFont);

            // Arena Message Text
            HFONT hArenaFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                          DEFAULT_PITCH | FF_ROMAN, "Georgia");
            SelectObject(memDC, hArenaFont);
            RECT bannerRect = {arenaRect.left, arenaRect.top + 10, arenaRect.right, arenaRect.top + 35};
            if (gameState == 1) {
                SetTextColor(memDC, RGB(74, 222, 128));
                DrawText(memDC, "VICTORY! You defeated the opponent!", -1, &bannerRect, DT_CENTER | DT_SINGLELINE);
            } else if (gameState == 2) {
                SetTextColor(memDC, RGB(239, 68, 68));
                DrawText(memDC, "DEFEAT! You have been slain...", -1, &bannerRect, DT_CENTER | DT_SINGLELINE);
            } else {
                SetTextColor(memDC, RGB(255, 215, 0));
                DrawText(memDC, arenaMsg, -1, &bannerRect, DT_CENTER | DT_SINGLELINE);
            }
            SelectObject(memDC, hFont);
            DeleteObject(hArenaFont);

            // Cards Setup
            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            // Opponent Cards
            int oppW = opponentCount * cardW + (opponentCount > 0 ? opponentCount - 1 : 0) * gap;
            int oppX = (cw - oppW) / 2;
            int oppY = 55;
            HBRUSH oppBrush = CreateSolidBrush(RGB(35, 20, 12));
            HPEN oppPen = CreatePen(PS_SOLID, 2, RGB(120, 78, 45));

            for (int i = 0; i < opponentCount; i++) {
                int cx = oppX + i * (cardW + gap);
                SelectObject(memDC, oppBrush);
                SelectObject(memDC, oppPen);
                RoundRect(memDC, cx, oppY, cx + cardW, oppY + cardH, 8, 8);
                
                SetTextColor(memDC, RGB(212, 175, 55));
                RECT textRect = {cx, oppY + 50, cx + cardW, oppY + 70};
                DrawText(memDC, "[GRIMOIRE]", -1, &textRect, DT_CENTER | DT_SINGLELINE);
            }
            DeleteObject(oppBrush);
            DeleteObject(oppPen);

            // Player Cards
            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;

            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                CardDef cd = sampleCards[playerHand[i]];

                COLORREF cardBg = RGB(35, 18, 50);
                COLORREF cardBorder = RGB(168, 85, 247);
                if (cd.type == 0) { cardBg = RGB(50, 15, 15); cardBorder = RGB(230, 92, 0); }
                else if (cd.type == 1) { cardBg = RGB(12, 35, 60); cardBorder = RGB(0, 180, 216); }
                else if (cd.type == 3) { cardBg = RGB(15, 45, 22); cardBorder = RGB(34, 197, 94); }
                else if (cd.type == 4) { cardBg = RGB(28, 15, 40); cardBorder = RGB(16, 185, 129); }

                HBRUSH pCardBr = CreateSolidBrush(cardBg);
                HPEN pCardPen = CreatePen(PS_SOLID, 2, cardBorder);
                SelectObject(memDC, pCardBr);
                SelectObject(memDC, pCardPen);
                RoundRect(memDC, cx, playerY, cx + cardW, playerY + cardH, 8, 8);
                DeleteObject(pCardBr);
                DeleteObject(pCardPen);

                // Mana Gem Orb
                HBRUSH manaBr = CreateSolidBrush(RGB(29, 78, 216));
                HPEN manaPen = CreatePen(PS_SOLID, 1, RGB(147, 197, 253));
                SelectObject(memDC, manaBr);
                SelectObject(memDC, manaPen);
                Ellipse(memDC, cx + 5, playerY + 5, cx + 23, playerY + 23);
                DeleteObject(manaBr);
                DeleteObject(manaPen);

                SetTextColor(memDC, RGB(255, 255, 255));
                char costStr[8]; wsprintf(costStr, "%d", cd.cost);
                RECT manaRect = {cx + 5, playerY + 6, cx + 23, playerY + 23};
                DrawText(memDC, costStr, -1, &manaRect, DT_CENTER | DT_SINGLELINE);

                // Card Title
                SetTextColor(memDC, RGB(255, 215, 0));
                RECT nameRect = {cx + 25, playerY + 6, cx + cardW - 4, playerY + 24};
                DrawText(memDC, cd.name, -1, &nameRect, DT_CENTER | DT_SINGLELINE);

                // Card Art Area
                HBRUSH artBg = CreateSolidBrush(RGB(10, 5, 18));
                SelectObject(memDC, artBg);
                Rectangle(memDC, cx + 12, playerY + 28, cx + cardW - 12, playerY + 75);
                DeleteObject(artBg);

                DrawSpellIconGDI(memDC, cx + cardW / 2, playerY + 51, cd.type);

                // Card Effect
                SetTextColor(memDC, RGB(220, 205, 180));
                RECT effRect = {cx + 4, playerY + 80, cx + cardW - 4, playerY + cardH - 4};
                DrawText(memDC, cd.effect, -1, &effRect, DT_CENTER | DT_WORDBREAK);
            }

            // HUD Labels
            SetTextColor(memDC, RGB(232, 216, 183));
            char oppLabel[256];
            char oppName[64];
            if (campaignLevel > 0) lstrcpyA(oppName, mages[campaignLevel-1].name);
            else lstrcpyA(oppName, "Opponent");
            int pos = wsprintf(oppLabel, "%s (HP: %d | Mana: %d/%d)", oppName, opponentHp, opponentMana, opponentMaxMana);
            if (opponentShield > 0) pos += wsprintf(oppLabel + pos, " [Shield %d]", opponentShield);
            if (opponentBurn > 0) pos += wsprintf(oppLabel + pos, " [Burn %d]", opponentBurn);
            if (opponentPoison > 0) pos += wsprintf(oppLabel + pos, " [Poison %d]", opponentPoison);
            if (opponentFreeze > 0) pos += wsprintf(oppLabel + pos, " [Frozen %d]", opponentFreeze);
            if (opponentRegen > 0) pos += wsprintf(oppLabel + pos, " [Regen %d]", opponentRegen);
            RECT lblOpp = {0, oppY - 20, cw, oppY};
            DrawText(memDC, oppLabel, -1, &lblOpp, DT_CENTER | DT_SINGLELINE);
            
            char playerLabel[256];
            pos = wsprintf(playerLabel, "Archmage (HP: %d | Mana: %d/%d)", playerHp, playerMana, playerMaxMana);
            if (playerShield > 0) pos += wsprintf(playerLabel + pos, " [Shield %d]", playerShield);
            if (playerBurn > 0) pos += wsprintf(playerLabel + pos, " [Burn %d]", playerBurn);
            if (playerPoison > 0) pos += wsprintf(playerLabel + pos, " [Poison %d]", playerPoison);
            if (playerFreeze > 0) pos += wsprintf(playerLabel + pos, " [Frozen %d]", playerFreeze);
            if (playerRegen > 0) pos += wsprintf(playerLabel + pos, " [Regen %d]", playerRegen);
            RECT lblPlayer = {0, playerY - 20, cw, playerY};
            DrawText(memDC, playerLabel, -1, &lblPlayer, DT_CENTER | DT_SINGLELINE);

            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            SelectObject(memDC, oldP);
            DeleteObject(runePen1);

            // Screen Shake Offset on BitBlt
            int sx = 0, sy = 0;
            if (screenShake > 0.1f) {
                sx = (int)((sinf((float)GetTickCount() * 0.05f)) * screenShake);
                sy = (int)((cosf((float)GetTickCount() * 0.07f)) * screenShake);
            }

            BitBlt(hdc, sx, sy, cw, ch, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KWizardClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KWizard",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 840, 640,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
