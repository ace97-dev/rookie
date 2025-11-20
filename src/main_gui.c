/* src/main_gui.c
   Small main() that initializes storage, loads CSV, runs the GTK GUI and saves on exit.
*/

#include <gtk/gtk.h>
#include "storage.h"
#include "csv.h"

/* Prototype for GUI builder returning a window widget */
GtkWidget *build_main_window(void);

int main(int argc, char *argv[]) {
    /* initialize storage */
    init_storage();

    /* load data */
    load_from_file("data/students.csv");

    /* init GTK */
    gtk_init(&argc, &argv);

    GtkWidget *win = build_main_window();
    gtk_widget_show_all(win);

    /* GTK main loop */
    gtk_main();

    /* save and cleanup */
    save_to_file("data/students.csv");
    free_storage();
    return 0;
}
