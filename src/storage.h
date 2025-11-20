#ifndef STORAGE_H
#define STORAGE_H


#include <stddef.h>

#define NAME_LENGTH 100

typedef struct {
    int id;
    char name[NAME_LENGTH];
    double grade;
} Student;

/* lifecycle */
void init_storage(void);
void free_storage(void);

/* operations */
void add_student(const char *name, double grade);
void remove_student(int id);
void list_students(void);
void sort_by_name(void);
void sort_by_grade_desc();
double compute_average(void);

/* helpers used by csv.c (expose minimal internals) */
Student *get_storage_array(void);
size_t get_storage_count(void);
/* replace_storage_content takes ownership of arr (caller allocated),
   new_next_id is the next id to use for newly added students */
void replace_storage_content(Student *arr, size_t new_count, int new_next_id);

#endif /* STORAGE_H */
