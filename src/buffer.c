#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer *b) {
    b->lines = malloc(sizeof(char*));
    if (b->lines) {
        b->lines[0] = strdup("");
        b->line_count = 1;
    } else {
        b->line_count = 0;
    }
}

void buffer_load(Buffer *b, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        buffer_init(b);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    // Liberar buffer actual
    for (int i = 0; i < b->line_count; i++) {
        free(b->lines[i]);
    }
    free(b->lines);
    
    b->lines = NULL;
    b->line_count = 0;

    while ((read = getline(&line, &len, f)) != -1) {
        size_t l = strlen(line);
        if (l > 0 && line[l-1] == '\n') line[l-1] = '\0';
        b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
        b->lines[b->line_count++] = strdup(line);
    }

    free(line);
    fclose(f);

    if (b->line_count == 0) {
        buffer_init(b);
    }
}

void buffer_save(Buffer *b, const char *filename) {
    if (!filename) return;
    FILE *f = fopen(filename, "w");
    if (!f) return;

    for (int i = 0; i < b->line_count; i++) {
        fputs(b->lines[i], f);
        fputc('\n', f);
    }

    fclose(f);
}

void buffer_free(Buffer *b) {
    for (int i = 0; i < b->line_count; i++) {
        free(b->lines[i]);
    }
    free(b->lines);
    b->line_count = 0;
    b->lines = NULL;
}

void buffer_insert_char(Buffer *b, int x, int y, char c) {
    if (y >= b->line_count) return;
    char *line = b->lines[y];
    int len = strlen(line);
    
    if (x > len) x = len;
    if (x < 0) x = 0;

    char *new_line = malloc(len + 2);
    if (!new_line) return;
    
    memcpy(new_line, line, x);
    new_line[x] = c;
    memcpy(new_line + x + 1, line + x, len - x + 1);

    free(line);
    b->lines[y] = new_line;
}

void buffer_delete_char(Buffer *b, int x, int y) {
    if (y >= b->line_count || x <= 0) return;
    char *line = b->lines[y];
    int len = strlen(line);
    
    if (x > len) x = len;
    if (x <= 0) return;
    
    memmove(line + x - 1, line + x, len - x + 1);
}

void buffer_insert_line(Buffer *b, int y) {
    if (y < 0 || y >= b->line_count) return;
    
    b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
    if (!b->lines) return;
    
    for (int i = b->line_count; i > y + 1; i--) {
        b->lines[i] = b->lines[i - 1];
    }
    b->lines[y + 1] = strdup("");
    b->line_count++;
}

void buffer_split_line(Buffer *b, int x, int y) {
    if (y >= b->line_count) return;
    
    char *line = b->lines[y];
    int len = strlen(line);
    if (x > len) x = len;
    if (x < 0) x = 0;
    
    char *right = strdup(line + x);
    line[x] = '\0';
    
    b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
    if (!b->lines) {
        free(right);
        return;
    }
    
    for (int i = b->line_count; i > y + 1; i--) {
        b->lines[i] = b->lines[i - 1];
    }
    b->lines[y + 1] = right;
    b->line_count++;
}