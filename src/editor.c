#include "editor.h"
#include "fileio.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>

/* Función para obtener tamaño de ventana */
static void get_window_size(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        *rows = 24;
        *cols = 80;
    } else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
    }
}

/* Limpia TODA la pantalla incluyendo scrollback */
static void clear_screen_completely(void) {
    // Secuencia completa: 1) reset device, 2) clear screen, 3) home
    write(STDOUT_FILENO, "\x1b[3J\x1b[2J\x1b[H", 11);
    fflush(stdout);
}

/* Inicializa editor */
void editor_init(Editor *e, const char *filename) {
    // Limpiar estructura
    memset(e, 0, sizeof(Editor));
    
    // Limpiar pantalla completamente antes de empezar
    clear_screen_completely();
    
    // Inicializar valores
    e->cx = 0;
    e->cy = 0;
    e->row_offset = 0;
    e->col_offset = 0;
    e->mode = MODE_NORMAL;
    
    // Obtener tamaño de terminal
    get_window_size(&e->screenrows, &e->screencols);
    
    // Inicializar buffer
    buffer_init(&e->buffer);
    e->filename = filename;
    
    // Cargar archivo si existe
    if (filename) {
        buffer_load(&e->buffer, filename);
    }
    
    // Dibujar pantalla inicial limpia
    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
}

