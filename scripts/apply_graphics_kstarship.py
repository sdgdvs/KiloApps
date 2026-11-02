"""
Apply graphics overhaul to KiloOS/public/apps/kstarship.html and KStarship/main.c:
1. Starship bridge & sector tactical display upgrade
2. Detailed hull subsystem schematics (Imagen 3 generated blueprint cutaway)
3. Nebula parallax backdrop (Deep cosmic dust clouds & ion filaments)
4. Enemy cruiser silhouettes (Corsair, Dreadnought, Syndicate Frigate, Xenon Hunter, Leviathan)
5. Player flagship sprite integration
6. Dual Win32 C parity update
7. Perimeter glint sweep verification
"""

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML_PATH = ROOT / "KiloOS" / "public" / "apps" / "kstarship.html"
C_PATH = ROOT / "KStarship" / "main.c"

# --- 1. UPDATE HTML ---
html = HTML_PATH.read_text(encoding="utf-8")

# Insert KSTARSHIP_ASSETS right after hCtx declaration
target_hctx = "const hCtx = hullCanvas ? hullCanvas.getContext('2d') : null;"
assert target_hctx in html, "Could not find hCtx declaration in kstarship.html"

asset_loader = """const hCtx = hullCanvas ? hullCanvas.getContext('2d') : null;

    // --- Starship Fleet & Sector Tactical Sprite Assets (Chroma-keyed 2-stage Imagen 3 pipeline) ---
    const KSTARSHIP_ASSETS = {
      hullSchematic: { img: null, loaded: false, src: '../assets/sprites/kstarship/hull_schematic.png', fallback: 'assets/sprites/kstarship/hull_schematic.png' },
      nebulaBackdrop: { img: null, loaded: false, src: '../assets/sprites/kstarship/nebula_backdrop.png', fallback: 'assets/sprites/kstarship/nebula_backdrop.png' },
      cruiserCorsair: { img: null, loaded: false, src: '../assets/sprites/kstarship/cruiser_corsair.png', fallback: 'assets/sprites/kstarship/cruiser_corsair.png' },
      cruiserDreadnought: { img: null, loaded: false, src: '../assets/sprites/kstarship/cruiser_dreadnought.png', fallback: 'assets/sprites/kstarship/cruiser_dreadnought.png' },
      cruiserSyndicate: { img: null, loaded: false, src: '../assets/sprites/kstarship/cruiser_syndicate.png', fallback: 'assets/sprites/kstarship/cruiser_syndicate.png' },
      cruiserXenon: { img: null, loaded: false, src: '../assets/sprites/kstarship/cruiser_xenon.png', fallback: 'assets/sprites/kstarship/cruiser_xenon.png' },
      cruiserLeviathan: { img: null, loaded: false, src: '../assets/sprites/kstarship/cruiser_leviathan.png', fallback: 'assets/sprites/kstarship/cruiser_leviathan.png' },
      shipFlagship: { img: null, loaded: false, src: '../assets/sprites/kstarship/ship_flagship.png', fallback: 'assets/sprites/kstarship/ship_flagship.png' }
    };

    function initKStarshipAssets() {
      Object.values(KSTARSHIP_ASSETS).forEach(entry => {
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
    initKStarshipAssets();"""

html = html.replace(target_hctx, asset_loader, 1)

# Now update drawHullSchematic:
old_hull_schematic = """      // 2. Main Titanium Composite Hull Plates & Interior Bulkheads
      hCtx.fillStyle = 'rgba(10, 26, 48, 0.92)';
      hCtx.strokeStyle = hullColor;
      hCtx.lineWidth = 1.3;
      hCtx.beginPath();
      hCtx.moveTo(36, 0); // Prow sensor nose
      hCtx.lineTo(20, -10);
      hCtx.lineTo(4, -16);
      hCtx.lineTo(-4, -18); // Port wing tip
      hCtx.lineTo(-24, -20); // Port nacelle outer
      hCtx.lineTo(-32, -12); // Port engine aft
      hCtx.lineTo(-22, -6);
      hCtx.lineTo(-28, 0); // Center aft notch
      hCtx.lineTo(-22, 6);
      hCtx.lineTo(-32, 12); // Starboard engine aft
      hCtx.lineTo(-24, 20); // Starboard nacelle outer
      hCtx.lineTo(-4, 18); // Starboard wing tip
      hCtx.lineTo(4, 16);
      hCtx.lineTo(20, 10);
      hCtx.closePath();
      hCtx.fill();
      hCtx.stroke();"""

