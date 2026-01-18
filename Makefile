CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Iinclude

SRC = src/main.c src/editor.c src/terminal.c src/buffer.c src/fileio.c
OBJ = $(SRC:.c=.o)

TARGET = minied

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean $(TARGET)
