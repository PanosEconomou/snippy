/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   decoder.h | talk to wayland to understand wtf they keys are saying         ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#include "decoder.h"
#include "keyboard.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <libevdev/libevdev.h>
#include <xkbcommon/xkbcommon.h>

/* ─────  Keyboard Listeners  ───────────────────────────────────────────────────── */

static void keyboard_keymap(void* data, struct wl_keyboard* keyboard, uint32_t format, 
                            int fd, uint32_t size)
{
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) { 
        fprintf(stderr, "No keymap found.\n"); 
        close(fd);
        return; 
    }
    struct decoder* decoder = data;
    
    char* wayland_keymap = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (wayland_keymap == MAP_FAILED) {
        fprintf(stderr, "Keymap couldn't be read.\n");
        close(fd);
        return;
    }

    if (decoder->keymap) xkb_keymap_unref(decoder->keymap);
    decoder->keymap = xkb_keymap_new_from_string(decoder->context, wayland_keymap, 
                                                 XKB_KEYMAP_FORMAT_TEXT_V1, 
                                                 XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(wayland_keymap, size);
    close(fd);

    if (!decoder->keymap) {
        fprintf(stderr, "Error converting the wayland keymap to xkb.\n");
        return;
    }
}

static void keyboard_enter(void* data, struct wl_keyboard* keyboard, uint32_t serial, 
                           struct wl_surface* surface, struct wl_array* keys) 
{
}

static void keyboard_leave(void* data, struct wl_keyboard* keyboard, uint32_t serial, 
                           struct wl_surface* surface) 
{
}

static void keyboard_key(void* data, struct wl_keyboard* keyboard, uint32_t serial, 
                         uint32_t time, uint32_t key, uint32_t state) 
{
}
static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, 
                               uint32_t mods_depressed, uint32_t mods_latched, 
                               uint32_t mods_locked, uint32_t group)
{
}

static void keyboard_repeat_info(void* data, struct wl_keyboard* keyboard, int32_t rate, 
                                 int32_t delay)
{
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap         = keyboard_keymap,
    .enter          = keyboard_enter,
    .leave          = keyboard_leave,
    .key            = keyboard_key,
    .modifiers      = keyboard_modifiers,
    .repeat_info    = keyboard_repeat_info,
};

/* ─────  Seat Listeners  ───────────────────────────────────────────────────────── */

static void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities) 
{
    struct decoder* decoder = data;

    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !decoder->keyboard) {
        decoder->keyboard = wl_seat_get_keyboard(seat);
        // printf("seat:\thas a keyboard!\n");

        wl_keyboard_add_listener(decoder->keyboard, &keyboard_listener, decoder);
    }
    else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && decoder->keyboard) {
        printf("seat:\tkeyboard lost.\n");
        wl_keyboard_release(decoder->keyboard);
        decoder->keyboard = NULL;
    }
}

static void seat_name(void* data, struct wl_seat* seat, const char* name) 
{
    // printf("seat:\tname=%s\n", name);
}

static const struct wl_seat_listener seat_listener = {
    .capabilities   = seat_capabilities,
    .name           = seat_name,
};

/* ───── Registry Listeners  ────────────────────────────────────────────────────── */

static void registry_global(void* data, struct wl_registry* registry, uint32_t name, 
                            const char* interface, uint32_t version)
{
    struct decoder* decoder = data;


    if (strcmp(interface, wl_seat_interface.name) == 0) {
        // printf("global:\tname=%u\tv%u\t%s\n", name, version, interface);
        // printf("^^^^^^^^^^^^^ FOUND A SEAT!\n");

        uint32_t v = version < 7 ? version : 7;
        decoder->seat = wl_registry_bind(registry, name, &wl_seat_interface, v);

        wl_seat_add_listener(decoder->seat, &seat_listener, decoder);
    }
    
}

static void registry_global_remove(void* data, struct wl_registry* registry, 
                                   uint32_t name) 
{
}

static const struct wl_registry_listener registry_listener = {
    .global         = registry_global,
    .global_remove  = registry_global_remove,
};

/* ─────  Decoders  ─────────────────────────────────────────────────────────────── */

int decoder_init(struct decoder* decoder) {

    *decoder = (struct decoder){0};

    decoder->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!decoder->context) return -ENOMEM;

    decoder->display = wl_display_connect(NULL);
    if (!decoder->display) {
        fprintf(stderr, "No wayland display.\n");
        goto fallback_keymap;
    }

    decoder->registry = wl_display_get_registry(decoder->display);
    if (!decoder->registry) {
        fprintf(stderr, "No wayland registry.\n");
        goto fallback_keymap;
    }

    wl_registry_add_listener (decoder->registry, &registry_listener, decoder);
    wl_display_roundtrip     (decoder->display);    /* globals -> seat      */
    wl_display_roundtrip     (decoder->display);    /* seat -> keyboard     */
    wl_display_roundtrip     (decoder->display);    /* keyboard -> keymap   */

    if (!decoder->seat) {
        fprintf(stderr, "No seat advertized.\n");
        goto fallback_keymap;
    }
    if (!decoder->keyboard) {
        fprintf(stderr, "No keyboard in seat.\n");
        goto fallback_keymap;
    }
    if (!decoder->keymap) {
        fprintf(stderr, "No wayland keymap obtained.\n");
        goto fallback_keymap;
    }

    return 0;

fallback_keymap:

    fprintf(stderr, "Falling back to default keymap.\n");

    decoder->keymap = decoder_get_keymap(decoder->context);

    if (!decoder->keymap) { 
        xkb_context_unref(decoder->context);
        decoder->context = NULL;
        return -ENOMEM;
    }

    return 0;
}

void decoder_cleanup(struct decoder *decoder){
    if(decoder->keyboard)   wl_keyboard_release     (decoder->keyboard);
    if(decoder->seat)       wl_seat_release         (decoder->seat);
    if(decoder->registry)   wl_registry_destroy     (decoder->registry);
    if(decoder->display)    wl_display_disconnect   (decoder->display);
    if(decoder->keymap)     xkb_keymap_unref        (decoder->keymap);
    if(decoder->context)    xkb_context_unref       (decoder->context);
    *decoder = (struct decoder){0};
}


/* ─────  Decoder Functionality  ────────────────────────────────────────────────── */

struct xkb_keymap* decoder_get_keymap(struct xkb_context* context) 
{
    if (!context) return NULL;

    struct xkb_rule_names names = {
        .rules      = NULL,
        .model      = NULL,
        .layout     = "us",
        .variant    = NULL,
        .options    = NULL,
    };

    return xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
}

int decoder_event(struct keyboard* keyboard, const struct input_event* event, 
                  char *buffer, size_t buffer_size)
{
    if (event->type != EV_KEY) return 0;

    xkb_keycode_t code = event->code + XKB_KEYCODE_OFFSET;
    int length = 0;

    if (event->value != 0) {
        bool ctrl = xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_CTRL, 
                                                 XKB_STATE_MODS_EFFECTIVE);
        bool alt  = xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_ALT, 
                                                 XKB_STATE_MODS_EFFECTIVE);
        if (!ctrl && !alt) 
            length = xkb_state_key_get_utf8(keyboard->state, code, buffer, buffer_size);
    }

    if (event->value != 2) {
        xkb_state_update_key(keyboard->state, code, event->value ? XKB_KEY_DOWN : XKB_KEY_UP);
    }

    return length;
}

/* ──────────────────────────────────────────────────────────────────────────────── */
