# KiloApps Platform Hardening & Anti-Lock-in Playbook

This document details the practical steps to protect KiloApps from platform-level single points of failure (e.g., automated Google Safe Browsing flags, fraudulent DMCA notices, or repository suspensions).

---

## 1. Google Safe Browsing Protection & Search Console Early Warning

Google Safe Browsing uses automated web risk scanners to evaluate pages. To maintain full visibility, early-warning alerts, and 1-click appeal recourse:

### A. Register in Google Search Console
1. Go to **[Google Search Console](https://search.google.com/search-console)**.
2. Add a new **URL prefix** property: `https://kiloapps.web.app`.
3. Select verification method:
   - **HTML file**: Download `google<hash>.html` and drop it into `KiloOS/public/google<hash>.html`, commit, and push.
   - **HTML tag**: Copy `<meta name="google-site-verification" content="..." />` and paste it into `KiloOS/index.html`.
4. Once verified, Search Console will monitor the site 24/7.

### B. Why This Protects You
- If Google's automated scanners ever detect a suspicious keyword or pattern, Search Console immediately sends an **email alert** detailing the exact page and reason.
- Inside Search Console under **Security & Manual Actions &rarr; Security Issues**, you have a direct **"Request Review"** button where you can explain: *"This page is a simulated retro computing art piece / ARG game installation; no functioning keygens, malware, or pirated binaries are hosted."* Review requests through Search Console are typically processed within 24–48 hours.

### C. Search Engine Suppression (`robots.txt` & `noindex`)
- `KiloOS/public/robots.txt` is configured to disallow crawlers from scraping `/web/warez.html`, `/web/darknet.html`, and `/exe/`.
- Both `warez.html` and `darknet.html` include `<meta name="robots" content="noindex, nofollow">`, ensuring search engines never index them or treat them as public warez destinations.

---

## 2. Multi-Remote Git Mirroring (No Single Platform Lock-In)

The single greatest defense against any platform suspension is maintaining an automated, synchronized mirror on a second git forge (such as **GitLab** or **Codeberg**).

### A. Automatic Dual-Push Configuration (One-Time Setup)
Create a free repository on GitLab (e.g. `https://gitlab.com/your-username/KiloApps.git`) or Codeberg.
Then run:
```bash
python scripts/mirror_sync.py --dual-push https://gitlab.com/your-username/KiloApps.git
```
Now, whenever you or any agent runs `git push origin main`, Git will simultaneously push to **both** GitHub and GitLab in the same command.

### B. Check Configured Remotes
```bash
python scripts/mirror_sync.py --status
```

### C. Offline Full-Repository Bundles
To export an offline, standalone backup containing every branch, tag, and commit into a single `.bundle` file:
```bash
python scripts/mirror_sync.py --bundle
```
To clone or restore the repository anywhere without internet access:
```bash
git clone kiloapps_full_backup.bundle KiloApps-Restored
```

---

## 3. Redundant Hosting Failover (Netlify & Cloudflare Pages)

If Firebase Hosting ever experiences an outage or account hold, the site can be live on alternative hosts within 60 seconds:

1. **Netlify:**
   - Pre-configured via `netlify.toml` in the repository root.
   - Simply connect the GitHub/GitLab repo to Netlify or run `npx netlify deploy --prod` from the command line.
2. **Cloudflare Pages:**
   - Connect the repository on the Cloudflare dashboard.
   - Set Build command: `npm run build`
   - Set Build output directory: `dist`
   - Set Root directory: `KiloOS`
