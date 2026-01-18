#include "terminal.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios orig;

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig);
    atexit(disable_raw_mode);

    struct termios raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}