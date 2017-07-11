/* controls.cpp
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

#include <stdio.h>
#include <termios.h>

#include "controls.h"

/* https://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux */

struct termios old, new_;

/* Initialize new_ terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new_ = old; /* make new_ settings same as old settings */
  new_.c_lflag &= ~ICANON; /* disable buffered i/o */
  new_.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new_); /* use these new_ terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character */
char getch() 
{
  char ch;
  initTermios(0);
  ch = getchar();
  resetTermios();
  return ch;
}

direction_t input()
{
    char key = getch();

    switch(key) {
    case 'w':
    case 'W':
        return FORWARD;
    case 's':
    case 'S':
        return BACK;
    case 'd':
    case 'D':
        return RIGHT;
    case 'a':
    case 'A':
        return LEFT;
    case ' ':
        return STOP;
    case 'm':
    case 'M':
        return TOGGLE;
    case 'q':
    case 'Q':
        return QUIT;
    default:
        return STOP;
    }
}
