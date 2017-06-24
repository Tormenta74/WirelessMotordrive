
#ifndef _CONNECT_H
#define _CONNECT_H

#define MAX_STR_LENGTH 72 // according to xbee-arduino documentation

void setup();
int connect(char *port);
int send(char *msg, int length);
char *receive();

#endif

