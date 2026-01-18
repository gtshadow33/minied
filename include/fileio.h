#ifndef FILEIO_H
#define FILEIO_H

#include "buffer.h"

void save_file(Buffer *b, const char *filename);
void load_file(Buffer *b, const char *filename);

#endif
