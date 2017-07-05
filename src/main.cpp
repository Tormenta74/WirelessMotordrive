
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>

#include "connect.h"
#include "controls.h"
#include "commands.h"
#include "timing.h"

int main(int argc, char *argv[])
{

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

    bool normal_mode = false;
    direction_t dir;
    char cmd_msg[5];
    while(1) {
        if(normal_mode) {
            std::string command;
            std::getline(std::cin,command);

            int data;
            command_t code = process_cmd(command,&data);
            switch(code) {
            case SPEED:
                //printf("speed command!\n");
                if(data < 100 || data > 2000) {
                    printf("speed must be between [100-2000]\n");
                }
                sprintf(cmd_msg,"%d",data);
                send(cmd_msg,4);
                break;
            case ENC:
                printf("encoder command!\n");
                break;
            case DUMMY:
                printf("Switching to dummy mode\n");
                normal_mode = false;
                break;
            case END:
                send((char*)"0",1);
                return 0; // correct exit point
            default:
                std::cerr << '"' << command << '"'
                    << " command not recognized" << std::endl;
            }

        } else {
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
            case TOGGLE:
                normal_mode = true;
                printf("Switching to normal mode\n");
                break;
            case QUIT:
                send((char*)"0",1);
                return 0; // correct exit point
            default:
                send((char*)"none",4);
            }
        }
    }

    return 3;
}