new_hull_schematic = """      // 2. Main Starship Hull Schematic (Imagen 3 Blueprint cutaway with procedural fallback)
      if (KSTARSHIP_ASSETS.hullSchematic && KSTARSHIP_ASSETS.hullSchematic.loaded && KSTARSHIP_ASSETS.hullSchematic.img) {
        hCtx.save();
        hCtx.globalAlpha = 0.95;
        const sw = 84;
        const sh = 56;
        hCtx.drawImage(KSTARSHIP_ASSETS.hullSchematic.img, -sw / 2, -sh / 2, sw, sh);
        hCtx.restore();
      } else {
        hCtx.fillStyle = 'rgba(10, 26, 48, 0.92)';
        hCtx.strokeStyle = hullColor;
        hCtx.lineWidth = 1.3;
        hCtx.beginPath();
        hCtx.moveTo(36, 0); // Prow sensor nose
        hCtx.lineTo(20, -10);
        hCtx.lineTo(4, -16);
        hCtx.lineTo(-4, -18); // Port wing tip
        hCtx.lineTo(-24, -20); // Port nacelle outer
        hCtx.lineTo(-32, -12); // Port engine aft
        hCtx.lineTo(-22, -6);
        hCtx.lineTo(-28, 0); // Center aft notch
        hCtx.lineTo(-22, 6);
        hCtx.lineTo(-32, 12); // Starboard engine aft
        hCtx.lineTo(-24, 20); // Starboard nacelle outer
        hCtx.lineTo(-4, 18); // Starboard wing tip
        hCtx.lineTo(4, 16);
        hCtx.lineTo(20, 10);
        hCtx.closePath();
        hCtx.fill();
        hCtx.stroke();
      }"""

assert old_hull_schematic in html, "Could not find old_hull_schematic in kstarship.html"
html = html.replace(old_hull_schematic, new_hull_schematic, 1)

# Update drawTacticalEnemyCruiser:
old_tac_cruiser_start = """    function drawTacticalEnemyCruiser(c, x, y, angle, type) {
      c.save();
      c.translate(x, y);
      c.rotate(angle);

      const time = Date.now();

      // Distinct enemy silhouettes per hostile faction
      if (type === 'Pirates') {"""

new_tac_cruiser_start = """    function drawTacticalEnemyCruiser(c, x, y, angle, type) {
      c.save();
      c.translate(x, y);
      c.rotate(angle);

      const time = Date.now();

      // Check for Imagen 3 cruiser silhouette sprite
      let cruiserSprite = null;
      let cW = 32, cH = 32;
      if (type === 'Pirates' && KSTARSHIP_ASSETS.cruiserCorsair.loaded) {
        cruiserSprite = KSTARSHIP_ASSETS.cruiserCorsair.img;
        cW = 34; cH = 34;
      } else if (type === 'Fleet' && KSTARSHIP_ASSETS.cruiserDreadnought.loaded) {
        cruiserSprite = KSTARSHIP_ASSETS.cruiserDreadnought.img;
        cW = 42; cH = 42;
      } else if (type === 'SyndicateFrigate' && KSTARSHIP_ASSETS.cruiserSyndicate.loaded) {
        cruiserSprite = KSTARSHIP_ASSETS.cruiserSyndicate.img;
        cW = 34; cH = 34;
      } else if (type === 'XenonHunter' && KSTARSHIP_ASSETS.cruiserXenon.loaded) {
        cruiserSprite = KSTARSHIP_ASSETS.cruiserXenon.img;
        cW = 36; cH = 36;
      } else if (KSTARSHIP_ASSETS.cruiserLeviathan.loaded) {
        cruiserSprite = KSTARSHIP_ASSETS.cruiserLeviathan.img;
        cW = 46; cH = 46;
      }

      if (cruiserSprite) {
        // High-definition cruiser silhouette sprite with drive plume
        c.drawImage(cruiserSprite, -cW / 2, -cH / 2, cW, cH);
        c.fillStyle = type === 'Pirates' ? '#ff4400' : (type === 'Fleet' ? '#38bdf8' : '#fbbf24');
        c.fillRect(-cW / 2 - 3, -1.5, 3, 3);
      } else if (type === 'Pirates') {"""

