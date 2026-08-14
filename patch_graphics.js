const fs = require('fs');

let mainC = fs.readFileSync('c:/KiloApps/KiloApps/KSpace/main.c', 'utf8');

// 1. main.c structs
mainC = mainC.replace(
    'typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; } Particle;',
    'typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; int wType; } Particle;'
);
mainC = mainC.replace(
    'typedef struct { float x, y, vx, vy, size, rot, vrot, life, decay; COLORREF color; } Debris;',
    'typedef struct { float x, y, vx, vy, size, rot, vrot, life, decay; COLORREF color; int shape; } Debris;'
);

// 2. main.c AddDebrisChunk & AddExplosion 
const addExplosionBlock = `void AddDebrisChunk(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_DEBRIS && added < count; i++) {
        if (debris[i].life <= 0.0f) {
            debris[i].x = x; debris[i].y = y;
            debris[i].vx = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].vy = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].rot = (float)(rnd() % 628) / 100.0f;
            debris[i].vrot = (float)((int)(rnd() % 10) - 5) * 0.05f;
            debris[i].size = 3.0f + (rnd() % 4);
            debris[i].color = col;
            debris[i].life = 1.0f;
            debris[i].decay = 0.02f + (float)(rnd() % 20) / 1000.0f;
            added++;
        }
    }
}`;
const newAddExplosionBlock = `void AddDebrisChunk(float x, float y, int count, COLORREF col, int shape) {
    int added = 0;
    for (int i = 0; i < MAX_DEBRIS && added < count; i++) {
        if (debris[i].life <= 0.0f) {
            debris[i].x = x; debris[i].y = y;
            debris[i].vx = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].vy = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].rot = (float)(rnd() % 628) / 100.0f;
            debris[i].vrot = (float)((int)(rnd() % 10) - 5) * 0.05f;
            debris[i].size = 3.0f + (rnd() % 4);
            debris[i].color = col;
            debris[i].life = 1.0f;
            debris[i].decay = 0.02f + (float)(rnd() % 20) / 1000.0f;
            debris[i].shape = shape;
            added++;
        }
    }
}`;
mainC = mainC.replace(addExplosionBlock, newAddExplosionBlock);

const expBlock = `void AddExplosion(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].vy = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].life = 14 + (rnd() % 16);
            particles[i].maxLife = particles[i].life;
            particles[i].color = col;
            added++;
        }
    }
    if (count >= 10) {
        AddShockwave(x, y, (float)count * 2.2f, col);
        AddDebrisChunk(x, y, (count / 2 > 10 ? 10 : count / 2), col);
    }
}`;
const newExpBlock = `void AddShipExplosion(float x, float y, int count, COLORREF col, int shape) {
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x; particles[i].y = y;
            particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].vy = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].life = 14 + (rnd() % 16);
            particles[i].maxLife = particles[i].life;
            particles[i].color = col;
            particles[i].wType = 0;
            added++;
        }
    }
    if (count >= 10) {
        AddShockwave(x, y, (float)count * 2.2f, col);
        AddDebrisChunk(x, y, (count / 2 > 10 ? 10 : count / 2), col, shape);
    }
}
void AddExplosion(float x, float y, int count, COLORREF col) { AddShipExplosion(x, y, count, col, 0); }
void AddWeaponHitParticles(float x, float y, int wType, COLORREF col) {
    int count = (wType == 3) ? 8 : ((wType == 1) ? 5 : 3);
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x; particles[i].y = y;
            if (wType == 1) { 
                particles[i].vx = (float)((int)(rnd() % 21) - 10) * 1.5f;
                particles[i].vy = (float)((int)(rnd() % 21) - 10) * 1.5f;
                particles[i].life = 8 + (rnd() % 8);
            } else if (wType == 2) { 
                particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.5f;
                particles[i].vy = 2.0f + (float)(rnd() % 5);
                particles[i].life = 6 + (rnd() % 6);
            } else if (wType == 3) { 
                particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.4f;
                particles[i].vy = (float)((int)(rnd() % 11) - 5) * 0.4f;
                particles[i].life = 20 + (rnd() % 15);
            } else { 
                particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.8f;
                particles[i].vy = (float)((int)(rnd() % 11) - 5) * 0.8f;
                particles[i].life = 14 + (rnd() % 16);
            }
            particles[i].maxLife = particles[i].life;
            particles[i].color = col;
            particles[i].wType = wType;
            added++;
        }
    }
}`;
mainC = mainC.replace(expBlock, newExpBlock);

