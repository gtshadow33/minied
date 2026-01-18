#include "fileio.h"
#include "buffer.h"

/* Wrapper para guardar archivo */
void save_file(Buffer *b, const char *filename) {
    buffer_save(b, filename);
}

/* Wrapper para cargar archivo */
void load_file(Buffer *b, const char *filename) {
    buffer_load(b, filename);
}
