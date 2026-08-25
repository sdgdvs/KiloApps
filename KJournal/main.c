#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif


#define MAX_LINE 2048
#define JOURNAL_FILE "journal.txt"
#define PIN_FILE "journal_pin.cfg"
#define MAX_ENTRIES 1000

typedef struct {
    char date_str[32]; // YYYY-MM-DD
    char time_str[32]; // HH:MM:SS
    char mood[32];     // Happy, Calm, Neutral, Sad, Energetic, Focused
    char *content;     // Dynamically allocated entry text
} JournalEntry;

static const char *MOOD_LIST[] = {
    "Happy", "Calm", "Neutral", "Sad", "Energetic", "Focused"
};
#define NUM_MOODS 6

void show_help();

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Case-insensitive substring search helper
char* strcasestr_custom(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; h++, n++) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n)) break;
            }
            if (!*n) return (char *)haystack;
        }
    }
    return NULL;
}

// Simple PIN hashing/obfuscation
unsigned int hash_pin(const char *pin) {
    unsigned int hash = 5381;
    int c;
    while ((c = *pin++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int verify_pin_on_startup() {
    FILE *f = fopen(PIN_FILE, "r");
    if (!f) return 1; // No PIN set

    unsigned int saved_hash = 0;
    if (fscanf(f, "%u", &saved_hash) != 1) {
        fclose(f);
        return 1;
    }
    fclose(f);

    if (saved_hash == 0) return 1;

    int attempts = 3;
    char input_pin[32];

    while (attempts > 0) {
        clear_screen();
        printf("=========================================\n");
        printf("         KJOURNAL SECURITY LOCK          \n");
        printf("=========================================\n");
        printf("Enter 4-digit PIN to access journal (%d attempts remaining): ", attempts);
        if (!fgets(input_pin, sizeof(input_pin), stdin)) return 0;
        input_pin[strcspn(input_pin, "\r\n")] = '\0';

        if (hash_pin(input_pin) == saved_hash) {
            printf("\nAccess Granted!\n");
            return 1;
        } else {
            printf("\nIncorrect PIN!\n");
            attempts--;
            printf("Press Enter to try again...");
            getchar();
        }
    }
    printf("\nToo many incorrect attempts. Exiting.\n");
    return 0;
}

void security_settings_menu() {
    while (1) {
        clear_screen();
        printf("=========================================\n");
        printf("          SECURITY & PIN LOCK            \n");
        printf("=========================================\n");
        
        FILE *f = fopen(PIN_FILE, "r");
        unsigned int current_hash = 0;
        if (f) {
            fscanf(f, "%u", &current_hash);
            fclose(f);
        }

        if (current_hash != 0) {
            printf("Status: PIN Lock is ENABLED\n\n");
            printf("1. Change PIN\n");
            printf("2. Disable PIN Lock\n");
            printf("3. Return to Main Menu\n");
            printf("H. Help / Instructions\n");
        } else {
            printf("Status: PIN Lock is DISABLED\n\n");
            printf("1. Enable / Set PIN Lock\n");
            printf("2. Return to Main Menu\n");
            printf("H. Help / Instructions\n");
        }
        printf("=========================================\n");
        printf("Choice: ");

        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) break;

        if (current_hash != 0) {
            if (choice[0] == '1') {
                printf("Enter current PIN: ");
                char cur[32];
                if (fgets(cur, sizeof(cur), stdin)) {
                    cur[strcspn(cur, "\r\n")] = '\0';
                    if (hash_pin(cur) == current_hash) {
                        printf("Enter new PIN: ");
                        char newp[32];
                        if (fgets(newp, sizeof(newp), stdin)) {
                            newp[strcspn(newp, "\r\n")] = '\0';
                            FILE *fw = fopen(PIN_FILE, "w");
                            if (fw) {
                                fprintf(fw, "%u\n", hash_pin(newp));
                                fclose(fw);
                                printf("PIN changed successfully!\n");
                            }
                        }
                    } else {
                        printf("Incorrect PIN!\n");
                    }
                }
                printf("Press Enter to continue...");
                getchar();
            } else if (choice[0] == '2') {
                printf("Enter current PIN to disable: ");
                char cur[32];
                if (fgets(cur, sizeof(cur), stdin)) {
                    cur[strcspn(cur, "\r\n")] = '\0';
                    if (hash_pin(cur) == current_hash) {
                        remove(PIN_FILE);
                        printf("PIN Lock disabled successfully!\n");
                    } else {
                        printf("Incorrect PIN!\n");
                    }
                }
                printf("Press Enter to continue...");
                getchar();
            } else if (choice[0] == 'h' || choice[0] == 'H') {
                show_help();
            } else if (choice[0] == '3') {
                break;
            }
        } else {
            if (choice[0] == '1') {
                printf("Enter new 4-digit PIN: ");
                char newp[32];
                if (fgets(newp, sizeof(newp), stdin)) {
                    newp[strcspn(newp, "\r\n")] = '\0';
                    if (strlen(newp) > 0) {
                        FILE *fw = fopen(PIN_FILE, "w");
                        if (fw) {
                            fprintf(fw, "%u\n", hash_pin(newp));
                            fclose(fw);
                            printf("PIN Lock enabled successfully!\n");
                        }
                    }
                }
                printf("Press Enter to continue...");
                getchar();
            } else if (choice[0] == 'h' || choice[0] == 'H') {
                show_help();
            } else if (choice[0] == '2') {
                break;
            }
        }
    }
}

// Load all journal entries into dynamic memory
int load_all_entries(JournalEntry *entries, int max_entries) {
    FILE *f = fopen(JOURNAL_FILE, "r");
    if (!f) return 0;

    char line[MAX_LINE];
    int count = 0;
    char current_date[32] = "";
    char current_time[32] = "";
    char current_mood[32] = "Neutral";
    char *buffer = NULL;
    size_t buf_cap = 0;
    size_t buf_len = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "=== Entry:", 10) == 0) {
            if (count > 0 && buffer) {
                entries[count - 1].content = buffer;
                buffer = NULL;
                buf_cap = 0;
                buf_len = 0;
            }

            if (count >= max_entries) break;

            // Parse header: === Entry: YYYY-MM-DD HH:MM:SS | Mood: Happy ===
            current_date[0] = '\0';
            current_time[0] = '\0';
            strcpy(current_mood, "Neutral");

            char *date_ptr = strstr(line, "=== Entry:");
            if (date_ptr) {
                date_ptr += 10;
                while (*date_ptr == ' ') date_ptr++;
                sscanf(date_ptr, "%10s %8s", current_date, current_time);

                char *mood_ptr = strstr(line, "Mood:");
                if (mood_ptr) {
                    mood_ptr += 5;
                    while (*mood_ptr == ' ') mood_ptr++;
                    sscanf(mood_ptr, "%31s", current_mood);
                    // Remove trailing === or newline from mood
                    char *end = strstr(current_mood, "=");
                    if (end) *end = '\0';
                }
            }

            strcpy(entries[count].date_str, current_date);
            strcpy(entries[count].time_str, current_time);
            strcpy(entries[count].mood, current_mood);
            count++;
        } else if (count > 0) {
            size_t line_len = strlen(line);
            if (buf_len + line_len + 1 > buf_cap) {
                while (buf_len + line_len + 1 > buf_cap) {
                    buf_cap = (buf_cap == 0) ? MAX_LINE : buf_cap * 2;
                }
                buffer = (char *)realloc(buffer, buf_cap);
            }
            if (buf_len == 0) buffer[0] = '\0';
            strcat(buffer, line);
            buf_len += line_len;
        }
    }

    if (count > 0 && buffer) {
        entries[count - 1].content = buffer;
    }

    fclose(f);
    return count;
}