const drawPartsBlock = `void DrawParticles(HDC hdc) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            HBRUSH pbr = CreateSolidBrush(particles[i].color);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, pbr);
            int sz = (particles[i].life > 8) ? 3 : 2;
            RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + sz};
            FillRect(hdc, &pr, pbr);
            SelectObject(hdc, oldBr);
            DeleteObject(pbr);
        }
    }
    SelectObject(hdc, oldPen);
}`;
const newDrawPartsBlock = `void DrawParticles(HDC hdc) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            HBRUSH pbr = CreateSolidBrush(particles[i].color);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, pbr);
            if (particles[i].wType == 3) {
                int sz = (particles[i].life > 10) ? 6 : 4;
                Ellipse(hdc, (int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + sz);
            } else if (particles[i].wType == 2) {
                int sz = (particles[i].life > 4) ? 4 : 2;
                RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + 1, (int)particles[i].y + sz};
                FillRect(hdc, &pr, pbr);
            } else if (particles[i].wType == 1) {
                int sz = (particles[i].life > 4) ? 3 : 1;
                RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + 1};
                FillRect(hdc, &pr, pbr);
            } else {
                int sz = (particles[i].life > 8) ? 3 : 2;
                RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + sz};
                FillRect(hdc, &pr, pbr);
            }
            SelectObject(hdc, oldBr);
            DeleteObject(pbr);
        }
    }
    SelectObject(hdc, oldPen);
}`;
mainC = mainC.replace(drawPartsBlock, newDrawPartsBlock);

// 3. main.c Replace AddExplosion with AddWeaponHitParticles where appropriate
mainC = mainC.replace(
    'shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;\r\n                            AddExplosion(b[i].x, b[i].y, 4, RGB(255, 23, 68));',
    'shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;\r\n                            AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(255, 23, 68));'
);
mainC = mainC.replace(
    'AddExplosion(b[i].x, b[i].y, 3, RGB(0, 229, 255)); // Deflector absorbed!',
    'AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(0, 229, 255)); // Deflector absorbed!'
);
mainC = mainC.replace(
    'shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;\r\n                        AddExplosion(b[i].x, b[i].y, 3, RGB(0, 229, 255));',
    'shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;\r\n                        AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(0, 229, 255));'
);
mainC = mainC.replace(
    'shotsHit++; e[i].hp -= (b[j].type == 1.0f) ? 6 : 1;\r\n                        AddExplosion(b[j].x, b[j].y, 3, RGB(0, 229, 255));',
    'shotsHit++; e[i].hp -= (b[j].type == 1.0f) ? 6 : 1;\r\n                        AddWeaponHitParticles(b[j].x, b[j].y, weaponType, RGB(0, 229, 255));'
);
mainC = mainC.replace(
    'if (!e[i].cloaked) e[i].hp -= 1;\r\n                    AddExplosion(p.x + 10.0f, e[i].y + eh/2.0f, 2, RGB(0, 229, 255));',
    'if (!e[i].cloaked) e[i].hp -= 1;\r\n                    AddWeaponHitParticles(p.x + 10.0f, e[i].y + eh/2.0f, 2, RGB(0, 229, 255));'
);

// 4. main.c Replace Enemy death AddExplosion with AddShipExplosion 
const enemyDeathBlock = `if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                comboTimer = 180;
                if (comboMultiplier < 10) comboMultiplier++;
                int baseScore = (e[i].type == 6.0f ? 300 : (e[i].type == 9.0f ? 200 : (e[i].type == 8.0f ? 150 : (e[i].type == 7.0f ? 120 : 30))));
                score += baseScore * comboMultiplier;
                enemiesKilled++;
                totalKills++;
                PlaySnd(1);
                AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 14, RGB(255, 152, 0));`;

