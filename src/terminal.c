#include "terminal.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios orig; // guarda la configuración original

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig);
    atexit(disable_raw_mode); // se llamará al salir automáticamente

    struct termios raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON); // desactiva eco y canonical mode
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