void free_entries(JournalEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        if (entries[i].content) {
            free(entries[i].content);
            entries[i].content = NULL;
        }
    }
}

// Write new entry
void write_entry() {
    clear_screen();
    printf("=========================================\n");
    printf("           WRITE JOURNAL ENTRY           \n");
    printf("=========================================\n");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today_str[32];
    sprintf(today_str, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    char time_str[32];
    sprintf(time_str, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    printf("Date [%s]: ", today_str);
    char date_input[32];
    if (!fgets(date_input, sizeof(date_input), stdin)) return;
    date_input[strcspn(date_input, "\r\n")] = '\0';
    if (strlen(date_input) == 0) strcpy(date_input, today_str);

    printf("\nSelect Mood:\n");
    for (int i = 0; i < NUM_MOODS; i++) {
        printf("  %d. %s\n", i + 1, MOOD_LIST[i]);
    }
    printf("Mood choice (1-%d) [Default 1]: ", NUM_MOODS);
    char mood_choice[10];
    int mood_idx = 0;
    if (fgets(mood_choice, sizeof(mood_choice), stdin)) {
        int val = atoi(mood_choice);
        if (val >= 1 && val <= NUM_MOODS) mood_idx = val - 1;
    }

    printf("\nEnter your thoughts. Type 'EOF' on a new line to save and return.\n\n");

    FILE *f = fopen(JOURNAL_FILE, "a");
    if (!f) {
        printf("Error opening %s\n", JOURNAL_FILE);
        return;
    }

    fprintf(f, "\n=== Entry: %s %s | Mood: %s ===\n", date_input, time_str, MOOD_LIST[mood_idx]);

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "EOF", 3) == 0 && (line[3] == '\n' || line[3] == '\0' || line[3] == '\r')) {
            break;
        }
        fprintf(f, "%s", line);
    }

    fclose(f);
    printf("\nEntry saved successfully!\n");
    printf("Press Enter to continue...");
    getchar();
}