const newEnemyDeathBlock = `if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                comboTimer = 180;
                if (comboMultiplier < 10) comboMultiplier++;
                int baseScore = (e[i].type == 6.0f ? 300 : (e[i].type == 9.0f ? 200 : (e[i].type == 8.0f ? 150 : (e[i].type == 7.0f ? 120 : 30))));
                score += baseScore * comboMultiplier;
                enemiesKilled++;
                totalKills++;
                PlaySnd(1);
                COLORREF eCol = RGB(255, 23, 68);
                if (e[i].type == 1.0f) eCol = RGB(255, 145, 0);
                else if (e[i].type == 2.0f) eCol = RGB(213, 0, 249);
                else if (e[i].type == 3.0f) eCol = RGB(120, 144, 156);
                else if (e[i].type == 4.0f) eCol = RGB(0, 229, 255);
                else if (e[i].type == 5.0f || e[i].type == 9.0f) eCol = RGB(141, 110, 99);
                else if (e[i].type == 6.0f) eCol = RGB(198, 40, 40);
                else if (e[i].type == 7.0f) eCol = RGB(255, 235, 59);
                else if (e[i].type == 8.0f) eCol = RGB(103, 58, 183);
                AddShipExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 14, eCol, (int)e[i].type);`;

mainC = mainC.replace(enemyDeathBlock, newEnemyDeathBlock);

// 5. main.c Replace debris rendering loop
const debrisDrawBlock = `// Debris Chunks
                    for (int i = 0; i < MAX_DEBRIS; i++) {
                        if (debris[i].life > 0.0f) {
                            debris[i].x += debris[i].vx; debris[i].y += debris[i].vy;
                            debris[i].life -= debris[i].decay;
                            HBRUSH dbr = CreateSolidBrush(debris[i].color);
                            int sz = (int)debris[i].size;
                            RECT dr = {(int)debris[i].x - sz/2, (int)debris[i].y - sz/2, (int)debris[i].x + sz/2, (int)debris[i].y + sz/2};
                            FillRect(memDC, &dr, dbr);
                            DeleteObject(dbr);
                        }
                    }`;
const newDebrisDrawBlock = `// Debris Chunks
                    for (int i = 0; i < MAX_DEBRIS; i++) {
                        if (debris[i].life > 0.0f) {
                            debris[i].x += debris[i].vx; debris[i].y += debris[i].vy;
                            debris[i].life -= debris[i].decay;
                            HBRUSH dbr = CreateSolidBrush(debris[i].color);
                            int sz = (int)debris[i].size;
                            if (debris[i].shape == 2 || debris[i].shape == 5) {
                                Ellipse(memDC, (int)debris[i].x - sz, (int)debris[i].y - sz, (int)debris[i].x + sz, (int)debris[i].y + sz);
                            } else if (debris[i].shape == 4 || debris[i].shape == 1) {
                                POINT pts[3] = { {(int)debris[i].x, (int)debris[i].y - sz}, {(int)debris[i].x + sz, (int)debris[i].y + sz}, {(int)debris[i].x - sz, (int)debris[i].y + sz} };
                                HPEN oldPen = (HPEN)SelectObject(memDC, GetStockObject(NULL_PEN));
                                HBRUSH oldBr = (HBRUSH)SelectObject(memDC, dbr);
                                Polygon(memDC, pts, 3);
                                SelectObject(memDC, oldPen); SelectObject(memDC, oldBr);
                            } else {
                                RECT dr = {(int)debris[i].x - sz/2, (int)debris[i].y - sz/2, (int)debris[i].x + sz/2, (int)debris[i].y + sz/2};
                                FillRect(memDC, &dr, dbr);
                            }
                            DeleteObject(dbr);
                        }
                    }`;
mainC = mainC.replace(debrisDrawBlock, newDebrisDrawBlock);

