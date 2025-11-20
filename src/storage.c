#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "storage.h"

static Student *students = NULL;
static size_t count = 0;
static size_t capacity = 0;
static int next_id = 1;

static void ensure_capacity(void) {
    if (capacity==0) {
        capacity = 8;
        students = malloc(capacity * sizeof(Student));
         if (!students) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    } else if (count >= capacity) {
        size_t newcap = capacity * 2;
        Student *tmp = realloc(students, newcap * sizeof(Student));
        if (!tmp) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
        students = tmp;
        capacity = newcap;
    }
}

void init_storage(void) {
    /* start empty; ensure variables are sane */
    /* (students NULL, count 0, capacity 0, next_id 1) */
    /* This function exists for explicit initialization if needed. */
    /* Nothing required here for now. */
}

void free_storage() {
    free(students);
    students = NULL;
    count = 0;
    capacity = 0;
    next_id = 1;
}

void add_student(const char *name, double grade) {
    if (!name) return;
    ensure_capacity();
    students[count].id = next_id++;
    strncpy(students[count].name, name, NAME_LENGTH - 1);
    students[count].name[NAME_LENGTH- 1] = '\0';
    students[count].grade = grade;
    count++;
    printf("Added student (id=%d)\n", next_id - 1);

}

void remove_student(int id) {
    size_t idx = SIZE_MAX;
    for (size_t i = 0; i < count; ++i) {
        if (students[i].id == id) { idx = i; break; }
    }
    if (idx == SIZE_MAX) {
        printf("No student with id %d\n", id);
        return;
    }
    for (size_t i = idx; i + 1 < count; ++i) students[i] = students[i+1];
    count--;
    printf("Removed student id %d\n", id);
}

void list_students(void) {
    if (count == 0) {
        puts("No students found.");
        return;
    }
    printf("%-5s %-30s %-6s\n", "ID", "Name", "Grade");
    puts("-------------------------------------------------");
    for (size_t i = 0; i < count; ++i) {
        printf("%-5d %-30s %-6.2f\n", students[i].id, students[i].name, students[i].grade);
    }
}

static int cmp_name(const void *a, const void *b) {
    const Student *sa = a;
    const Student *sb = b;
#if defined(_WIN32) || defined(_WIN64)
    return _stricmp(sa->name, sb->name);
#else
    return strcasecmp(sa->name, sb->name);
#endif
}

void sort_by_name(void) {
    if (count > 1) qsort(students, count, sizeof(Student), cmp_name);
    puts("Sorted by name.");
}

static int cmp_grade_desc(const void *a, const void *b) {
    const Student *sa = a;
    const Student *sb = b;
    if (sa->grade < sb->grade) return 1;
    if (sa->grade > sb->grade) return -1;
    return 0;
}

void sort_by_grade_desc(void) {
    if (count > 1) qsort(students, count, sizeof(Student), cmp_grade_desc);
    puts("Sorted by grade (desc).");
}

double compute_average(void) {
    if (count == 0) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < count; ++i) sum += students[i].grade;
    return sum / (double)count;
}

/* Expose minimal internals to csv.c */
Student *get_storage_array(void) { return students; }
size_t get_storage_count(void) { return count; }

void replace_storage_content(Student *arr, size_t new_count, int new_next_id) {
    /* free old array (we choose to free the old one) and replace */
    free(students);
    students = arr;
    count = new_count;
    capacity = (arr != NULL) ? new_count : 0;
    /* Ensure capacity has sensible minimum if we will add more later */
    if (capacity < 8) capacity = 8;
    next_id = new_next_id;
}