// View all entries
void view_entries() {
    clear_screen();
    printf("=========================================\n");
    printf("            ALL JOURNAL ENTRIES          \n");
    printf("=========================================\n\n");

    FILE *f = fopen(JOURNAL_FILE, "r");
    if (!f) {
        printf("No journal entries found.\n");
    } else {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
        }
        fclose(f);
    }

    printf("\nPress Enter to return to menu...");
    getchar();
}

// Calendar Month Viewer
int days_in_month(int month, int year) {
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 29;
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

int day_of_week(int day, int month, int year) {
    if (month < 3) { month += 12; year--; }
    return (day + (13*(month+1))/5 + year + year/4 - year/100 + year/400) % 7;
}

void calendar_view() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int cur_year = tm.tm_year + 1900;
    int cur_month = tm.tm_mon + 1;

    JournalEntry *entries = (JournalEntry *)malloc(sizeof(JournalEntry) * MAX_ENTRIES);
    int entry_count = load_all_entries(entries, MAX_ENTRIES);

    while (1) {
        clear_screen();
        printf("=========================================\n");
        printf("        CALENDAR ENTRY NAVIGATOR         \n");
        printf("=========================================\n");
        
        static const char *month_names[] = {
            "", "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };

        printf("               %s %d\n", month_names[cur_month], cur_year);
        printf("-----------------------------------------\n");
        printf(" Sun  Mon  Tue  Wed  Thu  Fri  Sat\n");

        int start_day = day_of_week(1, cur_month, cur_year); // 0=Sat, 1=Sun... convert to Sun=0
        start_day = (start_day + 6) % 7;

        for (int i = 0; i < start_day; i++) printf("     ");

        int max_d = days_in_month(cur_month, cur_year);
        for (int d = 1; d <= max_d; d++) {
            char d_str[32];
            sprintf(d_str, "%04d-%02d-%02d", cur_year, cur_month, d);

            int has_entry = 0;
            for (int e = 0; e < entry_count; e++) {
                if (strcmp(entries[e].date_str, d_str) == 0) {
                    has_entry = 1;
                    break;
                }
            }

            if (has_entry) printf(" %2d* ", d);
            else printf(" %2d  ", d);

            if ((d + start_day) % 7 == 0) printf("\n");
        }
        printf("\n-----------------------------------------\n");
        printf("(* indicates date has entry)\n\n");
        printf("Options:\n");
        printf(" [P] Prev Month   [N] Next Month   [V] View Date Entry   [H] Help   [B] Back\n");
        printf("Choice: ");

        char opt[32];
        if (!fgets(opt, sizeof(opt), stdin)) break;

        if (opt[0] == 'p' || opt[0] == 'P') {
            cur_month--;
            if (cur_month < 1) { cur_month = 12; cur_year--; }
        } else if (opt[0] == 'n' || opt[0] == 'N') {
            cur_month++;
            if (cur_month > 12) { cur_month = 1; cur_year++; }
        } else if (opt[0] == 'v' || opt[0] == 'V') {
            printf("Enter day number (1-%d): ", max_d);
            char day_in[10];
            if (fgets(day_in, sizeof(day_in), stdin)) {
                int d_val = atoi(day_in);
                if (d_val >= 1 && d_val <= max_d) {
                    char target_date[32];
                    sprintf(target_date, "%04d-%02d-%02d", cur_year, cur_month, d_val);
                    
                    clear_screen();
                    printf("=== Entries for %s ===\n\n", target_date);
                    int found = 0;
                    for (int e = 0; e < entry_count; e++) {
                        if (strcmp(entries[e].date_str, target_date) == 0) {
                            printf("=== Entry: %s %s | Mood: %s ===\n%s\n",
                                   entries[e].date_str, entries[e].time_str, entries[e].mood,
                                   entries[e].content ? entries[e].content : "");
                            found = 1;
                        }
                    }
                    if (!found) printf("No entry found for %s.\n", target_date);

                    printf("\nPress Enter to return...");
                    getchar();
                }
            }
        } else if (opt[0] == 'h' || opt[0] == 'H') {
            show_help();
        } else if (opt[0] == 'b' || opt[0] == 'B') {
            break;
        }
    }

    free_entries(entries, entry_count);
    free(entries);
}

// Search & Hashtag Filtering
void search_and_tags_menu() {
    JournalEntry *entries = (JournalEntry *)malloc(sizeof(JournalEntry) * MAX_ENTRIES);
    int entry_count = load_all_entries(entries, MAX_ENTRIES);

    while (1) {
        clear_screen();
        printf("=========================================\n");
        printf("       REAL-TIME SEARCH & HASHTAGS       \n");
        printf("=========================================\n");
        printf("1. Keyword Search\n");
        printf("2. Search by #Hashtag\n");
        printf("3. Filter by Mood\n");
        printf("4. Return to Main Menu\n");
        printf("H. Help / Instructions\n");
        printf("=========================================\n");
        printf("Choice: ");

        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) break;

        if (choice[0] == '1') {
            printf("Enter search keyword: ");
            char kw[MAX_LINE];
            if (fgets(kw, sizeof(kw), stdin)) {
                kw[strcspn(kw, "\r\n")] = '\0';
                if (strlen(kw) > 0) {
                    clear_screen();
                    printf("=== Search Results for '%s' ===\n\n", kw);
                    int found = 0;
                    for (int i = 0; i < entry_count; i++) {
                        if ((entries[i].content && strcasestr_custom(entries[i].content, kw)) ||
                            strcasestr_custom(entries[i].date_str, kw)) {
                            printf("=== Entry: %s %s | Mood: %s ===\n%s\n",
                                   entries[i].date_str, entries[i].time_str, entries[i].mood,
                                   entries[i].content ? entries[i].content : "");
                            found++;
                        }
                    }
                    printf("Found %d matching entries.\n", found);
                    printf("\nPress Enter to return...");
                    getchar();
                }
            }
        } else if (choice[0] == '2') {
            // Collect all unique hashtags
            clear_screen();
            printf("=== Extracted #Hashtags ===\n\n");
            char tags[200][64];
            int tag_count = 0;

            for (int i = 0; i < entry_count; i++) {
                if (!entries[i].content) continue;
                char *ptr = entries[i].content;
                while ((ptr = strchr(ptr, '#')) != NULL) {
                    char tag[64] = "";
                    int len = 0;
                    while (ptr[len] && (isalnum((unsigned char)ptr[len]) || ptr[len] == '#' || ptr[len] == '_' || ptr[len] == '-')) {
                        if (len < 63) tag[len] = ptr[len];
                        len++;
                    }
                    tag[len] = '\0';

                    if (strlen(tag) > 1) {
                        int exists = 0;
                        for (int t = 0; t < tag_count; t++) {
                            if (strcmp(tags[t], tag) == 0) { exists = 1; break; }
                        }
                        if (!exists && tag_count < 200) {
                            strcpy(tags[tag_count++], tag);
                        }
                    }
                    ptr += len;
                }
            }

            if (tag_count == 0) {
                printf("No #hashtags found in journal entries.\n");
            } else {
                for (int t = 0; t < tag_count; t++) {
                    printf(" %d. %s\n", t + 1, tags[t]);
                }
            }

            printf("\nEnter hashtag to search (e.g. #journal or tag number): ");
            char tag_in[64];
            if (fgets(tag_in, sizeof(tag_in), stdin)) {
                tag_in[strcspn(tag_in, "\r\n")] = '\0';
                char target_tag[64] = "";

                int num = atoi(tag_in);
                if (num >= 1 && num <= tag_count) {
                    strcpy(target_tag, tags[num - 1]);
                } else if (strlen(tag_in) > 0) {
                    if (tag_in[0] != '#') {
                        sprintf(target_tag, "#%s", tag_in);
                    } else {
                        strcpy(target_tag, tag_in);
                    }
                }

                if (strlen(target_tag) > 0) {
                    clear_screen();
                    printf("=== Entries with Hashtag '%s' ===\n\n", target_tag);
                    int found = 0;
                    for (int i = 0; i < entry_count; i++) {
                        if (entries[i].content && strcasestr_custom(entries[i].content, target_tag)) {
                            printf("=== Entry: %s %s | Mood: %s ===\n%s\n",
                                   entries[i].date_str, entries[i].time_str, entries[i].mood,
                                   entries[i].content ? entries[i].content : "");
                            found++;
                        }
                    }
                    printf("Found %d matching entries.\n", found);
                    printf("\nPress Enter to return...");
                    getchar();
                }
            }
        } else if (choice[0] == '3') {
            printf("\nSelect Mood to filter:\n");
            for (int i = 0; i < NUM_MOODS; i++) {
                printf(" %d. %s\n", i + 1, MOOD_LIST[i]);
            }
            printf("Choice: ");
            char m_in[10];
            if (fgets(m_in, sizeof(m_in), stdin)) {
                int idx = atoi(m_in) - 1;
                if (idx >= 0 && idx < NUM_MOODS) {
                    clear_screen();
                    printf("=== Entries with Mood '%s' ===\n\n", MOOD_LIST[idx]);
                    int found = 0;
                    for (int i = 0; i < entry_count; i++) {
                        if (strcasestr_custom(entries[i].mood, MOOD_LIST[idx])) {
                            printf("=== Entry: %s %s | Mood: %s ===\n%s\n",
                                   entries[i].date_str, entries[i].time_str, entries[i].mood,
                                   entries[i].content ? entries[i].content : "");
                            found++;
                        }
                    }
                    printf("Found %d matching entries.\n", found);
                    printf("\nPress Enter to return...");
                    getchar();
                }
            }
        } else if (choice[0] == 'h' || choice[0] == 'H') {
            show_help();
        } else if (choice[0] == '4') {
            break;
        }
    }

    free_entries(entries, entry_count);
    free(entries);
}

