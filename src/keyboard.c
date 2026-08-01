#include "keyboard.h"

#include <libudev.h>
#include <libevdev/libevdev.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <xkbcommon/xkbcommon.h>

int keyboard_list(char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t capacity){

    int number;
    struct udev* udev = udev_new();
    if (!udev) return -ENOMEM; 

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (!enumerate) { number = -ENOMEM; goto exit_udev; }

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEYBOARD", "1");
    udev_enumerate_scan_devices(enumerate);

    size_t count = 0;
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate)) {
        if (count >= capacity) break;

        struct udev_device* dev = 
            udev_device_new_from_syspath(udev, udev_list_entry_get_name(entry));
        if (!dev) continue;

        const char* node = udev_device_get_devnode(dev);
        if (node) {
            snprintf(nodes[count], KEYBOARD_NODE_SIZE_MAX, "%s", node);
            count++;
        }
        udev_device_unref(dev);
    }

    number = (int)count;
    udev_enumerate_unref(enumerate);
exit_udev:
    udev_unref(udev);
    return number;
}

int keyboard_open(struct keyboard* keyboard, const char *node, struct xkb_keymap* keymap) {
    *keyboard = (struct keyboard){0};
    keyboard->file_descriptor = -1;

    if (!keymap) return -ENOMEM;

    keyboard->file_descriptor = open(node, O_RDONLY | O_CLOEXEC | O_NONBLOCK);

    if (keyboard->file_descriptor < 0) return -errno;

    int error = libevdev_new_from_fd(keyboard->file_descriptor, &(keyboard->device));
    if(error < 0) goto error_evdev;

    keyboard->state = xkb_state_new(keymap);
    if (!keyboard->state) { error = -ENOMEM; goto error_xkb; }

    snprintf(keyboard->node, sizeof keyboard->node, "%s", node);

    return 0;

error_xkb:
    libevdev_free(keyboard->device);
    keyboard->device = NULL;

error_evdev:
    close(keyboard->file_descriptor);
    keyboard->file_descriptor = -1;

    return error;
}

void keyboard_close(struct keyboard* keyboard) {
    if (!keyboard) return;
    if (keyboard->device) {
        libevdev_free(keyboard->device);
        keyboard->device = NULL;
    }
    if (keyboard->file_descriptor >= 0) {
        close(keyboard->file_descriptor);
        keyboard->file_descriptor = -1;
    }
    if(keyboard->state){
        xkb_state_unref(keyboard->state);
        keyboard->state = NULL;
    }
}

int keyboard_set_open_many(struct keyboard_set* keyboards, const char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t found, struct xkb_keymap* keymap) {
    size_t n_open = 0;
    for (size_t i=0; i<found && n_open<MAX_KEYBOARDS; i++) {
        int error = keyboard_open(&keyboards->entries[n_open], nodes[i], keymap);
        if (error < 0) { 
            fprintf(stderr, "Couldn't open keyboard in %s:\n\t%s\n", nodes[i], strerror(-error)); 
            continue;
        }
        n_open++;
    }

    keyboards->count = (int)n_open;
    return keyboards ->count;
}

int keyboard_set_open_all(struct keyboard_set* keyboards, struct xkb_keymap* keymap) {
    char nodes[MAX_KEYBOARDS][KEYBOARD_NODE_SIZE_MAX];
    int found = keyboard_list(nodes, MAX_KEYBOARDS);
    if (found <  0) return found;

    int n_open = keyboard_set_open_many(keyboards, nodes, (size_t)found, keymap);

    return n_open;
}

void keyboard_set_remove(struct keyboard_set* keyboards, size_t index) {
    if (!keyboards || index >= keyboards->count) return;
    keyboard_close(&keyboards->entries[index]);
    keyboards->count--;
    if (index != keyboards->count) {
        keyboards->entries[index] = keyboards->entries[keyboards->count];
    }
    keyboards->entries[keyboards->count] = (struct keyboard){ .file_descriptor = -1 };

}

void keyboard_set_cleanup(struct keyboard_set* keyboards) {
    for (size_t i=0; i<keyboards->count; i++) {
        keyboard_close(&keyboards->entries[i]);
    }
    keyboards->count = 0;
}

enum keyboard_status keyboard_next_event(struct keyboard* keyboard, struct input_event* event) {
    unsigned int flags = LIBEVDEV_READ_FLAG_NORMAL;
    int status = libevdev_next_event(keyboard->device, flags, event);

    switch (status) {
        case LIBEVDEV_READ_STATUS_SUCCESS:  return KEYBOARD_EVENT;
        case -EAGAIN:                       return KEYBOARD_DRAINED;
        case -ENODEV:                       return KEYBOARD_GONE;
        case LIBEVDEV_READ_STATUS_SYNC:     {
            struct input_event discard;
            unsigned int discard_flags = LIBEVDEV_READ_FLAG_SYNC;
            while (libevdev_next_event(keyboard->device, discard_flags, &discard) == LIBEVDEV_READ_STATUS_SYNC);
            return KEYBOARD_DROPPED;
        }
    }
    return KEYBOARD_GONE;
}

size_t keyboard_set_pollfds(struct keyboard_set* keyboards, struct pollfd* pfds, size_t capacity){
    if (capacity > keyboards->count) capacity = keyboards->count;
    for (size_t i=0; i<capacity; i++) {
        pfds[i].fd      = keyboards->entries[i].file_descriptor; 
        pfds[i].events  = POLLIN;
    }

    return capacity;
}
