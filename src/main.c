

#include <stdio.h>
#include "storage.h"
#include "csv.h"
#include "ui.h"

#define DATA_FILE "data/students.csv"

int main(void) {
     /* Initialize storage (optional here, storage starts empty) */
    init_storage();

    /* Load persisted students (if file exists) */
    load_from_file(DATA_FILE);

    /* Run the interactive menu */
    menu();

    /* Save data on exit */
    save_to_file(DATA_FILE);

    /* Cleanup */
    free_storage();
    return 0;
}