assert old_tac_cruiser_start in html, "Could not find old_tac_cruiser_start in kstarship.html"
html = html.replace(old_tac_cruiser_start, new_tac_cruiser_start, 1)

# Update drawPlayerStarship to use shipFlagship sprite if loaded:
old_player_ship = """    function drawPlayerStarship(c, x, y, angle, moving, upgrades, shieldAlpha) {
      c.save();
      c.translate(x, y);
      c.rotate(angle);

      const wLevel = upgrades.weapons;
      const sLevel = upgrades.shields;
      const eLevel = upgrades.engines;

      if (shieldAlpha > 0.05) {
        c.save();
        c.strokeStyle = `rgba(0, 240, 255, ${shieldAlpha * 0.7})`;
        c.fillStyle = `rgba(0, 200, 255, ${shieldAlpha * 0.15})`;
        c.lineWidth = 1.5;
        c.beginPath();
        for (let i = 0; i < 6; i++) {
          let sa = (i / 6) * Math.PI * 2;
          let sr = 24;
          let px = Math.cos(sa) * sr;
          let py = Math.sin(sa) * sr;
          if (i === 0) c.moveTo(px, py);
          else c.lineTo(px, py);
        }
        c.closePath();
        c.fill();
        c.stroke();
        c.restore();
      }

      c.save();
      c.shadowColor = 'rgba(0, 255, 255, 0.5)';
      c.shadowBlur = moving ? 12 : 6;

      if (moving) {
        c.save();
        const baseFl = 14 + (Math.sin(Date.now() * 0.03) * 4) + eLevel * 3;
        const engines = [{ ox: -5, oy: 12 }, { ox: 5, oy: 12 }];
        
        engines.forEach(eng => {
          let fGrad = c.createLinearGradient(eng.ox, eng.oy, eng.ox, eng.oy + baseFl);
          fGrad.addColorStop(0, '#ffffff');
          fGrad.addColorStop(0.2, '#00ffff');
          fGrad.addColorStop(0.6, '#0066ff');
          fGrad.addColorStop(1, 'rgba(0, 30, 200, 0)');
          
          c.fillStyle = fGrad;
          c.beginPath();
          c.moveTo(eng.ox - 3, eng.oy);
          c.lineTo(eng.ox + 3, eng.oy);
          c.lineTo(eng.ox, eng.oy + baseFl);
          c.closePath();
          c.fill();

          c.fillStyle = '#ffffff';
          c.beginPath();
          c.moveTo(eng.ox - 1.5, eng.oy);
          c.lineTo(eng.ox + 1.5, eng.oy);
          c.lineTo(eng.ox, eng.oy + baseFl * 0.5);
          c.closePath();
          c.fill();
        });
        c.restore();
      }

      let wingGrad = c.createLinearGradient(-16, -10, 16, 16);"""

