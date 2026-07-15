#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_LINE 1024
#define JOURNAL_FILE "journal.txt"

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void write_entry() {
    clear_screen();
    printf("--- Write Journal Entry ---\n");
    printf("Enter your thoughts. Type 'EOF' on a new line to save and return.\n\n");

    FILE *f = fopen(JOURNAL_FILE, "a");
    if (!f) {
        printf("Error opening %s\n", JOURNAL_FILE);
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "\n=== Entry: %d-%02d-%02d %02d:%02d:%02d ===\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "EOF", 3) == 0 && (line[3] == '\n' || line[3] == '\0' || line[3] == '\r')) {
            break;
        }
        fprintf(f, "%s", line);
    }
    
    fclose(f);
    printf("\nEntry saved!\n");
    printf("Press Enter to continue...");
    getchar();
}

void view_entries() {
    clear_screen();
    printf("--- Journal Entries ---\n\n");
    
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

void search_entries() {
    clear_screen();
    printf("--- Search Journal ---\n");
    printf("Enter search keyword: ");
    char keyword[MAX_LINE];
    if (!fgets(keyword, sizeof(keyword), stdin)) return;
    keyword[strcspn(keyword, "\r\n")] = '\0';
    if (strlen(keyword) == 0) return;

    FILE *f = fopen(JOURNAL_FILE, "r");
    if (!f) {
        printf("No journal entries found.\n");
        printf("\nPress Enter to return...");
        getchar();
        return;
    }

    char line[MAX_LINE];
    char current_header[MAX_LINE] = "";
    char entry_buffer[8192] = "";
    int in_entry = 0;
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "=== Entry:", 10) == 0) {
            if (in_entry && strcasestr_custom(entry_buffer, keyword) != NULL) {
                printf("\n%s%s", current_header, entry_buffer);
                found++;
            }
            strcpy(current_header, line);
            entry_buffer[0] = '\0';
            in_entry = 1;
        } else if (in_entry) {
            if (strlen(entry_buffer) + strlen(line) < sizeof(entry_buffer)) {
                strcat(entry_buffer, line);
            }
        }
    }
    if (in_entry && strcasestr_custom(entry_buffer, keyword) != NULL) {
        printf("\n%s%s", current_header, entry_buffer);
        found++;
    }

    fclose(f);
    if (found == 0) printf("\nNo entries found containing '%s'.\n", keyword);
    else printf("\nFound %d matching entries.\n", found);

    printf("\nPress Enter to return...");
    getchar();
}

int main() {
    char choice[10];
    
    while (1) {
        clear_screen();
        printf("======================\n");
        printf("      KJOURNAL        \n");
        printf("======================\n");
        printf("1. Write new entry\n");
        printf("2. View entries\n");
        printf("3. Search entries\n");
        printf("4. Exit\n");
        printf("======================\n");
        printf("Choice: ");
        
        if (!fgets(choice, sizeof(choice), stdin)) break;
        
        if (choice[0] == '1') {
            write_entry();
        } else if (choice[0] == '2') {
            view_entries();
        } else if (choice[0] == '3') {
            search_entries();
        } else if (choice[0] == '4') {
            break;
        }
    }
    
    return 0;
}
