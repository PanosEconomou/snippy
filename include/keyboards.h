#ifndef SNIPPY_KEYBOARDS_H
#define SNIPPY_KEYBOARDS_H

/*  
 *  Discover available keyboards
 *  
 *  Takes in an array `nodes` and populates the first `capacity` elements with
 *  strings of KEYBOARD_NODES_SIZE_MAX that contain the location of the file
 *  to read from for the keyboard, e.g "/dev/input/event10".
 *
 *  It returns the number of found devices.
 */

#include <stddef.h>
#define KEYBOARD_NODES_SIZE_MAX 64

int keyboard_list(char nodes[][KEYBOARD_NODES_SIZE_MAX], size_t capacity);

#endif
