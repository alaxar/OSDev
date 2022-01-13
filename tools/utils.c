#include "utils.h"
#include "../drivers/screen.h"

void memory_copy(char *source, char *dest, int no_bytes) {
    int i;
    for(i = 0; i < no_bytes; i++) {
        *(dest+i) = *(source+i);
    }
}

int handling_scrolling(int cursor_offset) {
    // check if the cursor is within the screen.
    if(cursor_offset < 25*80*2) {
        return cursor_offset;
    }

    // suffle the rows back one
    for(int i = 0; i < 25; i++) {
        memory_copy(get_screen_offset(0, i) + VIDEO_MEMORY, get_screen_offset(0, i-1) + VIDEO_MEMORY, MAX_COLS*2);
    }

    // get the last line address using the offset and videomemory.
    // blank the last line
    char *last_line = get_screen_offset(0, MAX_ROWS-1) + VIDEO_MEMORY;
    for(int i = 0; i < MAX_COLS*2; i++) {
        last_line[i] = 0;
    }

    cursor_offset -= 2*80;
    return cursor_offset;
}