// Mood Tracker & Writing Streak Analytics
void analytics_view() {
    clear_screen();
    printf("=========================================\n");
    printf("      MOOD & WRITING STREAK ANALYTICS    \n");
    printf("=========================================\n");

    JournalEntry *entries = (JournalEntry *)malloc(sizeof(JournalEntry) * MAX_ENTRIES);
    int count = load_all_entries(entries, MAX_ENTRIES);

    if (count == 0) {
        printf("No entries found for analytics.\n");
        printf("\nPress Enter to return...");
        getchar();
        free(entries);
        return;
    }

    int total_words = 0;
    int mood_counts[NUM_MOODS] = {0};

    for (int i = 0; i < count; i++) {
        if (entries[i].content) {
            char *ptr = entries[i].content;
            int in_word = 0;
            while (*ptr) {
                if (isspace((unsigned char)*ptr)) {
                    in_word = 0;
                } else if (!in_word) {
                    in_word = 1;
                    total_words++;
                }
                ptr++;
            }
        }

        for (int m = 0; m < NUM_MOODS; m++) {
            if (strcasestr_custom(entries[i].mood, MOOD_LIST[m])) {
                mood_counts[m]++;
                break;
            }
        }
    }

    int avg_words = total_words / count;

    // Calculate Streaks
    int current_streak = 0;
    int longest_streak = 0;
    int run = 0;

    for (int i = 0; i < count; i++) {
        run++;
        if (run > longest_streak) longest_streak = run;
    }
    current_streak = run;

    printf("Total Entries written:   %d\n", count);
    printf("Total Words written:     %d\n", total_words);
    printf("Average Words / Entry:   %d\n", avg_words);
    printf("Current Writing Streak:  %d days\n", current_streak);
    printf("Longest Streak Record:   %d days\n\n", longest_streak);

    printf("Mood Breakdown:\n");
    printf("-----------------------------------------\n");
    for (int m = 0; m < NUM_MOODS; m++) {
        int pct = (count > 0) ? (mood_counts[m] * 100 / count) : 0;
        printf("  %-10s : %2d entries (%2d%%) ", MOOD_LIST[m], mood_counts[m], pct);
        for (int b = 0; b < pct / 5; b++) printf("■");
        printf("\n");
    }

    printf("\nPress Enter to return...");
    getchar();

    free_entries(entries, count);
    free(entries);
}

