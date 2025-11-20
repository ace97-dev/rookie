#include "storage.h"
#include "csv.h"
#include "ui.h"

#define DATA_FILE "data/students.csv"

int main(void) {
    load_from_file(DATA_FILE);
    menu();
    save_to_file(DATA_FILE);
    free_storage();
    return 0;
}

