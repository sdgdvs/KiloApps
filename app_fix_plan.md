# App Bug Fix Plan

**Target App:** KChart
**Status:** Ready for next agent

## Workflow
1. Pick the next untested app alphabetically.
2. Test web (.html) and native (build.bat) versions.
3. Log bugs found, fix them, verify.
4. Update this file with results.

## Tested Apps
- **KCalendar**: Fixed month-wrapping bug in kcalendar.html where changing months on the 31st skipped a month. Checked main.c (no issues found).
- **KAudio**: Fixed Web Audio API envelope bug in kaudio.html and prevented arrow key scrolling. Fixed GDI resource leak (HBITMAP) in main.c.
- **KBBS**: Fixed ANSI 'T' (scroll down) sequence to shift visible area rather than top of buffer in HTML. Fixed native scrolling behavior where mouse wheel changed scrollbar but not view; fixed out-of-bounds array bug when native scrollback hit limit.
- **KCalc**: Fixed M+ and M- evaluation bug in kcalc.html to format the expression before evaluating. Fixed VK_OEM_PLUS keyboard input in main.c so '=' types '=' instead of '+'.

## Backlog
All apps need systematic testing. Continue alphabetically from KCalendar.
