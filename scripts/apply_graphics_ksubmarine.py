"""
Applies graphics overhaul to KiloOS/public/apps/ksubmarine.html:
- Integrates Imagen 3 & 2-stage chroma-keyed sprites (Bathysphere submarine, Leviathan, Squid, Benthic Flora, Abyssal Fauna, Salvage Pod, Combat Drone, Hydrothermal Smoker)
- Upgrades getFaunaVisualSvg() for high-fidelity sprite rendering in Codex UI
- Upgrades drawSonarView and drawNavMapChart hydrothermal vent markers
- Preserves zero perimeter border glints and zero console errors
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML_PATH = ROOT / "KiloOS" / "public" / "apps" / "ksubmarine.html"

content = HTML_PATH.read_text(encoding="utf-8")

# 1. Assets definition
target_bubbles = "            // Cavitation Micro-Bubble Particle Pool\r\n            const SUB_BUBBLES = [];"
if target_bubbles not in content:
    target_bubbles = "            // Cavitation Micro-Bubble Particle Pool\n            const SUB_BUBBLES = [];"

asset_def = """            // --- KSUBMARINE SPRITE ASSET REGISTRY (PHASE VI ARGON REVIEW) ---
            const KSUBMARINE_ASSETS = {
                sub_bathysphere: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/sub_bathysphere.png', fallback: 'assets/sprites/ksubmarine/sub_bathysphere.png' },
                leviathan_trench: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/leviathan_trench.png', fallback: 'assets/sprites/ksubmarine/leviathan_trench.png' },
                squid_abyssal: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/squid_abyssal.png', fallback: 'assets/sprites/ksubmarine/squid_abyssal.png' },
                flora_benthic: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/flora_benthic.png', fallback: 'assets/sprites/ksubmarine/flora_benthic.png' },
                fauna_abyssal: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/fauna_abyssal.png', fallback: 'assets/sprites/ksubmarine/fauna_abyssal.png' },
                salvage_pod: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/salvage_pod.png', fallback: 'assets/sprites/ksubmarine/salvage_pod.png' },
                combat_drone: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/combat_drone.png', fallback: 'assets/sprites/ksubmarine/combat_drone.png' },
                vent_smoker: { img: null, loaded: false, src: '../assets/sprites/ksubmarine/vent_smoker.png', fallback: 'assets/sprites/ksubmarine/vent_smoker.png' }
            };

            function initKSubmarineAssets() {
                Object.values(KSUBMARINE_ASSETS).forEach(asset => {
                    const img = new Image();
                    img.onload = () => { asset.loaded = true; };
                    img.onerror = () => {
                        if (asset.fallback && img.src !== asset.fallback) {
                            img.src = asset.fallback;
                        }
                    };
                    img.src = asset.src;
                    asset.img = img;
                });
            }
            initKSubmarineAssets();

            // Cavitation Micro-Bubble Particle Pool
            const SUB_BUBBLES = [];"""

assert target_bubbles in content, "Could not find SUB_BUBBLES in content"
content = content.replace(target_bubbles, asset_def, 1)

# 2. getFaunaVisualSvg upgrade
fauna_svg_start = "            // --- FAUNA CODEX SYSTEM (PHASE 7) ---"
fauna_svg_end = "            function renderCodexUI() {"
idx_start = content.find(fauna_svg_start)
idx_end = content.find(fauna_svg_end)
assert idx_start != -1 and idx_end != -1, "Could not find getFaunaVisualSvg block"

new_fauna_svg = """            // --- FAUNA CODEX SYSTEM (PHASE 7) ---
            function getFaunaVisualSvg(type, discovered) {
                const opacity = discovered ? '1' : '0.25';
                const filterStyle = discovered ? '' : 'filter: grayscale(1);';
                const spriteMap = {
                    leviathan: '../assets/sprites/ksubmarine/leviathan_trench.png',
                    squid: '../assets/sprites/ksubmarine/squid_abyssal.png',
                    flora: '../assets/sprites/ksubmarine/flora_benthic.png',
                    trench: '../assets/sprites/ksubmarine/vent_smoker.png'
                };
                const spriteSrc = spriteMap[type] || '../assets/sprites/ksubmarine/fauna_abyssal.png';
                if (discovered) {
                    return `<div style="width: 100%; height: 28px; display: flex; align-items: center; justify-content: center; background: rgba(2, 11, 18, 0.7); border-radius: 2px; margin-top: 2px; opacity: ${opacity}; ${filterStyle}">
                        <img src="${spriteSrc}" style="max-height: 26px; max-width: 90px; object-fit: contain; image-rendering: pixelated;" alt="${type}" onerror="this.style.display='none'" />
                    </div>`;
                }
                return `<div style="width: 100%; height: 28px; display: flex; align-items: center; justify-content: center; background: rgba(2, 11, 18, 0.7); border-radius: 2px; margin-top: 2px; opacity: 0.25; filter: grayscale(1) brightness(0.4);">
                    <img src="${spriteSrc}" style="max-height: 24px; max-width: 80px; object-fit: contain; image-rendering: pixelated;" alt="${type}" onerror="this.style.display='none'" />
                </div>`;
            }