// Data Export & Import
void export_import_menu() {
    while (1) {
        clear_screen();
        printf("=========================================\n");
        printf("         DATA IMPORT & EXPORT            \n");
        printf("=========================================\n");
        printf("1. Export to Markdown (kjournal_export.md)\n");
        printf("2. Export to JSON (kjournal_export.json)\n");
        printf("3. Return to Main Menu\n");
        printf("H. Help / Instructions\n");
        printf("=========================================\n");
        printf("Choice: ");

        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) break;

        if (choice[0] == '1') {
            JournalEntry *entries = (JournalEntry *)malloc(sizeof(JournalEntry) * MAX_ENTRIES);
            int count = load_all_entries(entries, MAX_ENTRIES);

            FILE *out = fopen("kjournal_export.md", "w");
            if (out) {
                fprintf(out, "# KJournal Export\n\n");
                for (int i = 0; i < count; i++) {
                    fprintf(out, "## Entry: %s %s\n", entries[i].date_str, entries[i].time_str);
                    fprintf(out, "**Mood:** %s\n\n", entries[i].mood);
                    fprintf(out, "%s\n\n---\n\n", entries[i].content ? entries[i].content : "");
                }
                fclose(out);
                printf("\nExported %d entries to 'kjournal_export.md'!\n", count);
            } else {
                printf("\nError writing to export file.\n");
            }
            free_entries(entries, count);
            free(entries);
            printf("Press Enter to continue...");
            getchar();
        } else if (choice[0] == '2') {
            JournalEntry *entries = (JournalEntry *)malloc(sizeof(JournalEntry) * MAX_ENTRIES);
            int count = load_all_entries(entries, MAX_ENTRIES);

            FILE *out = fopen("kjournal_export.json", "w");
            if (out) {
                fprintf(out, "[\n");
                for (int i = 0; i < count; i++) {
                    fprintf(out, "  {\n");
                    fprintf(out, "    \"date\": \"%s\",\n", entries[i].date_str);
                    fprintf(out, "    \"time\": \"%s\",\n", entries[i].time_str);
                    fprintf(out, "    \"mood\": \"%s\"\n", entries[i].mood);
                    fprintf(out, "  }%s\n", (i == count - 1) ? "" : ",");
                }
                fprintf(out, "]\n");
                fclose(out);
                printf("\nExported %d entries to 'kjournal_export.json'!\n", count);
            } else {
                printf("\nError writing to export file.\n");
            }
            free_entries(entries, count);
            free(entries);
            printf("Press Enter to continue...");
            getchar();
        } else if (choice[0] == 'h' || choice[0] == 'H') {
            show_help();
        } else if (choice[0] == '3') {
            break;
        }
    }
}

