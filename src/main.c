#include "keyboard.h"
#include "decoder.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libevdev/libevdev.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>
#include <wchar.h>
#include <xkbcommon/xkbcommon.h>

int main(void) {
    struct decoder decoder = {0};
    int msg = decoder_init(&decoder);
    if (msg < 0) {
        fprintf(stderr, "Failed to create keyboard decoder: %s\n", strerror(-msg));
        return EXIT_FAILURE;
    }

    struct keyboard_set keyboards = {0};
    int found = keyboard_set_open_all(&keyboards, decoder.keymap);

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
    char buffer[16];
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
                    fprintf(stderr, "%s appears to be gone. Removing...\n", 
                            keyboards.entries[i].node);

                    keyboard_set_remove(&keyboards, i);

                    n_pollfds = keyboard_set_pollfds(&keyboards, pfd, MAX_KEYBOARDS);

                    removed = true;
                    break;
                }
                if (status == KEYBOARD_DROPPED) {
                    fprintf(stderr, "Events were dropped...\n");
                    continue;
                }
                if (status == KEYBOARD_EVENT)   {
                    int n = decoder_event(&keyboards.entries[i], &event, 
                                         buffer, sizeof(buffer));

                    if (n > 0) { printf("%.*s", n, buffer); fflush(stdout); }
                    // printf("%s\ttype = %3d\tcode = %3d\txkb = %3s\tvalue = %3d\n", keyboards.entries[i].node, event.type, event.code, buffer, event.value);

                }
            }
            if (!removed) i++;
        }
    }

    keyboard_set_cleanup(&keyboards);
    decoder_cleanup(&decoder);
    return EXIT_SUCCESS;
}
