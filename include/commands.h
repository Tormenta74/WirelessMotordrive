#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <string>

typedef enum {SPEED, CONST, DUMMY, END, CMD_ERR} command_t;

command_t process_cmd(std::string &cmd, int *param);

#endif

