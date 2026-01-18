#include "fileio.h"
#include "buffer.h"

void save_file(Buffer *b, const char *filename) {
    buffer_save(b, filename);
}

void load_file(Buffer *b, const char *filename) {
    buffer_load(b, filename);
}