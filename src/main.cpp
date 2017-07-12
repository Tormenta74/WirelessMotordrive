/* main.cpp
 * Author: Diego Sáinz de Medrano <diego.sainzdemedrano@gmail.com>
 *
 * This file is part of the Wireless Motordrive project.                   
 * Copyright (C) 2017 Diego Sáinz de Medrano.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, visit https://www.gnu.org/licenses/ to get
 * a copy.
 */

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connect.h"
#include "controls.h"
#include "commands.h"
#include "timing.h"

void print_normal_mode_help()
{
    printf("Type one of the following commands to interact with the robot:\n");
    printf("\tspeed <n>: sets the robot motor drive speed register to n (n must be between -128 and 127)\n");
    printf("\t[nyi]encoder <n>: fetches the latest encoder n value (n must be 1 or 2)\n");
    printf("\ttoggle: changes the controls to \"dummy\" mode\n");
    printf("\tquit: exits the program (sending a stop signal to the robot)\n");
}

void print_dummy_mode_help()
{
    printf("Use one of the following keys to interact with the robot:\n");
    printf("\tw,W: forward\n");
    printf("\ta,A: turn left\n");
    printf("\ts,S: backwards\n");
    printf("\td,D: turn right\n");
    printf("\tm,M: switch to normal mode\n");
    printf("\tq,Q: exits the program (sending a stop signal to the robot)\n");
    printf("\t<space>: stop\n");
}

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
    print_dummy_mode_help();
    direction_t dir;
    char cmd_msg[6];
    while(1) {
        if(normal_mode) {
            std::string command;
            std::getline(std::cin,command);

            int data;
            command_t code = process_cmd(command,&data);
            printf("sending ");
            switch(code) {
            case SPEED:
                //printf("speed command!\n");
                if(data < -200 || data > 200) {
                    printf("speed must be between [-200,200]\n");
                }
                sprintf(cmd_msg,"S%d",data);
                printf("%s\n",cmd_msg);
                send(cmd_msg,strlen(cmd_msg));
                break;
            case ENC:
                //printf("encoder command!\n");
                break;
            case DUMMY:
                normal_mode = false;
                printf("\nSwitching to dummy mode\n");
                print_dummy_mode_help();
                break;
            case END:
                send((char*)"D0",2);
                printf("D0\n");
                return 0; // correct exit point
            default:
                std::cerr << '"' << command << '"'
                    << " command not recognized" << std::endl;
            }
        } else {
            dir = input();
            switch(dir) {
            case FORWARD:
                send((char*)"D1",2);
                break;
            case BACK:
                send((char*)"D2",2);
                break;
            case LEFT:
                send((char*)"D3",2);
                break;
            case RIGHT:
                send((char*)"D4",2);
                break;
            case STOP:
                send((char*)"D0",2);
                break;
            case TOGGLE:
                normal_mode = true;
                printf("\nSwitching to normal mode\n");
                print_normal_mode_help();
                break;
            case QUIT:
                send((char*)"D0",1);
                return 0; // correct exit point
            default:
                send((char*)"none",4);
            }
        }
    }

    return 3;
}

