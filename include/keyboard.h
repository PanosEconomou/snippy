#ifndef SNIPPY_KEYBOARD_H
#define SNIPPY_KEYBOARD_H

#include <stddef.h>

#define KEYBOARD_NODE_SIZE_MAX 64
#define MAX_KEYBOARDS 16

enum keyboard_status {
    KEYBOARD_EVENT,
    KEYBOARD_DROPPED,
    KEYBOARD_GONE,
};

struct keyboard {
    char node[KEYBOARD_NODE_SIZE_MAX];
    struct libevdev* device;
    int file_descriptor;
};

struct keyboard_event;

int  keyboard_list      (char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t capacity);
int  keyboard_open      (struct keyboard* keyboard,  const char* node);
int  keyboard_open_many (struct keyboard* keyboards, size_t capacity, 
                         const char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t found);
int  keyboard_open_all  (struct keyboard* keyboards, size_t capacity);
void keyboard_close     (struct keyboard* keyboard);
void keyboard_cleanup   (struct keyboard* keyboards, size_t count);

enum keyboard_status keyboard_next_event(struct keyboard*, struct keyboard_event*);

#endif
