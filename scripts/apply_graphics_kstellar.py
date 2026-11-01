"""
Applies graphics overhaul to KiloOS/public/apps/kstellar.html:
- Integrates Imagen 3 spacefaring sprites (Player Cruiser, Interceptor, Marauder, Dreadnought, Infiltrator, Roaming Fleets, Orbital Station, Anomalies)
- Upgrades getCommodityIcon() and getPlanetSvg()
- Removes rotating specular glints and traveling perimeter border dots
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML_PATH = ROOT / "KiloOS" / "public" / "apps" / "kstellar.html"

content = HTML_PATH.read_text(encoding="utf-8")

# 1. Assets definition
asset_def = """        const mapCanvas = document.getElementById('map-canvas');
        const mapCtx = mapCanvas.getContext('2d');

        // --- Spacefaring Sprite Assets (Chroma-keyed 2-stage Imagen 3 pipeline) ---
        const KSTELLAR_ASSETS = {
            playerCruiser: { img: null, loaded: false, src: '../assets/sprites/kstellar/ship_player_cruiser.png', fallback: 'assets/sprites/kstellar/ship_player_cruiser.png' },
            interceptor: { img: null, loaded: false, src: '../assets/sprites/kstellar/ship_corsair_interceptor.png', fallback: 'assets/sprites/kstellar/ship_corsair_interceptor.png' },
            marauder: { img: null, loaded: false, src: '../assets/sprites/kstellar/ship_corsair_marauder.png', fallback: 'assets/sprites/kstellar/ship_corsair_marauder.png' },
            dreadnought: { img: null, loaded: false, src: '../assets/sprites/kstellar/ship_syndicate_dreadnought.png', fallback: 'assets/sprites/kstellar/ship_syndicate_dreadnought.png' },
            infiltrator: { img: null, loaded: false, src: '../assets/sprites/kstellar/ship_stealth_infiltrator.png', fallback: 'assets/sprites/kstellar/ship_stealth_infiltrator.png' },
            fleetFed: { img: null, loaded: false, src: '../assets/sprites/kstellar/fleet_federation_icon.png', fallback: 'assets/sprites/kstellar/fleet_federation_icon.png' },
            fleetTraders: { img: null, loaded: false, src: '../assets/sprites/kstellar/fleet_traders_icon.png', fallback: 'assets/sprites/kstellar/fleet_traders_icon.png' },
            fleetPirates: { img: null, loaded: false, src: '../assets/sprites/kstellar/fleet_pirates_icon.png', fallback: 'assets/sprites/kstellar/fleet_pirates_icon.png' },
            station: { img: null, loaded: false, src: '../assets/sprites/kstellar/orbital_station_facility.png', fallback: 'assets/sprites/kstellar/orbital_station_facility.png' },
            monolith: { img: null, loaded: false, src: '../assets/sprites/kstellar/anomaly_monolith.png', fallback: 'assets/sprites/kstellar/anomaly_monolith.png' },
            blackhole: { img: null, loaded: false, src: '../assets/sprites/kstellar/anomaly_blackhole.png', fallback: 'assets/sprites/kstellar/anomaly_blackhole.png' },
            ionstorm: { img: null, loaded: false, src: '../assets/sprites/kstellar/anomaly_ionstorm.png', fallback: 'assets/sprites/kstellar/anomaly_ionstorm.png' }
        };

        function initKStellarAssets() {
            Object.values(KSTELLAR_ASSETS).forEach(entry => {
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
        initKStellarAssets();"""

target_mapctx = """        const mapCanvas = document.getElementById('map-canvas');
        const mapCtx = mapCanvas.getContext('2d');"""

assert target_mapctx in content, "Could not find mapCtx in content"
content = content.replace(target_mapctx, asset_def, 1)

# 2. Commodity icons and Planet SVG
old_commodity_and_planet = """        // Commodity SVG generator
        function getCommodityIcon(type) {
            if (type === 'food') {
                return `<svg width="22" height="22" viewBox="0 0 24 24" fill="none"><rect x="3" y="4" width="18" height="16" rx="2" fill="#1b4d2e" stroke="#00ff66" stroke-width="2"/><path d="M7 8h10M7 12h10M7 16h6" stroke="#00ff66" stroke-width="2" stroke-linecap="round"/></svg>`;
            } else if (type === 'minerals') {
                return `<svg width="22" height="22" viewBox="0 0 24 24" fill="none"><polygon points="12,2 22,8 18,22 6,22 2,8" fill="#4d2f00" stroke="#ffaa00" stroke-width="2"/><polygon points="12,2 18,12 12,22 6,12" fill="#663e00" stroke="#ffaa00" stroke-width="1.5"/></svg>`;
            } else if (type === 'medicine') {
                return `<svg width="22" height="22" viewBox="0 0 24 24" fill="none"><rect x="3" y="4" width="18" height="16" rx="2" fill="#33001a" stroke="#ff3399" stroke-width="2"/><path d="M12 7v10M7 12h10" stroke="#ff3399" stroke-width="2.5" stroke-linecap="round"/></svg>`;
            } else {
                return `<svg width="22" height="22" viewBox="0 0 24 24" fill="none"><rect x="4" y="4" width="16" height="16" fill="#002b4d" stroke="#00ffff" stroke-width="2"/><rect x="8" y="8" width="8" height="8" fill="#00ffff"/><line x1="12" y1="1" x2="12" y2="4" stroke="#00ffff" stroke-width="2"/><line x1="12" y1="20" x2="12" y2="23" stroke="#00ffff" stroke-width="2"/><line x1="1" y1="12" x2="4" y2="12" stroke="#00ffff" stroke-width="2"/><line x1="20" y1="12" x2="23" y2="12" stroke="#00ffff" stroke-width="2"/></svg>`;
            }
        }

        // Planet SVG graphic generator for Station/System Header
        function getPlanetSvg(sys) {
            let pColor1 = "#228833", pColor2 = "#66ff66", ringSvg = "";
            if (sys.economy === "Mining") { pColor1 = "#884400"; pColor2 = "#ff9933"; }
            else if (sys.economy === "Industrial") { pColor1 = "#886600"; pColor2 = "#ffcc00"; }
            else if (sys.economy === "Tech Hub") { pColor1 = "#004488"; pColor2 = "#00ffff"; ringSvg = `<ellipse cx="32" cy="32" rx="28" ry="8" fill="none" stroke="rgba(0,255,255,0.6)" stroke-width="2" transform="rotate(-20 32 32)"/>`; }
            else if (sys.economy === "Trading Hub") { pColor1 = "#0066aa"; pColor2 = "#00ffcc"; ringSvg = `<ellipse cx="32" cy="32" rx="30" ry="7" fill="none" stroke="rgba(255,255,255,0.5)" stroke-width="1.5" transform="rotate(15 32 32)"/>`; }

            return `<svg width="64" height="64" viewBox="0 0 64 64" style="filter:drop-shadow(0 0 10px ${pColor2});">
                <defs>
                    <radialGradient id="planetGrad_${sys.id}" cx="35%" cy="35%" r="65%">
                        <stop offset="0%" stop-color="${pColor2}"/>
                        <stop offset="60%" stop-color="${pColor1}"/>
                        <stop offset="100%" stop-color="#020810"/>
                    </radialGradient>
                </defs>
                <circle cx="32" cy="32" r="22" fill="url(#planetGrad_${sys.id})"/>
                ${ringSvg}
                <!-- Orbital Station Facility (Clean static architecture) -->
                <rect x="44" y="14" width="8" height="3" fill="#00ffff" opacity="0.9"/>
                <rect x="46" y="11" width="4" height="9" fill="#004466" stroke="#00ffff" stroke-width="1"/>
                <line x1="43" y1="15" x2="53" y2="15" stroke="#00ffcc" stroke-width="1"/>
            </svg>`;
        }"""

new_commodity_and_planet = """        // High-Fidelity Multi-Layered Commodity SVG Icon Generator
        function getCommodityIcon(type) {
            if (type === 'food') {
                return `<svg width="24" height="24" viewBox="0 0 24 24" fill="none">
                    <rect x="5" y="3" width="14" height="18" rx="4" fill="#0d2818" stroke="#00ff66" stroke-width="1.5"/>
                    <rect x="7" y="6" width="10" height="12" rx="2" fill="#1b4d2e"/>
                    <path d="M12 7v10M9 10c1.5-2 4.5-2 6 0M9 14c1.5 2 4.5 2 6 0" stroke="#55ff99" stroke-width="1.2" stroke-linecap="round"/>
                    <circle cx="12" cy="12" r="1.5" fill="#00ff66"/>
                    <rect x="8" y="2" width="8" height="2" fill="#55ff99" rx="1"/>
                    <rect x="8" y="20" width="8" height="2" fill="#55ff99" rx="1"/>
                </svg>`;
            } else if (type === 'minerals') {
                return `<svg width="24" height="24" viewBox="0 0 24 24" fill="none">
                    <polygon points="12,2 21,7 18,21 6,21 3,7" fill="#3a2000" stroke="#ffaa00" stroke-width="1.5"/>
                    <polygon points="12,2 18,10 12,21 6,10" fill="#663a00" stroke="#ffd280" stroke-width="1"/>
                    <polygon points="12,2 12,21 18,10" fill="#884d00"/>
                    <line x1="3" y1="7" x2="6" y2="10" stroke="#ffaa00" stroke-width="1"/>
                    <line x1="21" y1="7" x2="18" y2="10" stroke="#ffaa00" stroke-width="1"/>
                    <circle cx="12" cy="10" r="1.5" fill="#ffffff"/>
                </svg>`;
            } else if (type === 'medicine') {
                return `<svg width="24" height="24" viewBox="0 0 24 24" fill="none">
                    <rect x="5" y="3" width="14" height="18" rx="3" fill="#30001a" stroke="#ff3399" stroke-width="1.5"/>
                    <rect x="8" y="6" width="8" height="12" rx="1" fill="#4d002b"/>
                    <path d="M12 8v8M8 12h8" stroke="#ff66b2" stroke-width="2.2" stroke-linecap="round"/>
                    <circle cx="12" cy="12" r="1" fill="#ffffff"/>
                    <line x1="5" y1="6" x2="19" y2="6" stroke="#ff3399" stroke-width="1"/>
                    <line x1="5" y1="18" x2="19" y2="18" stroke="#ff3399" stroke-width="1"/>
                </svg>`;
            } else {
                return `<svg width="24" height="24" viewBox="0 0 24 24" fill="none">
                    <rect x="4" y="4" width="16" height="16" rx="2" fill="#002244" stroke="#00ffff" stroke-width="1.5"/>
                    <rect x="8" y="8" width="8" height="8" fill="#004466" stroke="#00ffcc" stroke-width="1"/>
                    <rect x="10" y="10" width="4" height="4" fill="#00ffff"/>
                    <line x1="12" y1="1" x2="12" y2="4" stroke="#00ffff" stroke-width="1.5"/>
                    <line x1="12" y1="20" x2="12" y2="23" stroke="#00ffff" stroke-width="1.5"/>
                    <line x1="1" y1="12" x2="4" y2="12" stroke="#00ffff" stroke-width="1.5"/>
                    <line x1="20" y1="12" x2="23" y2="12" stroke="#00ffff" stroke-width="1.5"/>
                    <circle cx="6" cy="6" r="1" fill="#ffaa00"/>
                    <circle cx="18" cy="18" r="1" fill="#00ffcc"/>
                </svg>`;
            }
        }

        // High-Fidelity Planetary & Orbital Megastructure SVG Graphic Generator
        function getPlanetSvg(sys) {
            let pColor1 = "#228833", pColor2 = "#66ff66", atmosphere = "rgba(0, 255, 120, 0.25)", ringSvg = "", surfacePattern = "";
            
            if (sys.economy === "Mining") {
                pColor1 = "#552a00"; pColor2 = "#d47a22"; atmosphere = "rgba(255, 140, 40, 0.25)";
                surfacePattern = `
                    <ellipse cx="26" cy="24" rx="5" ry="3" fill="#3a1c00" stroke="#ff6600" stroke-width="0.8"/>
                    <ellipse cx="38" cy="36" rx="7" ry="4" fill="#3a1c00" stroke="#ff8800" stroke-width="0.8"/>
                    <path d="M22 36 Q30 38 34 46 Q40 48 44 42" stroke="#ff4400" stroke-width="1.2" fill="none" opacity="0.8"/>
                `;
            } else if (sys.economy === "Industrial") {
                pColor1 = "#4d3a10"; pColor2 = "#c29a24"; atmosphere = "rgba(255, 200, 50, 0.22)";
                surfacePattern = `
                    <rect x="22" y="24" width="8" height="6" fill="#2a1f05" stroke="#ffcc00" stroke-width="0.7"/>
                    <rect x="34" y="28" width="10" height="7" fill="#2a1f05" stroke="#ff9900" stroke-width="0.7"/>
                    <line x1="20" y1="38" x2="44" y2="38" stroke="#ffaa00" stroke-dasharray="2,2" stroke-width="1"/>
                `;
            } else if (sys.economy === "Tech Hub") {
                pColor1 = "#003366"; pColor2 = "#00aaff"; atmosphere = "rgba(0, 240, 255, 0.35)";
                ringSvg = `<ellipse cx="32" cy="32" rx="28" ry="8" fill="none" stroke="rgba(0,255,255,0.7)" stroke-width="2" transform="rotate(-20 32 32)"/>`;
                surfacePattern = `
                    <path d="M22 22 Q30 18 38 24 Q44 32 36 38 Q26 42 22 34 Z" fill="#005588" opacity="0.85"/>
                    <circle cx="32" cy="28" r="2" fill="#00ffff"/>
                    <line x1="28" y1="28" x2="36" y2="28" stroke="#00ffff" stroke-width="0.8"/>
                `;
            } else if (sys.economy === "Trading Hub") {
                pColor1 = "#004488"; pColor2 = "#00ddaa"; atmosphere = "rgba(0, 255, 180, 0.3)";
                ringSvg = `<ellipse cx="32" cy="32" rx="30" ry="7" fill="none" stroke="rgba(200,255,255,0.6)" stroke-width="1.5" transform="rotate(15 32 32)"/>`;
                surfacePattern = `
                    <path d="M20 26 Q28 20 36 28 Q40 38 32 40 Q22 42 18 34 Z" fill="#228844" opacity="0.85"/>
                    <path d="M24 20 Q32 24 38 18" stroke="#ffffff" stroke-width="1.4" fill="none" opacity="0.6"/>
                    <path d="M26 36 Q34 32 42 38" stroke="#ffffff" stroke-width="1.4" fill="none" opacity="0.6"/>
                `;
            } else {
                surfacePattern = `
                    <path d="M20 28 Q28 22 38 30 Q34 40 24 38 Z" fill="#1b6e35" opacity="0.85"/>
                    <path d="M22 24 Q30 26 38 22" stroke="#ffffff" stroke-width="1.2" fill="none" opacity="0.5"/>
                `;
            }

            let facilitySvg = `
                <!-- Deep Space Orbital Starbase Module (Zero perimeter glints) -->
                <rect x="44" y="12" width="10" height="4" fill="#002244" stroke="#00ffff" stroke-width="1"/>
                <rect x="47" y="9" width="4" height="10" fill="#003366" stroke="#00ffcc" stroke-width="1"/>
                <line x1="42" y1="14" x2="56" y2="14" stroke="#00ffff" stroke-width="1.2"/>
                <circle cx="49" cy="8" r="1.2" fill="#ffcc00"/>
            `;
            if (sys.hasRingworld) {
                facilitySvg = `
                    <!-- Megastructure Orbital Ring Spoke Terminal -->
                    <circle cx="32" cy="32" r="26" fill="none" stroke="rgba(0,255,255,0.7)" stroke-width="1.5"/>
                    <rect x="48" y="16" width="6" height="6" fill="#003355" stroke="#00ffff" stroke-width="1"/>
                    <circle cx="51" cy="19" r="1.5" fill="#00ffcc"/>
                `;
            }

            return `<svg width="72" height="72" viewBox="0 0 64 64" style="filter:drop-shadow(0 0 10px ${pColor2});">
                <defs>
                    <radialGradient id="planetAtmosphere_${sys.id}" cx="35%" cy="35%" r="65%">
                        <stop offset="0%" stop-color="${pColor2}"/>
                        <stop offset="55%" stop-color="${pColor1}"/>
                        <stop offset="90%" stop-color="#020810"/>
                        <stop offset="100%" stop-color="#000000"/>
                    </radialGradient>
                    <filter id="atmoGlow_${sys.id}" x="-20%" y="-20%" width="140%" height="140%">
                        <feGaussianBlur in="SourceGraphic" stdDeviation="2"/>
                    </filter>
                </defs>
                <circle cx="32" cy="32" r="23" fill="${atmosphere}" filter="url(#atmoGlow_${sys.id})"/>
                <circle cx="32" cy="32" r="21" fill="url(#planetAtmosphere_${sys.id})"/>
                ${surfacePattern}
                ${ringSvg}
                ${facilitySvg}
            </svg>`;
        }"""

assert old_commodity_and_planet in content, "Could not find old_commodity_and_planet in content"
content = content.replace(old_commodity_and_planet, new_commodity_and_planet, 1)

# 3. Anomaly illustration in showSystemInfo
old_phenom = """            if (sys.phenomenon && sys.phenomenon !== "None") {
                let pColor = sys.phenomenon === "Black Hole" ? "#ff00ff" : (sys.phenomenon === "Solar Flare" ? "#ffaa00" : (sys.phenomenon === "Alien Monolith" ? "#00ff88" : (sys.phenomenon === "Ion Storm" ? "#33aaff" : "#aaccff")));
                phenomBadge = `<div style="margin: 8px 0; padding: 6px; border: 1px dashed ${pColor}; color: ${pColor}; background: rgba(0,0,0,0.5); font-size: 13px;">
                    <strong>[★ PHENOMENON DETECTED]</strong><br>${sys.phenomenon.toUpperCase()}
                </div>`;
            }"""

new_phenom = """            if (sys.phenomenon && sys.phenomenon !== "None") {
                let pColor = sys.phenomenon === "Black Hole" ? "#ff00ff" : (sys.phenomenon === "Solar Flare" ? "#ffaa00" : (sys.phenomenon === "Alien Monolith" ? "#00ff88" : (sys.phenomenon === "Ion Storm" ? "#33aaff" : "#aaccff")));
                let anomImg = "";
                if (sys.phenomenon === "Alien Monolith") anomImg = `<img src="../assets/sprites/kstellar/anomaly_monolith.png" width="34" height="34" style="vertical-align:middle; margin-right:8px; border:1px solid ${pColor};" onerror="this.style.display='none'">`;
                else if (sys.phenomenon === "Black Hole") anomImg = `<img src="../assets/sprites/kstellar/anomaly_blackhole.png" width="34" height="34" style="vertical-align:middle; margin-right:8px; border:1px solid ${pColor};" onerror="this.style.display='none'">`;
                else if (sys.phenomenon === "Ion Storm") anomImg = `<img src="../assets/sprites/kstellar/anomaly_ionstorm.png" width="34" height="34" style="vertical-align:middle; margin-right:8px; border:1px solid ${pColor};" onerror="this.style.display='none'">`;
                phenomBadge = `<div style="display:flex; align-items:center; margin: 8px 0; padding: 6px; border: 1px dashed ${pColor}; color: ${pColor}; background: rgba(0,0,0,0.5); font-size: 13px;">
                    ${anomImg}<div><strong>[★ PHENOMENON DETECTED]</strong><br>${sys.phenomenon.toUpperCase()}</div>
                </div>`;
            }"""

assert old_phenom in content, "Could not find old_phenom in content"
content = content.replace(old_phenom, new_phenom, 1)

# 4. Ringworld rotating glint removal
old_ring_rot = """                    const numSegments = 10;
                    const segAngle = Math.PI / numSegments;
                    const rotOffset = (animFrameTime * 0.18) % (segAngle * 2);
                    for (let s = 0; s < numSegments; s++) {"""

new_ring_rot = """                    const numSegments = 10;
                    const segAngle = Math.PI / numSegments;
                    const rotOffset = 0; // Static habitat quadrants, zero rotating specular glints
                    for (let s = 0; s < numSegments; s++) {"""

assert old_ring_rot in content, "Could not find old_ring_rot in content"
content = content.replace(old_ring_rot, new_ring_rot, 1)

# 5. Course line dash moving offset removal
old_dash = """                    mapCtx.setLineDash([6, 6]);
                    mapCtx.lineDashOffset = -animFrameTime * 25;"""

new_dash = """                    mapCtx.setLineDash([4, 4]); // Clean stationary tactical vector line, zero traveling border dots"""

assert old_dash in content, "Could not find old_dash in content"
content = content.replace(old_dash, new_dash, 1)

# 6. Roaming Fleets on Map Canvas
old_fleets = """                if (flt.type === 'freighter') {
                    // Trader Bulk Hauler: Modular cargo containers & amber thrusters
                    mapCtx.fillStyle = '#ff8800';
                    mapCtx.beginPath();
                    mapCtx.moveTo(-10, -3); mapCtx.lineTo(-15 - Math.random() * 4, 0); mapCtx.lineTo(-10, 3);
                    mapCtx.fill();
                    // Cargo pods
                    mapCtx.fillStyle = '#d48800';
                    mapCtx.fillRect(-8, -5, 12, 10);
                    mapCtx.strokeStyle = '#ffaa00';
                    mapCtx.lineWidth = 1;
                    mapCtx.strokeRect(-8, -5, 12, 10);
                    // Bridge module
                    mapCtx.fillStyle = '#ffffff';
                    mapCtx.fillRect(4, -2, 3, 4);
                } else if (flt.type === 'cruiser') {
                    // Federation Cruiser: Sleek arrow hull & blue hyperdrive trail
                    mapCtx.fillStyle = '#00ffff';
                    mapCtx.beginPath();
                    mapCtx.moveTo(-8, -2); mapCtx.lineTo(-16 - Math.random() * 5, 0); mapCtx.lineTo(-8, 2);
                    mapCtx.fill();
                    // Arrowhead command fuselage
                    mapCtx.fillStyle = '#005577';
                    mapCtx.strokeStyle = '#00ffff';
                    mapCtx.lineWidth = 1.2;
                    mapCtx.beginPath();
                    mapCtx.moveTo(10, 0); mapCtx.lineTo(-6, -6); mapCtx.lineTo(-3, 0); mapCtx.lineTo(-6, 6);
                    mapCtx.closePath();
                    mapCtx.fill();
                    mapCtx.stroke();
                } else {
                    // Pirate Raider: Jagged swept-wing with crimson wake
                    mapCtx.fillStyle = '#ff3344';
                    mapCtx.beginPath();
                    mapCtx.moveTo(-6, -2); mapCtx.lineTo(-13 - Math.random() * 4, 0); mapCtx.lineTo(-6, 2);
                    mapCtx.fill();
                    // Razor wing dagger
                    mapCtx.fillStyle = '#440505';
                    mapCtx.strokeStyle = '#ff3344';
                    mapCtx.lineWidth = 1.2;
                    mapCtx.beginPath();
                    mapCtx.moveTo(8, 0); mapCtx.lineTo(-5, -5); mapCtx.lineTo(-7, 0); mapCtx.lineTo(-5, 5);
                    mapCtx.closePath();
                    mapCtx.fill();
                    mapCtx.stroke();
                }"""

new_fleets = """                if (flt.type === 'freighter') {
                    if (KSTELLAR_ASSETS.fleetTraders && KSTELLAR_ASSETS.fleetTraders.loaded) {
                        mapCtx.drawImage(KSTELLAR_ASSETS.fleetTraders.img, -10, -10, 20, 20);
                    } else {
                        // Trader Bulk Hauler fallback: Modular cargo containers & amber thrusters
                        mapCtx.fillStyle = '#ff8800';
                        mapCtx.beginPath();
                        mapCtx.moveTo(-10, -3); mapCtx.lineTo(-15 - Math.random() * 4, 0); mapCtx.lineTo(-10, 3);
                        mapCtx.fill();
                        mapCtx.fillStyle = '#d48800';
                        mapCtx.fillRect(-8, -5, 12, 10);
                        mapCtx.strokeStyle = '#ffaa00';
                        mapCtx.lineWidth = 1;
                        mapCtx.strokeRect(-8, -5, 12, 10);
                        mapCtx.fillStyle = '#ffffff';
                        mapCtx.fillRect(4, -2, 3, 4);
                    }
                } else if (flt.type === 'cruiser') {
                    if (KSTELLAR_ASSETS.fleetFed && KSTELLAR_ASSETS.fleetFed.loaded) {
                        mapCtx.drawImage(KSTELLAR_ASSETS.fleetFed.img, -10, -10, 20, 20);
                    } else {
                        // Federation Cruiser fallback: Sleek arrow hull & blue hyperdrive trail
                        mapCtx.fillStyle = '#00ffff';
                        mapCtx.beginPath();
                        mapCtx.moveTo(-8, -2); mapCtx.lineTo(-16 - Math.random() * 5, 0); mapCtx.lineTo(-8, 2);
                        mapCtx.fill();
                        mapCtx.fillStyle = '#005577';
                        mapCtx.strokeStyle = '#00ffff';
                        mapCtx.lineWidth = 1.2;
                        mapCtx.beginPath();
                        mapCtx.moveTo(10, 0); mapCtx.lineTo(-6, -6); mapCtx.lineTo(-3, 0); mapCtx.lineTo(-6, 6);
                        mapCtx.closePath();
                        mapCtx.fill();
                        mapCtx.stroke();
                    }
                } else {
                    if (KSTELLAR_ASSETS.fleetPirates && KSTELLAR_ASSETS.fleetPirates.loaded) {
                        mapCtx.drawImage(KSTELLAR_ASSETS.fleetPirates.img, -10, -10, 20, 20);
                    } else {
                        // Pirate Raider fallback: Jagged swept-wing with crimson wake
                        mapCtx.fillStyle = '#ff3344';
                        mapCtx.beginPath();
                        mapCtx.moveTo(-6, -2); mapCtx.lineTo(-13 - Math.random() * 4, 0); mapCtx.lineTo(-6, 2);
                        mapCtx.fill();
                        mapCtx.fillStyle = '#440505';
                        mapCtx.strokeStyle = '#ff3344';
                        mapCtx.lineWidth = 1.2;
                        mapCtx.beginPath();
                        mapCtx.moveTo(8, 0); mapCtx.lineTo(-5, -5); mapCtx.lineTo(-7, 0); mapCtx.lineTo(-5, 5);
                        mapCtx.closePath();
                        mapCtx.fill();
                        mapCtx.stroke();
                    }
                }"""

assert old_fleets in content, "Could not find old_fleets in content"
content = content.replace(old_fleets, new_fleets, 1)

# 7. Player Command Flagship Icon on Map Canvas
old_player_icon = """                // 4X Command Flagship Sprite
                mapCtx.save();
                mapCtx.translate(px, py - 20);
                const bob = Math.sin(animFrameTime * 5) * 2;
                mapCtx.translate(0, bob);

                // Dual Ion Engine Plasma Exhaust
                mapCtx.fillStyle = '#00ffff';
                const flm = 8 + Math.random() * 5;
                mapCtx.beginPath();
                mapCtx.moveTo(-4, 7); mapCtx.lineTo(-4, 7 + flm); mapCtx.lineTo(-2, 7);
                mapCtx.moveTo(2, 7); mapCtx.lineTo(4, 7 + flm); mapCtx.lineTo(4, 7);
                mapCtx.fill();

                // Armored Command Battlecruiser Chassis
                mapCtx.fillStyle = '#002838';
                mapCtx.strokeStyle = '#00ffcc';
                mapCtx.lineWidth = 1.4;
                mapCtx.beginPath();
                mapCtx.moveTo(0, -9);       // Sensor prow
                mapCtx.lineTo(6, -2);      // Starboard shoulder
                mapCtx.lineTo(8, 5);       // Starboard warp nacelle
                mapCtx.lineTo(4, 7);       // Starboard engine block
                mapCtx.lineTo(0, 4);       // Central keel
                mapCtx.lineTo(-4, 7);      // Port engine block
                mapCtx.lineTo(-8, 5);      // Port warp nacelle
                mapCtx.lineTo(-6, -2);     // Port shoulder
                mapCtx.closePath();
                mapCtx.fill();
                mapCtx.stroke();

                // Command Bridge Superstructure
                mapCtx.fillStyle = '#00ffff';
                mapCtx.beginPath();
                mapCtx.ellipse(0, -1, 2, 4, 0, 0, Math.PI * 2);
                mapCtx.fill();

                // Gold Command Chevron
                mapCtx.fillStyle = '#ffcc00';
                mapCtx.beginPath();
                mapCtx.moveTo(0, 1); mapCtx.lineTo(3, 3); mapCtx.lineTo(0, 2); mapCtx.lineTo(-3, 3);
                mapCtx.closePath();
                mapCtx.fill();

                mapCtx.restore();"""

new_player_icon = """                // 4X Command Flagship Sprite
                mapCtx.save();
                mapCtx.translate(px, py - 20);
                const bob = Math.sin(animFrameTime * 5) * 2;
                mapCtx.translate(0, bob);

                if (KSTELLAR_ASSETS.playerCruiser && KSTELLAR_ASSETS.playerCruiser.loaded) {
                    mapCtx.save();
                    mapCtx.rotate(-Math.PI / 2); // Rotate facing upwards for galactic map
                    mapCtx.drawImage(KSTELLAR_ASSETS.playerCruiser.img, -12, -12, 24, 24);
                    mapCtx.restore();
                } else {
                    // Dual Ion Engine Plasma Exhaust fallback
                    mapCtx.fillStyle = '#00ffff';
                    const flm = 8 + Math.random() * 5;
                    mapCtx.beginPath();
                    mapCtx.moveTo(-4, 7); mapCtx.lineTo(-4, 7 + flm); mapCtx.lineTo(-2, 7);
                    mapCtx.moveTo(2, 7); mapCtx.lineTo(4, 7 + flm); mapCtx.lineTo(4, 7);
                    mapCtx.fill();

                    // Armored Command Battlecruiser Chassis
                    mapCtx.fillStyle = '#002838';
                    mapCtx.strokeStyle = '#00ffcc';
                    mapCtx.lineWidth = 1.4;
                    mapCtx.beginPath();
                    mapCtx.moveTo(0, -9);       // Sensor prow
                    mapCtx.lineTo(6, -2);      // Starboard shoulder
                    mapCtx.lineTo(8, 5);       // Starboard warp nacelle
                    mapCtx.lineTo(4, 7);       // Starboard engine block
                    mapCtx.lineTo(0, 4);       // Central keel
                    mapCtx.lineTo(-4, 7);      // Port engine block
                    mapCtx.lineTo(-8, 5);      // Port warp nacelle
                    mapCtx.lineTo(-6, -2);     // Port shoulder
                    mapCtx.closePath();
                    mapCtx.fill();
                    mapCtx.stroke();

                    // Command Bridge Superstructure
                    mapCtx.fillStyle = '#00ffff';
                    mapCtx.beginPath();
                    mapCtx.ellipse(0, -1, 2, 4, 0, 0, Math.PI * 2);
                    mapCtx.fill();

                    // Gold Command Chevron
                    mapCtx.fillStyle = '#ffcc00';
                    mapCtx.beginPath();
                    mapCtx.moveTo(0, 1); mapCtx.lineTo(3, 3); mapCtx.lineTo(0, 2); mapCtx.lineTo(-3, 3);
                    mapCtx.closePath();
                    mapCtx.fill();
                }

                mapCtx.restore();"""

assert old_player_icon in content, "Could not find old_player_icon in content"
content = content.replace(old_player_icon, new_player_icon, 1)

# 8. Combat Canvas: Player Ship and Enemy Ships
old_combat_player = """                // Hull Flash on Damage
                if (playerShipState.flash > 0) {
                    cCtx.fillStyle = '#ffffff';
                    playerShipState.flash--;
                } else {
                    cCtx.fillStyle = '#00ffcc';
                }

                // Player Cruiser Hull
                cCtx.beginPath();
                cCtx.moveTo(35, 0);       // Nose
                cCtx.lineTo(10, -16);     // Upper wing
                cCtx.lineTo(-25, -22);    // Upper tip
                cCtx.lineTo(-20, -8);     // Engine notch
                cCtx.lineTo(-28, -8);     // Engine block
                cCtx.lineTo(-28, 8);      // Engine block
                cCtx.lineTo(-20, 8);      // Lower notch
                cCtx.lineTo(-25, 22);     // Lower tip
                cCtx.lineTo(10, 16);      // Lower wing
                cCtx.closePath();
                cCtx.fill();

                // Cockpit Canopy
                cCtx.fillStyle = '#002244';
                cCtx.strokeStyle = '#00ffff';
                cCtx.lineWidth = 1.5;
                cCtx.beginPath();
                cCtx.ellipse(5, 0, 12, 5, 0, 0, Math.PI * 2);
                cCtx.fill();
                cCtx.stroke();

                // Wingtip Laser Mounts (Weapon Upgrade)
                if (weaponLevel > 1) {
                    cCtx.strokeStyle = '#00ffff';
                    cCtx.lineWidth = 2;
                    cCtx.beginPath();
                    cCtx.moveTo(-15, -20); cCtx.lineTo(8, -20);
                    cCtx.moveTo(-15, 20); cCtx.lineTo(8, 20);
                    cCtx.stroke();
                }

                // Center Afterburner Plume (Engine Upgrade)
                if (engineLevel > 1) {
                    const cFlmLen = 16 + Math.random() * 8;
                    cCtx.fillStyle = '#ffaa00';
                    cCtx.beginPath();
                    cCtx.moveTo(-28, -2);
                    cCtx.lineTo(-28 - cFlmLen, 0);
                    cCtx.lineTo(-28, 2);
                    cCtx.fill();
                }"""

new_combat_player = """                if (KSTELLAR_ASSETS.playerCruiser && KSTELLAR_ASSETS.playerCruiser.loaded) {
                    // Render high-fidelity Imagen 3 chroma-keyed Player Battlecruiser sprite
                    if (playerShipState.flash > 0) {
                        cCtx.filter = 'brightness(2.2)';
                        playerShipState.flash--;
                    }
                    cCtx.drawImage(KSTELLAR_ASSETS.playerCruiser.img, -36, -36, 72, 72);
                    cCtx.filter = 'none';

                    // Upgraded laser hardpoint mounts
                    if (weaponLevel > 1) {
                        cCtx.strokeStyle = '#00ffff';
                        cCtx.lineWidth = 2;
                        cCtx.beginPath();
                        cCtx.moveTo(-10, -22); cCtx.lineTo(14, -22);
                        cCtx.moveTo(-10, 22); cCtx.lineTo(14, 22);
                        cCtx.stroke();
                    }
                    // Upgraded center afterburner plume
                    if (engineLevel > 1) {
                        const cFlmLen = 16 + Math.random() * 8;
                        cCtx.fillStyle = '#ffaa00';
                        cCtx.beginPath();
                        cCtx.moveTo(-28, -2);
                        cCtx.lineTo(-28 - cFlmLen, 0);
                        cCtx.lineTo(-28, 2);
                        cCtx.fill();
                    }
                } else {
                    // Procedural Fallback Hull
                    if (playerShipState.flash > 0) {
                        cCtx.fillStyle = '#ffffff';
                        playerShipState.flash--;
                    } else {
                        cCtx.fillStyle = '#00ffcc';
                    }

                    cCtx.beginPath();
                    cCtx.moveTo(35, 0);       // Nose
                    cCtx.lineTo(10, -16);     // Upper wing
                    cCtx.lineTo(-25, -22);    // Upper tip
                    cCtx.lineTo(-20, -8);     // Engine notch
                    cCtx.lineTo(-28, -8);     // Engine block
                    cCtx.lineTo(-28, 8);      // Engine block
                    cCtx.lineTo(-20, 8);      // Lower notch
                    cCtx.lineTo(-25, 22);     // Lower tip
                    cCtx.lineTo(10, 16);      // Lower wing
                    cCtx.closePath();
                    cCtx.fill();

                    // Cockpit Canopy
                    cCtx.fillStyle = '#002244';
                    cCtx.strokeStyle = '#00ffff';
                    cCtx.lineWidth = 1.5;
                    cCtx.beginPath();
                    cCtx.ellipse(5, 0, 12, 5, 0, 0, Math.PI * 2);
                    cCtx.fill();
                    cCtx.stroke();

                    if (weaponLevel > 1) {
                        cCtx.strokeStyle = '#00ffff';
                        cCtx.lineWidth = 2;
                        cCtx.beginPath();
                        cCtx.moveTo(-15, -20); cCtx.lineTo(8, -20);
                        cCtx.moveTo(-15, 20); cCtx.lineTo(8, 20);
                        cCtx.stroke();
                    }

                    if (engineLevel > 1) {
                        const cFlmLen = 16 + Math.random() * 8;
                        cCtx.fillStyle = '#ffaa00';
                        cCtx.beginPath();
                        cCtx.moveTo(-28, -2);
                        cCtx.lineTo(-28 - cFlmLen, 0);
                        cCtx.lineTo(-28, 2);
                        cCtx.fill();
                    }
                }"""

assert old_combat_player in content, "Could not find old_combat_player in content"
content = content.replace(old_combat_player, new_combat_player, 1)

# 9. Combat Enemy Ships
old_combat_enemies = """                if (currentEnemyClass && currentEnemyClass.id === 'interceptor') {
                    // --- CORSAIR INTERCEPTOR SPRITE ---
                    const eFlmLen = 14 + Math.random() * 8;
                    cCtx.fillStyle = '#ff8800';
                    cCtx.beginPath();
                    cCtx.moveTo(20, -10); cCtx.lineTo(20 + eFlmLen, -10); cCtx.lineTo(20, -7); cCtx.fill();
                    cCtx.beginPath();
                    cCtx.moveTo(20, 7); cCtx.lineTo(20 + eFlmLen, 10); cCtx.lineTo(20, 10); cCtx.fill();

                    if (enemyShipState.flash <= 0) cCtx.fillStyle = '#cc4422';
                    cCtx.beginPath();
                    cCtx.moveTo(-38, 0);      // Needle nose
                    cCtx.lineTo(-12, -8);     // Front taper
                    cCtx.lineTo(16, -20);     // Upper swept wing
                    cCtx.lineTo(10, -5);      // Inner notch
                    cCtx.lineTo(20, -12);     // Upper engine housing
                    cCtx.lineTo(18, 0);       // Rear core
                    cCtx.lineTo(20, 12);      // Lower engine housing
                    cCtx.lineTo(10, 5);       // Inner notch
                    cCtx.lineTo(16, 20);      // Lower swept wing
                    cCtx.lineTo(-12, 8);      // Front taper
                    cCtx.closePath();
                    cCtx.fill();

                    cCtx.fillStyle = '#ffcc00';
                    cCtx.shadowColor = '#ffaa00';
                    cCtx.shadowBlur = 6;
                    cCtx.fillRect(-18, -2, 8, 4);

                } else if (currentEnemyClass && currentEnemyClass.id === 'dreadnought') {
                    // --- SYNDICATE DREADNOUGHT SPRITE ---
                    const eFlmLen = 16 + Math.random() * 9;
                    cCtx.fillStyle = '#ff2200';
                    cCtx.beginPath();
                    cCtx.moveTo(28, -12); cCtx.lineTo(28 + eFlmLen, -12); cCtx.lineTo(28, -8); cCtx.fill();
                    cCtx.beginPath();
                    cCtx.moveTo(30, -3); cCtx.lineTo(30 + eFlmLen * 1.2, 0); cCtx.lineTo(30, 3); cCtx.fill();
                    cCtx.beginPath();
                    cCtx.moveTo(28, 8); cCtx.lineTo(28 + eFlmLen, 12); cCtx.lineTo(28, 12); cCtx.fill();

                    if (enemyShipState.flash <= 0) cCtx.fillStyle = '#771111';
                    cCtx.beginPath();
                    cCtx.moveTo(-42, -8);     // Heavy prow top
                    cCtx.lineTo(-42, 8);      // Heavy prow bottom
                    cCtx.lineTo(-20, 22);     // Forward armor slab
                    cCtx.lineTo(5, 30);       // Heavy broadside flank
                    cCtx.lineTo(25, 20);      // Aft armor block
                    cCtx.lineTo(28, 6);       // Engine casing
                    cCtx.lineTo(28, -6);      // Engine casing
                    cCtx.lineTo(25, -20);     // Aft armor block
                    cCtx.lineTo(5, -30);      // Heavy broadside flank
                    cCtx.lineTo(-20, -22);    // Forward armor slab
                    cCtx.closePath();
                    cCtx.fill();

                    cCtx.fillStyle = '#440808';
                    cCtx.fillRect(-5, -12, 22, 24);

                    cCtx.fillStyle = '#ff0033';
                    cCtx.shadowColor = '#ff0000';
                    cCtx.shadowBlur = 10;
                    cCtx.fillRect(-22, -6, 12, 3);
                    cCtx.fillRect(-22, 3, 12, 3);

                } else if (currentEnemyClass && currentEnemyClass.id === 'infiltrator') {
                    // --- SYNDICATE STEALTH INFILTRATOR SPRITE ---
                    const eFlmLen = 15 + Math.random() * 8;
                    cCtx.fillStyle = '#aa00ff';
                    cCtx.beginPath();
                    cCtx.moveTo(22, 0); cCtx.lineTo(22 + eFlmLen, 0); cCtx.lineTo(22, 2); cCtx.fill();

                    if (enemyShipState.flash <= 0) cCtx.fillStyle = '#551188';
                    // Sleek dagger wing stealth fuselage
                    cCtx.beginPath();
                    cCtx.moveTo(-36, 0);      // Dagger nose
                    cCtx.lineTo(-10, -6);     // Upper chine
                    cCtx.lineTo(18, -26);     // Swept stealth wing
                    cCtx.lineTo(8, -12);      // Trailing notch
                    cCtx.lineTo(22, -6);      // Aft nacelle
                    cCtx.lineTo(16, 0);       // Center keel
                    cCtx.lineTo(22, 6);       // Lower nacelle
                    cCtx.lineTo(8, 12);       // Trailing notch
                    cCtx.lineTo(18, 26);      // Lower swept wing
                    cCtx.lineTo(-10, 6);      // Lower chine
                    cCtx.closePath();
                    cCtx.fill();

                    cCtx.strokeStyle = '#cc66ff';
                    cCtx.lineWidth = 1.5;
                    cCtx.stroke();

                    // Violet sensor slit
                    cCtx.fillStyle = '#ff00ff';
                    cCtx.shadowColor = '#ff00ff';
                    cCtx.shadowBlur = 8;
                    cCtx.fillRect(-16, -2, 10, 4);

                } else {
                    // --- CORSAIR MARAUDER SPRITE (Standard) ---
                    const eFlmLen = 12 + Math.random() * 7;
                    cCtx.fillStyle = '#ff2200';
                    cCtx.beginPath();
                    cCtx.moveTo(25, -5);
                    cCtx.lineTo(25 + eFlmLen, 0);
                    cCtx.lineTo(25, 5);
                    cCtx.fill();

                    cCtx.beginPath();
                    cCtx.moveTo(-35, 0);      // Front ramming blade
                    cCtx.lineTo(-10, -18);    // Upper fang
                    cCtx.lineTo(15, -25);     // Wing spire
                    cCtx.lineTo(5, -8);       // Wing groove
                    cCtx.lineTo(25, -10);     // Rear fin
                    cCtx.lineTo(20, 0);       // Rear core
                    cCtx.lineTo(25, 10);      // Rear fin
                    cCtx.lineTo(5, 8);        // Wing groove
                    cCtx.lineTo(15, 25);      // Wing spire
                    cCtx.lineTo(-10, 18);     // Lower fang
                    cCtx.closePath();
                    cCtx.fill();

                    cCtx.fillStyle = '#ff0000';
                    cCtx.shadowColor = '#ff0000';
                    cCtx.shadowBlur = 8;
                    cCtx.fillRect(-15, -3, 10, 6);
                }"""

new_combat_enemies = """                if (currentEnemyClass && currentEnemyClass.id === 'interceptor') {
                    // --- CORSAIR INTERCEPTOR ---
                    if (KSTELLAR_ASSETS.interceptor && KSTELLAR_ASSETS.interceptor.loaded) {
                        if (enemyShipState.flash > 0) {
                            cCtx.filter = 'brightness(2.2)';
                            enemyShipState.flash--;
                        }
                        cCtx.drawImage(KSTELLAR_ASSETS.interceptor.img, -36, -36, 72, 72);
                        cCtx.filter = 'none';
                        const eFlmLen = 14 + Math.random() * 8;
                        cCtx.fillStyle = '#ff8800';
                        cCtx.beginPath();
                        cCtx.moveTo(20, -10); cCtx.lineTo(20 + eFlmLen, -10); cCtx.lineTo(20, -7); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(20, 7); cCtx.lineTo(20 + eFlmLen, 10); cCtx.lineTo(20, 10); cCtx.fill();
                    } else {
                        // Fallback procedural interceptor
                        const eFlmLen = 14 + Math.random() * 8;
                        cCtx.fillStyle = '#ff8800';
                        cCtx.beginPath();
                        cCtx.moveTo(20, -10); cCtx.lineTo(20 + eFlmLen, -10); cCtx.lineTo(20, -7); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(20, 7); cCtx.lineTo(20 + eFlmLen, 10); cCtx.lineTo(20, 10); cCtx.fill();

                        if (enemyShipState.flash <= 0) cCtx.fillStyle = '#cc4422';
                        cCtx.beginPath();
                        cCtx.moveTo(-38, 0);
                        cCtx.lineTo(-12, -8);
                        cCtx.lineTo(16, -20);
                        cCtx.lineTo(10, -5);
                        cCtx.lineTo(20, -12);
                        cCtx.lineTo(18, 0);
                        cCtx.lineTo(20, 12);
                        cCtx.lineTo(10, 5);
                        cCtx.lineTo(16, 20);
                        cCtx.lineTo(-12, 8);
                        cCtx.closePath();
                        cCtx.fill();

                        cCtx.fillStyle = '#ffcc00';
                        cCtx.shadowColor = '#ffaa00';
                        cCtx.shadowBlur = 6;
                        cCtx.fillRect(-18, -2, 8, 4);
                    }

                } else if (currentEnemyClass && currentEnemyClass.id === 'dreadnought') {
                    // --- SYNDICATE DREADNOUGHT ---
                    if (KSTELLAR_ASSETS.dreadnought && KSTELLAR_ASSETS.dreadnought.loaded) {
                        if (enemyShipState.flash > 0) {
                            cCtx.filter = 'brightness(2.2)';
                            enemyShipState.flash--;
                        }
                        cCtx.drawImage(KSTELLAR_ASSETS.dreadnought.img, -44, -44, 88, 88);
                        cCtx.filter = 'none';
                        const eFlmLen = 16 + Math.random() * 9;
                        cCtx.fillStyle = '#ff2200';
                        cCtx.beginPath();
                        cCtx.moveTo(28, -12); cCtx.lineTo(28 + eFlmLen, -12); cCtx.lineTo(28, -8); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(30, -3); cCtx.lineTo(30 + eFlmLen * 1.2, 0); cCtx.lineTo(30, 3); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(28, 8); cCtx.lineTo(28 + eFlmLen, 12); cCtx.lineTo(28, 12); cCtx.fill();
                    } else {
                        // Fallback procedural dreadnought
                        const eFlmLen = 16 + Math.random() * 9;
                        cCtx.fillStyle = '#ff2200';
                        cCtx.beginPath();
                        cCtx.moveTo(28, -12); cCtx.lineTo(28 + eFlmLen, -12); cCtx.lineTo(28, -8); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(30, -3); cCtx.lineTo(30 + eFlmLen * 1.2, 0); cCtx.lineTo(30, 3); cCtx.fill();
                        cCtx.beginPath();
                        cCtx.moveTo(28, 8); cCtx.lineTo(28 + eFlmLen, 12); cCtx.lineTo(28, 12); cCtx.fill();

                        if (enemyShipState.flash <= 0) cCtx.fillStyle = '#771111';
                        cCtx.beginPath();
                        cCtx.moveTo(-42, -8);
                        cCtx.lineTo(-42, 8);
                        cCtx.lineTo(-20, 22);
                        cCtx.lineTo(5, 30);
                        cCtx.lineTo(25, 20);
                        cCtx.lineTo(28, 6);
                        cCtx.lineTo(28, -6);
                        cCtx.lineTo(25, -20);
                        cCtx.lineTo(5, -30);
                        cCtx.lineTo(-20, -22);
                        cCtx.closePath();
                        cCtx.fill();

                        cCtx.fillStyle = '#440808';
                        cCtx.fillRect(-5, -12, 22, 24);

                        cCtx.fillStyle = '#ff0033';
                        cCtx.shadowColor = '#ff0000';
                        cCtx.shadowBlur = 10;
                        cCtx.fillRect(-22, -6, 12, 3);
                        cCtx.fillRect(-22, 3, 12, 3);
                    }

                } else if (currentEnemyClass && currentEnemyClass.id === 'infiltrator') {
                    // --- SYNDICATE STEALTH INFILTRATOR ---
                    if (KSTELLAR_ASSETS.infiltrator && KSTELLAR_ASSETS.infiltrator.loaded) {
                        if (enemyShipState.flash > 0) {
                            cCtx.filter = 'brightness(2.2)';
                            enemyShipState.flash--;
                        }
                        cCtx.drawImage(KSTELLAR_ASSETS.infiltrator.img, -36, -36, 72, 72);
                        cCtx.filter = 'none';
                        const eFlmLen = 15 + Math.random() * 8;
                        cCtx.fillStyle = '#aa00ff';
                        cCtx.beginPath();
                        cCtx.moveTo(22, 0); cCtx.lineTo(22 + eFlmLen, 0); cCtx.lineTo(22, 2); cCtx.fill();
                    } else {
                        // Fallback procedural infiltrator
                        const eFlmLen = 15 + Math.random() * 8;
                        cCtx.fillStyle = '#aa00ff';
                        cCtx.beginPath();
                        cCtx.moveTo(22, 0); cCtx.lineTo(22 + eFlmLen, 0); cCtx.lineTo(22, 2); cCtx.fill();

                        if (enemyShipState.flash <= 0) cCtx.fillStyle = '#551188';
                        cCtx.beginPath();
                        cCtx.moveTo(-36, 0);
                        cCtx.lineTo(-10, -6);
                        cCtx.lineTo(18, -26);
                        cCtx.lineTo(8, -12);
                        cCtx.lineTo(22, -6);
                        cCtx.lineTo(16, 0);
                        cCtx.lineTo(22, 6);
                        cCtx.lineTo(8, 12);
                        cCtx.lineTo(18, 26);
                        cCtx.lineTo(-10, 6);
                        cCtx.closePath();
                        cCtx.fill();

                        cCtx.strokeStyle = '#cc66ff';
                        cCtx.lineWidth = 1.5;
                        cCtx.stroke();

                        cCtx.fillStyle = '#ff00ff';
                        cCtx.shadowColor = '#ff00ff';
                        cCtx.shadowBlur = 8;
                        cCtx.fillRect(-16, -2, 10, 4);
                    }

                } else {
                    // --- CORSAIR MARAUDER ---
                    if (KSTELLAR_ASSETS.marauder && KSTELLAR_ASSETS.marauder.loaded) {
                        if (enemyShipState.flash > 0) {
                            cCtx.filter = 'brightness(2.2)';
                            enemyShipState.flash--;
                        }
                        cCtx.drawImage(KSTELLAR_ASSETS.marauder.img, -36, -36, 72, 72);
                        cCtx.filter = 'none';
                        const eFlmLen = 12 + Math.random() * 7;
                        cCtx.fillStyle = '#ff2200';
                        cCtx.beginPath();
                        cCtx.moveTo(25, -5);
                        cCtx.lineTo(25 + eFlmLen, 0);
                        cCtx.lineTo(25, 5);
                        cCtx.fill();
                    } else {
                        // Fallback procedural marauder
                        const eFlmLen = 12 + Math.random() * 7;
                        cCtx.fillStyle = '#ff2200';
                        cCtx.beginPath();
                        cCtx.moveTo(25, -5);
                        cCtx.lineTo(25 + eFlmLen, 0);
                        cCtx.lineTo(25, 5);
                        cCtx.fill();

                        cCtx.beginPath();
                        cCtx.moveTo(-35, 0);
                        cCtx.lineTo(-10, -18);
                        cCtx.lineTo(15, -25);
                        cCtx.lineTo(5, -8);
                        cCtx.lineTo(25, -10);
                        cCtx.lineTo(20, 0);
                        cCtx.lineTo(25, 10);
                        cCtx.lineTo(5, 8);
                        cCtx.lineTo(15, 25);
                        cCtx.lineTo(-10, 18);
                        cCtx.closePath();
                        cCtx.fill();

                        cCtx.fillStyle = '#ff0000';
                        cCtx.shadowColor = '#ff0000';
                        cCtx.shadowBlur = 8;
                        cCtx.fillRect(-15, -3, 10, 6);
                    }
                }"""

assert old_combat_enemies in content, "Could not find old_combat_enemies in content"
content = content.replace(old_combat_enemies, new_combat_enemies, 1)

HTML_PATH.write_text(content, encoding="utf-8")
print("kstellar.html successfully updated with high-fidelity graphics and glint sweep.")
