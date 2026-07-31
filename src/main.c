#include "keyboards.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYBOARDS 16

int main() {
    char nodes[MAX_KEYBOARDS][KEYBOARD_NODES_SIZE_MAX];
    int found = keyboard_list(nodes, MAX_KEYBOARDS);

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
        printf("\t%s\n", nodes[i]);
    }

    return EXIT_SUCCESS;
}
