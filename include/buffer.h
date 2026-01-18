#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
    char **lines;
    int line_count;
} Buffer;

void buffer_init(Buffer *b);
void buffer_load(Buffer *b, const char *filename);
void buffer_save(Buffer *b, const char *filename);
void buffer_free(Buffer *b);

void buffer_insert_char(Buffer *b, int x, int y, char c);
void buffer_delete_char(Buffer *b, int x, int y);
void buffer_insert_line(Buffer *b, int y);
void buffer_split_line(Buffer *b, int x, int y);

#endif