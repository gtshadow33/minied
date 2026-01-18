#ifndef CONFIG_H
#define CONFIG_H

#define CTRL_KEY(k) ((k) & 0x1f)

typedef enum {
    MODE_NORMAL,
    MODE_INSERT
} EditorMode;

#endif