new_player_ship = """    function drawPlayerStarship(c, x, y, angle, moving, upgrades, shieldAlpha) {
      c.save();
      c.translate(x, y);
      c.rotate(angle);

      const wLevel = upgrades.weapons;
      const sLevel = upgrades.shields;
      const eLevel = upgrades.engines;

      if (shieldAlpha > 0.05) {
        c.save();
        c.strokeStyle = `rgba(0, 240, 255, ${shieldAlpha * 0.7})`;
        c.fillStyle = `rgba(0, 200, 255, ${shieldAlpha * 0.15})`;
        c.lineWidth = 1.5;
        c.beginPath();
        for (let i = 0; i < 6; i++) {
          let sa = (i / 6) * Math.PI * 2;
          let sr = 24;
          let px = Math.cos(sa) * sr;
          let py = Math.sin(sa) * sr;
          if (i === 0) c.moveTo(px, py);
          else c.lineTo(px, py);
        }
        c.closePath();
        c.fill();
        c.stroke();
        c.restore();
      }

      c.save();
      c.shadowColor = 'rgba(0, 255, 255, 0.5)';
      c.shadowBlur = moving ? 12 : 6;

      if (moving) {
        c.save();
        const baseFl = 14 + (Math.sin(Date.now() * 0.03) * 4) + eLevel * 3;
        const engines = [{ ox: -5, oy: 12 }, { ox: 5, oy: 12 }];
        
        engines.forEach(eng => {
          let fGrad = c.createLinearGradient(eng.ox, eng.oy, eng.ox, eng.oy + baseFl);
          fGrad.addColorStop(0, '#ffffff');
          fGrad.addColorStop(0.2, '#00ffff');
          fGrad.addColorStop(0.6, '#0066ff');
          fGrad.addColorStop(1, 'rgba(0, 30, 200, 0)');
          
          c.fillStyle = fGrad;
          c.beginPath();
          c.moveTo(eng.ox - 3, eng.oy);
          c.lineTo(eng.ox + 3, eng.oy);
          c.lineTo(eng.ox, eng.oy + baseFl);
          c.closePath();
          c.fill();

          c.fillStyle = '#ffffff';
          c.beginPath();
          c.moveTo(eng.ox - 1.5, eng.oy);
          c.lineTo(eng.ox + 1.5, eng.oy);
          c.lineTo(eng.ox, eng.oy + baseFl * 0.5);
          c.closePath();
          c.fill();
        });
        c.restore();
      }

      // Check for Flagship sprite asset
      if (KSTARSHIP_ASSETS.shipFlagship && KSTARSHIP_ASSETS.shipFlagship.loaded && KSTARSHIP_ASSETS.shipFlagship.img) {
        c.save();
        const fSize = 38 + (wLevel > 2 ? 4 : 0);
        c.drawImage(KSTARSHIP_ASSETS.shipFlagship.img, -fSize / 2, -fSize / 2, fSize, fSize);
        if (wLevel >= 2) {
          c.fillStyle = '#00ffff';
          c.fillRect(-fSize / 2 - 2, 0, 3, 6);
          c.fillRect(fSize / 2 - 1, 0, 3, 6);
        }
        c.restore();
        c.restore();
        c.restore();
        return;
      }

      let wingGrad = c.createLinearGradient(-16, -10, 16, 16);"""

assert old_player_ship in html, "Could not find old_player_ship in kstarship.html"
html = html.replace(old_player_ship, new_player_ship, 1)

# Update renderModalPreview to draw sprite preview if available:
old_modal_preview = """    function renderModalPreview(type) {
      pCtx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);
      const pw = previewCanvas.width;
      const ph = previewCanvas.height;
      const cx = pw / 2;
      const cy = ph / 2;
      const time = Date.now();

      pCtx.fillStyle = '#030510';
      pCtx.fillRect(0, 0, pw, ph);

      pCtx.fillStyle = 'rgba(255, 255, 255, 0.4)';
      for (let i=0; i<30; i++) {
        let sx = (Math.sin(i * 99) * 1000 + pw) % pw;
        let sy = (Math.cos(i * 77) * 1000 + ph) % ph;
        pCtx.fillRect(sx, sy, 1.5, 1.5);
      }

      if (type === 'Pirates') {"""

new_modal_preview = """    function renderModalPreview(type) {
      pCtx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);
      const pw = previewCanvas.width;
      const ph = previewCanvas.height;
      const cx = pw / 2;
      const cy = ph / 2;
      const time = Date.now();

      pCtx.fillStyle = '#030510';
      pCtx.fillRect(0, 0, pw, ph);

      pCtx.fillStyle = 'rgba(255, 255, 255, 0.4)';
      for (let i=0; i<30; i++) {
        let sx = (Math.sin(i * 99) * 1000 + pw) % pw;
        let sy = (Math.cos(i * 77) * 1000 + ph) % ph;
        pCtx.fillRect(sx, sy, 1.5, 1.5);
      }

      // Check for high-res sprite modal preview
      let mSprite = null;
      let mW = 72, mH = 72;
      if (type === 'Pirates' && KSTARSHIP_ASSETS.cruiserCorsair.loaded) {
        mSprite = KSTARSHIP_ASSETS.cruiserCorsair.img;
        mW = 80; mH = 80;
      } else if (type === 'Fleet' && KSTARSHIP_ASSETS.cruiserDreadnought.loaded) {
        mSprite = KSTARSHIP_ASSETS.cruiserDreadnought.img;
        mW = 96; mH = 96;
      } else if (type === 'SyndicateFrigate' && KSTARSHIP_ASSETS.cruiserSyndicate.loaded) {
        mSprite = KSTARSHIP_ASSETS.cruiserSyndicate.img;
        mW = 80; mH = 80;
      } else if (type === 'XenonHunter' && KSTARSHIP_ASSETS.cruiserXenon.loaded) {
        mSprite = KSTARSHIP_ASSETS.cruiserXenon.img;
        mW = 84; mH = 84;
      } else if (type === 'Leviathan' && KSTARSHIP_ASSETS.cruiserLeviathan.loaded) {
        mSprite = KSTARSHIP_ASSETS.cruiserLeviathan.img;
        mW = 100; mH = 100;
      }

      if (mSprite) {
        pCtx.save();
        const bob = Math.sin(time * 0.003) * 3;
        pCtx.translate(cx, cy + bob);
        // Energy aura behind sprite
        let aGrad = pCtx.createRadialGradient(0, 0, 10, 0, 0, mW * 0.65);
        aGrad.addColorStop(0, type === 'Pirates' ? 'rgba(255, 50, 0, 0.25)' : (type === 'Fleet' ? 'rgba(56, 189, 248, 0.25)' : 'rgba(245, 158, 11, 0.25)'));
        aGrad.addColorStop(1, 'rgba(0,0,0,0)');
        pCtx.fillStyle = aGrad;
        pCtx.fillRect(-mW, -mH, mW * 2, mH * 2);
        pCtx.drawImage(mSprite, -mW / 2, -mH / 2, mW, mH);
        pCtx.restore();
        return;
      }

      if (type === 'Pirates') {"""

