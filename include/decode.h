#ifndef SNIPPY_DECODE_H
#define SNIPPY_DECODE_H

#include "keyboard.h"

#define XKB_KEYCODE_OFFSET 8

struct xkb_keymap* decode_get_keymap();
void decode_keymap_unref(struct xkb_keymap* keymap);

int decode_event(struct keyboard* keyboard, const struct input_event* event,
           char *buffer, size_t buffer_size);

#endif