// 6. main.c Replace nebulae rendering loop to make them procedural
const nebulaeDrawBlock = `// Deep Space Nebulae Rendering (Loop 2)
                for (int n = 0; n < 3; n++) {
                    nebulae[n].x += nebulae[n].vx;
                    nebulae[n].y += nebulae[n].vy;
                    if (nebulae[n].x < -nebulae[n].r) nebulae[n].x = W + nebulae[n].r;
                    if (nebulae[n].x > W + nebulae[n].r) nebulae[n].x = -nebulae[n].r;
                    if (nebulae[n].y < -nebulae[n].r) nebulae[n].y = H + nebulae[n].r;
                    if (nebulae[n].y > H + nebulae[n].r) nebulae[n].y = -nebulae[n].r;

                    HBRUSH nbr = CreateSolidBrush(nebulae[n].col1);
                    HBRUSH oldBr = (HBRUSH)SelectObject(memDC, nbr);
                    int nr = (int)(nebulae[n].r);
                    Ellipse(memDC, (int)nebulae[n].x - nr, (int)nebulae[n].y - nr, (int)nebulae[n].x + nr, (int)nebulae[n].y + nr);
                    SelectObject(memDC, oldBr); DeleteObject(nbr);
                }`;
const newNebulaeDrawBlock = `// Deep Space Nebulae Rendering (Loop 2)
                for (int n = 0; n < 3; n++) {
                    nebulae[n].x += nebulae[n].vx;
                    nebulae[n].y += nebulae[n].vy;
                    nebulae[n].phase += 0.02f;
                    if (nebulae[n].x < -nebulae[n].r) nebulae[n].x = W + nebulae[n].r;
                    if (nebulae[n].x > W + nebulae[n].r) nebulae[n].x = -nebulae[n].r;
                    if (nebulae[n].y < -nebulae[n].r) nebulae[n].y = H + nebulae[n].r;
                    if (nebulae[n].y > H + nebulae[n].r) nebulae[n].y = -nebulae[n].r;

                    HBRUSH nbr = CreateSolidBrush(nebulae[n].col1);
                    HBRUSH oldBr = (HBRUSH)SelectObject(memDC, nbr);
                    int nr = (int)(nebulae[n].r + sin(nebulae[n].phase) * 12.0f);
                    Ellipse(memDC, (int)nebulae[n].x - nr, (int)nebulae[n].y - nr, (int)nebulae[n].x + nr, (int)nebulae[n].y + nr);
                    
                    HBRUSH nbr2 = CreateSolidBrush(RGB(GetRValue(nebulae[n].col1)/2, GetGValue(nebulae[n].col1)/2, GetBValue(nebulae[n].col1)/2));
                    SelectObject(memDC, nbr2);
                    int nr2 = (int)(nr * 0.7f);
                    Ellipse(memDC, (int)nebulae[n].x - nr2, (int)nebulae[n].y - nr2, (int)nebulae[n].x + nr2, (int)nebulae[n].y + nr2);
                    SelectObject(memDC, oldBr); DeleteObject(nbr); DeleteObject(nbr2);
                }`;
mainC = mainC.replace(nebulaeDrawBlock, newNebulaeDrawBlock);

// 7. main.c Replace star twinkle
const starTwinkleBlock = `                    } else if (stars[i].layer == 1) {
                        HBRUSH sbr = CreateSolidBrush(RGB(180, 230, 255));
                        RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + 1, (int)stars[i].y + 1};
                        FillRect(memDC, &sr, sbr);
                        DeleteObject(sbr);
                    } else {`;
const newStarTwinkleBlock = `                    } else if (stars[i].layer == 1) {
                        int r = 180 + (int)(sin(frameCount * 0.1f + stars[i].x) * 75.0f);
                        int g = 230 + (int)(cos(frameCount * 0.1f + stars[i].y) * 25.0f);
                        if (r > 255) r = 255; if (r < 0) r = 0;
                        if (g > 255) g = 255; if (g < 0) g = 0;
                        HBRUSH sbr = CreateSolidBrush(RGB(r, g, 255));
                        RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + 2, (int)stars[i].y + 2};
                        FillRect(memDC, &sr, sbr);
                        DeleteObject(sbr);
                    } else {`;
mainC = mainC.replace(starTwinkleBlock, newStarTwinkleBlock);

// Add math.h if not present
if (!mainC.includes('#include <math.h>')) {
    mainC = '#include <math.h>\r\n' + mainC;
}
fs.writeFileSync('c:/KiloApps/KiloApps/KSpace/main.c', mainC, 'utf8');


