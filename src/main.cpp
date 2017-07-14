/* main.cpp
 * Author: Diego Sáinz de Medrano <diego.sainzdemedrano@gmail.com>
 *
 * This file is part of the Wireless Motordrive project.
 * Copyright (C) 2017 Diego Sáinz de Medrano.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation in its second version.

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

#define MAX_COMMAND_STR 6

void print_normal_mode_help()
{
    printf("Type one of the following commands to interact with the robot:\n");
    printf("\tspeed <n>: sets the robot motor drive speed to n (n must be between\n\t\t-2000 and 2000, and is measured in mm/s)\n");
    printf("\tconstant <n>: changes the speed regulation constant (must be between 1 and 200)\n");
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

    // connect to the router
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
    char cmd_msg[MAX_COMMAND_STR];
    while(1) {
        // switch between the modes: read lines, or read keystrokes
        if(normal_mode) {
            std::string command;
            std::getline(std::cin,command);

            // commands can contain an integer
            int data;
            command_t code = process_cmd(command,&data);
            switch(code) {
            case SPEED:
                if(data < -2000 || data > 2000) {
                    printf("speed must be between [-2000,2000]\n");
                    break;
                }
                // 'S' prefix for speed messages
                sprintf(cmd_msg,"S%d",data);
                // and down the channel
                send(cmd_msg,strlen(cmd_msg));
                break;
            case CONST:
                if(data < 1 || data > 100) {
                    printf("constant must be between [1,200]\n");
                }
                // 'K' prefix for constant messages
                sprintf(cmd_msg,"K%d",data);
                // and down the channel
                send(cmd_msg,strlen(cmd_msg));
                break;
            case DUMMY:
                // in the next iteration of the while, we will enter the other branch
                normal_mode = false;
                printf("\nSwitching to dummy mode\n");
                print_dummy_mode_help();
                break;
            case END:
                // stop the robot and exit
                send((char*)"D0",2);
                return 0; // correct exit point
            default:
                // yeah, I mix C and C++ just like that
                std::cerr << '"' << command << '"'
                    << " command not recognized" << std::endl;
            }
        } else {
            // get the keystroke
            dir = input();
            // pretty straight forward
            // 'D' prefix for dummy mode directions
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
                // in the next iteration of the while, we will enter the other branch
                normal_mode = true;
                printf("\nSwitching to normal mode\n");
                print_normal_mode_help();
                break;
            case QUIT:
                // stop the robot and exit
                send((char*)"D0",2);
                return 0; // correct exit point
            default:
                // it should never happen
                send((char*)"none",4);
            }
        }
    }
    // indication of something going wrong
    return 3;
}

