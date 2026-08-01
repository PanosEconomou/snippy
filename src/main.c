#include "keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 6

int main() {
    struct keyboard keyboards[CAPACITY] = {0};
    int found = keyboard_open_all(keyboards, CAPACITY);

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
        printf("\t%s\n", keyboards[i].node);
    }

    return EXIT_SUCCESS;
}