// ---------------------------------------------------------
// HTML VERSION
// ---------------------------------------------------------

let html = fs.readFileSync('c:/KiloApps/KiloApps/KiloOS/public/apps/kspace.html', 'utf8');

const htmlAddExplosionBlock = `  function addDebrisChunk(x, y, count, color = '#ff9800') {
    for (let i = 0; i < count; i++) {
      debris.push({
        x: x, y: y,
        vx: (randomFloat() - 0.5) * 8, vy: (randomFloat() - 0.5) * 8,
        size: 3 + randomFloat() * 4,
        rot: randomFloat() * Math.PI * 2, vrot: (randomFloat() - 0.5) * 0.5,
        life: 1.0, decay: 0.02 + randomFloat() * 0.02,
        color: color
      });
    }
  }

  function addExplosion(x, y, count, color = '#ff9800') {
    for (let i = 0; i < count; i++) {
      particles.push({
        x: x, y: y,
        vx: (randomFloat() - 0.5) * 10, vy: (randomFloat() - 0.5) * 10,
        life: 20 + randomFloat() * 15, maxLife: 35,
        color: color
      });
    }
    if (count >= 10) {
      addShockwave(x, y, count * 2, color);
      addDebrisChunk(x, y, Math.min(10, count / 2), color);
    }
  }`;

const newHtmlAddExplosionBlock = `  function addDebrisChunk(x, y, count, color = '#ff9800', shape = 0) {
    for (let i = 0; i < count; i++) {
      debris.push({
        x: x, y: y,
        vx: (randomFloat() - 0.5) * 8, vy: (randomFloat() - 0.5) * 8,
        size: 3 + randomFloat() * 4,
        rot: randomFloat() * Math.PI * 2, vrot: (randomFloat() - 0.5) * 0.5,
        life: 1.0, decay: 0.02 + randomFloat() * 0.02,
        color: color, shape: shape
      });
    }
  }

  function addShipExplosion(x, y, count, color = '#ff9800', shape = 0) {
    for (let i = 0; i < count; i++) {
      particles.push({
        x: x, y: y,
        vx: (randomFloat() - 0.5) * 10, vy: (randomFloat() - 0.5) * 10,
        life: 20 + randomFloat() * 15, maxLife: 35,
        color: color, wType: 0
      });
    }
    if (count >= 10) {
      addShockwave(x, y, count * 2, color);
      addDebrisChunk(x, y, Math.min(10, count / 2), color, shape);
    }
  }
  function addExplosion(x, y, count, color = '#ff9800') { addShipExplosion(x, y, count, color, 0); }
  function addWeaponHitParticles(x, y, wType, color) {
    let count = wType === 3 ? 8 : (wType === 1 ? 5 : 3);
    for (let i = 0; i < count; i++) {
      let vx = 0, vy = 0, life = 0;
      if (wType === 1) { // Spread
        vx = (randomFloat() - 0.5) * 18; vy = (randomFloat() - 0.5) * 18;
        life = 10 + randomFloat() * 8;
      } else if (wType === 2) { // Laser
        vx = (randomFloat() - 0.5) * 4; vy = 3 + randomFloat() * 5;
        life = 8 + randomFloat() * 6;
      } else if (wType === 3) { // Plasma
        vx = (randomFloat() - 0.5) * 5; vy = (randomFloat() - 0.5) * 5;
        life = 25 + randomFloat() * 20;
      } else {
        vx = (randomFloat() - 0.5) * 10; vy = (randomFloat() - 0.5) * 10;
        life = 15 + randomFloat() * 10;
      }
      particles.push({ x: x, y: y, vx: vx, vy: vy, life: life, maxLife: life, color: color, wType: wType });
    }
  }`;
html = html.replace(htmlAddExplosionBlock, newHtmlAddExplosionBlock);

// Draw Explosions
const htmlDrawExpBlock = `  function drawExplosions() {
    for (let i = particles.length - 1; i >= 0; i--) {
      let p = particles[i];
      p.x += p.vx; p.y += p.vy; p.life--;
      if (p.life <= 0) { particles.splice(i, 1); continue; }
      ctx.fillStyle = p.color;
      let sz = (p.life > 10) ? 3 : 2;
      ctx.fillRect(p.x, p.y, sz, sz);
    }
  }`;
