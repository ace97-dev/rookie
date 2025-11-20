# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src -g

# Source files
SRC = src/main.c src/storage.c src/csv.c src/ui.c
OBJ = $(SRC:.c=.o)

# Output binary
TARGET = grade_system

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# compile each source
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