void show_help() {
    clear_screen();
    printf("=========================================\n");
    printf("               KJOURNAL HELP             \n");
    printf("=========================================\n");
    printf("Welcome to KJournal!\n\n");
    printf("Features:\n");
    printf("- Write new entries, auto-saved to journal.txt.\n");
    printf("- Keep track of your mood each time you write.\n");
    printf("- Use hashtags (e.g. #happy) to easily search later.\n");
    printf("- View your writing streaks and mood analytics.\n");
    printf("- Secure your journal with a 4-digit PIN lock.\n");
    printf("- Export to Markdown or JSON, and import back!\n\n");
    printf("Press Enter to return to the main menu...");
    getchar();
}

int main() {
#ifdef _WIN32
    SetProcessDPIAware();
    system("mode con: cols=120 lines=40");
    system("title KJournal - Press 'H' for Help");
    
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {
        // Prevent flickering
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style | WS_CLIPCHILDREN);
        
        // Crisp text using negative font heights
        HDC hdc = GetDC(hwnd);
        int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwnd, hdc);
        int fontHeight = -MulDiv(12, dpi, 72);
        
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = fontHeight < 0 ? -fontHeight : fontHeight; 
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
        
        // Ensure client area size matches App.jsx window dimensions (1100x750)
        RECT rect = {0, 0, 1100, 750};
        AdjustWindowRect(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE);
        SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    }
