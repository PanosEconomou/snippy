#include "decoder.h"

#include <libevdev/libevdev.h>
#include <stdint.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-client.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static void registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    printf("global:name=%u\tv%u\t%s\n", name, version, interface);
    if (strcmp(interface, wl_seat_interface.name) == 0) {
        printf("^^^^^^^^^^^^^ FOUND A SEAT!\n");
        uint32_t v = version < 7 ? version : 7;
        ((struct decoder*)data)->seat = wl_registry_bind(registry, name, &wl_seat_interface, v);
    }
    
}

static void registry_global_remove(void* data, struct wl_registry* registry, uint32_t name) {

}

static const struct wl_registry_listener registry_listener = {
    .global         = registry_global,
    .global_remove  = registry_global_remove,
};

int decoder_init(struct decoder* decoder) {
    *decoder = (struct decoder){0};
    decoder->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!decoder->context) return -ENOMEM;

    decoder->display = wl_display_connect(NULL);
    if (!decoder->display) {
        fprintf(stderr, "No wayland display. Falling back to default layout\n");
        decoder->keymap = decoder_get_keymap(decoder->context);
        if (!decoder->keymap) { 
            xkb_context_unref(decoder->context);
            decoder->context = NULL;
            return -ENOMEM;
        }
        return 0;
    }

    struct wl_registry* registry = wl_display_get_registry(decoder->display);
    if (!registry) return -ENOMEM;
    wl_registry_add_listener(registry, &registry_listener, decoder);
    wl_display_roundtrip(decoder->display);

    return 0;
}

void decoder_cleanup(struct decoder *decoder){
    if(decoder->keyboard)   wl_keyboard_release     (decoder->keyboard);
    if(decoder->seat)       wl_seat_release         (decoder->seat);
    if(decoder->display)    wl_display_disconnect   (decoder->display);
    if(decoder->keymap)     xkb_keymap_unref        (decoder->keymap);
    if(decoder->context)    xkb_context_unref       (decoder->context);
    *decoder = (struct decoder){0};
}

struct xkb_keymap* decoder_get_keymap(struct xkb_context* context){
    struct xkb_rule_names names = {
        .rules      = NULL,
        .model      = NULL,
        .layout     = "us",
        .variant    = NULL,
        .options    = NULL,
    };
    if (!context) return NULL;

    struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkb_context_unref(context);

    return keymap;
}

int decoder_event(struct keyboard* keyboard, const struct input_event* event, 
           char *buffer, size_t buffer_size) {

    if (event->type != EV_KEY) return 0;
    xkb_keycode_t code = event->code + XKB_KEYCODE_OFFSET;
    int n = 0;

    if (event->value != 0) {
        bool ctrl = xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_CTRL, 
                                                 XKB_STATE_MODS_EFFECTIVE);
        bool alt  = xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_ALT, 
                                                 XKB_STATE_MODS_EFFECTIVE);
        if (!ctrl && !alt) 
            n = xkb_state_key_get_utf8(keyboard->state, code, buffer, buffer_size);
    }

    if (event->value != 2) {
        xkb_state_update_key(keyboard->state, code, event->value ? XKB_KEY_DOWN : XKB_KEY_UP);
    }

    return n;
}
