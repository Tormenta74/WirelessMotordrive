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
    char *the_time_is = time_str(), *receit;
    printf("Program starting at %s\n",the_time_is);
    send(the_time_is,MAX_TIME_STR);
    receit = receive(); // blocking call
    if(receit)
        printf("Router 1 ack: %s\n",receit);
    free(receit);
    free(the_time_is);
        
    direction_t dir;
    init_controls();
    while(1) {
        dir = input();
        switch(dir) {
        case FORWARD:
            send((char*)"1",1);
            break;
        case BACK:
            send((char*)"2",1);
            break;
        case LEFT:
            send((char*)"3",1);
            break;
        case RIGHT:
            send((char*)"4",1);
            break;
        case STOP:
            send((char*)"0",1);
            break;
        case QUIT:
            send((char*)"0",1);
            return 0; // correct exit point
        default:
            send((char*)"none",4);
        }
    }

    return 3;
}