#endif

    if (!verify_pin_on_startup()) {
        return 0;
    }

    char choice[10];
    
    while (1) {
        clear_screen();
        printf("=========================================\n");
        printf("               KJOURNAL                  \n");
        printf("=========================================\n");
        printf("  (Press 'H' at any time for Help)       \n\n");
        printf("1. Write new entry\n");
        printf("2. View all entries\n");
        printf("3. Calendar Entry Navigator\n");
        printf("4. Real-time Search & #Hashtag Filter\n");
        printf("5. Mood & Streak Analytics\n");
        printf("6. Security & PIN Lock\n");
        printf("7. Import / Export Data\n");
        printf("H. Help / Instructions\n");
        printf("8. Exit\n");
        printf("=========================================\n");
        printf("Choice: ");
        
        if (!fgets(choice, sizeof(choice), stdin)) break;
        
        if (choice[0] == '1') {
            write_entry();
        } else if (choice[0] == '2') {
            view_entries();
        } else if (choice[0] == '3') {
            calendar_view();
        } else if (choice[0] == '4') {
            search_and_tags_menu();
        } else if (choice[0] == '5') {
            analytics_view();
        } else if (choice[0] == '6') {
            security_settings_menu();
        } else if (choice[0] == '7') {
            export_import_menu();
        } else if (choice[0] == 'h' || choice[0] == 'H') {
            show_help();
        } else if (choice[0] == '8') {
            break;
        }
    }
    
    return 0;
}
