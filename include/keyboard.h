/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   keyboard.h | disover keyboards - steal their strokes                       ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_KEYBOARD_H
#define SNIPPY_KEYBOARD_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include <poll.h>                       /* struct pollfd                            */
#include <stddef.h>                     /* size_t                                   */
#include <xkbcommon/xkbcommon.h>        /* struct xkb_state, struct xkb_keymap      */
#include <libevdev/libevdev.h>          /* struct libevdev, struct input_event      */

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define KEYBOARD_NODE_SIZE_MAX  64      /* enough to store "/dev/input/eventX"      */
#define MAX_KEYBOARDS           16      /* devices that one keyboard_set can hold   */

/* ─────  Status Codes  ─────────────────────────────────────────────────────────── */

enum keyboard_status {
    KEYBOARD_EVENT,                     /* normal keboard event                     */
    KEYBOARD_DRAINED,                   /* no more events for this keyboard         */
    KEYBOARD_DROPPED,                   /* had to drop some events while reading    */
    KEYBOARD_GONE,                      /* can't see the keyboard anymore           */
};

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct keyboard {
    int                 file_descriptor;                /* -1 when closed or ansent */
    char                node[KEYBOARD_NODE_SIZE_MAX];   /* "/dev/input/eventX" path */
    struct libevdev*    device;                         /* NULL when closed or gone */
    struct xkb_state*   state;                          /* modifier e.g. ALT, state */
};

struct keyboard_set {
    struct keyboard     entries[MAX_KEYBOARDS];         /* kyboard list             */
    size_t              count;                          /* number of set keyboards  */
};

/* ─────  Discover Keyboards  ───────────────────────────────────────────────────── */

/*
 * Use udev to see which of the many devices in the computer are keyboards.
 * then print their nodes (aka their /dev/input/eventX paths)
 *
 * @return number of opened keyboards or -ENOMEM if it can't open udev.
 */

int  keyboard_list          (char   nodes[][KEYBOARD_NODE_SIZE_MAX], 
                             size_t capacity);

/* ─────  Keyboard  ─────────────────────────────────────────────────────────────── */

/*
 * Open a keyboard. Default unset or closed file_descriptor is -1.
 *
 * @return 0 on success or a negative errno.
 */

int  keyboard_open          (struct keyboard*   keyboard,  
                             const  char*       node, 
                             struct xkb_keymap* keymap);

void keyboard_close         (struct keyboard*   keyboard);

/* ─────  Keyboard Sets  ────────────────────────────────────────────────────────── */

/*
 * Open up to MAX_KEYBOARDS, either by manually giving them nodes, or by having
 * udev search for them automatically. 
 *
 * @return how many are open, could just be 0.
 */

int  keyboard_set_open_many (struct keyboard_set*   keyboards, 
                             const  char            nodes[][KEYBOARD_NODE_SIZE_MAX], 
                             size_t                 found,
                             struct xkb_keymap*     keymap);

int  keyboard_set_open_all  (struct keyboard_set*   keyboards, 
                             struct xkb_keymap*     keymap);

void keyboard_set_cleanup   (struct keyboard_set*   keyboards);

/* Remove a keyboard from a set and put the last one at its place */
void keyboard_set_remove    (struct keyboard_set*   keyboards, 
                             size_t                 index);

/* ─────  Parse events  ─────────────────────────────────────────────────────────── */

/*
 * Ask a keyboard to give you the next event and check if something finicky 
 * is going on.
 *
 * @return enum keyboard_status for the specific event.
 */

enum keyboard_status keyboard_next_event  (struct keyboard*      keyboard, 
                                           struct input_event*   event);

/*
 * Helper to keep the list of files to poll parallel to the list of keyboards.
 *
 * @return the number of files set to be polled.
 */

size_t keyboard_set_pollfds (struct keyboard_set*  keyboards, 
                             struct pollfd*        pfds, 
                             size_t                capacity);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
