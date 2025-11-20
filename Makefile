# Makefile — CLI + GTK GUI build (macOS / Linux)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src -g

# GTK flags (evaluated at make time)
GTK_CFLAGS  := $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LIBS    := $(shell pkg-config --libs   gtk+-3.0 2>/dev/null)

# CLI sources
CLI_SRC = src/main.c src/storage.c src/csv.c src/ui.c
CLI_OBJ = $(CLI_SRC:.c=.o)
CLI_TARGET = grade_system

# GUI sources
GUI_SRC = src/main_gui.c src/gui.c src/storage.c src/csv.c
# GUI object names — compile GUI sources with GTK_CFLAGS
GUI_OBJ = $(GUI_SRC:.c=.o)
GUI_TARGET = grade_system_gui

.PHONY: all gui gui_run clean

all: $(CLI_TARGET)

# CLI build (no GTK flags)
$(CLI_TARGET): $(CLI_OBJ)
	$(CC) $(CLI_OBJ) -o $(CLI_TARGET)

# Generic compile rule for normal sources (CLI)
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# GUI build: must compile GUI sources with GTK_CFLAGS so gtk/gtk.h is found
# We'll compile GUI sources with a separate rule that appends GTK_CFLAGS
# and then link with GTK_LIBS.

# Pattern rule for GUI source files: if GTK_CFLAGS is non-empty we include it.
# We force exact matching to ensure it is used for GUI build only.
src/main_gui.o: src/main_gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

src/gui.o: src/gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# storage.c and csv.c are shared; ensure compilation uses GTK_CFLAGS when building GUI target
# but avoid duplicating rules: we'll recompile them with GTK_CFLAGS if building GUI.

src/storage.o: src/storage.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

src/csv.o: src/csv.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

gui: $(GUI_TARGET)

$(GUI_TARGET): $(GUI_OBJ)
	@if [ -z "$(GTK_CFLAGS)" ] || [ -z "$(GTK_LIBS)" ]; then \
	  echo "Error: GTK not found. Make sure gtk+3 and pkg-config are installed and PKG_CONFIG_PATH is set."; \
	  echo "Try: brew install gtk+3 pkg-config"; \
	  exit 1; \
	fi
	$(CC) $(GUI_OBJ) $(GTK_LIBS) -o $(GUI_TARGET)

gui_run: gui
	./$(GUI_TARGET)

clean:
	rm -f src/*.o $(CLI_TARGET) $(GUI_TARGET)
