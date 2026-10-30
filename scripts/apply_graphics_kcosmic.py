import os
import re

def update_kcosmic_html():
    path = "KiloOS/public/apps/kcosmic.html"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Glint Sweep: replace rotating precursor glyph runes with stationary resonant crystal nodes
    glint_target = """                                    // Rotating precursor glyph runes orbiting the spire
                                    const glyphAng = sim.time * 1.5 + sIdx * 1.57;
                                    const gx = spireTipX + Math.cos(glyphAng) * (7 * z);
                                    const gy = spireTipY + Math.sin(glyphAng) * (7 * z);
                                    ctx.fillStyle = '#a855f7';
                                    ctx.fillRect(gx - z, gy - z, 2 * z, 2 * z);"""
    glint_replacement = """                                    // Ancient precursor monolithic glyph facets (stationary resonant crystal nodes)
                                    [-0.7, 0.7].forEach(angOffset => {
                                        const gx = spireTipX + Math.cos(sAng + angOffset) * (6 * z);
                                        const gy = spireTipY + Math.sin(sAng + angOffset) * (6 * z);
                                        ctx.fillStyle = '#a855f7';
                                        ctx.fillRect(gx - z, gy - z, 2 * z, 2 * z);
                                    });"""
    if glint_target in content:
        content = content.replace(glint_target, glint_replacement, 1)
        print("Replaced glint in kcosmic.html")
    else:
        print("Warning: glint_target not found in kcosmic.html")

    # 2. Add Hyperion Gate trade route in sim.tradeRoutes
    trade_needle = "speed: 0.008,\n                    color: '#10b981'\n                },"
    trade_add = """speed: 0.008,
                    color: '#10b981'
                },
                {
                    id: 'interstellar',
                    name: 'Hyperion Gate Trade Corridor',
                    from: 'Zephyr Station',
                    to: 'Hyperion Gate',
                    cargo: 'commerce',
                    active: true,
                    freighters: 1,
                    baseYield: 80,
                    costMin: 180,
                    costEnergy: 140,
                    progress: 0.1,
                    speed: 0.005,
                    color: '#a855f7'
                },"""
    if trade_needle in content:
        content = content.replace(trade_needle, trade_add, 1)
        print("Added trade route in kcosmic.html")

    # 3. Add Jump Gate to celestialBodies in loadStarSystem
    jumpgate_needle = "if (p.hasStation) {\n                    celestialBodies.push({\n                        id: 'zephyr',\n                        name: 'Zephyr Station',\n                        type: 'Orbital Shipyard',\n                        parent: p.id,\n                        orbitRadius: 65,\n                        orbitSpeed: -0.18,\n                        angle: 3.0,\n                        radius: 7,\n                        color: '#10b981',\n                        isStation: true\n                    });\n                }\n            });"
    jumpgate_add = """if (p.hasStation) {
                    celestialBodies.push({
                        id: 'zephyr',
                        name: 'Zephyr Station',
                        type: 'Orbital Shipyard',
                        parent: p.id,
                        orbitRadius: 65,
                        orbitSpeed: -0.18,
                        angle: 3.0,
                        radius: 7,
                        color: '#10b981',
                        isStation: true
                    });
                }
            });

            // Hyperion Jump Gate (Interstellar Transit Corridor at 460 AU)
            celestialBodies.push({
                id: 'jumpgate',
                name: 'Hyperion Jump Gate',
                type: 'Sub-Space Warp Conduit',
                orbitRadius: 460,
                orbitSpeed: 0.04,
                angle: -1.75,
                radius: 18,
                color: '#00f0ff',
                isJumpGate: true
            });"""
    if jumpgate_needle in content:
        content = content.replace(jumpgate_needle, jumpgate_add, 1)
        print("Added Jump Gate to celestialBodies in kcosmic.html")

    # 4. Add drawCosmicNebulae, drawJumpGateSprite, drawJumpGateWarpFX and triggerJumpGateWarp
    helpers_code = """        // --- Procedural Cosmic Anomaly Nebulae (Multi-Stop Deep Space Gas Clouds) ---
        function drawCosmicNebulae(ctx, cx, cy, z, theme, time, sysIndex) {
            ctx.save();
            const sunObj = celestialBodies[0] || { x: -300, y: 0 };
            const sX = cx + sunObj.x * z;
            const sY = cy + sunObj.y * z;

            let nebulae = [];
            if (sysIndex === 0) {
                nebulae = [
                    {
                        name: 'CYGNUS ION VEIL',
                        code: 'ANOMALY-01',
                        x: sX + 320 * z,
                        y: sY - 260 * z,
                        rx: 340 * z,
                        ry: 210 * z,
                        rot: -0.32,
                        c1: 'rgba(168, 85, 247, 0.22)',
                        c2: 'rgba(56, 189, 248, 0.14)',
                        c3: 'rgba(0, 240, 255, 0.05)',
                        tagColor: '#38bdf8'
                    },
                    {
                        name: 'TACHYON RIFT SHEAR',
                        code: 'ANOMALY-02',
                        x: sX - 220 * z,
                        y: sY + 310 * z,
                        rx: 290 * z,
                        ry: 180 * z,
                        rot: 0.45,
                        c1: 'rgba(245, 158, 11, 0.20)',
                        c2: 'rgba(244, 63, 94, 0.12)',
                        c3: 'rgba(168, 85, 247, 0.04)',
                        tagColor: '#f59e0b'
                    }
                ];
            } else if (sysIndex === 1) {
                nebulae = [
                    {
                        name: 'HELIOS CORONA PLUME',
                        code: 'ANOMALY-01',
                        x: sX + 280 * z,
                        y: sY + 220 * z,
                        rx: 320 * z,
                        ry: 200 * z,
                        rot: 0.25,
                        c1: 'rgba(244, 63, 94, 0.24)',
                        c2: 'rgba(245, 158, 11, 0.15)',
                        c3: 'rgba(251, 146, 60, 0.05)',
                        tagColor: '#f43f5e'
                    },
                    {
                        name: 'TARTARUS EMISSION DUST',
                        code: 'ANOMALY-02',
                        x: sX - 300 * z,
                        y: sY - 220 * z,
                        rx: 260 * z,
                        ry: 170 * z,
                        rot: -0.5,
                        c1: 'rgba(129, 140, 248, 0.18)',
                        c2: 'rgba(99, 102, 241, 0.10)',
                        c3: 'rgba(14, 165, 233, 0.03)',
                        tagColor: '#818cf8'
                    }
                ];
            } else {
                nebulae = [
                    {
                        name: 'PERSEUS EMERALD CLOUD',
                        code: 'ANOMALY-01',
                        x: sX + 310 * z,
                        y: sY - 280 * z,
                        rx: 310 * z,
                        ry: 220 * z,
                        rot: -0.2,
                        c1: 'rgba(16, 185, 129, 0.20)',
                        c2: 'rgba(0, 240, 255, 0.12)',
                        c3: 'rgba(52, 211, 153, 0.04)',
                        tagColor: '#10b981'
                    },
                    {
                        name: 'ASTROMETRIC VOID EXPANSION',
                        code: 'ANOMALY-02',
                        x: sX - 250 * z,
                        y: sY + 260 * z,
                        rx: 280 * z,
                        ry: 190 * z,
                        rot: 0.35,
                        c1: 'rgba(168, 85, 247, 0.18)',
                        c2: 'rgba(244, 63, 94, 0.10)',
                        c3: 'rgba(30, 58, 138, 0.04)',
                        tagColor: '#a855f7'
                    }
                ];
            }

            nebulae.forEach((neb, nIdx) => {
                const pulse = 1.0 + Math.sin(time * 0.4 + nIdx * 1.5) * 0.05;
                ctx.save();
                ctx.translate(neb.x, neb.y);
                ctx.rotate(neb.rot);

                const gradOuter = ctx.createRadialGradient(0, 0, neb.ry * 0.2 * pulse, 0, 0, neb.rx * pulse);
                gradOuter.addColorStop(0, neb.c1);
                gradOuter.addColorStop(0.5, neb.c2);
                gradOuter.addColorStop(0.85, neb.c3);
                gradOuter.addColorStop(1, 'rgba(0, 0, 0, 0)');
                ctx.fillStyle = gradOuter;
                ctx.beginPath();
                ctx.ellipse(0, 0, neb.rx * pulse, neb.ry * pulse, 0, 0, Math.PI * 2);
                ctx.fill();

                const gradCore = ctx.createRadialGradient(-neb.rx * 0.15, -neb.ry * 0.1, 5 * z, 0, 0, neb.rx * 0.55 * pulse);
                gradCore.addColorStop(0, neb.c1);
                gradCore.addColorStop(0.6, neb.c2);
                gradCore.addColorStop(1, 'rgba(0, 0, 0, 0)');
                ctx.fillStyle = gradCore;
                ctx.beginPath();
                ctx.ellipse(-neb.rx * 0.1, 0, neb.rx * 0.6 * pulse, neb.ry * 0.45 * pulse, 0.2, 0, Math.PI * 2);
                ctx.fill();

                ctx.fillStyle = 'rgba(5, 8, 17, 0.45)';
                ctx.beginPath();
                ctx.ellipse(neb.rx * 0.12, neb.ry * 0.08, neb.rx * 0.35, neb.ry * 0.18, -0.4, 0, Math.PI * 2);
                ctx.fill();

                ctx.restore();

                if (sim.showGrid) {
                    ctx.save();
                    ctx.strokeStyle = neb.tagColor;
                    ctx.lineWidth = 1;
                    ctx.setLineDash([2, 4]);
                    const retX = neb.x - 30 * z;
                    const retY = neb.y - 20 * z;
                    ctx.strokeRect(retX - 8, retY - 8, 16, 16);
                    ctx.setLineDash([]);
                    ctx.font = `${Math.max(8, 8.5 * z)}px monospace`;
                    ctx.fillStyle = neb.tagColor;
                    ctx.textAlign = 'left';
                    ctx.fillText(`[${neb.code} // ${neb.name}]`, retX + 12, retY + 3);
                    ctx.restore();
                }
            });

            ctx.restore();
        }

        // --- Hyperion Jump Gate Sprite & Aperture Shader ---
        function drawJumpGateSprite(ctx, gx, gy, gr, time, z, isWarping) {
            const rad = Math.max(14, gr);
            ctx.save();
            ctx.translate(gx, gy);

            ctx.strokeStyle = '#0284c7';
            ctx.lineWidth = Math.max(1.5, 2 * z);
            ctx.fillStyle = '#0a192f';
            ctx.beginPath();
            for (let i = 0; i < 6; i++) {
                const a = (i * Math.PI / 3);
                const vx = Math.cos(a) * rad;
                const vy = Math.sin(a) * (rad * 0.85);
                if (i === 0) ctx.moveTo(vx, vy);
                else ctx.lineTo(vx, vy);
            }
            ctx.closePath();
            ctx.fill();
            ctx.stroke();

            ctx.fillStyle = '#1e293b';
            ctx.fillRect(-rad * 1.3, -4 * z, rad * 0.35, 8 * z);
            ctx.fillRect(rad * 0.95, -4 * z, rad * 0.35, 8 * z);
            ctx.strokeStyle = '#f59e0b';
            ctx.lineWidth = 1;
            ctx.strokeRect(-rad * 1.3, -4 * z, rad * 0.35, 8 * z);
            ctx.strokeRect(rad * 0.95, -4 * z, rad * 0.35, 8 * z);

            for (let i = 0; i < 6; i++) {
                const a = (i * Math.PI / 3);
                const tx = Math.cos(a) * (rad * 0.72);
                const ty = Math.sin(a) * (rad * 0.6);
                ctx.fillStyle = isWarping ? '#ffffff' : '#38bdf8';
                ctx.fillRect(tx - 2 * z, ty - 2 * z, 4 * z, 4 * z);
            }

            const apertureGrad = ctx.createRadialGradient(0, 0, 1, 0, 0, rad * 0.58);
            if (isWarping) {
                apertureGrad.addColorStop(0, '#ffffff');
                apertureGrad.addColorStop(0.3, '#00f0ff');
                apertureGrad.addColorStop(0.7, '#a855f7');
                apertureGrad.addColorStop(1, 'rgba(15, 23, 42, 0.9)');
            } else {
                const pulseA = 0.4 + Math.sin(time * 3) * 0.15;
                apertureGrad.addColorStop(0, '#00f0ff');
                apertureGrad.addColorStop(0.5, `rgba(168, 85, 247, ${pulseA})`);
                apertureGrad.addColorStop(1, 'rgba(5, 8, 17, 0.85)');
            }
            ctx.fillStyle = apertureGrad;
            ctx.beginPath();
            ctx.ellipse(0, 0, rad * 0.58, rad * 0.5, 0, 0, Math.PI * 2);
            ctx.fill();

            ctx.strokeStyle = isWarping ? '#ffffff' : '#00f0ff';
            ctx.lineWidth = isWarping ? 2.5 : 1.2;
            ctx.beginPath();
            ctx.ellipse(0, 0, rad * 0.58, rad * 0.5, 0, 0, Math.PI * 2);
            ctx.stroke();

            const beaconOn = (Math.sin(time * 5) > 0);
            if (beaconOn || isWarping) {
                ctx.fillStyle = '#00f0ff';
                ctx.beginPath();
                ctx.arc(-rad * 1.2, 0, 2.5 * z, 0, Math.PI * 2);
                ctx.arc(rad * 1.2, 0, 2.5 * z, 0, Math.PI * 2);
                ctx.fill();
            }

            ctx.restore();
        }

        // --- Jump-Gate Warp Transit Animation (Hyperspace Metric Compression) ---
        function drawJumpGateWarpFX(ctx, w, h, warpProgress, z, time) {
            ctx.save();
            const cx = w / 2;
            const cy = h / 2;

            const numRings = 5;
            for (let r = 0; r < numRings; r++) {
                const ringRad = ((1.0 - warpProgress) * w * 0.7 + r * 60) % (w * 0.8);
                const alpha = Math.max(0, Math.min(1, warpProgress * (1.0 - ringRad / (w * 0.8))));
                ctx.strokeStyle = (r % 2 === 0) ? `rgba(0, 240, 255, ${alpha * 0.8})` : `rgba(168, 85, 247, ${alpha * 0.8})`;
                ctx.lineWidth = (2 + r) * z;
                ctx.beginPath();
                ctx.ellipse(cx, cy, ringRad, ringRad * 0.65, 0, 0, Math.PI * 2);
                ctx.stroke();
            }

            const numStreaks = 42;
            for (let i = 0; i < numStreaks; i++) {
                const angle = (i * (Math.PI * 2 / numStreaks)) + 0.15;
                const innerDist = 40 * z + (1.0 - warpProgress) * 60 * z;
                const streakLen = 140 * z * warpProgress;
                const outerDist = innerDist + streakLen;

                const x1 = cx + Math.cos(angle) * innerDist;
                const y1 = cy + Math.sin(angle) * (innerDist * 0.7);
                const x2 = cx + Math.cos(angle) * outerDist;
                const y2 = cy + Math.sin(angle) * (outerDist * 0.7);

                const grad = ctx.createLinearGradient(x1, y1, x2, y2);
                grad.addColorStop(0, 'rgba(255, 255, 255, 0.9)');
                grad.addColorStop(0.4, (i % 2 === 0) ? '#00f0ff' : '#a855f7');
                grad.addColorStop(1, 'rgba(0, 240, 255, 0)');

                ctx.strokeStyle = grad;
                ctx.lineWidth = Math.max(1, 2.2 * z * warpProgress);
                ctx.beginPath();
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);
                ctx.stroke();
            }

            if (warpProgress > 0.4) {
                const flashAlpha = (warpProgress - 0.4) * 0.8;
                const flashGrad = ctx.createRadialGradient(cx, cy, 10, cx, cy, 220 * z);
                flashGrad.addColorStop(0, `rgba(255, 255, 255, ${flashAlpha})`);
                flashGrad.addColorStop(0.3, `rgba(0, 240, 255, ${flashAlpha * 0.6})`);
                flashGrad.addColorStop(0.8, `rgba(168, 85, 247, ${flashAlpha * 0.2})`);
                flashGrad.addColorStop(1, 'rgba(0, 0, 0, 0)');
                ctx.fillStyle = flashGrad;
                ctx.beginPath();
                ctx.arc(cx, cy, 220 * z, 0, Math.PI * 2);
                ctx.fill();
            }

            ctx.font = 'bold 12px monospace';
            ctx.fillStyle = '#00f0ff';
            ctx.textAlign = 'center';
            const sectorName = STAR_SYSTEMS[sim.currentSystemIndex] ? STAR_SYSTEMS[sim.currentSystemIndex].name : 'INTERSTELLAR CORRIDOR';
            ctx.fillText(`// HYPERION JUMP GATE TRANSIT // METRIC COMPRESSION: ${(warpProgress * 100).toFixed(0)}% // SECTOR: ${sectorName}`, cx, h - 34);

            ctx.restore();
        }

        function triggerJumpGateWarp(targetIdx) {
            initAudio();
            const nextIdx = (targetIdx !== undefined) ? targetIdx : (sim.currentSystemIndex + 1) % STAR_SYSTEMS.length;
            sim.warpAnim = 1.0;
            loadStarSystem(nextIdx);
            showToast(`Warp transit engaged: Displacing to ${STAR_SYSTEMS[nextIdx].name}`, 'info');
            playSound('success');
        }
"""
    station_func_needle = "function drawStationSprite(ctx, stX, stY, stR, time, z) {"
    if station_func_needle in content:
        content = content.replace(station_func_needle, helpers_code + "\n        " + station_func_needle, 1)
        print("Inserted helper functions in kcosmic.html")

    # 5. In render(), call drawCosmicNebulae before starfield
    nebula_render_needle = "// 1. Draw Starfield (Parallax with zoom)"
    nebula_render_code = """// 0. Draw Cosmic Anomaly Nebulae (Multi-Stop Deep Space Gas Clouds)
            drawCosmicNebulae(ctx, cx, cy, z, theme, sim.time, sim.currentSystemIndex);

            // 1. Draw Starfield (Parallax with zoom)"""
    if nebula_render_needle in content:
        content = content.replace(nebula_render_needle, nebula_render_code, 1)
        print("Hooked drawCosmicNebulae into render() in kcosmic.html")

    # 6. In render(), draw Jump Gate
    jumpgate_render_needle = "// 6. Moons & Orbital Stations"
    jumpgate_render_code = """// 6. Moons & Orbital Stations
            celestialBodies.forEach(b => {
                if (b.isJumpGate) {
                    if (!sim.paused) b.angle += (b.orbitSpeed || 0.04) * 0.005 * sim.speed;
                    const jDist = b.orbitRadius * z;
                    const jx = sunX + Math.cos(b.angle) * jDist;
                    const jy = sunY + Math.sin(b.angle) * (jDist * 0.7);
                    const jr = (b.radius || 18) * z;
                    b.currX = jx;
                    b.currY = jy;

                    ctx.save();
                    ctx.strokeStyle = 'rgba(0, 240, 255, 0.25)';
                    ctx.setLineDash([5, 5]);
                    ctx.beginPath();
                    ctx.ellipse(sunX, sunY, jDist, jDist * 0.7, 0, 0, Math.PI * 2);
                    ctx.stroke();
                    ctx.restore();

                    drawJumpGateSprite(ctx, jx, jy, jr, sim.time, z, sim.warpAnim > 0);

                    if (sim.selected && sim.selected.id === b.id) {
                        ctx.strokeStyle = '#00f0ff';
                        ctx.lineWidth = 1.5;
                        ctx.strokeRect(jx - jr - 8, jy - jr - 8, (jr + 8) * 2, (jr + 8) * 2);
                    }

                    ctx.fillStyle = '#00f0ff';
                    ctx.font = `bold ${Math.max(9, 9.5 * z)}px monospace`;
                    ctx.textAlign = 'center';
                    ctx.fillText(b.name, jx, jy + jr + 14 * z);
                    ctx.fillStyle = 'rgba(168, 85, 247, 0.9)';
                    ctx.font = `${Math.max(8, 8 * z)}px monospace`;
                    ctx.fillText('WARP GATE [TRANSIT READY]', jx, jy + jr + 24 * z);
                }
            });

            // 7. Moons & Orbital Stations"""
    if jumpgate_render_needle in content:
        content = content.replace(jumpgate_render_needle, jumpgate_render_code, 1)
        print("Hooked Jump Gate rendering into render() in kcosmic.html")

    # 7. In render(), render warp animation FX
    warp_fx_needle = "drawAudioVisualizer();\n            drawThreatRadar();\n            requestAnimationFrame(render);"
    warp_fx_code = """// Jump-Gate Warp Transit FX
            if (sim.warpAnim > 0) {
                drawJumpGateWarpFX(ctx, viewW, viewH, sim.warpAnim, z, sim.time);
                if (!sim.paused) {
                    sim.warpAnim -= 0.02 * sim.speed;
                    if (sim.warpAnim < 0) sim.warpAnim = 0;
                }
            }

            drawAudioVisualizer();
            drawThreatRadar();
            requestAnimationFrame(render);"""
    if warp_fx_needle in content:
        content = content.replace(warp_fx_needle, warp_fx_code, 1)
        print("Hooked drawJumpGateWarpFX into render() in kcosmic.html")

    # 8. Trade route 5 in drawTradeRoutes
    trade_routes_needle = """            const routeCoords = [
                { startX: beltX, startY: beltY, endX: pX, endY: pY, tag: 'MINERALS' },
                { startX: mX, startY: mY, endX: zX, endY: zY, tag: 'VOLATILES' },
                { startX: pX, startY: pY, endX: gX, endY: gY, tag: 'FOOD' },
                { startX: sX, startY: sY, endX: dX, endY: dY, tag: 'HE-3 FUEL' }
            ];"""
    trade_routes_code = """            const jumpgateObj = celestialBodies.find(b => b.isJumpGate);
            const jgX = (jumpgateObj && jumpgateObj.currX !== undefined) ? jumpgateObj.currX : (sunX + 380 * z);
            const jgY = (jumpgateObj && jumpgateObj.currY !== undefined) ? jumpgateObj.currY : (sunY - 260 * z);

            const routeCoords = [
                { startX: beltX, startY: beltY, endX: pX, endY: pY, tag: 'MINERALS' },
                { startX: mX, startY: mY, endX: zX, endY: zY, tag: 'VOLATILES' },
                { startX: pX, startY: pY, endX: gX, endY: gY, tag: 'FOOD' },
                { startX: sX, startY: sY, endX: dX, endY: dY, tag: 'HE-3 FUEL' },
                { startX: zX, startY: zY, endX: jgX, endY: jgY, tag: 'GATE EXPORTS' }
            ];"""
    if trade_routes_needle in content:
        content = content.replace(trade_routes_needle, trade_routes_code, 1)
        print("Updated routeCoords in drawTradeRoutes in kcosmic.html")

    # 9. Jump Gate selection handling in selectEntity
    sel_needle = """                if (item.role) {
                    act1.textContent = 'Inspect Vessel Telemetry';"""
    sel_code = """                if (item.isJumpGate) {
                    act1.textContent = 'Engage Jump Gate (Warp Transit)';
                    act2.textContent = 'Center Jump Gate View';
                    act1.onclick = () => {
                        triggerJumpGateWarp();
                    };
                    act2.onclick = () => {
                        sim.camX -= (item.currX - viewW / 2);
                        sim.camY -= (item.currY - viewH / 2);
                        playSound('click');
                    };
                } else if (item.role) {
                    act1.textContent = 'Inspect Vessel Telemetry';"""
    if sel_needle in content:
        content = content.replace(sel_needle, sel_code, 1)
        print("Added Jump Gate action buttons to selectEntity in kcosmic.html")

    # 10. Key 's' warp transit trigger
    key_needle = """            // Sector Navigation
            if (key === 's') {
                const nextIdx = (sim.currentSystemIndex + 1) % STAR_SYSTEMS.length;
                loadStarSystem(nextIdx);
                showToast(`Warping to star system: ${STAR_SYSTEMS[nextIdx].name}`, 'info');
                playSound('success');
                return;
            }"""
    key_code = """            // Sector Navigation & Jump Gate Warp
            if (key === 's') {
                triggerJumpGateWarp();
                return;
            }"""
    if key_needle in content:
        content = content.replace(key_needle, key_code, 1)
        print("Hooked 's' key to triggerJumpGateWarp in kcosmic.html")

    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print("kcosmic.html updated successfully.")

update_kcosmic_html()