/* Refresca la pantalla */
void editor_refresh_screen(Editor *e) {
    char buf[64];
    
    // Ocultar cursor temporalmente
    write(STDOUT_FILENO, "\x1b[?25l", 6);
    
    // Mover cursor a inicio (1,1)
    write(STDOUT_FILENO, "\x1b[1;1H", 6);
    
    int screen_rows = e->screenrows - 1; // Última fila para status
    
    // Dibujar cada línea del buffer visible
    for (int y = 0; y < screen_rows; y++) {
        int filerow = y + e->row_offset;
        
        // Limpiar la línea actual
        write(STDOUT_FILENO, "\x1b[2K", 4);
        
        if (filerow >= e->buffer.line_count) {
            // Línea más allá del buffer: mostrar ~
            if (e->buffer.line_count == 0 && y == 0) {
                // Buffer vacío: mostrar ~ en primera línea
                write(STDOUT_FILENO, "~", 1);
            } else if (e->buffer.line_count > 0) {
                // Buffer con contenido: mostrar ~ después del contenido
                write(STDOUT_FILENO, "~", 1);
            }
        } else {
            // Mostrar contenido real de la línea
            char *line = e->buffer.lines[filerow];
            int line_len = strlen(line);
            int col_start = e->col_offset;
            
            if (col_start < line_len) {
                int draw_len = line_len - col_start;
                if (draw_len > e->screencols) {
                    draw_len = e->screencols;
                }
                write(STDOUT_FILENO, line + col_start, draw_len);
            }
        }
        
        // Si no es la última línea de visualización, bajar a siguiente línea
        if (y < screen_rows - 1) {
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
    
    /* ===== BARRA DE ESTADO ===== */
    snprintf(buf, sizeof(buf), "\x1b[%d;1H", e->screenrows);
    write(STDOUT_FILENO, buf, strlen(buf));
    
    // Limpiar línea de estado
    write(STDOUT_FILENO, "\x1b[2K", 4);
    
    // Crear texto de estado
    char status[256];
    snprintf(status, sizeof(status),
             "^O:Guardar | ^X:Salir | %s | %s | Ln:%d,Col:%d",
             e->filename ? e->filename : "[Sin nombre]",
             e->mode == MODE_INSERT ? "INSERT" : "NORMAL",
             e->cy + 1, e->cx + 1);
    
    // Aplicar inversión (negro sobre blanco)
    write(STDOUT_FILENO, "\x1b[7m", 4);
    
    // Mostrar status (truncar si es muy largo)
    int status_len = strlen(status);
    int max_len = e->screencols;
    if (status_len > max_len) {
        status_len = max_len;
    }
    write(STDOUT_FILENO, status, status_len);
    
    // Rellenar resto de la barra si es necesario
    for (int i = status_len; i < max_len; i++) {
        write(STDOUT_FILENO, " ", 1);
    }
    
    // Quitar inversión
    write(STDOUT_FILENO, "\x1b[0m", 4);
    
    /* ===== POSICIONAR CURSOR ===== */
    // Calcular posición del cursor en pantalla
    int cursor_screen_y = (e->cy - e->row_offset) + 1;
    int cursor_screen_x = (e->cx - e->col_offset) + 1;
    
    // Asegurar límites
    if (cursor_screen_y < 1) cursor_screen_y = 1;
    if (cursor_screen_y > screen_rows) cursor_screen_y = screen_rows;
    if (cursor_screen_x < 1) cursor_screen_x = 1;
    if (cursor_screen_x > e->screencols) cursor_screen_x = e->screencols;
    
    // Mover cursor a posición correcta
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursor_screen_y, cursor_screen_x);
    write(STDOUT_FILENO, buf, strlen(buf));
    
    // Mostrar cursor
    write(STDOUT_FILENO, "\x1b[?25h", 6);
    
    // Forzar flush
    fflush(stdout);
}

/* Procesa tecla presionada */
void editor_process_key(Editor *e) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return;
    
    if (e->mode == MODE_NORMAL) {
        switch (c) {
            case 'h': 
                if (e->cx > 0) e->cx--; 
                break;
            case 'l': 
                e->cx++; 
                break;
            case 'k': 
                if (e->cy > 0) e->cy--; 
                break;
            case 'j': 
                if (e->cy < e->buffer.line_count - 1) e->cy++; 
                break;
            case 'i': 
                e->mode = MODE_INSERT; 
                break;
            case CTRL_KEY('o'): 
                if (e->filename) {
                    buffer_save(&e->buffer, e->filename);
                }
                break;
            case CTRL_KEY('x'):
                // Limpiar pantalla al salir
                write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
                buffer_free(&e->buffer);
                exit(0);
                break;
        }
    } else { /* MODO INSERT */
        if (c == 27) { // ESC
            e->mode = MODE_NORMAL;
            return;
        }
        
        if (c == 127) { // BACKSPACE
            if (e->cx > 0) {
                buffer_delete_char(&e->buffer, e->cx, e->cy);
                e->cx--;
            } else if (e->cy > 0) {
                int prev_len = strlen(e->buffer.lines[e->cy - 1]);
                
                char *new_line = malloc(prev_len + strlen(e->buffer.lines[e->cy]) + 1);
                if (new_line) {
                    strcpy(new_line, e->buffer.lines[e->cy - 1]);
                    strcat(new_line, e->buffer.lines[e->cy]);
                    
                    free(e->buffer.lines[e->cy - 1]);
                    free(e->buffer.lines[e->cy]);
                    
                    e->buffer.lines[e->cy - 1] = new_line;
                    
                    // Mover líneas hacia arriba
                    for (int i = e->cy; i < e->buffer.line_count - 1; i++) {
                        e->buffer.lines[i] = e->buffer.lines[i + 1];
                    }
                    
                    e->buffer.line_count--;
                    e->cy--;
                    e->cx = prev_len;
                }
            }
            return;
        }
        
        if (c == '\r' || c == '\n') { // ENTER
            buffer_split_line(&e->buffer, e->cx, e->cy);
            e->cy++;
            e->cx = 0;
            return;
        }
        
        if ((c >= 32 && c <= 126) || c == '\t') { // Caracteres imprimibles
            buffer_insert_char(&e->buffer, e->cx, e->cy, c);
            e->cx++;
        }
    }
    
    /* ===== AJUSTAR SCROLL ===== */
    // Scroll vertical
    if (e->cy < e->row_offset) {
        e->row_offset = e->cy;
    }
    if (e->cy >= e->row_offset + (e->screenrows - 1)) {
        e->row_offset = e->cy - (e->screenrows - 1) + 1;
    }
    
    // Scroll horizontal
    if (e->cx < e->col_offset) {
        e->col_offset = e->cx;
    }
    if (e->cx >= e->col_offset + e->screencols) {
        e->col_offset = e->cx - e->screencols + 1;
    }
    
    // Asegurar valores no negativos
    if (e->col_offset < 0) e->col_offset = 0;
    if (e->row_offset < 0) e->row_offset = 0;
    if (e->cx < 0) e->cx = 0;
    if (e->cy < 0) e->cy = 0;
}