const newHtmlDrawExpBlock = `  function drawExplosions() {
    for (let i = particles.length - 1; i >= 0; i--) {
      let p = particles[i];
      p.x += p.vx; p.y += p.vy; p.life--;
      if (p.life <= 0) { particles.splice(i, 1); continue; }
      ctx.fillStyle = p.color;
      if (p.wType === 3) {
        ctx.beginPath(); ctx.arc(p.x, p.y, p.life > 12 ? 4 : 2, 0, Math.PI*2); ctx.fill();
      } else if (p.wType === 2) {
        let sz = p.life > 6 ? 4 : 2; ctx.fillRect(p.x, p.y, 1, sz);
      } else if (p.wType === 1) {
        let sz = p.life > 6 ? 3 : 1; ctx.fillRect(p.x, p.y, sz, 1);
      } else {
        let sz = (p.life > 10) ? 3 : 2; ctx.fillRect(p.x, p.y, sz, sz);
      }
    }
  }`;
html = html.replace(htmlDrawExpBlock, newHtmlDrawExpBlock);

// Replace hits
html = html.replace(
  'boss.hp -= (b.type === 1) ? 8 : 2;\n              addExplosion(b.x, b.y, 4, \'#ff1744\');',
  'boss.hp -= (b.type === 1) ? 8 : 2;\n              addWeaponHitParticles(b.x, b.y, weaponType, \'#ff1744\');'
);
html = html.replace(
  'addExplosion(b.x, b.y, 3, \'#00e5ff\'); // Deflector absorbed!',
  'addWeaponHitParticles(b.x, b.y, weaponType, \'#00e5ff\'); // Deflector absorbed!'
);
html = html.replace(
  'boss.hp -= (b.type === 1) ? 8 : 2;\n            addExplosion(b.x, b.y, 3, \'#00e5ff\');',
  'boss.hp -= (b.type === 1) ? 8 : 2;\n            addWeaponHitParticles(b.x, b.y, weaponType, \'#00e5ff\');'
);
html = html.replace(
  'e.hp -= (b.type === 1) ? 6 : 1;\n            addExplosion(b.x, b.y, 3, \'#00e5ff\');',
  'e.hp -= (b.type === 1) ? 6 : 1;\n            addWeaponHitParticles(b.x, b.y, weaponType, \'#00e5ff\');'
);
html = html.replace(
  'if (!e.cloaked) e.hp -= 1;\n            addExplosion(p.x + 10, e.y + e.h / 2, 2, \'#00e5ff\');',
  'if (!e.cloaked) e.hp -= 1;\n            addWeaponHitParticles(p.x + 10, e.y + e.h / 2, weaponType, \'#00e5ff\');'
);

// Replace enemy death
const htmlEnemyDeathBlock = `if (e.hp <= 0) {
          e.active = false;
          comboTimer = 180;
          if (comboMultiplier < 10) comboMultiplier++;
          let baseScore = (e.type === 6 ? 300 : (e.type === 9 ? 200 : (e.type === 8 ? 150 : (e.type === 7 ? 120 : 30))));
          score += baseScore * comboMultiplier;
          enemiesKilled++; totalKills++;
          playSound(SND_EXPLOSION);
          addExplosion(e.x + 10, e.y + 10, e.type === 6 ? 24 : 12);`;
const newHtmlEnemyDeathBlock = `if (e.hp <= 0) {
          e.active = false;
          comboTimer = 180;
          if (comboMultiplier < 10) comboMultiplier++;
          let baseScore = (e.type === 6 ? 300 : (e.type === 9 ? 200 : (e.type === 8 ? 150 : (e.type === 7 ? 120 : 30))));
          score += baseScore * comboMultiplier;
          enemiesKilled++; totalKills++;
          playSound(SND_EXPLOSION);
          let col = '#ff1744';
          if (e.type === 1) col = '#ff9100'; else if (e.type === 2) col = '#d500f9';
          else if (e.type === 3) col = '#78909c'; else if (e.type === 4) col = '#00e5ff';
          else if (e.type === 5 || e.type === 9) col = '#8d6e63'; else if (e.type === 6) col = '#c62828';
          else if (e.type === 7) col = '#ffeb3b'; else if (e.type === 8) col = '#673ab7';
          addShipExplosion(e.x + 10, e.y + 10, e.type === 6 ? 24 : 12, col, e.type);`;
