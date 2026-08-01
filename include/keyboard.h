#ifndef SNIPPY_KEYBOARD_H
#define SNIPPY_KEYBOARD_H

#include <stddef.h>
#include <poll.h>

#define KEYBOARD_NODE_SIZE_MAX 64
#define MAX_KEYBOARDS 16

enum keyboard_status {
    KEYBOARD_EVENT,
    KEYBOARD_DRAINED,
    KEYBOARD_DROPPED,
    KEYBOARD_GONE,
};

struct keyboard {
    char node[KEYBOARD_NODE_SIZE_MAX];
    struct libevdev* device;
    int file_descriptor;
};
struct keyboard_set {
    struct keyboard entries[MAX_KEYBOARDS];
    size_t count;
};

struct input_event;

int  keyboard_list          (char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t capacity);
int  keyboard_open          (struct keyboard*     keyboard,  const char* node);
int  keyboard_set_open_many (struct keyboard_set* keyboards, 
                             const char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t found);
int  keyboard_set_open_all  (struct keyboard_set* keyboards);
void keyboard_close         (struct keyboard*     keyboard);
void keyboard_set_remove    (struct keyboard_set* keyboards, size_t index);
void keyboard_set_cleanup   (struct keyboard_set* keyboards);

enum keyboard_status keyboard_next_event(struct keyboard* keyboard, struct input_event* event);
size_t keyboard_set_pollfds (struct keyboard_set* keyboards, struct pollfd* pfds, size_t capacity);

#endif
