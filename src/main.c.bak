#include <libevdev/libevdev.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

struct libevdev* find_device_by_name(const char* name){
    struct libevdev* dev = NULL;
    char path[32];
    for (int i = 0;; i++){
        snprintf(path, sizeof path, "/dev/input/event%d", i); 
        int file_descriptor = open(path, O_RDONLY | O_CLOEXEC);

        if (file_descriptor == -1) break;

        if (libevdev_new_from_fd(file_descriptor, &dev) == 0) {
            if (!strcmp(name,libevdev_get_name(dev))) {
                return dev;
            }
            libevdev_free(dev);
            dev = NULL;
        }

        close(file_descriptor);
    }
    return NULL;
}

static bool is_error(int status){ return status < 0 && status != -EAGAIN; };
static bool has_next_event(int status){ return status >= 0; };

void process_events(struct libevdev* dev) {
    struct input_event ev = {0};
    int status = 0;
    unsigned int flags = LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING;
    while (status = libevdev_next_event(dev, flags, &ev), !is_error(status)){
        if (!has_next_event(status)) continue;
        printf("Got input event!\t type = %5d\t code = %5d\t value = %5d\n",ev.type, ev.code, ev.value);
    }
}

int main() {
    struct libevdev *dev = find_device_by_name("keyd virtual keyboard");
    if (dev == NULL) {
        printf("Couldn't find device!");
        return EXIT_FAILURE;
    }

    process_events(dev);

    libevdev_free(dev);
    return 0;
}
