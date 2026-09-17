# 🌐 KiloApps Autonomous Fleet Contributor Guide

Welcome! You can contribute to the growth, polish, and expansion of KiloApps without writing a single line of code—simply by donating a small fraction of your **free Google Gemini API quota** and running autonomous agent turns on GitHub's free runners.

This entire process is **100% free** ($0.00 compute cost, $0.00 API cost).

---

## ⚡ How It Works

1. You fork the KiloApps repository to your GitHub account.
2. You obtain a free Gemini API key from Google AI Studio.
3. You add that key to your fork's GitHub Secrets.
4. You click **"Run workflow"** (or enable a cron schedule).
5. GitHub's free Windows runner spins up, activates `agy` with **Gemini 3.8 Flash (High Reasoning)**, refactors/creates an application, verifies that all C code and React UI compile cleanly, and opens a Pull Request back to the main repository.
6. The upstream Gatekeeper verifies the builds and automatically merges the work!

---

## 🛠️ Step-by-Step Setup

### Step 1: Get Your Free Gemini API Key
1. Visit [Google AI Studio](https://aistudio.google.com/).
2. Sign in with any free Google account.
3. Click **"Get API key"** -> **"Create API key in new project"**.
4. Copy your API key.

> [!NOTE]
> Google AI Studio provides **15 Requests Per Minute (RPM)**, **1,000,000 Tokens Per Minute (TPM)**, and **1,500 Requests Per Day (RPD)** for free on Flash models. A single 15-minute KiloApps turn only uses ~40–60 requests, meaning you can run dozens of turns per day without ever paying a cent.

### Step 2: Fork the Repository
Click the **Fork** button at the top-right of [mrbos/KiloApps](https://github.com/mrbos/KiloApps).

### Step 3: Add Your Secret
1. In your forked repository, go to **Settings** -> **Secrets and variables** -> **Actions**.
2. Click **New repository secret**.
3. Set Name: `GEMINI_API_KEY`
4. Set Secret: Paste your API key from Step 1.
5. Click **Add secret**.

### Step 4: Run an Autonomous Turn
1. In your fork, go to the **Actions** tab.
2. Click **"I understand my workflows, go ahead and enable them"** if prompted.
3. In the left sidebar, click **"Contributor Fleet Turn"**.
4. Click **Run workflow** (leave inputs default, or select a specific agent).
5. Watch the agent execute! Once finished, it will automatically open a Pull Request back to upstream `main`.

---

## 🛡️ Privacy & Safety
- **Zero Credential Sharing**: Your API key lives exclusively in your fork's encrypted GitHub Secrets. Upstream maintainers can never see or access your key.
- **Sandboxed Compilation**: Turns execute in isolated Windows virtual machines provided by GitHub Actions.
- **Deterministic Gates**: Every turn is compiled with Microsoft Visual C++ (`cl.exe`) and Vite (`npm run build`) before opening a PR.

Thank you for fueling the autonomous fleet! 🚀
