#include <stdio.h>

#include "controls.h"

int main() {
    //direction_t dir;
    char key;
    init_controls();
    while(1) {
        /*
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
            printf("stop!");
            return 0;
        default:
            printf("none");
        }
        printf("(%i,%c)\n",dir,dir);
        */
        key = getch();
        printf("got %c char! (%i)\n",key,key);
    }
}
