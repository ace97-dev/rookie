/* src/ui.c — polished minimal terminal UI */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <io.h>
  #include <windows.h>
  #define is_atty _isatty
  #define STDOUT_FD _fileno(stdout)
#else
  #include <unistd.h>
  #define is_atty isatty
  #define STDOUT_FD STDOUT_FILENO
#endif

#include "ui.h"
#include "storage.h"
#include "csv.h"

#define TERM_COLS 80
#define NAME_COL_WIDTH 30
#define GRADE_COL_WIDTH 7

/* Styling (enabled only when stdout is a TTY) */
static int use_colors = 0;
static const char *ANSI_RESET = "";
static const char *ANSI_BOLD = "";
static const char *ANSI_DIM = "";
static const char *ANSI_HEADER = "";
static const char *ANSI_OK = "";
static const char *ANSI_WARN = "";
static const char *ANSI_PROMPT = "";

static void init_style(void) {
#ifdef _WIN32
    /* Try to enable ANSI processing on Windows 10+ (no harm if it fails) */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif

    if (is_atty(STDOUT_FD)) {
        use_colors = 1;
        ANSI_RESET  = "\x1b[0m";
        ANSI_BOLD   = "\x1b[1m";
        ANSI_DIM    = "\x1b[2m";
        ANSI_HEADER = "\x1b[36m";
        ANSI_OK     = "\x1b[32m";
        ANSI_WARN   = "\x1b[33m";
        ANSI_PROMPT = "\x1b[35m";
    }
}

/* Clear screen (minimal) */
static void clear_screen(void) {
    if (use_colors) {
        printf("\x1b[2J\x1b[H");
    } else {
        putchar('\n'); putchar('\n'); putchar('\n');
    }
}

/* Header and footer */
static void draw_header(void) {
    char line[TERM_COLS + 1];
    for (int i = 0; i < TERM_COLS; ++i) line[i] = '-';
    line[TERM_COLS] = '\0';

    if (use_colors) printf("%s%s%s\n", ANSI_DIM, line, ANSI_RESET);
    else puts(line);

    if (use_colors) printf("%s%s  Student Grade Manager  %s\n", ANSI_HEADER, ANSI_BOLD, ANSI_RESET);
    else puts("  Student Grade Manager  ");

    if (use_colors) printf("%s%s%s\n", ANSI_DIM, line, ANSI_RESET);
    else puts(line);
}

static void draw_footer_hint(const char *hint) {
    if (use_colors) printf("%s%s%s\n\n", ANSI_DIM, hint, ANSI_RESET);
    else { puts(hint); putchar('\n'); }
}

/* Safe line input */
static void read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* Prompt helper */
static void prompt(const char *label, char *out, size_t n) {
    if (use_colors) printf("%s> %s%s", ANSI_PROMPT, label, ANSI_RESET);
    else printf("> %s", label);
    fflush(stdout);
    read_line(out, n);
}

/* Confirmation prompt (y/n) */
static int confirm(const char *message) {
    char ans[8];
    for (;;) {
        prompt(message, ans, sizeof(ans));
        if (ans[0] == '\0') return 0;
        if (ans[0] == 'y' || ans[0] == 'Y') return 1;
        if (ans[0] == 'n' || ans[0] == 'N') return 0;
        if (use_colors) printf("%sPlease answer y or n.%s\n", ANSI_WARN, ANSI_RESET);
        else puts("Please answer y or n.");
    }
}

/* Print table of students */
static void print_students_table(void) {
    size_t cnt = get_storage_count();
    Student *arr = get_storage_array();

    if (cnt == 0) {
        if (use_colors) printf("%sNo students found.%s\n", ANSI_DIM, ANSI_RESET);
        else puts("No students found.");
        return;
    }

    if (use_colors) printf("%s%-5s %-*s %-*s%s\n", ANSI_BOLD, "ID", NAME_COL_WIDTH, "Name", GRADE_COL_WIDTH, "Grade", ANSI_RESET);
    else printf("%-5s %-*s %-*s\n", "ID", NAME_COL_WIDTH, "Name", GRADE_COL_WIDTH, "Grade");

    for (int i = 0; i < 5 + 1 + NAME_COL_WIDTH + 1 + GRADE_COL_WIDTH; ++i) putchar('-');
    putchar('\n');

    for (size_t i = 0; i < cnt; ++i) {
        if (use_colors && (i % 2 == 1)) printf("%s", ANSI_DIM);
        printf("%-5d %-*s %*.2f\n", arr[i].id, NAME_COL_WIDTH, arr[i].name, GRADE_COL_WIDTH - 1, arr[i].grade);
        if (use_colors && (i % 2 == 1)) printf("%s", ANSI_RESET);
    }
}

