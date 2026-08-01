#include "decode.h"

#include <libevdev/libevdev.h>
#include <xkbcommon/xkbcommon.h>
#include <stdbool.h>

struct xkb_keymap* decode_get_keymap(){
    struct xkb_rule_names names = {
        .rules      = NULL,
        .model      = NULL,
        .layout     = "us",
        .variant    = NULL,
        .options    = NULL,
    };
    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) return NULL;

    struct xkb_keymap* keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkb_context_unref(ctx);

    return keymap;
}

void decode_keymap_unref(struct xkb_keymap* keymap) {
    xkb_keymap_unref(keymap);
}

int decode_event(struct keyboard* keyboard, const struct input_event* event, 
           char *buffer, size_t buffer_size) {

    if (event->type != EV_KEY) return 0;
    xkb_keycode_t code = event->code + XKB_KEYCODE_OFFSET;
    int n = 0;

    if (event->value != 0) {
        bool ctrl = xkb_state_mod_name_is_active(keyboard->state, 
                                                 XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);
        bool alt  = xkb_state_mod_name_is_active(keyboard->state, 
                                                 XKB_MOD_NAME_ALT , XKB_STATE_MODS_EFFECTIVE);
        if (!ctrl && !alt) 
            n = xkb_state_key_get_utf8(keyboard->state, code, buffer, buffer_size);
    }

    if (event->value != 2) {
        xkb_state_update_key(keyboard->state, code, event->value ? XKB_KEY_DOWN : XKB_KEY_UP);
    }

    return n;
}
