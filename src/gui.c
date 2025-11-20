/* src/gui.c
   GTK3-based GUI for Student Grade Manager.
   Uses the existing storage.h / csv.h API.
*/

#define _POSIX_C_SOURCE 200809L
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage.h"
#include "csv.h"

/* Columns in the GtkListStore */
enum {
    COL_ID = 0,
    COL_NAME,
    COL_GRADE_STR,
    N_COLUMNS
};

/* Small context passed to callbacks */
typedef struct {
    GtkListStore *store;
    GtkWidget *tree;
} AppContext;

/* Forward declarations */
static void refresh_list(AppContext *ctx);
static void on_add(GtkButton *button, gpointer user_data);
static void on_remove(GtkButton *button, gpointer user_data);
static void on_save(GtkButton *button, gpointer user_data);
static void on_sort_name(GtkButton *button, gpointer user_data);
static void on_sort_grade(GtkButton *button, gpointer user_data);
static void on_average(GtkButton *button, gpointer user_data);

/* Build the main window */
GtkWidget *build_main_window(void) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Student Grade Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 480);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    /* Vertical box */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* Toolbar / buttons */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *btn_add = gtk_button_new_with_label("Add");
    GtkWidget *btn_remove = gtk_button_new_with_label("Remove");
    GtkWidget *btn_save = gtk_button_new_with_label("Save");
    GtkWidget *btn_sort_name = gtk_button_new_with_label("Sort by Name");
    GtkWidget *btn_sort_grade = gtk_button_new_with_label("Sort by Grade");
    GtkWidget *btn_avg = gtk_button_new_with_label("Average");

    gtk_box_pack_start(GTK_BOX(hbox), btn_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_remove, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_sort_name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), btn_sort_grade, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_avg, FALSE, FALSE, 0);

    /* Scrolled window with treeview */
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    /* Use strings for name and grade display (simple and portable) */
    GtkListStore *store = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add(GTK_CONTAINER(scrolled), tree);

    /* Columns */
    GtkCellRenderer *renderer;

    /* ID column */
    renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col_id = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", COL_ID, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_id);

    /* Name column */
    renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col_name = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COL_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_name);

    /* Grade column (string) */
    renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col_grade = gtk_tree_view_column_new_with_attributes("Grade", renderer, "text", COL_GRADE_STR, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_grade);

    /* Allocate and populate context */
    AppContext *ctx = g_new0(AppContext, 1);
    ctx->store = store;
    ctx->tree = tree;

    /* Connect signals */
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add), ctx);
    g_signal_connect(btn_remove, "clicked", G_CALLBACK(on_remove), ctx);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save), ctx);
    g_signal_connect(btn_sort_name, "clicked", G_CALLBACK(on_sort_name), ctx);
    g_signal_connect(btn_sort_grade, "clicked", G_CALLBACK(on_sort_grade), ctx);
    g_signal_connect(btn_avg, "clicked", G_CALLBACK(on_average), ctx);

    /* When window is closed, quit GTK loop (we save in main before exit) */
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* Initial fill */
    refresh_list(ctx);

    return window;
}

/* Helper: refresh list store from storage */
static void refresh_list(AppContext *ctx) {
    gtk_list_store_clear(ctx->store);
    Student *arr = get_storage_array();
    size_t cnt = get_storage_count();
    for (size_t i = 0; i < cnt; ++i) {
        GtkTreeIter iter;
        char gradebuf[32];
        snprintf(gradebuf, sizeof(gradebuf), "%.2f", arr[i].grade);
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
                           COL_ID, arr[i].id,
                           COL_NAME, arr[i].name,
                           COL_GRADE_STR, gradebuf,
                           -1);
    }
}

/* Dialog: add a new student */
static void on_add(GtkButton *button, gpointer user_data) {
    /* avoid unused-parameter warnings */
    (void)button;
    AppContext *ctx = (AppContext *)user_data;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Student",
                                                    NULL,
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_Add", GTK_RESPONSE_ACCEPT,
                                                    "_Cancel", GTK_RESPONSE_REJECT,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget *lbl_name = gtk_label_new("Name:");
    GtkWidget *ent_name = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_name, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_name, 1, 0, 1, 1);

    GtkWidget *lbl_grade = gtk_label_new("Grade:");
    GtkWidget *ent_grade = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ent_grade), "0-100");
    gtk_grid_attach(GTK_GRID(grid), lbl_grade, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_grade, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(ent_name));
        const char *grade_text = gtk_entry_get_text(GTK_ENTRY(ent_grade));
        double g = atof(grade_text);
        if (name && name[0] != '\0' && g >= 0 && g <= 100) {
            add_student(name, g);
            refresh_list(ctx);
        } else {
            GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                                    "Invalid input. Name must not be empty and grade must be 0-100.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }
    }

    gtk_widget_destroy(dialog);
}

/* Remove selected student */
static void on_remove(GtkButton *button, gpointer user_data) {
    (void)button;
    AppContext *ctx = (AppContext *)user_data;
    GtkWidget *tree = ctx->tree;

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int id = 0;
        gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
        /* Confirm */
        char msg[128];
        snprintf(msg, sizeof(msg), "Delete student with ID %d?", id);
        GtkWidget *confirm = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", msg);
        gint resp = gtk_dialog_run(GTK_DIALOG(confirm));
        gtk_widget_destroy(confirm);
        if (resp == GTK_RESPONSE_YES) {
            remove_student(id);
            refresh_list(ctx);
        }
    } else {
        GtkWidget *info = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                 "No student selected.");
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    }
}

/* Save to file */
static void on_save(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    save_to_file("data/students.csv");
}

/* Sort by name */
static void on_sort_name(GtkButton *button, gpointer user_data) {
    (void)button;
    AppContext *ctx = (AppContext *)user_data;
    sort_by_name();
    refresh_list(ctx);
}

/* Sort by grade */
static void on_sort_grade(GtkButton *button, gpointer user_data) {
    (void)button;
    AppContext *ctx = (AppContext *)user_data;
    sort_by_grade_desc();
    refresh_list(ctx);
}

/* Show average dialog */
static void on_average(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    double avg = compute_average();
    char msg[128];
    if (get_storage_count() == 0) {
        snprintf(msg, sizeof(msg), "No students to average.");
    } else {
        snprintf(msg, sizeof(msg), "Class average: %.2f", avg);
    }
    GtkWidget *info = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(info));
    gtk_widget_destroy(info);
}
