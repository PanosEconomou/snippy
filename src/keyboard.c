#include "keyboard.h"

#include <libudev.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int keyboard_list(char nodes[][KEYBOARD_NODE_SIZE_MAX], size_t capacity){

    int number;
    struct udev *udev = udev_new();
    if (!udev) { number = -ENOMEM; goto exit_udev; }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    if (!enumerate) { number = -ENOMEM; goto exit_enumerate; }

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEYBOARD", "1");
    udev_enumerate_scan_devices(enumerate);

    size_t count = 0;
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate)) {
        if (count >= capacity) break;

        struct udev_device *dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(entry));

        const char* node = udev_device_get_devnode(dev);
        if (node) {
            snprintf(nodes[count], KEYBOARD_NODE_SIZE_MAX, "%s", node);
            count++;
        }
        udev_device_unref(dev);
    }

    number = (int)count;
exit_enumerate:
    udev_enumerate_unref(enumerate);
exit_udev:
    udev_unref(udev);
    return number;
}

int keyboard_open(struct keyboard* keyboard, const char *node) {
    keyboard->device          = NULL;
    keyboard->file_descriptor = -1;
    keyboard->file_descriptor = open(node, O_RDONLY | O_CLOEXEC | O_NONBLOCK);

    if (keyboard->file_descriptor < 0) return -errno;

    int error = libevdev_new_from_fd(keyboard->file_descriptor, &(keyboard->device));
    if(error < 0) {
        close(keyboard->file_descriptor);
        keyboard->file_descriptor = -1;
        return error;
    }

    snprintf(keyboard->node, sizeof keyboard->node, "%s", node);

    return 0;
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
}

int keyboard_open_many(struct keyboard* keyboards, const size_t capacity, const char nodes[][KEYBOARD_NODE_SIZE_MAX], const size_t found) {
    size_t n_open = 0;
    for (size_t i=0; i<found && n_open<capacity; i++) {
        int error = keyboard_open(&keyboards[n_open], nodes[i]);
        if (error < 0) { 
            fprintf(stderr, "Couldn't open keyboard in %s:\n\t%s\n", nodes[i], strerror(-error)); 
            continue;
        }
        n_open++;
    }

    return (int)n_open;
}

int keyboard_open_all(struct keyboard* keyboards, size_t capacity) {
    char nodes[MAX_KEYBOARDS][KEYBOARD_NODE_SIZE_MAX];
    if (capacity > MAX_KEYBOARDS) capacity = MAX_KEYBOARDS;
    int found = keyboard_list(nodes, capacity);
    if (found <  0) return found;

    int n_open = keyboard_open_many(keyboards, capacity, nodes, (size_t)found);

    return n_open;
}

void keyboard_cleanup(struct keyboard* keyboards, const size_t count) {
    for (size_t i=0; i<count; i++) {
        keyboard_close(&keyboards[i]);
    }
}