assert old_modal_preview in html, "Could not find old_modal_preview in kstarship.html"
html = html.replace(old_modal_preview, new_modal_preview, 1)

# Update nebula rendering to include nebula_backdrop sprite
old_nebula = """      // Multi-Tier Parallax Volumetric Nebulae (Deep cosmic dust, multi-lobe emission clouds, & ionized filaments)
      state.nebulas.forEach(neb => {
        const pFactor = neb.parallax || 0.08;
        const screenX = centerX + (neb.x - state.ship.x * pFactor);
        const screenY = centerY + (neb.y - state.ship.y * pFactor);
        if (screenX > -neb.radius && screenX < canvas.width + neb.radius && screenY > -neb.radius && screenY < canvas.height + neb.radius) {"""

new_nebula = """      // Multi-Tier Parallax Volumetric Nebulae (Deep cosmic dust, multi-lobe emission clouds, & ionized filaments)
      if (KSTARSHIP_ASSETS.nebulaBackdrop && KSTARSHIP_ASSETS.nebulaBackdrop.loaded && KSTARSHIP_ASSETS.nebulaBackdrop.img) {
        ctx.save();
        ctx.globalCompositeOperation = 'screen';
        ctx.globalAlpha = 0.22;
        const nbSize = 256;
        for (let i = 0; i < 4; i++) {
          const nX = ((i * 450 - state.ship.x * 0.02) % (canvas.width + nbSize) + (canvas.width + nbSize)) % (canvas.width + nbSize) - nbSize / 2;
          const nY = (((i * 270 + 80) - state.ship.y * 0.02) % (canvas.height + nbSize) + (canvas.height + nbSize)) % (canvas.height + nbSize) - nbSize / 2;
          ctx.drawImage(KSTARSHIP_ASSETS.nebulaBackdrop.img, nX, nY, nbSize, nbSize);
        }
        ctx.restore();
      }

      state.nebulas.forEach(neb => {
        const pFactor = neb.parallax || 0.08;
        const screenX = centerX + (neb.x - state.ship.x * pFactor);
        const screenY = centerY + (neb.y - state.ship.y * pFactor);
        if (screenX > -neb.radius && screenX < canvas.width + neb.radius && screenY > -neb.radius && screenY < canvas.height + neb.radius) {"""

assert old_nebula in html, "Could not find old_nebula in kstarship.html"
html = html.replace(old_nebula, new_nebula, 1)

HTML_PATH.write_text(html, encoding="utf-8")
print(f"Updated HTML: {HTML_PATH.name} ({len(html)} bytes)")

# --- 2. UPDATE WIN32 C PARITY ---
c_code = C_PATH.read_text(encoding="utf-8")