"""
content = content[:idx_start] + new_fauna_svg + content[idx_end:]

# 3. drawSubmarineSprite sprite rendering
sub_target = """                // Cavitation micro-bubbles from stern propeller
                if (Math.abs(speed) > 0.1 || isPinging) {
                    if (Math.random() < 0.4) {
                        const bx = cx - Math.cos(headingRad) * (17 * dpr) + (Math.random() - 0.5) * 4 * dpr;
                        const by = cy - Math.sin(headingRad) * (17 * dpr) + (Math.random() - 0.5) * 4 * dpr;
                        addSubBubble(bx, by, -Math.cos(headingRad) * (speed * 0.4 + 0.2), -Math.sin(headingRad) * (speed * 0.4 + 0.2), 0.7 + Math.random() * 0.5, (1.2 + Math.random() * 1.5) * dpr);
                    }
                }"""
if sub_target not in content:
    sub_target = sub_target.replace("\n", "\r\n")

sub_replacement = """                // Cavitation micro-bubbles from stern propeller
                if (Math.abs(speed) > 0.1 || isPinging) {
                    if (Math.random() < 0.4) {
                        const bx = cx - Math.cos(headingRad) * (17 * dpr) + (Math.random() - 0.5) * 4 * dpr;
                        const by = cy - Math.sin(headingRad) * (17 * dpr) + (Math.random() - 0.5) * 4 * dpr;
                        addSubBubble(bx, by, -Math.cos(headingRad) * (speed * 0.4 + 0.2), -Math.sin(headingRad) * (speed * 0.4 + 0.2), 0.7 + Math.random() * 0.5, (1.2 + Math.random() * 1.5) * dpr);
                    }
                }

                // Imagen 3 Bathysphere Sprite Integration
                if (KSUBMARINE_ASSETS.sub_bathysphere && KSUBMARINE_ASSETS.sub_bathysphere.loaded) {
                    const sSize = 36 * dpr;
                    // Draw bathysphere sprite centered (oriented along heading)
                    ctx.drawImage(KSUBMARINE_ASSETS.sub_bathysphere.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    if (isPinging) {
                        ctx.fillStyle = 'rgba(0, 240, 255, 0.22)';
                        ctx.beginPath();
                        ctx.moveTo(10 * dpr, 0);
                        ctx.arc(10 * dpr, 0, 24 * dpr, -0.32, 0.32);
                        ctx.closePath();
                        ctx.fill();
                    }
                    ctx.restore();
                    return;
                }"""

assert sub_target in content, "Could not find sub_target in content"
content = content.replace(sub_target, sub_replacement, 1)

# 4. drawLeviathanCreature sprite rendering
lev_target = """                // Ambient Bioluminescent Radial Aura Haze
                if (isSwept || isSelected) {
                    const auraRad = (24 + Math.sin(time * 3) * 3) * dpr;
                    const auraGrad = ctx.createRadialGradient(0, 0, 2 * dpr, 0, 0, auraRad);
                    auraGrad.addColorStop(0, 'rgba(244, 63, 94, 0.32)');
                    auraGrad.addColorStop(0.5, 'rgba(168, 85, 247, 0.15)');
                    auraGrad.addColorStop(1, 'rgba(0, 240, 255, 0)');
                    ctx.fillStyle = auraGrad;
                    ctx.beginPath();
                    ctx.arc(0, 0, auraRad, 0, Math.PI * 2);
                    ctx.fill();

                    ctx.shadowColor = glowColor;
                    ctx.shadowBlur = 12 * dpr;
                }"""
if lev_target not in content:
    lev_target = lev_target.replace("\n", "\r\n")

lev_replacement = """                // Ambient Bioluminescent Radial Aura Haze
                if (isSwept || isSelected) {
                    const auraRad = (28 + Math.sin(time * 3) * 4) * dpr;
                    const auraGrad = ctx.createRadialGradient(0, 0, 2 * dpr, 0, 0, auraRad);
                    auraGrad.addColorStop(0, 'rgba(244, 63, 94, 0.35)');
                    auraGrad.addColorStop(0.5, 'rgba(168, 85, 247, 0.18)');
                    auraGrad.addColorStop(1, 'rgba(0, 240, 255, 0)');
                    ctx.fillStyle = auraGrad;
                    ctx.beginPath();
                    ctx.arc(0, 0, auraRad, 0, Math.PI * 2);
                    ctx.fill();

                    ctx.shadowColor = glowColor;
                    ctx.shadowBlur = 12 * dpr;
                }

                // Imagen 3 Leviathan Serpent Sprite Integration
                if (KSUBMARINE_ASSETS.leviathan_trench && KSUBMARINE_ASSETS.leviathan_trench.loaded) {
                    const sSize = (isSelected ? 52 : 46) * dpr;
                    const sway = Math.sin(time * 2.8) * 0.08;
                    ctx.rotate(sway);
                    ctx.drawImage(KSUBMARINE_ASSETS.leviathan_trench.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    ctx.restore();
                    return;
                }"""

assert lev_target in content, "Could not find lev_target in content"
content = content.replace(lev_target, lev_replacement, 1)

# 5. drawSquidCreature sprite rendering
squid_target = """                if (isSwept || isSelected) {
                    ctx.shadowColor = '#f472b6';
                    ctx.shadowBlur = 8 * dpr;
                }"""
if squid_target not in content:
    squid_target = squid_target.replace("\n", "\r\n")

squid_replacement = """                if (isSwept || isSelected) {
                    ctx.shadowColor = '#f472b6';
                    ctx.shadowBlur = 8 * dpr;
                }

                // Imagen 3 Abyssal Squid Sprite Integration
                if (KSUBMARINE_ASSETS.squid_abyssal && KSUBMARINE_ASSETS.squid_abyssal.loaded) {
                    const sSize = (isSelected ? 42 : 36) * dpr;
                    const sway = Math.sin(time * 3.2) * 0.06;
                    ctx.rotate(sway);
                    ctx.drawImage(KSUBMARINE_ASSETS.squid_abyssal.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    ctx.restore();
                    return;
                }"""

assert squid_target in content, "Could not find squid_target in content"
content = content.replace(squid_target, squid_replacement, 1)

# 6. drawFloraCreature sprite rendering
flora_target = """            // 4. Bioluminescent Siphonophore / Flora Sprite
            function drawFloraCreature(ctx, cX, cY, dpr, isSwept, isSelected, time) {
                ctx.save();
                ctx.translate(cX, cY);

                const baseColor = isSwept || isSelected ? '#34d399' : 'rgba(52, 211, 153, 0.35)';
                ctx.fillStyle = baseColor;
                if (isSwept || isSelected) {
                    ctx.shadowColor = '#34d399';
                    ctx.shadowBlur = 8 * dpr;
                }"""
if flora_target not in content:
    flora_target = flora_target.replace("\n", "\r\n")

flora_replacement = """            // 4. Bioluminescent Siphonophore / Flora Sprite
            function drawFloraCreature(ctx, cX, cY, dpr, isSwept, isSelected, time) {
                ctx.save();
                ctx.translate(cX, cY);

                const baseColor = isSwept || isSelected ? '#34d399' : 'rgba(52, 211, 153, 0.35)';
                ctx.fillStyle = baseColor;
                if (isSwept || isSelected) {
                    ctx.shadowColor = '#34d399';
                    ctx.shadowBlur = 8 * dpr;
                }

                // Benthic Vent Flora Sprite Integration
                if (KSUBMARINE_ASSETS.flora_benthic && KSUBMARINE_ASSETS.flora_benthic.loaded) {
                    const sSize = (isSelected ? 36 : 30) * dpr;
                    const sway = Math.sin(time * 2.2) * 0.05;
                    ctx.rotate(sway);
                    ctx.drawImage(KSUBMARINE_ASSETS.flora_benthic.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    ctx.restore();
                    return;
                }"""

assert flora_target in content, "Could not find flora_target in content"
content = content.replace(flora_target, flora_replacement, 1)

# 7. drawFaunaCreature sprite rendering
fauna_target = """            // 5. Abyssal Anglerfish / Deep-Sea Fauna Sprite
            function drawFaunaCreature(ctx, cX, cY, dpr, isSwept, isSelected, time) {
                ctx.save();
                ctx.translate(cX, cY);

                const baseColor = isSwept || isSelected ? '#38bdf8' : 'rgba(56, 189, 248, 0.35)';
                ctx.fillStyle = baseColor;
                if (isSwept || isSelected) {
                    ctx.shadowColor = '#38bdf8';
                    ctx.shadowBlur = 8 * dpr;
                }"""
if fauna_target not in content:
    fauna_target = fauna_target.replace("\n", "\r\n")

fauna_replacement = """            // 5. Abyssal Anglerfish / Deep-Sea Fauna Sprite
            function drawFaunaCreature(ctx, cX, cY, dpr, isSwept, isSelected, time) {
                ctx.save();
                ctx.translate(cX, cY);

                const baseColor = isSwept || isSelected ? '#38bdf8' : 'rgba(56, 189, 248, 0.35)';
                ctx.fillStyle = baseColor;
                if (isSwept || isSelected) {
                    ctx.shadowColor = '#38bdf8';
                    ctx.shadowBlur = 8 * dpr;
                }

                // Abyssal Anglerfish Fauna Sprite Integration
                if (KSUBMARINE_ASSETS.fauna_abyssal && KSUBMARINE_ASSETS.fauna_abyssal.loaded) {
                    const sSize = (isSelected ? 34 : 28) * dpr;
                    const wag = Math.sin(time * 4.5) * 0.06;
                    ctx.rotate(wag);
                    ctx.drawImage(KSUBMARINE_ASSETS.fauna_abyssal.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    ctx.restore();
                    return;
                }"""

assert fauna_target in content, "Could not find fauna_target in content"
content = content.replace(fauna_target, fauna_replacement, 1)

# 8. drawSalvageSprite sprite rendering
salv_target = """            // 6. Sunken Titanic Salvage Module Sprite
            function drawSalvageSprite(ctx, cX, cY, dpr, isSwept, isSelected, time, harvested) {
                ctx.save();
                ctx.translate(cX, cY);"""
if salv_target not in content:
    salv_target = salv_target.replace("\n", "\r\n")

salv_replacement = """            // 6. Sunken Titanic Salvage Module Sprite
            function drawSalvageSprite(ctx, cX, cY, dpr, isSwept, isSelected, time, harvested) {
                ctx.save();
                ctx.translate(cX, cY);

                // Titanium Salvage Pod Sprite Integration
                if (KSUBMARINE_ASSETS.salvage_pod && KSUBMARINE_ASSETS.salvage_pod.loaded) {
                    if ((isSwept || isSelected) && !harvested) {
                        ctx.shadowColor = '#fbbf24';
                        ctx.shadowBlur = 9 * dpr;
                    }
                    if (harvested) ctx.globalAlpha = 0.4;
                    const sSize = (isSelected ? 32 : 26) * dpr;
                    ctx.drawImage(KSUBMARINE_ASSETS.salvage_pod.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    if (!harvested) {
                        const beaconOn = ((time * 2.8) % 1.0) > 0.45;
                        ctx.fillStyle = beaconOn ? '#ffffff' : '#b45309';
                        ctx.shadowColor = '#f59e0b';
                        ctx.shadowBlur = beaconOn ? 8 * dpr : 0;
                        ctx.beginPath();
                        ctx.arc(0, -sSize * 0.42, 1.4 * dpr, 0, Math.PI * 2);
                        ctx.fill();
                    }
                    ctx.restore();
                    return;
                }"""

assert salv_target in content, "Could not find salv_target in content"
content = content.replace(salv_target, salv_replacement, 1)

# 9. drawCombatDroneSprite sprite rendering
drone_target = """            // 7. Hostile Autonomous Combat Drone Sprite
            function drawCombatDroneSprite(ctx, tX, tY, dpr, thr, isThreatLocked, time) {
                ctx.save();
                ctx.translate(tX, tY);"""
if drone_target not in content:
    drone_target = drone_target.replace("\n", "\r\n")

drone_replacement = """            // 7. Hostile Autonomous Combat Drone Sprite
            function drawCombatDroneSprite(ctx, tX, tY, dpr, thr, isThreatLocked, time) {
                ctx.save();
                ctx.translate(tX, tY);

                // Autonomous Combat Drone Sprite Integration
                if (KSUBMARINE_ASSETS.combat_drone && KSUBMARINE_ASSETS.combat_drone.loaded) {
                    const aggroColor = thr.state === 'attacking' ? '#ef4444' : (thr.state === 'stunned' ? '#00f0ff' : '#f59e0b');
                    ctx.shadowColor = aggroColor;
                    ctx.shadowBlur = (isThreatLocked ? 10 : 5) * dpr;
                    const sSize = (isThreatLocked ? 36 : 30) * dpr;
                    ctx.drawImage(KSUBMARINE_ASSETS.combat_drone.img, -sSize * 0.5, -sSize * 0.5, sSize, sSize);
                    ctx.restore();
                    return;
                }"""

assert drone_target in content, "Could not find drone_target in content"
content = content.replace(drone_target, drone_replacement, 1)

# 10. Hydrothermal vent black smoker in drawSonarView
vent_sonar_target = """                    // Basalt Chimney Spire Silhouette (Seabed Black Smoker Tower)
                    ctx.fillStyle = '#0f172a';
                    ctx.strokeStyle = '#334155';
                    ctx.lineWidth = 1 * dpr;
                    ctx.beginPath();
                    ctx.moveTo(vx - 7 * dpr, vy + 14 * dpr);
                    ctx.lineTo(vx - 4 * dpr, vy + 4 * dpr);
                    ctx.lineTo(vx - 2 * dpr, vy - 2 * dpr);
                    ctx.lineTo(vx + 2 * dpr, vy - 2 * dpr);
                    ctx.lineTo(vx + 4 * dpr, vy + 5 * dpr);
                    ctx.lineTo(vx + 8 * dpr, vy + 14 * dpr);
                    ctx.closePath();
                    ctx.fill();
                    ctx.stroke();"""
if vent_sonar_target not in content:
    vent_sonar_target = vent_sonar_target.replace("\n", "\r\n")

vent_sonar_replacement = """                    // Basalt Chimney Spire Silhouette (Seabed Black Smoker Tower)
                    if (KSUBMARINE_ASSETS.vent_smoker && KSUBMARINE_ASSETS.vent_smoker.loaded) {
                        const vSize = 38 * dpr;
                        ctx.drawImage(KSUBMARINE_ASSETS.vent_smoker.img, vx - vSize * 0.5, vy - vSize * 0.35, vSize, vSize);
                    } else {
                        ctx.fillStyle = '#0f172a';
                        ctx.strokeStyle = '#334155';
                        ctx.lineWidth = 1 * dpr;
                        ctx.beginPath();
                        ctx.moveTo(vx - 7 * dpr, vy + 14 * dpr);
                        ctx.lineTo(vx - 4 * dpr, vy + 4 * dpr);
                        ctx.lineTo(vx - 2 * dpr, vy - 2 * dpr);
                        ctx.lineTo(vx + 2 * dpr, vy - 2 * dpr);
                        ctx.lineTo(vx + 4 * dpr, vy + 5 * dpr);
                        ctx.lineTo(vx + 8 * dpr, vy + 14 * dpr);
                        ctx.closePath();
                        ctx.fill();
                        ctx.stroke();
                    }"""

assert vent_sonar_target in content, "Could not find vent_sonar_target in content"
content = content.replace(vent_sonar_target, vent_sonar_replacement, 1)

# 11. Hydrothermal vent in drawNavMapChart
vent_nav_target = """                // Vent Crag Basalt Chimney & Convective Smoke Marker
                ctx.save();
                ctx.fillStyle = '#0f172a';
                ctx.strokeStyle = '#ef4444';
                ctx.lineWidth = 1 * dpr;
                ctx.beginPath();
                ctx.moveTo(ventX - 6 * dpr, ventY + 8 * dpr);
                ctx.lineTo(ventX - 3 * dpr, ventY + 2 * dpr);
                ctx.lineTo(ventX - 2 * dpr, ventY - 4 * dpr);
                ctx.lineTo(ventX + 2 * dpr, ventY - 4 * dpr);
                ctx.lineTo(ventX + 3 * dpr, ventY + 3 * dpr);
                ctx.lineTo(ventX + 6 * dpr, ventY + 8 * dpr);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();"""
if vent_nav_target not in content:
    vent_nav_target = vent_nav_target.replace("\n", "\r\n")

vent_nav_replacement = """                // Vent Crag Basalt Chimney & Convective Smoke Marker
                ctx.save();
                if (KSUBMARINE_ASSETS.vent_smoker && KSUBMARINE_ASSETS.vent_smoker.loaded) {
                    const mvSize = 24 * dpr;
                    ctx.drawImage(KSUBMARINE_ASSETS.vent_smoker.img, ventX - mvSize * 0.5, ventY - mvSize * 0.35, mvSize, mvSize);
                } else {
                    ctx.fillStyle = '#0f172a';
                    ctx.strokeStyle = '#ef4444';
                    ctx.lineWidth = 1 * dpr;
                    ctx.beginPath();
                    ctx.moveTo(ventX - 6 * dpr, ventY + 8 * dpr);
                    ctx.lineTo(ventX - 3 * dpr, ventY + 2 * dpr);
                    ctx.lineTo(ventX - 2 * dpr, ventY - 4 * dpr);
                    ctx.lineTo(ventX + 2 * dpr, ventY - 4 * dpr);
                    ctx.lineTo(ventX + 3 * dpr, ventY + 3 * dpr);
                    ctx.lineTo(ventX + 6 * dpr, ventY + 8 * dpr);
                    ctx.closePath();
                    ctx.fill();
                    ctx.stroke();
                }"""

assert vent_nav_target in content, "Could not find vent_nav_target in content"
content = content.replace(vent_nav_target, vent_nav_replacement, 1)

HTML_PATH.write_text(content, encoding="utf-8")
print("Successfully applied graphics overhaul to ksubmarine.html")
