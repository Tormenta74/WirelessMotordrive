#ifndef _CONTROLS_H
#define _CONTROLS_H

typedef enum {FORWARD, BACK, LEFT, RIGHT, STOP, QUIT} direction_t;

void init_controls();
direction_t input();

#endif
