#include <stdio.h>
#include <stdlib.h>

#include "connect.h"
#include "controls.h"
#include "timing.h"

int main(int argc, char *argv[]) {

    if(argc < 2) {
        fprintf(stderr, "No serial port provided!");
        fprintf(stderr, "Usage: %s <serial port>\n",argv[0]);
        return 1;
    }

    setup_conn();
    if(connect(argv[1]) != 0) {
        return 2;
    }

    // timing
    char *the_time_is = time_str();
    printf("Program starting at %s\n",the_time_is);
    send(the_time_is,MAX_TIME_STR);
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
            return 0; // correct exit point
        default:
            printf("none");
        }
        printf("\n");
    }

    return 3;
}

