#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "config.h"

typedef struct {
    int cx, cy;             // cursor
    int row_offset;         // scroll vertical
    int col_offset;         // scroll horizontal
    int screenrows;         // filas visibles
    int screencols;         // columnas visibles
    EditorMode mode;
    Buffer buffer;
    const char *filename;
} Editor;

void editor_init(Editor *e, const char *filename);
void editor_refresh_screen(Editor *e);
void editor_process_key(Editor *e);

#endif
