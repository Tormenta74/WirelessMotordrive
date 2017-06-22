#ifndef _CONTROLS_H
#define _CONTROLS_H

typedef enum {FORWARD, BACK, LEFT, RIGHT, NONE, STOP} direction_t;

void init_controls();
direction_t input();

#endif
