#include <stdio.h>
#include <stdlib.h>

#include "controls.h"
#include "timing.h"

int main() {

    char *the_time_is = time_str();
    printf("Program starting at %s\n",the_time_is);
    free(the_time_is);

    direction_t dir;
    init_controls();
    while(1) {
        dir = input();
        printf("selected direction ");
        switch(dir) {
        case FORWARD:
            printf("forward");
            break;
        case BACK:
            printf("back");
            break;
        case LEFT:
            printf("left");
            break;
        case RIGHT:
            printf("right");
            break;
        case STOP:
            printf("stop!\n");
            return 0;
        default:
            printf("none");
        }
        printf("\n");
    }
    return 1;
}

