#include "editor.h"
#include "fileio.h"
#include "terminal.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

/* Obtiene tamaño de ventana terminal */
static void get_window_size(int *rows, int *cols) {
    struct winsize ws;
    ioctl(1, TIOCGWINSZ, &ws);
    *rows = ws.ws_row;
    *cols = ws.ws_col;
}

/* Limpia pantalla y mueve cursor al inicio */
static void clear_screen(void) {
    write(1, "\x1b[2J\x1b[H", 7);
}

/* Inicializa editor */
void editor_init(Editor *e, const char *filename) {
    e->cx = 0; e->cy = 0;
    e->row_offset = 0; e->col_offset = 0;
    e->mode = MODE_NORMAL;
    get_window_size(&e->screenrows, &e->screencols);
    buffer_init(&e->buffer);
    e->filename = filename;
    if (filename) load_file(&e->buffer, filename);
}

/* Dibuja buffer, barra de estado y cursor */
void editor_refresh_screen(Editor *e) {
    // 1️⃣ Limpia toda la pantalla y mueve cursor a 0,0
    write(1, "\x1b[2J\x1b[H", 7);

    // 2️⃣ Dibuja todas las líneas visibles del buffer
    for (int y = 0; y < e->buffer.line_count && y < e->screenrows; y++) {
        int len = strlen(e->buffer.lines[y]);
        int col_start = e->col_offset;
        if (len > col_start)
            write(1, e->buffer.lines[y] + col_start, len - col_start);
        write(1, "\r\n", 2); // salto de línea
    }

    // 3️⃣ Dibuja la barra de estado al final
    char status[80];
    snprintf(status, sizeof(status),
             "-- %s --  Ctrl+O Guardar | Ctrl+X Salir",
             e->mode == MODE_INSERT ? "INSERT" : "NORMAL");
    write(1, status, strlen(status));

    // 4️⃣ Mueve cursor a la posición actual
    char cursor[32];
    snprintf(cursor, sizeof(cursor), "\x1b[%d;%dH",
             e->cy + 1 - e->row_offset,
             e->cx + 1 - e->col_offset);
    write(1, cursor, strlen(cursor));
}


/* Procesa tecla presionada */
void editor_process_key(Editor *e) {
    char c;
    read(0, &c, 1);

    if (e->mode == MODE_NORMAL) {
        switch (c) {
            case 'h': if (e->cx > 0) e->cx--; break;
            case 'l': e->cx++; break;
            case 'k': if (e->cy > 0) e->cy--; break;
            case 'j': if (e->cy < e->buffer.line_count - 1) e->cy++; break;
            case 'i': e->mode = MODE_INSERT; break;
            case CTRL_KEY('o'): save_file(&e->buffer, e->filename); break;
            case CTRL_KEY('x'):
                buffer_free(&e->buffer);
                clear_screen();
                exit(0); // restaura terminal
        }
    } else { // Modo INSERT
        if (c == 27) { e->mode = MODE_NORMAL; return; } // ESC

        if (c == 127) { // Backspace
            if (e->cx > 0) {
                buffer_delete_char(&e->buffer, e->cx, e->cy);
                e->cx--;
            } else if (e->cy > 0) {
                int prev_len = strlen(e->buffer.lines[e->cy - 1]);
                char *new_line = malloc(prev_len + strlen(e->buffer.lines[e->cy]) + 1);
                strcpy(new_line, e->buffer.lines[e->cy - 1]);
                strcat(new_line, e->buffer.lines[e->cy]);

                free(e->buffer.lines[e->cy - 1]);
                free(e->buffer.lines[e->cy]);

                for (int i = e->cy; i < e->buffer.line_count - 1; i++)
                    e->buffer.lines[i] = e->buffer.lines[i + 1];

                e->buffer.lines[e->cy - 1] = new_line;
                e->buffer.line_count--;
                e->cy--;
                e->cx = prev_len;
            }
            return;
        }

        if (c == '\r') { // Enter
            if (e->cx > strlen(e->buffer.lines[e->cy]))
                e->cx = strlen(e->buffer.lines[e->cy]);
            buffer_split_line(&e->buffer, e->cx, e->cy);
            e->cy++;
            e->cx = 0;
            return;
        }

        // Solo caracteres imprimibles
        if ((c >= 32 && c <= 126) || c == '\t') {
            buffer_insert_char(&e->buffer, e->cx, e->cy, c);
            e->cx++;
        }
    }

    // Scroll vertical
    if (e->cy < e->row_offset) e->row_offset = e->cy;
    if (e->cy >= e->row_offset + e->screenrows)
        e->row_offset = e->cy - e->screenrows + 1;

    // Scroll horizontal
    if (e->cx < e->col_offset) e->col_offset = e->cx;
    if (e->cx >= e->col_offset + e->screencols)
        e->col_offset = e->cx - e->screencols + 1;
}
