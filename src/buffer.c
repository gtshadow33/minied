#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inicializa buffer vacío */
void buffer_init(Buffer *b) {
    b->lines = malloc(sizeof(char*));
    b->lines[0] = strdup("");
    b->line_count = 1;
}

/* Carga archivo al buffer */
void buffer_load(Buffer *b, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    char *line = NULL;
    size_t len = 0;
    b->line_count = 0;
    free(b->lines);
    b->lines = NULL;

    while (getline(&line, &len, f) != -1) {
        size_t l = strlen(line);
        if (l > 0 && line[l-1] == '\n') line[l-1] = '\0';
        b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
        b->lines[b->line_count++] = strdup(line);
    }

    free(line);
    fclose(f);

    if (b->line_count == 0)
        buffer_init(b);
}

/* Guarda buffer en archivo */
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

/* Libera memoria del buffer */
void buffer_free(Buffer *b) {
    for (int i = 0; i < b->line_count; i++)
        free(b->lines[i]);
    free(b->lines);
}

/* Inserta un carácter en línea y posición dadas */
void buffer_insert_char(Buffer *b, int x, int y, char c) {
    if (y >= b->line_count) return;
    char *line = b->lines[y];
    int len = strlen(line);
    if (x > len) x = len;

    char *new_line = malloc(len + 2);
    memcpy(new_line, line, x);
    new_line[x] = c;
    memcpy(new_line + x + 1, line + x, len - x + 1);

    free(line);
    b->lines[y] = new_line;
}

/* Borra un carácter de línea */
void buffer_delete_char(Buffer *b, int x, int y) {
    if (y >= b->line_count || x <= 0) return;
    char *line = b->lines[y];
    int len = strlen(line);
    if (x > len) return;
    memmove(line + x - 1, line + x, len - x + 1);
}

/* Inserta línea vacía después de la línea y */
void buffer_insert_line(Buffer *b, int y) {
    b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
    for (int i = b->line_count; i > y + 1; i--)
        b->lines[i] = b->lines[i - 1];
    b->lines[y + 1] = strdup("");
    b->line_count++;
}

/* Divide la línea actual en Enter */
void buffer_split_line(Buffer *b, int x, int y) {
    if (y >= b->line_count) return;

    char *line = b->lines[y];
    int len = strlen(line);
    if (x > len) x = len;

    char *right = strdup(line + x);
    line[x] = '\0';

    b->lines = realloc(b->lines, sizeof(char*) * (b->line_count + 1));
    for (int i = b->line_count; i > y + 1; i--)
        b->lines[i] = b->lines[i - 1];
    b->lines[y + 1] = right;
    b->line_count++;
}