# Enhance mini hull schematic in C_PATH
old_c_schematic = """    // Mini Hull Blueprint Drawing
    int scx = mapWidth + 48;
    int scy = height - 125;
    COLORREF hullColor = res_hull > 65 ? RGB(0, 255, 204) : (res_hull > 30 ? RGB(255, 170, 0) : RGB(255, 50, 60));
    HPEN bPen = CreatePen(PS_SOLID, 1, hullColor);
    HBRUSH bBrush = CreateSolidBrush(RGB(10, 26, 48));
    SelectObject(memDC, bPen);
    SelectObject(memDC, bBrush);
    POINT bpPts[8] = {
        {scx, scy - 20}, {scx + 12, scy - 4}, {scx + 22, scy + 12}, {scx + 16, scy + 18},
        {scx, scy + 14}, {scx - 16, scy + 18}, {scx - 22, scy + 12}, {scx - 12, scy - 4}
    };
    Polygon(memDC, bpPts, 8);
    DeleteObject(bPen);
    DeleteObject(bBrush);

    // Keel and Bulkheads
    HPEN kPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
    SelectObject(memDC, kPen);
    MoveToEx(memDC, scx, scy - 18, NULL); LineTo(memDC, scx, scy + 12);
    MoveToEx(memDC, scx - 12, scy + 2, NULL); LineTo(memDC, scx + 12, scy + 2);
    DeleteObject(kPen);

    // Core Reactor Node
    HBRUSH rBrush = CreateSolidBrush(RGB(0, 255, 255));
    SelectObject(memDC, rBrush);
    SelectObject(memDC, GetStockObject(NULL_PEN));
    Ellipse(memDC, scx - 3, scy - 4, scx + 3, scy + 2);
    DeleteObject(rBrush);"""

new_c_schematic = """    // Mini Hull Blueprint Drawing (Detailed Hull Subsystem Schematics with zero glints)
    int scx = mapWidth + 48;
    int scy = height - 125;
    COLORREF hullColor = res_hull > 65 ? RGB(0, 255, 204) : (res_hull > 30 ? RGB(255, 170, 0) : RGB(255, 50, 60));
    HPEN bPen = CreatePen(PS_SOLID, 1, hullColor);
    HBRUSH bBrush = CreateSolidBrush(RGB(10, 26, 48));
    SelectObject(memDC, bPen);
    SelectObject(memDC, bBrush);
    POINT bpPts[12] = {
        {scx, scy - 22}, {scx + 8, scy - 12}, {scx + 16, scy - 6}, {scx + 24, scy + 10},
        {scx + 18, scy + 18}, {scx + 8, scy + 14}, {scx, scy + 10},
        {scx - 8, scy + 14}, {scx - 18, scy + 18}, {scx - 24, scy + 10},
        {scx - 16, scy - 6}, {scx - 8, scy - 12}
    };
    Polygon(memDC, bpPts, 12);
    DeleteObject(bPen);
    DeleteObject(bBrush);

    // Keel, Transverse Bulkheads, & Conduit Busses
    HPEN kPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
    SelectObject(memDC, kPen);
    MoveToEx(memDC, scx, scy - 20, NULL); LineTo(memDC, scx, scy + 10);
    MoveToEx(memDC, scx - 14, scy - 2, NULL); LineTo(memDC, scx + 14, scy - 2);
    MoveToEx(memDC, scx - 10, scy + 6, NULL); LineTo(memDC, scx + 10, scy + 6);
    // Port & Starboard Warp Nacelle Brackets
    MoveToEx(memDC, scx - 18, scy + 8, NULL); LineTo(memDC, scx - 22, scy + 18);
    MoveToEx(memDC, scx + 18, scy + 8, NULL); LineTo(memDC, scx + 22, scy + 18);
    DeleteObject(kPen);

    // Forward Subspace Sensor Dome
    HBRUSH sensBrush = CreateSolidBrush(RGB(0, 255, 220));
    SelectObject(memDC, sensBrush);
    SelectObject(memDC, GetStockObject(NULL_PEN));
    Ellipse(memDC, scx - 2, scy - 22, scx + 2, scy - 18);
    DeleteObject(sensBrush);

    // Core Warp Reactor Node
    HBRUSH rBrush = CreateSolidBrush(res_fuel > 2000 ? RGB(0, 255, 255) : RGB(255, 140, 0));
    SelectObject(memDC, rBrush);
    SelectObject(memDC, GetStockObject(NULL_PEN));
    Ellipse(memDC, scx - 4, scy - 4, scx + 4, scy + 4);
    DeleteObject(rBrush);"""

assert old_c_schematic in c_code, "Could not find old_c_schematic in main.c"
c_code = c_code.replace(old_c_schematic, new_c_schematic, 1)

C_PATH.write_text(c_code, encoding="utf-8")
print(f"Updated C: {C_PATH.name} ({len(c_code)} bytes)")
print("Graphics integration script completed successfully!")
