#include <libevdev/libevdev.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct libevdev* find_device_by_name(char* name){
    struct libevdev* dev = nullptr;
    for (int i = 0;; i++){
        char path[50];
        sprintf(path,"/dev/input/event%d",i); 
        int file_descriptor = open(path, O_RDWR | O_CLOEXEC);

        if (file_descriptor == -1) {
            break;
        }

        if (libevdev_new_from_fd(file_descriptor, &dev) == 0) {
            if (!strcmp(name,libevdev_get_name(dev))) {
                return dev;
            }
            libevdev_free(dev);
            dev = nullptr;
        }

        close(file_descriptor);
    }
    return nullptr;
}

bool is_error(int status){ return status < 0 && status != -EAGAIN; };
bool has_nex_event(int status){ return status >= 0; };
void process_events(struct libevdev* dev) {
    struct input_event ev = {};
    int status = 0;
    const auto flags = LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING;
    while (status = libevdev_next_event(dev, flags, &ev), !is_error(status)){
        if (!has_nex_event(status)) continue;
        printf("Got input event!\n\t type = %d\n\t code = %d\n\tvalue = %d\n",ev.type, ev.code, ev.value);
    }
}

int main() {
    struct libevdev *dev = find_device_by_name("keyd virtual keyboard");
    if (dev == nullptr) {
        printf("Couldn't find device!");
        return -1;
    }

    process_events(dev);

    libevdev_free(dev);
    return 0;
}
