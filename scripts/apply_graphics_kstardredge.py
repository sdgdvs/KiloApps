"""
Applies graphics overhaul to KiloOS/public/apps/kstardredge.html:
- Integrates Imagen 3 deep space salvager sprites (derelict hull wreckage, pirate wreckage, cosmic anomaly geode/singularity, salvage barge, raiders, spaceport)
- Upgrades magnetic dredge beam effects with dual-frequency flux cones and oscillating magnetic tethers
- Preserves clean static perimeters (glint sweep)
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML_PATH = ROOT / "KiloOS" / "public" / "apps" / "kstardredge.html"

content = HTML_PATH.read_text(encoding="utf-8")

# 1. Asset Definitions
target_dom = """        const logBox = document.getElementById('logBox');"""

asset_def = """        const logBox = document.getElementById('logBox');

        // --- Deep Space Salvage Sprite Assets (Chroma-keyed 2-stage Imagen 3 pipeline) ---
        const KSTARDREDGE_ASSETS = {
            derelictHull: { img: null, loaded: false, src: '../assets/sprites/kstardredge/derelict_hull.png', fallback: 'assets/sprites/kstardredge/derelict_hull.png' },
            pirateWreckage: { img: null, loaded: false, src: '../assets/sprites/kstardredge/pirate_wreckage.png', fallback: 'assets/sprites/kstardredge/pirate_wreckage.png' },
            anomalyGeode: { img: null, loaded: false, src: '../assets/sprites/kstardredge/anomaly_geode.png', fallback: 'assets/sprites/kstardredge/anomaly_geode.png' },
            anomalySingularity: { img: null, loaded: false, src: '../assets/sprites/kstardredge/anomaly_singularity.png', fallback: 'assets/sprites/kstardredge/anomaly_singularity.png' },
            salvageBarge: { img: null, loaded: false, src: '../assets/sprites/kstardredge/salvage_barge.png', fallback: 'assets/sprites/kstardredge/salvage_barge.png' },
            raiderCorsair: { img: null, loaded: false, src: '../assets/sprites/kstardredge/raider_corsair.png', fallback: 'assets/sprites/kstardredge/raider_corsair.png' },
            raiderGunship: { img: null, loaded: false, src: '../assets/sprites/kstardredge/raider_gunship.png', fallback: 'assets/sprites/kstardredge/raider_gunship.png' },
            raiderDread: { img: null, loaded: false, src: '../assets/sprites/kstardredge/raider_dread.png', fallback: 'assets/sprites/kstardredge/raider_dread.png' },
            orbitalSpaceport: { img: null, loaded: false, src: '../assets/sprites/kstardredge/orbital_spaceport.png', fallback: 'assets/sprites/kstardredge/orbital_spaceport.png' }
        };

        function initKStarDredgeAssets() {
            Object.values(KSTARDREDGE_ASSETS).forEach(entry => {
                const img = new Image();
                img.onload = () => { entry.img = img; entry.loaded = true; };
                img.onerror = () => {
                    const fb = new Image();
                    fb.onload = () => { entry.img = fb; entry.loaded = true; };
                    fb.src = entry.fallback;
                };
                img.src = entry.src;
            });
        }
        initKStarDredgeAssets();"""

assert target_dom in content, "Could not find logBox DOM declaration in content"
content = content.replace(target_dom, asset_def, 1)

# 2. Enhanced Magnetic Dredge Beam & Tethers
target_tractor = """            // Tractor Beam Flux Waves
            if (state.ship.tractorActive) {
                const tractorRange = 320;
                ctx.save();
                ctx.translate(state.ship.x, state.ship.y);
                ctx.rotate(state.ship.angle);
                ctx.strokeStyle = pal.tractorGlow;
                ctx.fillStyle = pal.tractorFill;
                ctx.lineWidth = 1.5;

                ctx.beginPath();
                ctx.moveTo(15, 0);
                ctx.arc(0, 0, tractorRange, -Math.PI / 4, Math.PI / 4);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();

                // Concentric ripples
                const timePulse = (Date.now() * 0.003) % 1;
                for (let r = 0; r < 3; r++) {
                    const arcRadius = 40 + ((timePulse + r / 3) % 1) * (tractorRange - 40);
                    ctx.beginPath();
                    ctx.arc(0, 0, arcRadius, -Math.PI / 4, Math.PI / 4);
                    ctx.stroke();
                }
                ctx.restore();

                // Dynamic magnetic flux tether lines to pulled chunks
                const emitterX = state.ship.x + Math.cos(state.ship.angle) * 18;
                const emitterY = state.ship.y + Math.sin(state.ship.angle) * 18;
                for (const chunk of state.oreChunks) {
                    const cdx = chunk.x - state.ship.x;
                    const cdy = chunk.y - state.ship.y;
                    const dist = Math.hypot(cdx, cdy);
                    if (dist < tractorRange) {
                        ctx.save();
                        ctx.strokeStyle = 'rgba(0, 240, 255, 0.45)';
                        ctx.lineWidth = 1;
                        ctx.setLineDash([3, 3]);
                        ctx.beginPath();
                        ctx.moveTo(emitterX, emitterY);
                        ctx.lineTo(chunk.x, chunk.y);
                        ctx.stroke();
                        ctx.restore();
                    }
                }
            }"""

new_tractor = """            // Magnetic Dredge Beam & Polarized Induction Cone
            if (state.ship.tractorActive) {
                const tractorRange = 320;
                ctx.save();
                ctx.translate(state.ship.x, state.ship.y);
                ctx.rotate(state.ship.angle);

                // Multi-frequency polarized magnetic induction cone
                const grad = ctx.createRadialGradient(18, 0, 10, 18, 0, tractorRange);
                grad.addColorStop(0, 'rgba(0, 240, 255, 0.42)');
                grad.addColorStop(0.3, 'rgba(16, 185, 129, 0.2)');
                grad.addColorStop(0.7, 'rgba(56, 189, 248, 0.1)');
                grad.addColorStop(1, 'rgba(2, 132, 199, 0)');
                ctx.fillStyle = grad;
                ctx.strokeStyle = pal.tractorGlow;
                ctx.lineWidth = 1.8;

                ctx.beginPath();
                ctx.moveTo(18, 0);
                ctx.arc(0, 0, tractorRange, -Math.PI / 4, Math.PI / 4);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();

                // High-frequency magnetic induction pulses (4 concentric energized arcs)
                const timePulse = (Date.now() * 0.0035) % 1;
                for (let r = 0; r < 4; r++) {
                    const arcRadius = 35 + ((timePulse + r / 4) % 1) * (tractorRange - 35);
                    const pulseAlpha = 1 - (arcRadius / tractorRange);
                    ctx.strokeStyle = `rgba(0, 240, 255, ${pulseAlpha * 0.75})`;
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    ctx.arc(0, 0, arcRadius, -Math.PI / 4.2, Math.PI / 4.2);
                    ctx.stroke();
                }
                ctx.restore();

                // Oscillating Magnetic Flux Helix Tethers to Pulled Ore & Wreckage
                const emitterX = state.ship.x + Math.cos(state.ship.angle) * 18;
                const emitterY = state.ship.y + Math.sin(state.ship.angle) * 18;
                const targets = [...state.oreChunks, ...state.pirateWrecks];
                const nowSec = Date.now() * 0.008;

                for (const t of targets) {
                    const tdx = t.x - state.ship.x;
                    const tdy = t.y - state.ship.y;
                    const dist = Math.hypot(tdx, tdy);
                    if (dist < tractorRange) {
                        ctx.save();
                        ctx.strokeStyle = 'rgba(0, 240, 255, 0.65)';
                        ctx.lineWidth = 1.6;
                        ctx.shadowColor = '#00f0ff';
                        ctx.shadowBlur = 6;

                        const steps = 14;
                        ctx.beginPath();
                        ctx.moveTo(emitterX, emitterY);
                        for (let s = 1; s <= steps; s++) {
                            const frac = s / steps;
                            const bx = emitterX + (t.x - emitterX) * frac;
                            const by = emitterY + (t.y - emitterY) * frac;
                            const perpX = -(t.y - emitterY) / (dist + 1e-4);
                            const perpY = (t.x - emitterX) / (dist + 1e-4);
                            const waveAmp = Math.sin(frac * Math.PI * 4 - nowSec) * 5 * (1 - frac);
                            ctx.lineTo(bx + perpX * waveAmp, by + perpY * waveAmp);
                        }
                        ctx.stroke();

                        // Traveling magnetic particle packet along flux line
                        const pFrac = (nowSec * 0.6) % 1;
                        const px = emitterX + (t.x - emitterX) * (1 - pFrac);
                        const py = emitterY + (t.y - emitterY) * (1 - pFrac);
                        ctx.fillStyle = '#ffffff';
                        ctx.beginPath();
                        ctx.arc(px, py, 2.5, 0, Math.PI * 2);
                        ctx.fill();
                        ctx.shadowBlur = 0;
                        ctx.restore();
                    }
                }
            }"""

assert target_tractor in content, "Could not find target_tractor in content"
content = content.replace(target_tractor, new_tractor, 1)

# 3. Cosmic Anomaly Silhouettes (Geode & Singularity)
target_geode = """                // Geode Anomaly Halo & Internal Resonator
                if (ast.scanned && ast.geodeType !== 'None') {
                    const isDark = ast.geodeType.includes('Dark');
                    ctx.strokeStyle = isDark ? '#f43f5e' : '#c084fc';
                    ctx.lineWidth = 1.5;
                    ctx.setLineDash([4, 4]);
                    ctx.beginPath();
                    ctx.arc(0, 0, ast.radius + 6, 0, Math.PI * 2);
                    ctx.stroke();
                    ctx.setLineDash([]);

                    const pulse = Math.sin(Date.now() * 0.005) * 0.5 + 0.5;
                    ctx.fillStyle = isDark ? `rgba(244, 63, 94, ${0.4 + pulse * 0.4})` : `rgba(192, 132, 252, ${0.4 + pulse * 0.4})`;
                    ctx.beginPath();
                    ctx.arc(0, 0, 7, 0, Math.PI * 2);
                    ctx.fill();
                }"""

new_geode = """                // Geode Anomaly Halo & Internal Resonator (Cosmic Anomaly Sprite Silhouette)
                if (ast.scanned && ast.geodeType !== 'None') {
                    const isDark = ast.geodeType.includes('Dark');
                    const anomAsset = isDark ? KSTARDREDGE_ASSETS.anomalySingularity : KSTARDREDGE_ASSETS.anomalyGeode;
                    if (anomAsset.loaded && anomAsset.img) {
                        ctx.save();
                        const aSize = ast.radius * 2.2;
                        const pulse = Math.sin(Date.now() * 0.005) * 0.2 + 0.8;
                        ctx.globalAlpha = pulse;
                        ctx.drawImage(anomAsset.img, -aSize / 2, -aSize / 2, aSize, aSize);
                        ctx.restore();
                    } else {
                        ctx.strokeStyle = isDark ? '#f43f5e' : '#c084fc';
                        ctx.lineWidth = 1.5;
                        ctx.setLineDash([4, 4]);
                        ctx.beginPath();
                        ctx.arc(0, 0, ast.radius + 6, 0, Math.PI * 2);
                        ctx.stroke();
                        ctx.setLineDash([]);

                        const pulse = Math.sin(Date.now() * 0.005) * 0.5 + 0.5;
                        ctx.fillStyle = isDark ? `rgba(244, 63, 94, ${0.4 + pulse * 0.4})` : `rgba(192, 132, 252, ${0.4 + pulse * 0.4})`;
                        ctx.beginPath();
                        ctx.arc(0, 0, 7, 0, Math.PI * 2);
                        ctx.fill();
                    }
                }"""

assert target_geode in content, "Could not find target_geode in content"
content = content.replace(target_geode, new_geode, 1)

# 4. Derelict Spaceship Wreckage Sprites
target_derelict = """            // Phase 8: Render Derelict Spaceships (Evocative ruined star-hulks)
            for (const der of state.derelicts) {
                ctx.save();
                ctx.translate(der.x, der.y);
                ctx.rotate(der.rot);

                const isSelectedDer = (der === state.selectedDerelict);

                // Hull Geometry Base
                ctx.fillStyle = '#0a101f';
                ctx.strokeStyle = der.airlockBreached ? '#10b981' : '#f43f5e';
                ctx.lineWidth = isSelectedDer ? 2.5 : 1.6;

                ctx.beginPath();
                ctx.moveTo(36, 0);
                ctx.lineTo(16, -18);
                ctx.lineTo(-16, -24);
                ctx.lineTo(-36, -13);
                ctx.lineTo(-28, 0);
                ctx.lineTo(-36, 13);
                ctx.lineTo(-16, 24);
                ctx.lineTo(16, 18);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();

                // Exposed Titanium Internal Bulkhead Ribs
                ctx.strokeStyle = 'rgba(100, 116, 139, 0.7)';
                ctx.lineWidth = 1.2;
                ctx.beginPath();
                ctx.moveTo(16, -18); ctx.lineTo(16, 18);
                ctx.moveTo(0, -21); ctx.lineTo(0, 21);
                ctx.moveTo(-16, -24); ctx.lineTo(-16, 24);
                ctx.stroke();

                // Spinal Keel Conduit
                ctx.strokeStyle = 'rgba(56, 189, 248, 0.4)';
                ctx.beginPath();
                ctx.moveTo(-28, 0); ctx.lineTo(34, 0);
                ctx.stroke();

                // Jagged Hull Breach & Plasma Scorch Marks
                ctx.strokeStyle = '#ef4444';
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(-8, -16); ctx.lineTo(6, 2); ctx.lineTo(-4, 18);
                ctx.stroke();

                // Airlock Hatch Indicator
                ctx.fillStyle = der.airlockBreached ? '#10b981' : '#ef4444';
                ctx.fillRect(8, -4, 6, 8);
                ctx.strokeStyle = '#f8fafc';
                ctx.lineWidth = 0.8;
                ctx.strokeRect(8, -4, 6, 8);

                // Flashing Emergency Transponder Beacon
                const flashTime = (Date.now() * 0.007) % (Math.PI * 2);
                const beaconPulse = Math.sin(flashTime) > 0.3;
                if (beaconPulse) {
                    ctx.fillStyle = '#f43f5e';
                    ctx.shadowColor = '#f43f5e';
                    ctx.shadowBlur = 12;
                    ctx.beginPath();
                    ctx.arc(-26, 0, 3.5, 0, Math.PI * 2);
                    ctx.fill();

                    // Expanding rescue pulse ripple
                    ctx.strokeStyle = 'rgba(244, 63, 94, 0.4)';
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.arc(-26, 0, 12, 0, Math.PI * 2);
                    ctx.stroke();
                    ctx.shadowBlur = 0;
                } else {
                    ctx.fillStyle = '#881337';
                    ctx.beginPath();
                    ctx.arc(-26, 0, 2.5, 0, Math.PI * 2);
                    ctx.fill();
                }

                // Internal Reactor Core
                if (!der.reactorHarvested) {
                    ctx.fillStyle = '#f59e0b';
                    ctx.shadowColor = '#f59e0b';
                    ctx.shadowBlur = 8;
                    ctx.beginPath();
                    ctx.arc(-12, 0, 3.5, 0, Math.PI * 2);
                    ctx.fill();
                    ctx.shadowBlur = 0;
                }

                ctx.restore();"""

new_derelict = """            // Phase 8: Render Derelict Spaceships (Imagen 3 Ruined Star-Hulks)
            for (const der of state.derelicts) {
                ctx.save();
                ctx.translate(der.x, der.y);
                ctx.rotate(der.rot);

                const isSelectedDer = (der === state.selectedDerelict);

                if (KSTARDREDGE_ASSETS.derelictHull.loaded && KSTARDREDGE_ASSETS.derelictHull.img) {
                    const dw = 76;
                    const dh = 76;
                    ctx.drawImage(KSTARDREDGE_ASSETS.derelictHull.img, -dw / 2, -dh / 2, dw, dh);

                    // Airlock breach indicator highlight
                    ctx.fillStyle = der.airlockBreached ? '#10b981' : '#ef4444';
                    ctx.fillRect(8, -4, 6, 8);
                    ctx.strokeStyle = '#f8fafc';
                    ctx.lineWidth = 0.8;
                    ctx.strokeRect(8, -4, 6, 8);

                    // Emergency transponder beacon
                    const flashTime = (Date.now() * 0.007) % (Math.PI * 2);
                    if (Math.sin(flashTime) > 0.3) {
                        ctx.fillStyle = '#f43f5e';
                        ctx.shadowColor = '#f43f5e';
                        ctx.shadowBlur = 12;
                        ctx.beginPath();
                        ctx.arc(-26, 0, 3.5, 0, Math.PI * 2);
                        ctx.fill();
                        ctx.strokeStyle = 'rgba(244, 63, 94, 0.4)';
                        ctx.lineWidth = 1;
                        ctx.beginPath();
                        ctx.arc(-26, 0, 12, 0, Math.PI * 2);
                        ctx.stroke();
                        ctx.shadowBlur = 0;
                    }

                    // Reactor core glow
                    if (!der.reactorHarvested) {
                        ctx.fillStyle = '#f59e0b';
                        ctx.shadowColor = '#f59e0b';
                        ctx.shadowBlur = 8;
                        ctx.beginPath();
                        ctx.arc(-12, 0, 3.5, 0, Math.PI * 2);
                        ctx.fill();
                        ctx.shadowBlur = 0;
                    }
                } else {
                    // Procedural Fallback Hull Geometry
                    ctx.fillStyle = '#0a101f';
                    ctx.strokeStyle = der.airlockBreached ? '#10b981' : '#f43f5e';
                    ctx.lineWidth = isSelectedDer ? 2.5 : 1.6;

                    ctx.beginPath();
                    ctx.moveTo(36, 0);
                    ctx.lineTo(16, -18);
                    ctx.lineTo(-16, -24);
                    ctx.lineTo(-36, -13);
                    ctx.lineTo(-28, 0);
                    ctx.lineTo(-36, 13);
                    ctx.lineTo(-16, 24);
                    ctx.lineTo(16, 18);
                    ctx.closePath();
                    ctx.fill();
                    ctx.stroke();

                    // Exposed Titanium Internal Bulkhead Ribs
                    ctx.strokeStyle = 'rgba(100, 116, 139, 0.7)';
                    ctx.lineWidth = 1.2;
                    ctx.beginPath();
                    ctx.moveTo(16, -18); ctx.lineTo(16, 18);
                    ctx.moveTo(0, -21); ctx.lineTo(0, 21);
                    ctx.moveTo(-16, -24); ctx.lineTo(-16, 24);
                    ctx.stroke();

                    // Spinal Keel Conduit
                    ctx.strokeStyle = 'rgba(56, 189, 248, 0.4)';
                    ctx.beginPath();
                    ctx.moveTo(-28, 0); ctx.lineTo(34, 0);
                    ctx.stroke();

                    // Jagged Hull Breach & Plasma Scorch Marks
                    ctx.strokeStyle = '#ef4444';
                    ctx.lineWidth = 1.5;
                    ctx.beginPath();
                    ctx.moveTo(-8, -16); ctx.lineTo(6, 2); ctx.lineTo(-4, 18);
                    ctx.stroke();

                    // Airlock Hatch Indicator
                    ctx.fillStyle = der.airlockBreached ? '#10b981' : '#ef4444';
                    ctx.fillRect(8, -4, 6, 8);
                    ctx.strokeStyle = '#f8fafc';
                    ctx.lineWidth = 0.8;
                    ctx.strokeRect(8, -4, 6, 8);

                    // Flashing Emergency Transponder Beacon
                    const flashTime = (Date.now() * 0.007) % (Math.PI * 2);
                    const beaconPulse = Math.sin(flashTime) > 0.3;
                    if (beaconPulse) {
                        ctx.fillStyle = '#f43f5e';
                        ctx.shadowColor = '#f43f5e';
                        ctx.shadowBlur = 12;
                        ctx.beginPath();
                        ctx.arc(-26, 0, 3.5, 0, Math.PI * 2);
                        ctx.fill();

                        ctx.strokeStyle = 'rgba(244, 63, 94, 0.4)';
                        ctx.lineWidth = 1;
                        ctx.beginPath();
                        ctx.arc(-26, 0, 12, 0, Math.PI * 2);
                        ctx.stroke();
                        ctx.shadowBlur = 0;
                    } else {
                        ctx.fillStyle = '#881337';
                        ctx.beginPath();
                        ctx.arc(-26, 0, 2.5, 0, Math.PI * 2);
                        ctx.fill();
                    }

                    // Internal Reactor Core
                    if (!der.reactorHarvested) {
                        ctx.fillStyle = '#f59e0b';
                        ctx.shadowColor = '#f59e0b';
                        ctx.shadowBlur = 8;
                        ctx.beginPath();
                        ctx.arc(-12, 0, 3.5, 0, Math.PI * 2);
                        ctx.fill();
                        ctx.shadowBlur = 0;
                    }
                }

                ctx.restore();"""

assert target_derelict in content, "Could not find target_derelict in content"
content = content.replace(target_derelict, new_derelict, 1)

# 5. Pirate Wreckage Hulks
target_pw = """            // Render Pirate Wreckage Hulks
            for (const pw of state.pirateWrecks) {
                ctx.save();
                ctx.translate(pw.x, pw.y);
                ctx.rotate(pw.angle);
                ctx.strokeStyle = '#64748b';
                ctx.fillStyle = '#0f172a';
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(12, 0); ctx.lineTo(-8, -8); ctx.lineTo(-4, 0); ctx.lineTo(-8, 8);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
                ctx.restore();
            }"""

new_pw = """            // Render Pirate Wreckage Hulks (Imagen 3 Corsair Debris)
            for (const pw of state.pirateWrecks) {
                ctx.save();
                ctx.translate(pw.x, pw.y);
                ctx.rotate(pw.angle);
                if (KSTARDREDGE_ASSETS.pirateWreckage.loaded && KSTARDREDGE_ASSETS.pirateWreckage.img) {
                    const pwSize = 34;
                    ctx.drawImage(KSTARDREDGE_ASSETS.pirateWreckage.img, -pwSize / 2, -pwSize / 2, pwSize, pwSize);
                } else {
                    ctx.strokeStyle = '#64748b';
                    ctx.fillStyle = '#0f172a';
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.moveTo(12, 0); ctx.lineTo(-8, -8); ctx.lineTo(-4, 0); ctx.lineTo(-8, 8);
                    ctx.closePath();
                    ctx.fill();
                    ctx.stroke();
                }
                ctx.restore();
            }"""

assert target_pw in content, "Could not find target_pw in content"
content = content.replace(target_pw, new_pw, 1)

# 6. Pirate Raider Ships
target_raider = """                // Main Warship Chassis
                ctx.fillStyle = '#1e1b4b';
                ctx.strokeStyle = r.color;
                ctx.lineWidth = 2;
                ctx.beginPath();
                if (isCorsair) {
                    // Fast Razor Dagger with forward swept winglets
                    ctx.moveTo(r.radius + 8, 0);
                    ctx.lineTo(-r.radius * 0.3, -r.radius * 0.9);
                    ctx.lineTo(-r.radius, -r.radius * 0.5);
                    ctx.lineTo(-r.radius * 0.5, 0);
                    ctx.lineTo(-r.radius, r.radius * 0.5);
                    ctx.lineTo(-r.radius * 0.3, r.radius * 0.9);
                } else if (isGunship) {
                    // Heavy twin-nacelle marauder with forward cannon barrels
                    ctx.moveTo(r.radius + 10, 0);
                    ctx.lineTo(r.radius * 0.4, -r.radius * 0.5);
                    ctx.lineTo(r.radius * 0.8, -r.radius * 0.7);
                    ctx.lineTo(-r.radius, -r.radius * 0.9);
                    ctx.lineTo(-r.radius * 0.5, 0);
                    ctx.lineTo(-r.radius, r.radius * 0.9);
                    ctx.lineTo(r.radius * 0.8, r.radius * 0.7);
                    ctx.lineTo(r.radius * 0.4, r.radius * 0.5);
                } else {
                    // Massive spiked dreadnought
                    ctx.moveTo(r.radius + 14, 0);
                    ctx.lineTo(r.radius * 0.4, -r.radius * 0.8);
                    ctx.lineTo(-r.radius * 0.2, -r.radius * 1.1);
                    ctx.lineTo(-r.radius, -r.radius * 0.7);
                    ctx.lineTo(-r.radius * 0.6, 0);
                    ctx.lineTo(-r.radius, r.radius * 0.7);
                    ctx.lineTo(-r.radius * 0.2, r.radius * 1.1);
                    ctx.lineTo(r.radius * 0.4, r.radius * 0.8);
                }
                ctx.closePath();
                ctx.fill();
                ctx.stroke();"""

new_raider = """                // Main Warship Chassis (Sprite Asset with Vector Fallback)
                const rAsset = isCorsair ? KSTARDREDGE_ASSETS.raiderCorsair : (isGunship ? KSTARDREDGE_ASSETS.raiderGunship : KSTARDREDGE_ASSETS.raiderDread);
                if (rAsset.loaded && rAsset.img) {
                    const rSize = r.radius * 2.4;
                    ctx.drawImage(rAsset.img, -rSize / 2, -rSize / 2, rSize, rSize);
                } else {
                    ctx.fillStyle = '#1e1b4b';
                    ctx.strokeStyle = r.color;
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    if (isCorsair) {
                        ctx.moveTo(r.radius + 8, 0);
                        ctx.lineTo(-r.radius * 0.3, -r.radius * 0.9);
                        ctx.lineTo(-r.radius, -r.radius * 0.5);
                        ctx.lineTo(-r.radius * 0.5, 0);
                        ctx.lineTo(-r.radius, r.radius * 0.5);
                        ctx.lineTo(-r.radius * 0.3, r.radius * 0.9);
                    } else if (isGunship) {
                        ctx.moveTo(r.radius + 10, 0);
                        ctx.lineTo(r.radius * 0.4, -r.radius * 0.5);
                        ctx.lineTo(r.radius * 0.8, -r.radius * 0.7);
                        ctx.lineTo(-r.radius, -r.radius * 0.9);
                        ctx.lineTo(-r.radius * 0.5, 0);
                        ctx.lineTo(-r.radius, r.radius * 0.9);
                        ctx.lineTo(r.radius * 0.8, r.radius * 0.7);
                        ctx.lineTo(r.radius * 0.4, r.radius * 0.5);
                    } else {
                        ctx.moveTo(r.radius + 14, 0);
                        ctx.lineTo(r.radius * 0.4, -r.radius * 0.8);
                        ctx.lineTo(-r.radius * 0.2, -r.radius * 1.1);
                        ctx.lineTo(-r.radius, -r.radius * 0.7);
                        ctx.lineTo(-r.radius * 0.6, 0);
                        ctx.lineTo(-r.radius, r.radius * 0.7);
                        ctx.lineTo(-r.radius * 0.2, r.radius * 1.1);
                        ctx.lineTo(r.radius * 0.4, r.radius * 0.8);
                    }
                    ctx.closePath();
                    ctx.fill();
                    ctx.stroke();
                }"""

assert target_raider in content, "Could not find target_raider in content"
content = content.replace(target_raider, new_raider, 1)

HTML_PATH.write_text(content, encoding="utf-8")
print("Successfully applied graphics overhaul to kstardredge.html!")
