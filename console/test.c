#include <stdio.h>

#include "controls.h"

int main() {
    direction_t dir;
    while(1) {
        dir = input();
        printf("selected direction ");
        switch(dir) {
        case FORWARD:
            printf("forward\n");
            break;
        case BACK:
            printf("back\n");
            break;
        case LEFT:
            printf("left\n");
            break;
        case RIGHT:
            printf("right\n");
            break;
        case STOP:
            printf("stop!\n");
            return 0;
        default:
            printf("none\n");
        }
    }
}
