#include "keyboard.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libevdev/libevdev.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>
#include <xkbcommon/xkbcommon.h>

#define XKB_KEYCODE_OFFSET 8

int main(void) {
    struct xkb_rule_names names = {
        .rules      = NULL,
        .model      = NULL,
        .layout     = "us",
        .variant    = NULL,
        .options    = NULL,
    };
    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) return -ENOMEM;

    struct xkb_keymap* keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    struct xkb_state *state = xkb_state_new(keymap);

    struct keyboard_set keyboards = {0};
    int found = keyboard_set_open_all(&keyboards);

    if (found < 0) {
        fprintf(stderr, "Failed to find keyboards: %s\n", strerror(-found));
        return EXIT_FAILURE;
    }

    if (found == 0) {
        fprintf(stderr, "Do you have no keyboards? What's going on?\n");
        return EXIT_FAILURE;
    }

    printf("Here are your keyboards!\n");
    for (int i=0; i<found; i++) {
        printf("\t%s\n", keyboards.entries[i].node);
    }

    struct pollfd pfd[MAX_KEYBOARDS] = {0};
    size_t n_pollfds = keyboard_set_pollfds(&keyboards, pfd, MAX_KEYBOARDS);

    struct input_event event = {0};
    char xkb_buffer[16];
    xkb_keycode_t xkb_code;
    while (keyboards.count > 0) {
        int response = poll(pfd, n_pollfds, -1);
        if (response < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            return EXIT_FAILURE;
        }
        for (size_t i=0; i<keyboards.count;) {
            if (!pfd[i].revents) { i++; continue; }
            bool removed = false;
            enum keyboard_status status;
            for (;;) {
                status = keyboard_next_event(&keyboards.entries[i], &event);
                if (status == KEYBOARD_DRAINED) break;
                if (status == KEYBOARD_GONE)    {
                    fprintf(stderr, 
                            "%s appears to be gone. Removing...\n", 
                            keyboards.entries[i].node);
                    keyboard_set_remove(&keyboards, i);
                    pfd[i].revents = 0;
                    n_pollfds = keyboard_set_pollfds(&keyboards, pfd, MAX_KEYBOARDS);
                    removed = true;
                    break;
                }
                if (status == KEYBOARD_DROPPED) {
                    fprintf(stderr, "Events were dropped...\n");
                    continue;
                }
                if (status == KEYBOARD_EVENT)   {
                    if (event.type != EV_KEY) continue;
                    xkb_code = event.code + XKB_KEYCODE_OFFSET;
                    int n = xkb_state_key_get_utf8(state, xkb_code, xkb_buffer, sizeof xkb_buffer);
                    if (event.value != 2)
                        xkb_state_update_key(state, xkb_code, event.value ? XKB_KEY_DOWN : XKB_KEY_UP);
                    printf("%s\ttype = %3d\tcode = %3d\txkb = %3s\tvalue = %3d\n",
                           keyboards.entries[i].node, event.type, event.code, xkb_buffer, event.value);
                }
            }
            if (!removed) i++;
        }
    }

    keyboard_set_cleanup(&keyboards);
    xkb_context_unref(ctx);
    return EXIT_SUCCESS;
}
