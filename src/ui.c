#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "storage.h"
#include "csv.h"

static void read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}

void menu(void) {
    char choice[16];
    for (;;) {
        puts("\n--- Student Grade Manager ---");
        puts("1) List students");
        puts("2) Add student");
        puts("3) Remove student by ID");
        puts("4) Class average");
        puts("5) Save (manual)");
        puts("6) Sort by name");
        puts("7) Sort by grade (desc)");
        puts("8) Exit");
        printf("Choose: ");
        read_line(choice, sizeof(choice));

        if (strcmp(choice, "1") == 0) {
            list_students();
        }
        else if (strcmp(choice, "2") == 0) {
            char name[NAME_LENGTH];
            char grades[32];
            printf("Name: ");
            read_line(name, sizeof(name));
            if (strlen(name) == 0) { puts("Name cannot be empty."); continue; }

            printf("Grade (0-100): ");
            read_line(grades, sizeof(grades));
            double g = atof(grades);
            if (g < 0 || g > 100) { puts("Invalid grade."); continue; }

            add_student(name, g);
        }
        else if (strcmp(choice, "3") == 0) {
            char idstr[16];
            printf("ID to remove: ");
            read_line(idstr, sizeof(idstr));
            int id = atoi(idstr);
            if (id <= 0) { puts("Invalid ID."); continue; }
            remove_student(id);
        }
        else if (strcmp(choice, "4") == 0) {
            double avg = compute_average();
            if (avg == 0.0) puts("No students to average.");
            else printf("Class average: %.2f\n", avg);
        }
        else if (strcmp(choice, "5") == 0) {
            /* manual save via storage+csv in main - expose save via csv.h */
            save_to_file("data/students.csv");
        }
        else if (strcmp(choice, "6") == 0) {
            sort_by_name();
        }
        else if (strcmp(choice, "7") == 0) {
            sort_by_grade_desc();
        }
        else if (strcmp(choice, "8") == 0) {
            puts("Exiting. Changes will be saved.");
            break;
        }
        else {
            puts("Invalid option.");
        }
    }
}
