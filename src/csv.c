#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csv.h"
#include "storage.h"


#define LINE_LEN 1024

/* Helper: trim newline/end CR */
static void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[n-1] = '\0';
}

/* Save students to CSV.
   Names containing commas or quotes are quoted and quotes doubled per CSV rules.
*/
void save_to_file(const char *filename) {
    if (!filename) return;
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    Student *arr = get_storage_array();
    size_t cnt = get_storage_count();

    for (size_t i = 0; i < cnt; ++i) {
        const char *name = arr[i].name;
        int needs_quotes = strchr(name, ',') || strchr(name, '"') || strchr(name, '\n');
        if (needs_quotes) {
            fputc('"', f);
            for (const char *p = name; *p; ++p) {
                if (*p == '"') { fputc('"', f); fputc('"', f); }
                else fputc(*p, f);
            }
            fputc('"', f);
            fprintf(f, ",%.2f\n", arr[i].grade);
        } else {
            fprintf(f, "%d,%s,%.2f\n", arr[i].id, name, arr[i].grade);
        }
    }

    fclose(f);
    printf("Saved %zu students to %s\n", cnt, filename);
}

/* Parse a single CSV line into id, name, grade.
   Returns 1 on success, 0 on failure.
   Handles quoted name fields with doubled quotes.
*/
static int parse_csv_line(const char *line, int *out_id, char out_name[NAME_LENGTH], double *out_grade) {
    const char *p = line;
    /* parse id */
    while (*p && isspace((unsigned char)*p)) p++;
    char idbuf[32];
    size_t i = 0;
    while (*p && *p != ',') {
        if (i + 1 < sizeof(idbuf)) idbuf[i++] = *p;
        p++;
    }
    idbuf[i] = '\0';
    if (*p != ',') return 0;
    p++; /* skip comma */
    int id = atoi(idbuf);

    /* parse name - support quoted and unquoted */
    char namebuf[NAME_LENGTH] = {0};
    if (*p == '"') {
        p++; /* skip opening quote */
        size_t ni = 0;
        while (*p) {
            if (*p == '"') {
                if (p[1] == '"') {
                    /* escaped quote */
                    if (ni + 1 < NAME_LENGTH - 1) namebuf[ni++] = '"';
                    p += 2;
                } else {
                    /* closing quote */
                    p++;
                    break;
                }
            } else {
                if (ni + 1 < NAME_LENGTH - 1) namebuf[ni++] = *p;
                p++;
            }
        }
        namebuf[ni] = '\0';
        /* skip until comma (should be at comma after quote) */
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    } else {
        /* unquoted name: read until comma */
        size_t ni = 0;
        while (*p && *p != ',') {
            if (ni + 1 < NAME_LENGTH - 1) namebuf[ni++] = *p;
            p++;
        }
        namebuf[ni] = '\0';
        if (*p == ',') p++;
    }

    /* parse grade (rest of line) */
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') return 0;
    char *endptr;
    double grade = strtod(p, &endptr);
    if (p == endptr) return 0;

    /* copy out */
    *out_id = id;
    strncpy(out_name, namebuf, NAME_LENGTH - 1);
    out_name[NAME_LENGTH - 1] = '\0';
    *out_grade = grade;
    return 1;
}

/* Load all students from file. This replaces the in-memory array. */
void load_from_file(const char *filename) {
    if (!filename) return;
    FILE *f = fopen(filename, "r");
    if (!f) {
        /* no file yet is OK */
        return;
    }
    char line[LINE_LEN];
    Student *arr = NULL;
    size_t cap = 0;
    size_t cnt = 0;
    int max_id = 0;

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (line[0] == '\0') continue;
        int id;
        char name[NAME_LENGTH];
        double grade;
        if (!parse_csv_line(line, &id, name, &grade)) {
            fprintf(stderr, "Warning: failed to parse line: %s\n", line);
            continue;
        }
        if (cnt >= cap) {
            size_t newcap = cap == 0 ? 8 : cap * 2;
            Student *tmp = realloc(arr, newcap * sizeof(Student));
            if (!tmp) {
                fprintf(stderr, "Memory allocation failed while loading CSV\n");
                free(arr);
                fclose(f);
                return;
            }
            arr = tmp;
            cap = newcap;
        }
        arr[cnt].id = id;
        strncpy(arr[cnt].name, name, NAME_LENGTH - 1);
        arr[cnt].name[NAME_LENGTH - 1] = '\0';
        arr[cnt].grade = grade;
        cnt++;
        if (id > max_id) max_id = id;
    }
    fclose(f);

    /* Replace storage content with the loaded array.
       next_id should be max_id + 1 so we don't reuse ids. */
    replace_storage_content(arr, cnt, max_id + 1);
    printf("Loaded %zu students from %s\n", cnt, filename);
}
