import os
import re

PORTAL_PATH = "KiloOS/public/web/portal.html"

with open(PORTAL_PATH, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Add tab button
tab_target = '<div class="portal-tab" id="tabNav-y2k" onclick="switchPortalTab(\'y2k\')">🛡️ Y2K Readiness Lab</div>'
tab_replacement = (
    '<div class="portal-tab" id="tabNav-y2k" onclick="switchPortalTab(\'y2k\')">🛡️ Y2K Readiness Lab</div>\n'
    '      <div class="portal-tab" id="tabNav-auctions" onclick="switchPortalTab(\'auctions\')">🏛️ CyberBid Auctions \'99</div>'
)
if tab_target in content:
    content = content.replace(tab_target, tab_replacement, 1)
    print("1. Added tab navigation button")
else:
    print("Error: tab_target not found")

# 2. Add search filter chip
chip_target = '<a class="search-chip-link" onclick="quickFilter(\'Web\')">🌐 Web Destinations</a>'
chip_replacement = (
    '<a class="search-chip-link" onclick="quickFilter(\'Web\')">🌐 Web Destinations</a>\n'
    '        <a class="search-chip-link" onclick="switchPortalTab(\'auctions\')" style="color:#d97706; font-weight:bold;">🏛️ CyberBid \'99</a>'
)
if chip_target in content:
    content = content.replace(chip_target, chip_replacement, 1)
    print("2. Added search filter chip")
else:
    print("Error: chip_target not found")

# 3. Add CSS for CyberBid Auctions before </style>
css_target = "/* Retro Toast Notification */"
auction_css = """  /* CyberBid Auctions '99 */
  .auctions-hero {
    background: linear-gradient(180deg, #fff7ed 0%, #ffedd5 100%);
    border: 2px outset #f97316;
    padding: 10px 14px;
    margin-bottom: 12px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 8px;
  }
  .auctions-stats-bar {
    display: flex;
    gap: 12px;
    background: #431407;
    color: #ffedd5;
    padding: 6px 12px;
    border: 2px inset #9a3412;
    font-family: 'Courier New', monospace;
    font-size: 11px;
    font-weight: bold;
    flex-wrap: wrap;
    margin-bottom: 10px;
  }
  .auctions-nav-bar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 8px;
    background: #f1f5f9;
    border: 1px solid #cbd5e1;
    padding: 6px 10px;
    margin-bottom: 10px;
  }
  .auctions-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(290px, 1fr));
    gap: 12px;
    margin-bottom: 14px;
  }
  .auction-card {
    background: #ffffff;
    border: 2px outset #d4d0c8;
    box-shadow: 2px 2px 5px rgba(0,0,0,0.15);
    display: flex;
    flex-direction: column;
    padding: 10px;
    position: relative;
    font-family: Arial, sans-serif;
  }
  .auction-card.snipe-alert {
    border-color: #ef4444;
    animation: snipePulse 1.2s infinite;
  }
  @keyframes snipePulse {
    0%, 100% { box-shadow: 0 0 4px #ef4444; }
    50% { box-shadow: 0 0 12px #dc2626; }
  }
  .auction-badge {
    position: absolute;
    top: 6px;
    right: 6px;
    font-size: 9px;
    font-weight: 900;
    padding: 2px 6px;
    border-radius: 3px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }
  .badge-hot { background: #fee2e2; color: #dc2626; border: 1px solid #f87171; }
  .badge-ending { background: #fef3c7; color: #d97706; border: 1px solid #f59e0b; }
  .badge-reserve { background: #dcfce7; color: #15803d; border: 1px solid #86efac; }
  .badge-sold { background: #334155; color: #f8fafc; border: 1px solid #0f172a; }
  .auction-title {
    font-size: 13px;
    font-weight: bold;
    color: #000080;
    margin-bottom: 4px;
    padding-right: 60px;
    line-height: 1.3;
  }
  .auction-meta {
    font-size: 11px;
    color: #64748b;
    margin-bottom: 8px;
    display: flex;
    gap: 8px;
  }
  .auction-price-row {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
    background: #f8fafc;
    border: 1px solid #e2e8f0;
    padding: 6px 8px;
    margin-bottom: 8px;
  }
  .auction-bid-val {
    font-family: 'Courier New', monospace;
    font-size: 18px;
    font-weight: 900;
    color: #15803d;
  }
  .auction-timer {
    font-family: 'Courier New', monospace;
    font-size: 13px;
    font-weight: bold;
    color: #0369a1;
  }
  .auction-timer.urgent {
    color: #dc2626;
    font-weight: 900;
    animation: urgentBlink 0.8s infinite;
  }
  @keyframes urgentBlink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.4; }
  }
  .auction-bid-box {
    display: flex;
    gap: 4px;
    margin-top: auto;
  }
  .auction-bid-input {
    width: 65px;
    padding: 3px 5px;
    font-family: 'Courier New', monospace;
    font-size: 12px;
    font-weight: bold;
    border: 1px inset #94a3b8;
  }
  .auction-user-high {
    border-color: #16a34a !important;
    background: #f0fdf4 !important;
  }

  /* Retro Toast Notification */"""

if css_target in content:
    content = content.replace(css_target, auction_css, 1)
    print("3. Added CSS styles for CyberBid Auctions")
else:
    print("Error: css_target not found")

# 4. Add Section HTML after tab-y2k section
section_target = '    </div>\n  </section>\n\n  <!-- Webring Member Widget -->'
if section_target not in content:
    # Try CRLF
    section_target = '    </div>\r\n  </section>\r\n\r\n  <!-- Webring Member Widget -->'

auction_section_html = """    </div>
  </section>

  <!-- TAB 20: CYBERBID AUCTIONS '99 -->
  <section id="tab-auctions" class="tab-content">
    <div class="auctions-hero">
      <div>
        <h3 style="margin:0 0 2px 0; color:#c2410c; font-family:'Times New Roman', serif; font-size:20px;">
          🏛️ CyberBid Auctions '99 — Retro Silicon &amp; Hardware Bidding Exchange
        </h3>
        <p style="margin:0; font-size:12px; color:#7c2d12;">
          The premier 1999 hypermedia auction trading floor! Compete in real-time proxy bidding against global retro enthusiasts.
        </p>
      </div>
      <div style="display:flex; gap:6px; flex-wrap:wrap;">
        <button class="btn-retro" style="background:#ea580c; color:#ffffff; font-weight:bold;" onclick="openAuctionListingModal()">➕ List Your Item</button>
        <button class="btn-retro" onclick="showWonAuctionsModal()">🏆 My Won Lots (<span id="wonLotsCount">0</span>)</button>
        <button class="btn-retro" onclick="resetAuctionsDefaults()">🔄 Reset Lots</button>
      </div>
    </div>

    <!-- Live Telemetry / Exchange Ticker -->
    <div class="auctions-stats-bar">
      <div>LOTS ACTIVE: <span id="auctionActiveCount" style="color:#38bdf8;">12</span></div>
      <div>TOTAL BIDS: <span id="auctionTotalBids" style="color:#4ade80;">184</span></div>
      <div>TRADING VOLUME: <span id="auctionVolumeUSD" style="color:#facc15;">$2,488.40</span></div>
      <div>AUCTIONEER GAVEL: <span id="auctionStatusIndicator" style="color:#4ade80;">● ACTIVE BIDDING</span></div>
    </div>

    <!-- Category Filter Controls & Search -->
    <div class="auctions-nav-bar">
      <div style="display:flex; gap:6px; align-items:center; flex-wrap:wrap;">
        <span style="font-size:11px; font-weight:bold; color:#334155;">Filter Category:</span>
        <button class="btn-retro active" id="btnFilter-all" onclick="filterAuctions('all')">All Lots</button>
        <button class="btn-retro" id="btnFilter-hardware" onclick="filterAuctions('hardware')">🖥️ Hardware &amp; CPUs</button>
        <button class="btn-retro" id="btnFilter-audio" onclick="filterAuctions('audio')">🎹 Sound &amp; MIDI</button>
        <button class="btn-retro" id="btnFilter-consoles" onclick="filterAuctions('consoles')">🕹️ Consoles</button>
        <button class="btn-retro" id="btnFilter-storage" onclick="filterAuctions('storage')">💾 Storage</button>
        <button class="btn-retro" id="btnFilter-surplus" onclick="filterAuctions('surplus')">📡 Skunkworks Surplus</button>
      </div>
      <div style="display:flex; gap:4px; align-items:center;">
        <input type="text" id="auctionSearchInput" class="form-input" placeholder="Search lots (e.g. Voodoo, Slot 1, MIDI)..." oninput="handleAuctionSearch(this.value)" style="width:180px; padding:3px 6px; font-size:11px;" />
        <button class="btn-retro" onclick="clearAuctionSearch()">Clear</button>
      </div>
    </div>

    <!-- Active Auction Cards Grid -->
    <div class="auctions-grid" id="auctionsGridContainer">
      <!-- Populated via JavaScript -->
    </div>

    <!-- Auction House Rules & Buyer Guarantee -->
    <div style="background:#fefce8; border:1px solid #fef08a; padding:8px 12px; margin-top:10px; font-size:11px; color:#854d0e; line-height:1.4;">
      <strong>🛡️ CyberBid 1999 Buyer Protection Guarantee:</strong>
      All items are inspected for authentic 1999 silicon provenance. Bidding increments follow standard proxy escalation. Sniping within the final 30 seconds triggers auctioneer overtime warnings. Gavel strikes are final and recorded in local storage!
    </div>
  </section>

  <!-- Webring Member Widget -->"""

if section_target in content:
    content = content.replace(section_target, auction_section_html, 1)
    print("4. Added CyberBid Auctions HTML section")
else:
    print("Error: section_target not found")

# 5. Add CyberBid item to PORTAL_CATALOG
catalog_target = "  { id: 'web-webmail', title: 'KiloMail Web Express '99', folder: 'Web', url: '#tab-webmail', desc: 'Free Web 1.0 webmail inbox simulator: compose, check email, simulated SMTP dispatch & storage meter.', tags: 'webmail,email,mail,inbox,outbox,smtp,pop3,messages,compose,web' }"
catalog_replacement = (
    catalog_target + ",\n"
    "  { id: 'web-auctions', title: 'CyberBid Auctions \\'99 Exchange', folder: 'Web', url: '#tab-auctions', desc: 'Authentic 1999 online auction marketplace: live proxy bidding, snipe countdowns, retro computing hardware & surplus.', tags: 'auctions,ebay,bid,hardware,voodoo,pentium,dreamcast,retro,buy,sell,web' }"
)
if catalog_target in content:
    content = content.replace(catalog_target, catalog_replacement, 1)
    print("5. Added CyberBid to PORTAL_CATALOG")
else:
    print("Error: catalog_target not found")

# 6. Add JavaScript module before switchPortalTab
js_target = "function switchPortalTab(tabName) {"
auction_js_code = """/* =========================================================================
   CYBERBID AUCTIONS '99 TRADING FLOOR SYSTEM
   ========================================================================= */
const DEFAULT_AUCTION_LOTS = [
  {
    id: "lot-01",
    title: "3dfx Voodoo3 3000 AGP 16MB (Factory Sealed)",
    category: "hardware",
    seller: "glide_master_99",
    bids: 18,
    currentBid: 145.00,
    reserve: 120.00,
    timeLeft: 145,
    highBidder: "quake_frag_king",
    minIncrement: 5.00,
    icon: "🚀",
    desc: "Original 3dfx Glide accelerator! 166MHz core/memory clock, 16MB 143MHz SDRAM. 350MHz RAMDAC supports up to 2048x1536 resolution. Factory shrinkwrap intact with Surreal Tournament demo CD!"
  },
  {
    id: "lot-02",
    title: "Intel Pentium III 500MHz Slot 1 \\"Katmai\\" (SECC2 w/ Retail Fan)",
    category: "hardware",
    seller: "overclock_wizard",
    bids: 12,
    currentBid: 210.00,
    reserve: 180.00,
    timeLeft: 310,
    highBidder: "slot1_believer",
    minIncrement: 5.00,
    icon: "⚡",
    desc: "0.25-micron Katmai core with 70 new Streaming SIMD Extensions (SSE). 512KB L2 cache running at half CPU speed. 100MHz FSB. Tested stable at 550MHz with Award BIOS!"
  },
  {
    id: "lot-03",
    title: "Roland MT-32 Multi-Timbre Sound Module (L/A Synthesis)",
    category: "audio",
    seller: "midi_maestro",
    bids: 24,
    currentBid: 285.00,
    reserve: 250.00,
    timeLeft: 65,
    highBidder: "sierra_soundtrack_fan",
    minIncrement: 10.00,
    icon: "🎹",
    desc: "Legendary 32-voice Linear Arithmetic synthesizer module. 128 preset sounds, 30 rhythm sounds, 16x2 backlit LCD display. Brings classic DOS game MIDI soundtracks to life!"
  },
  {
    id: "lot-04",
    title: "Iomega Zip 250MB SCSI External Drive + 10x Disks",
    category: "storage",
    seller: "floppy_collector",
    bids: 8,
    currentBid: 68.00,
    reserve: 50.00,
    timeLeft: 420,
    highBidder: "backup_hero",
    minIncrement: 2.00,
    icon: "💾",
    desc: "High-speed 250MB SCSI Zip drive! Backward compatible with 100MB disks. Includes 10 pre-formatted disks with cases, 25-pin SCSI cable, and original Iomega Ware CD."
  },
  {
    id: "lot-05",
    title: "Creative Sound Blaster AWE64 Gold ISA (CT4390)",
    category: "audio",
    seller: "retro_sound_guy",
    bids: 15,
    currentBid: 125.00,
    reserve: 100.00,
    timeLeft: 95,
    highBidder: "dos_gamer_99",
    minIncrement: 5.00,
    icon: "🎵",
    desc: "Top-of-the-line EMU8000 wavetable synthesizer with 4MB on-board RAM and gold-plated RCA stereo outputs. 64-voice polyphony with SoundFont 2.0 support. Cleanest SNR of any ISA card."
  },
  {
    id: "lot-06",
    title: "SEGA Dreamcast Launch Edition (NTSC-U) w/ VMU & Sonic Demo",
    category: "consoles",
    seller: "arcade_dreamer",
    bids: 31,
    currentBid: 199.00,
    reserve: 175.00,
    timeLeft: 40,
    highBidder: "shenmue_fanatic",
    minIncrement: 5.00,
    icon: "🌀",
    desc: "9/9/99 launch edition system in mint box! NEC PowerVR2 CLX2 graphics chip, 200MHz Hitachi SH-4 CPU, built-in 56K V.90 modem, Visual Memory Unit (VMU) with mini LCD screen."
  },
  {
    id: "lot-07",
    title: "IBM Model M Clicky Keyboard (Part #1391401, 1993)",
    category: "hardware",
    seller: "clicky_clack",
    bids: 22,
    currentBid: 110.00,
    reserve: 90.00,
    timeLeft: 510,
    highBidder: "terminal_typist",
    minIncrement: 5.00,
    icon: "⌨️",
    desc: "Heavy steel backplate buckling spring keyboard with PS/2 cable and detachable coiled cord. Indestructible tank construction. Every keypress produces pure auditory satisfaction."
  },
  {
    id: "lot-08",
    title: "Silicon Graphics O2 Desktop Workstation (MIPS R10000 / IRIX 6.5)",
    category: "hardware",
    seller: "irix_collector",
    bids: 19,
    currentBid: 650.00,
    reserve: 500.00,
    timeLeft: 720,
    highBidder: "unix_sysadmin_99",
    minIncrement: 25.00,
    icon: "🔷",
    desc: "Iconic cobalt blue O2 chassis with Unified Memory Architecture. 250MHz MIPS R10000 CPU, 256MB RAM, 9GB Ultra Wide SCSI drive, pre-loaded with IRIX 6.5 and Softimage 3D demo."
  },
  {
    id: "lot-09",
    title: "USRobotics 56K V.90 Sportster External Faxmodem",
    category: "hardware",
    seller: "dialup_pioneer",
    bids: 7,
    currentBid: 42.00,
    reserve: 35.00,
    timeLeft: 230,
    highBidder: "bbs_caller_01",
    minIncrement: 2.00,
    icon: "📠",
    desc: "Genuine USRobotics Courier Sportster with external LED status array (HS, AA, CD, OH, RD, SD, TR, CS). Supports x2 and ITU V.90 standards. Includes 9V AC transformer."
  },
  {
    id: "lot-10",
    title: "Diamond Rio PMP300 32MB Portable MP3 Player",
    category: "audio",
    seller: "mp3_revolution",
    bids: 14,
    currentBid: 85.00,
    reserve: 70.00,
    timeLeft: 180,
    highBidder: "winamp_skin_artist",
    minIncrement: 5.00,
    icon: "🎧",
    desc: "The player that started the digital music revolution! 32MB onboard flash holds roughly 30 minutes of CD-quality 128kbps MP3s. Parallel port transfer cable and software included."
  },
  {
    id: "lot-11",
    title: "Permian Halite Acoustic Subcarrier Transponder Mod. 7F (Surplus)",
    category: "surplus",
    seller: "skunkworks_surplus",
    bids: 29,
    currentBid: 199.90,
    reserve: 150.00,
    timeLeft: 85,
    highBidder: "carlsbad_scout",
    minIncrement: 10.00,
    icon: "📡",
    desc: "Government surplus seismic transceiver module salvaged from Carlsbad testing station sector 7F. Calibrated to 1999Hz harmonic acoustic subcarrier. 9-pin RS-232 telemetry interface. Internal buffer offset at hex 0x7F1999. Intranet broadcast address: 10.19.99.4."
  },
  {
    id: "lot-12",
    title: "Matrox Millennium G400 DualHead 32MB AGP",
    category: "hardware",
    seller: "cad_enthusiast",
    bids: 11,
    currentBid: 135.00,
    reserve: 110.00,
    timeLeft: 340,
    highBidder: "dual_monitor_pro",
    minIncrement: 5.00,
    icon: "🖥️",
    desc: "32MB SGRAM with revolutionary DualHead display technology driving two CRT monitors simultaneously. Environment-Mapped Bump Mapping (EMBM) hardware support for next-gen 3D."
  }
];

let auctionLots = [];
let wonLots = [];
let activeAuctionCategory = 'all';
let auctionSearchQuery = '';
let auctionTimerInterval = null;
let auctionSimInterval = null;

const NPC_BIDDERS = [
  "glide_kid_99", "voodoo_fanatic", "overclock_king", "turbo_bidder",
  "retro_hoarder", "irix_hacker", "soundfont_dj", "scsi_snatcher",
  "zip_baron", "slot1_overclocker", "baud_runner", "quake_sniper"
];

function initAuctionsData() {
  try {
    const savedLots = localStorage.getItem('kilonet_auction_lots');
    if (savedLots) {
      auctionLots = JSON.parse(savedLots);
    } else {
      auctionLots = JSON.parse(JSON.stringify(DEFAULT_AUCTION_LOTS));
    }
    const savedWon = localStorage.getItem('kilonet_won_lots');
    if (savedWon) wonLots = JSON.parse(savedWon);
  } catch(e) {
    auctionLots = JSON.parse(JSON.stringify(DEFAULT_AUCTION_LOTS));
    wonLots = [];
  }
}

function saveAuctionsState() {
  try {
    localStorage.setItem('kilonet_auction_lots', JSON.stringify(auctionLots));
    localStorage.setItem('kilonet_won_lots', JSON.stringify(wonLots));
  } catch(e) {}
}

function playGavelStrike() {
  if (!audioEnabled) return;
  initAudio();
  if (!audioCtx) return;
  // Triple woodblock gavel impact
  [0, 110, 220].forEach((delay, idx) => {
    setTimeout(() => {
      if (!audioCtx) return;
      const now = audioCtx.currentTime;
      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();
      osc.type = 'square';
      osc.frequency.setValueAtTime(480 - idx * 30, now);
      osc.frequency.exponentialRampToValueAtTime(120, now + 0.05);
      gain.gain.setValueAtTime(0.28, now);
      gain.gain.exponentialRampToValueAtTime(0.001, now + 0.05);
      osc.connect(gain);
      gain.connect(masterGain || audioCtx.destination);
      osc.start(now);
      osc.stop(now + 0.06);
    }, delay);
  });
}

function playOutbidTone() {
  if (!audioEnabled) return;
  initAudio();
  if (!audioCtx) return;
  // Descending two-tone warning
  playFmTone(587.33, 2.0, 1.2, 0.20, 0.09, false); // D5
  setTimeout(() => playFmTone(440.00, 2.0, 1.2, 0.25, 0.12, false), 100); // A4
}

function renderAuctions() {
  initAuctionsData();
  updateAuctionStats();
  renderAuctionCards();

  if (!auctionTimerInterval) {
    auctionTimerInterval = setInterval(tickAuctionTimers, 1000);
  }
  if (!auctionSimInterval) {
    auctionSimInterval = setInterval(simulateNpcBidding, 8500);
  }
}

function updateAuctionStats() {
  const activeLots = auctionLots.filter(l => l.timeLeft > 0);
  const totalBids = auctionLots.reduce((acc, l) => acc + (l.bids || 0), 0);
  const volume = auctionLots.reduce((acc, l) => acc + (l.currentBid || 0), 0);

  const activeEl = document.getElementById('auctionActiveCount');
  const bidsEl = document.getElementById('auctionTotalBids');
  const volEl = document.getElementById('auctionVolumeUSD');
  const wonEl = document.getElementById('wonLotsCount');

  if (activeEl) activeEl.textContent = activeLots.length;
  if (bidsEl) bidsEl.textContent = totalBids;
  if (volEl) volEl.textContent = '$' + volume.toFixed(2);
  if (wonEl) wonEl.textContent = wonLots.length;
}

function renderAuctionCards() {
  const container = document.getElementById('auctionsGridContainer');
  if (!container) return;

  const filtered = auctionLots.filter(lot => {
    if (activeAuctionCategory !== 'all' && lot.category !== activeAuctionCategory) return false;
    if (auctionSearchQuery) {
      const q = auctionSearchQuery.toLowerCase();
      const matchTitle = (lot.title || '').toLowerCase().includes(q);
      const matchDesc = (lot.desc || '').toLowerCase().includes(q);
      const matchSeller = (lot.seller || '').toLowerCase().includes(q);
      if (!matchTitle && !matchDesc && !matchSeller) return false;
    }
    return true;
  });

  if (filtered.length === 0) {
    container.innerHTML = `<div style="grid-column:1/-1; padding:30px; text-align:center; background:#f8fafc; border:2px dashed #cbd5e1; font-family:Arial; color:#64748b;">No auction lots found matching your filter or query. <button class="btn-retro" onclick="clearAuctionSearch()">Reset Filters</button></div>`;
    return;
  }

  container.innerHTML = filtered.map(lot => {
    const isEnded = lot.timeLeft <= 0;
    const isUrgent = lot.timeLeft > 0 && lot.timeLeft <= 60;
    const isUserHigh = (lot.highBidder === "YOU" || lot.highBidder === "You");

    let badgeClass = 'badge-reserve';
    let badgeText = 'RESERVE MET';
    if (isEnded) {
      badgeClass = 'badge-sold';
      badgeText = isUserHigh ? 'YOU WON!' : 'SOLD';
    } else if (isUrgent) {
      badgeClass = 'badge-ending';
      badgeText = 'ENDING SOON';
    } else if (lot.bids >= 20) {
      badgeClass = 'badge-hot';
      badgeText = 'HOT LOT 🔥';
    }

    const minutes = Math.floor(lot.timeLeft / 60);
    const seconds = lot.timeLeft % 60;
    const timerStr = isEnded ? "AUCTION CLOSED" : `${minutes}m ${seconds < 10 ? '0' : ''}${seconds}s`;

    return `
      <div class="auction-card ${isUrgent ? 'snipe-alert' : ''} ${isUserHigh ? 'auction-user-high' : ''}" id="card-${lot.id}">
        <span class="auction-badge ${badgeClass}">${badgeText}</span>
        <div style="display:flex; align-items:center; gap:8px; margin-bottom:4px;">
          <span style="font-size:22px;">${lot.icon || '📦'}</span>
          <div class="auction-title">${escapeHtml(lot.title)}</div>
        </div>
        <div class="auction-meta">
          <span>Seller: <strong>${escapeHtml(lot.seller)}</strong></span> •
          <span>Bids: <strong>${lot.bids}</strong></span>
        </div>
        <div style="font-size:11px; color:#475569; line-height:1.35; margin-bottom:8px; flex:1;">
          ${escapeHtml(lot.desc)}
        </div>
        <div class="auction-price-row">
          <div>
            <div style="font-size:10px; color:#64748b; text-transform:uppercase;">Current Bid</div>
            <div class="auction-bid-val">$${Number(lot.currentBid).toFixed(2)}</div>
            <div style="font-size:10px; color:${isUserHigh ? '#16a34a' : '#475569'}; font-weight:${isUserHigh ? 'bold' : 'normal'};">
              High Bidder: ${escapeHtml(lot.highBidder)} ${isUserHigh ? '👑 (YOU)' : ''}
            </div>
          </div>
          <div style="text-align:right;">
            <div style="font-size:10px; color:#64748b; text-transform:uppercase;">Time Remaining</div>
            <div class="auction-timer ${isUrgent ? 'urgent' : ''}">${timerStr}</div>
          </div>
        </div>
        ${!isEnded ? `
          <div class="auction-bid-box">
            <button class="btn-retro" style="font-weight:bold; color:#15803d; background:#ecfdf5;" onclick="placeQuickBid('${lot.id}', 5)">+$5</button>
            <button class="btn-retro" style="font-weight:bold; color:#15803d; background:#ecfdf5;" onclick="placeQuickBid('${lot.id}', 10)">+$10</button>
            <input type="number" id="customBid-${lot.id}" class="auction-bid-input" value="${(lot.currentBid + (lot.minIncrement || 5)).toFixed(2)}" step="1" />
            <button class="btn-retro btn-primary-retro" style="flex:1;" onclick="placeCustomBid('${lot.id}')">🔨 Place Bid</button>
          </div>
        ` : `
          <div style="background:#f1f5f9; text-align:center; padding:6px; font-size:11px; font-weight:bold; color:#475569; border:1px inset #cbd5e1;">
            ${isUserHigh ? '🎉 Winning Bidder: YOU! Item recorded in won vault.' : `Sold to ${escapeHtml(lot.highBidder)} for $${Number(lot.currentBid).toFixed(2)}`}
          </div>
        `}
      </div>
    `;
  }).join('');
}

function tickAuctionTimers() {
  let stateChanged = false;
  auctionLots.forEach(lot => {
    if (lot.timeLeft > 0) {
      lot.timeLeft--;
      stateChanged = true;
      if (lot.timeLeft === 0) {
        // Auction concluded!
        playGavelStrike();
        if (lot.highBidder === "YOU" || lot.highBidder === "You") {
          playSuccessChime();
          showRetroToast(`🎉 GAVEL STRUCK! You won: ${lot.title}!`);
          if (!wonLots.some(w => w.id === lot.id)) {
            wonLots.push({
              id: lot.id,
              title: lot.title,
              price: lot.currentBid,
              seller: lot.seller,
              time: Date.now()
            });
          }
        } else {
          showRetroToast(`🔨 SOLD! ${lot.title} hammered for $${lot.currentBid.toFixed(2)} to ${lot.highBidder}`);
        }
      }
    }
  });

  if (stateChanged) {
    saveAuctionsState();
    updateAuctionStats();
    // Only re-render if active tab is auctions
    const auctionsTab = document.getElementById('tab-auctions');
    if (auctionsTab && auctionsTab.classList.contains('active')) {
      renderAuctionCards();
    }
  }
}

function simulateNpcBidding() {
  // Randomly pick 1 active lot with > 5s left
  const active = auctionLots.filter(l => l.timeLeft > 5);
  if (active.length === 0) return;

  const lot = active[Math.floor(Math.random() * active.length)];
  const wasUserHigh = (lot.highBidder === "YOU" || lot.highBidder === "You");

  const increment = lot.minIncrement || 5;
  lot.currentBid = Math.round((lot.currentBid + increment) * 100) / 100;
  lot.bids = (lot.bids || 0) + 1;
  const newBidder = NPC_BIDDERS[Math.floor(Math.random() * NPC_BIDDERS.length)];
  lot.highBidder = newBidder;

  // Anti-sniping extension: if < 15 seconds, add 20 seconds
  if (lot.timeLeft < 15) {
    lot.timeLeft += 20;
    showRetroToast(`⚡ SNIPE EXTENSION: 20s added to lot: ${lot.title}!`);
  }

  saveAuctionsState();
  updateAuctionStats();

  const auctionsTab = document.getElementById('tab-auctions');
  if (auctionsTab && auctionsTab.classList.contains('active')) {
    renderAuctionCards();
  }

  if (wasUserHigh) {
    playOutbidTone();
    showRetroToast(`⚠️ OUTBID! ${newBidder} bid $${lot.currentBid.toFixed(2)} on ${lot.title}!`);
  }
}

function placeQuickBid(lotId, inc) {
  playClick();
  const lot = auctionLots.find(l => l.id === lotId);
  if (!lot || lot.timeLeft <= 0) return;

  const newBid = Math.round((lot.currentBid + inc) * 100) / 100;
  lot.currentBid = newBid;
  lot.bids = (lot.bids || 0) + 1;
  lot.highBidder = "YOU";

  playStockChime();
  showRetroToast(`✅ Bid Placed! You are high bidder at $${newBid.toFixed(2)} on ${lot.title}`);
  saveAuctionsState();
  updateAuctionStats();
  renderAuctionCards();
}

function placeCustomBid(lotId) {
  playClick();
  const lot = auctionLots.find(l => l.id === lotId);
  const input = document.getElementById(`customBid-${lotId}`);
  if (!lot || !input || lot.timeLeft <= 0) return;

  const val = parseFloat(input.value);
  const minRequired = Math.round((lot.currentBid + (lot.minIncrement || 1)) * 100) / 100;

  if (isNaN(val) || val < minRequired) {
    alert(`Minimum bid required is $${minRequired.toFixed(2)}`);
    return;
  }

  lot.currentBid = val;
  lot.bids = (lot.bids || 0) + 1;
  lot.highBidder = "YOU";

  playStockChime();
  showRetroToast(`✅ High Bid Updated! $${val.toFixed(2)} on ${lot.title}`);
  saveAuctionsState();
  updateAuctionStats();
  renderAuctionCards();
}

function filterAuctions(category) {
  playClick();
  activeAuctionCategory = category;
  ['all', 'hardware', 'audio', 'consoles', 'storage', 'surplus'].forEach(c => {
    const btn = document.getElementById(`btnFilter-${c}`);
    if (btn) btn.classList.toggle('active', c === category);
  });
  renderAuctionCards();
}

function handleAuctionSearch(q) {
  auctionSearchQuery = q.trim();
  renderAuctionCards();
}

function clearAuctionSearch() {
  playClick();
  auctionSearchQuery = '';
  const input = document.getElementById('auctionSearchInput');
  if (input) input.value = '';
  filterAuctions('all');
}

function resetAuctionsDefaults() {
  playClick();
  if (confirm("Reset all auction lots and timers to 1999 defaults?")) {
    auctionLots = JSON.parse(JSON.stringify(DEFAULT_AUCTION_LOTS));
    wonLots = [];
    saveAuctionsState();
    updateAuctionStats();
    renderAuctionCards();
    showRetroToast("Auction exchange reset to factory 1999 lots!");
  }
}

function openAuctionListingModal() {
  playClick();
  const title = prompt("Item Title (e.g. Diamond Monster 3D II 12MB Voodoo2):", "Diamond Monster 3D II 12MB Voodoo2");
  if (!title) return;
  const cat = prompt("Category (hardware, audio, consoles, storage, surplus):", "hardware") || "hardware";
  const startBid = parseFloat(prompt("Starting Bid ($ USD):", "50.00") || "50.00");
  const duration = parseInt(prompt("Duration in seconds (e.g. 180 for 3 mins):", "180") || "180", 10);
  const desc = prompt("Item Description:", "Tested functional in Windows 98 SE. Includes pass-through VGA cable!");

  const newLot = {
    id: "lot-" + Date.now(),
    title: title.trim(),
    category: cat.toLowerCase().trim(),
    seller: "YOU",
    bids: 0,
    currentBid: startBid,
    reserve: startBid,
    timeLeft: duration,
    highBidder: "No Bids",
    minIncrement: 5.00,
    icon: "📦",
    desc: (desc || "").trim()
  };

  auctionLots.unshift(newLot);
  saveAuctionsState();
  updateAuctionStats();
  renderAuctionCards();
  playSuccessChime();
  showRetroToast(`🏛️ New Lot Listed: ${newLot.title}! Ready for bids.`);
}

function showWonAuctionsModal() {
  playClick();
  if (wonLots.length === 0) {
    alert("You have not won any auctions yet! Place bids and hold the highest bid until the countdown expires.");
    return;
  }
  const summary = wonLots.map((w, i) => `${i + 1}. ${w.title} — Hammer: $${Number(w.price).toFixed(2)} (Seller: ${w.seller})`).join('\\n');
  alert(`🏆 CyberBid '99 Won Lots Vault (${wonLots.length} items):\\n\\n` + summary);
}

"""

if js_target in content:
    content = content.replace(js_target, auction_js_code + js_target, 1)
    print("6. Added CyberBid JavaScript module")
else:
    print("Error: js_target not found")

# 7. Update switchPortalTab to include 'auctions'
switch_tab_target = "  const tabs = ['directory', 'widgetlab', 'mykilonet', 'downloads', 'webmail', 'comics', 'guestbook', 'noc', 'arcade', 'news', 'radio', 'stocks', 'classifieds', 'yellowpages', 'weather', 'speedtest', 'horoscope', 'trivia', 'y2k'];"
switch_tab_replacement = (
    "  const tabs = ['directory', 'widgetlab', 'mykilonet', 'downloads', 'webmail', 'comics', 'guestbook', 'noc', 'arcade', 'news', 'radio', 'stocks', 'classifieds', 'yellowpages', 'weather', 'speedtest', 'horoscope', 'trivia', 'y2k', 'auctions'];"
)

render_call_target = "  } else if (tabName === 'comics') {\n    renderComicEpisode(currentComicIndex);\n  }"
if render_call_target not in content:
    render_call_target = "  } else if (tabName === 'comics') {\r\n    renderComicEpisode(currentComicIndex);\r\n  }"

render_call_replacement = (
    render_call_target + "\n  } else if (tabName === 'auctions') {\n    renderAuctions();"
)

if switch_tab_target in content:
    content = content.replace(switch_tab_target, switch_tab_replacement, 1)
    print("7a. Added 'auctions' to tabs array in switchPortalTab")
else:
    print("Error: switch_tab_target not found")

if render_call_target in content:
    content = content.replace(render_call_target, render_call_replacement, 1)
    print("7b. Added renderAuctions() call in switchPortalTab")
else:
    print("Error: render_call_target not found")

with open(PORTAL_PATH, "w", encoding="utf-8") as f:
    f.write(content)

print("Finished updating portal.html!")
