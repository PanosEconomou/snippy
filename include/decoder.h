/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   decoder.h | talk to wayland to understand wtf they keys are saying         ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_DECODER_H
#define SNIPPY_DECODER_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "keyboard.h"                       /* struct keyboard                      */

#include <stdlib.h>                         /* size_t                               */

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define XKB_KEYCODE_OFFSET  8               /* diff between evdev and xkb keycode   */


/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct decoder {
    struct xkb_context*     context;        /* NULL when starting or unset          */
    struct xkb_keymap*      keymap;         /* current keymap                       */
    struct wl_display*      display;        /* wayland access point                 */
    struct wl_registry*     registry;       /* wayland access point                 */
    struct wl_seat*         seat;           /* even further wayland access point    */
    struct wl_keyboard*     keyboard;       /* correponding device                  */
};

/* ─────  Decoders  ─────────────────────────────────────────────────────────────── */

int  decoder_init    (struct decoder* decoder);
void decoder_cleanup (struct decoder* decoder);

/* ─────  Decoder Functionality  ────────────────────────────────────────────────── */

/*
 * Gets a keymap directly from xkb defaults as a fallback when wayland is being
 * weird.
 *
 * @return xkb_keymap with hopefully no leaks.
 */

struct xkb_keymap*  decoder_get_keymap   (struct xkb_context*       context);


/*
 * Given a valid keyboard event obtain what character was actually pressed.
 * Honestly if I was writing this in python, this would probably be the only 
 * method in the code lmao
 *
 * @return the number of characters the corresponding keycode needs (e.g. 0 for 
 * SHIFT, 1 for a, etc.)
 */

int decoder_event (struct keyboard*          keyboard, 
                   const struct input_event* event,
                   char*                     buffer, 
                   size_t                    buffer_size);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
