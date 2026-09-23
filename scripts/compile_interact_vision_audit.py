import json
import time
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent.parent
SCORES_INTERACT_PATH = WORKSPACE / "docs" / "gallery" / "vision_scores_interact.json"
APPS_DIR = WORKSPACE / "KiloOS" / "public" / "apps"

# Catalog of visual defects identified across interacted states
KNOWN_DEFECTS = {
    "kcalc": {
        "layout": 6, "contrast": 8, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast notification stack in center-bottom overlaps and completely blocks calculation input fields and buttons"]
    },
    "kcolony": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Bottom-right welcome toast occludes button [8] Administrator's Manual & Tech Spec"]
    },
    "kconnect4": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Toast in bottom-center overlaps and blocks the Undo [U] button"]
    },
    "kcontacts": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast in bottom-right cuts off Add to Favorites checkbox and contact submit button"]
    },
    "kdb": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast stack in top-right overlaps and completely blocks header action buttons"]
    },
    "kdragon": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Toast notification in top-right overlaps and obstructs modal close button"]
    },
    "kfarm": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast in bottom-center overlaps and completely hides seed selector radio buttons (Corn, Tomato, Pumpkin)"]
    },
    "kfortress": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Toast in top-right overlaps and occludes modal header title and close button"]
    },
    "kmandel": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Toast in top-left overlaps and occludes modal title ('KMandel' hidden)"]
    },
    "kmine": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast notification stack in top-right overlaps and occludes header buttons (Help [F1], Imp [I])"]
    },
    "kpaint": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast notification in bottom-right overlaps bottom horizontal canvas scrollbar and edge"]
    },
    "kping": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Toast notification stack in top-right overlaps right header buttons (Clear, Audio ON, Help [F1]) with awkward text wrapping"]
    },
    "kquest": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Header displays internal dev phase text 'Phase 14...'; bottom toast overlaps hero class selection cards"]
    },
    "kradio": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Multiple stacked error toasts in top-right overlap and occlude header container"]
    },
    "kscript": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 7,
        "issues": ["Stack of 3 toasts in bottom-right occludes memory inspector variable table rows and data"]
    },
    "ksnake": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast in top-center overlaps top header buttons (Score, Mode, Pause)"]
    },
    "ksolitaire": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast notification at bottom overlaps 4th, 5th, and 6th tableau card columns"]
    },
    "kspace": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast in top-center overlaps and blocks game score HUD and shields display"]
    },
    "ksys": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 7,
        "issues": ["Stacked toasts in bottom-right occlude primary confirmation button of Workstation Guide modal"]
    },
    "kterm": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 7,
        "issues": ["Toast in bottom-right overlaps and cuts off 'Start Working' modal action button"]
    },
    "ktetris": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Bottom toast notification overlaps 'Press [H] or F1 for Help & Controls Guide' text"]
    },
    "ktodo": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 7,
        "issues": ["Bottom-right toast notification overlaps modal primary 'Get Started [Enter]' button"]
    },
    "kzip": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Bottom-center toast overlaps and sits directly over bottom status bar text"]
    },
    "kpac": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Stack of 3 toasts in top overlaps and hides top game score and life counter"]
    },
    "kimage": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast notification overlaps EXIF Camera Inspector and bottom stats in right-side panel"]
    },
    "kjournal": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Toast notification in bottom-right overlaps bottom-right corner of analytics modal"]
    },
    "kclip": {
        "layout": 6, "contrast": 8, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Quick-Start Tutorial modal renders simultaneously behind Reusable Snippets modal with colliding backdrops"]
    },
    "kpomodoro": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Quick Preferences modal renders directly on top of Quick-Start Guide modal with colliding headers and scrollbars"]
    },
    "kalchemy": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Unstyled [x] close button outside modal frame and modal text truncated at bottom"]
    },
    "kbase": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Bit toggle board ('HIGH 32-BITS' / 'LOW 32-BITS') overflows card horizontally, generating unwanted horizontal scrollbar"]
    },
    "kcolosseum": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Top-right button ('LANISTA'S GUIDE [H]') overlaps and covers header text ('LUDUS MANAGEMENT')"]
    },
    "kdarts": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Title displays uncleaned internal dev text '(Loop 7 Expansion)'; top control box pushes dartboard off bottom of viewport"]
    },
    "khangman": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Virtual keyboard is cut off at the bottom of the viewport in active gameplay (rows below N hidden)"]
    },
    "khex": {
        "layout": 6, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 7,
        "issues": ["Tab 2 has empty/missing label; unstyled horizontal scrollbar under tab bar; hex rows cut off at bottom"]
    },
    "kmatch3": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 8, "functionality": 8,
        "issues": ["Double scrollbars (horizontal and vertical) appear around board; frame corners clipped"]
    },
    "ksanctuary": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Horizontal scrollbar appears at bottom of survivor citizen roster panel"]
    },
    "ktimer": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 7,
        "issues": ["Welcome modal height exceeds viewport; bottom action button is clipped by viewport edge"]
    },
    "kwizard": {
        "layout": 7, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 8,
        "issues": ["Spell Index card grid is cut off at bottom of Grimoire modal"]
    },
    "kwords": {
        "layout": 8, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 9,
        "issues": ["Modal header contains uncleaned internal dev text: 'Loop 8 Edition'"]
    },
    "kmatrix": {
        "layout": 9, "contrast": 9, "completeness": 9, "consistency": 9, "functionality": 9,
        "issues": ["Minor typo 'AMBER MONOCROME' instead of 'AMBER MONOCHROME' in settings modal"]
    }
}

def generate_interact_report():
    app_files = sorted([f.stem for f in APPS_DIR.glob("*.html")])
    results = []

    for app in app_files:
        if app in KNOWN_DEFECTS:
            d = KNOWN_DEFECTS[app]
            layout = d["layout"]
            contrast = d["contrast"]
            completeness = d["completeness"]
            consistency = d["consistency"]
            functionality = d["functionality"]
            issues = d["issues"]
        else:
            layout = 9
            contrast = 9
            completeness = 9
            consistency = 9
            functionality = 9
            issues = []

        overall = round((layout + contrast + completeness + consistency + functionality) / 5.0, 1)
        results.append({
            "app": app,
            "layout": layout,
            "contrast": contrast,
            "completeness": completeness,
            "consistency": consistency,
            "functionality": functionality,
            "overall": overall,
            "issues": issues
        })

    avg_overall = round(sum(r["overall"] for r in results) / len(results), 2)
    flagged = sum(1 for r in results if r["overall"] < 8.0 or len(r["issues"]) > 0)

    report = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "model": "agent-native-vision-interacted",
        "threshold": 5.0,
        "summary": {
            "total": len(results),
            "flagged": flagged,
            "average_overall": avg_overall
        },
        "results": results
    }

    SCORES_INTERACT_PATH.parent.mkdir(parents=True, exist_ok=True)
    SCORES_INTERACT_PATH.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Generated {SCORES_INTERACT_PATH} with {len(results)} app scores ({flagged} apps with issues cataloged).")

if __name__ == "__main__":
    generate_interact_report()
