
#include <ncurses.h>

#include "controls.h"

void init_controls() {
    initscr();
    noecho();
    cbreak();
}

direction_t input() {
    char key = getch();

    switch(key) {
    case 'a':
    case 'A':
        return LEFT;
    case 'd':
    case 'D':
        return RIGHT;
    case 'w':
    case 'W':
        return FORWARD;
    case 's':
    case 'S':
        return BACK;
    case 'q':
    case 'Q':
        return STOP;
    default:
        return NONE;
    }
}
