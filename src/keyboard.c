#include "keyboards.h"
#include <libudev.h>
#include <stdio.h>
#include <errno.h>

int keyboard_list(char nodes[][KEYBOARD_NODES_SIZE_MAX], size_t capacity){

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
            snprintf(nodes[count], KEYBOARD_NODES_SIZE_MAX, "%s", node);
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