/* Show average */
static void show_average(void) {
    size_t cnt = get_storage_count();
    if (cnt == 0) {
        if (use_colors) printf("%sNo students to average.%s\n", ANSI_DIM, ANSI_RESET);
        else puts("No students to average.");
        return;
    }
    double avg = compute_average();
    if (use_colors) printf("%sClass average: %s%.2f%s\n", ANSI_OK, ANSI_BOLD, avg, ANSI_RESET);
    else printf("Class average: %.2f\n", avg);
}

/* Public menu implementation */
void menu(void) {
    init_style();
    clear_screen();
    draw_header();
    draw_footer_hint("Tip: Type the option number and press Enter. Use Ctrl+C to quit.");

    char choice[16];
    for (;;) {
        if (use_colors) {
            printf("%s1) List%s   %s2) Add%s   %s3) Remove%s   %s4) Average\n",
                   ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET, ANSI_BOLD);
            printf("%s5) Save%s   %s6) Sort name%s   %s7) Sort grade%s   %s8) Exit%s\n",
                   ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET);
        } else {
            printf("1) List   2) Add   3) Remove   4) Average\n");
            printf("5) Save   6) Sort name   7) Sort grade   8) Exit\n");
        }

        prompt("Choose:", choice, sizeof(choice));

        if (strcmp(choice, "1") == 0) {
            clear_screen();
            draw_header();
            print_students_table();
        }
        else if (strcmp(choice, "2") == 0) {
            char name[NAME_LENGTH];
            char grade_str[32];

            prompt("Student name:", name, sizeof(name));
            if (strlen(name) == 0) {
                if (use_colors) printf("%sName cannot be empty.%s\n", ANSI_WARN, ANSI_RESET);
                else puts("Name cannot be empty.");
                continue;
            }

            prompt("Grade (0-100):", grade_str, sizeof(grade_str));
            double g = atof(grade_str);
            if (g < 0 || g > 100) {
                if (use_colors) printf("%sInvalid grade.%s\n", ANSI_WARN, ANSI_RESET);
                else puts("Invalid grade.");
                continue;
            }

            add_student(name, g);
        }
        else if (strcmp(choice, "3") == 0) {
            char idstr[16];
            prompt("ID to remove:", idstr, sizeof(idstr));
            int id = atoi(idstr);
            if (id <= 0) {
                if (use_colors) printf("%sInvalid ID.%s\n", ANSI_WARN, ANSI_RESET);
                else puts("Invalid ID.");
                continue;
            }
            char msg[128];
            snprintf(msg, sizeof(msg), "Delete student %d? (y/n)", id);
            if (confirm(msg)) remove_student(id);
            else {
                if (use_colors) printf("%sCancelled.%s\n", ANSI_DIM, ANSI_RESET);
                else puts("Cancelled.");
            }
        }
        else if (strcmp(choice, "4") == 0) {
            show_average();
        }
        else if (strcmp(choice, "5") == 0) {
            save_to_file("data/students.csv");
        }
        else if (strcmp(choice, "6") == 0) {
            sort_by_name();
        }
        else if (strcmp(choice, "7") == 0) {
            sort_by_grade_desc();
        }
        else if (strcmp(choice, "8") == 0) {
            if (confirm("Save changes and exit? (y/n)")) {
                save_to_file("data/students.csv");
                if (use_colors) printf("%sGoodbye!%s\n", ANSI_OK, ANSI_RESET);
                else puts("Goodbye!");
                break;
            } else {
                if (use_colors) printf("%sExit cancelled.%s\n", ANSI_DIM, ANSI_RESET);
                else puts("Exit cancelled.");
            }
        }
        else {
            if (use_colors) printf("%sInvalid option.%s\n", ANSI_WARN, ANSI_RESET);
            else puts("Invalid option.");
        }
    }
}
