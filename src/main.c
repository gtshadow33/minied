#include "editor.h"
#include "terminal.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // Limpiar terminal completamente al inicio
    write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
    fflush(stdout);
    
    // Activar modo raw
    enable_raw_mode();
    
    // Inicializar editor
    Editor editor;
    const char* file = (argc > 1) ? argv[1] : NULL;
    
    editor_init(&editor, file);
    
    // Bucle principal
    while (1) {
        editor_refresh_screen(&editor);
        editor_process_key(&editor);
    }
    
    // Nunca llega aquí
    return 0;
}