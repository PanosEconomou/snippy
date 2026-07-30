#include <libevdev/libevdev.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    struct libevdev *dev = nullptr;

    int i = 0;
    while (true){
        char path[50]; 
        sprintf(path,"/dev/input/event%d",i); 
        int file_descriptor = open(path, O_RDWR | O_CLOEXEC);

        if (file_descriptor == -1) {
            printf("Bye!");
            break;
        }
        if (libevdev_new_from_fd(file_descriptor, &dev) == 0) {
            printf("path: %s\nname: %s\n phys: %s\n", path, libevdev_get_name(dev), libevdev_get_phys(dev));
        }

        close(file_descriptor);
        i++;
    }

    return 0;
}