html = html.replace(htmlEnemyDeathBlock, newHtmlEnemyDeathBlock);

// Draw debris
const htmlDrawDebrisBlock = `      // Debris Chunks
      for (let i = debris.length - 1; i >= 0; i--) {
        let d = debris[i];
        d.x += d.vx; d.y += d.vy; d.rot += d.vrot; d.life -= d.decay;
        if (d.life <= 0) { debris.splice(i, 1); continue; }
        ctx.save();
        ctx.translate(d.x, d.y); ctx.rotate(d.rot);
        ctx.globalAlpha = Math.max(0, d.life);
        ctx.fillStyle = d.color;
        ctx.fillRect(-d.size / 2, -d.size / 2, d.size, d.size * 0.6);
        ctx.restore();
      }`;
const newHtmlDrawDebrisBlock = `      // Debris Chunks
      for (let i = debris.length - 1; i >= 0; i--) {
        let d = debris[i];
        d.x += d.vx; d.y += d.vy; d.rot += d.vrot; d.life -= d.decay;
        if (d.life <= 0) { debris.splice(i, 1); continue; }
        ctx.save();
        ctx.translate(d.x, d.y); ctx.rotate(d.rot);
        ctx.globalAlpha = Math.max(0, d.life);
        ctx.fillStyle = d.color;
        if (d.shape === 2 || d.shape === 5) {
          ctx.beginPath(); ctx.arc(0, 0, d.size/2, 0, Math.PI*2); ctx.fill();
        } else if (d.shape === 4 || d.shape === 1) {
          ctx.beginPath(); ctx.moveTo(0, -d.size/2); ctx.lineTo(d.size/2, d.size/2); ctx.lineTo(-d.size/2, d.size/2); ctx.fill();
        } else {
          ctx.fillRect(-d.size / 2, -d.size / 2, d.size, d.size * 0.6);
        }
        ctx.restore();
      }`;
html = html.replace(htmlDrawDebrisBlock, newHtmlDrawDebrisBlock);

// Nebulae rendering
const htmlNebulaeDrawBlock = `      let pr = n.r + Math.sin(n.phase) * 12;
      let g = ctx.createRadialGradient(n.x, n.y, 5, n.x, n.y, pr);
      g.addColorStop(0, n.col1);
      g.addColorStop(1, n.col2);
      ctx.fillStyle = g;
      ctx.beginPath(); ctx.arc(n.x, n.y, pr, 0, Math.PI * 2); ctx.fill();`;
const newHtmlNebulaeDrawBlock = `      let pr = n.r + Math.sin(n.phase) * 12;
      let g = ctx.createRadialGradient(n.x, n.y, 5, n.x, n.y, pr);
      g.addColorStop(0, n.col1);
      g.addColorStop(1, n.col2);
      ctx.globalCompositeOperation = 'screen';
      ctx.fillStyle = g;
      ctx.beginPath(); ctx.arc(n.x, n.y, pr, 0, Math.PI * 2); ctx.fill();
      
      let g2 = ctx.createRadialGradient(n.x, n.y, 0, n.x, n.y, pr * 0.5);
      g2.addColorStop(0, n.col2); g2.addColorStop(1, 'transparent');
      ctx.fillStyle = g2;
      ctx.beginPath(); ctx.arc(n.x, n.y, pr * 0.5, 0, Math.PI * 2); ctx.fill();
      ctx.globalCompositeOperation = 'source-over';`;
html = html.replace(htmlNebulaeDrawBlock, newHtmlNebulaeDrawBlock);


fs.writeFileSync('c:/KiloApps/KiloApps/KiloOS/public/apps/kspace.html', html, 'utf8');
console.log('PATCH COMPLETE');
