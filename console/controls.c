#include <stdio.h>
#include <termios.h>

#include "controls.h"

void init_controls() {}

/* https://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux */
struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
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
