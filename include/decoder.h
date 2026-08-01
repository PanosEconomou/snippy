#ifndef SNIPPY_DECODER_H
#define SNIPPY_DECODER_H

#include "keyboard.h"

#define XKB_KEYCODE_OFFSET 8

struct decoder {
    struct xkb_context*     context;
    struct xkb_keymap*      keymap;
    struct wl_display*      display;
    struct wl_seat*         seat;
    struct wl_keyboard*     keyboard;
};

int                 decoder_init        (struct decoder*           decoder);
void                decoder_cleanup     (struct decoder*           decoder);
struct xkb_keymap*  decoder_get_keymap  (struct xkb_context*       context);
void                decoder_keymap_unref(struct xkb_keymap*        keymap);
int                 decoder_event       (struct keyboard*          keyboard, 
                                         const struct input_event* event,
                                         char *buffer, size_t buffer_size);

#endif
