
#include <iterator>
#include <sstream>
#include <vector>

#include "commands.h"

const std::string speed_str = "speed";
const std::string enc_str = "encoder";
const std::string toggle_str = "toggle";
const std::string quit_str = "quit";

/* https://stackoverflow.com/questions/236129/split-a-string-in-c#answer-236803 */
template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

command_t process_cmd(std::string &cmd, int *param)
{
    if(param == NULL)
        return CMD_ERR;

    std::vector<std::string> tokens = split(cmd,' ');
        // now process commands
        if(tokens.size() > 2) {
            return CMD_ERR;
        }
        if(tokens.front() == speed_str) {
            try {
                *param = std::stoi(tokens.back());
            } catch (const char* msg) {
                fprintf(stderr, "commands: %s", msg);
            }
            return SPEED;
        } else if(tokens.front() == enc_str) {
            return ENC;
        } else if(tokens.front() == toggle_str) {
            return DUMMY;
        } else if(tokens.front() == quit_str) {
            return END;
        } else {
            return CMD_ERR;
        }
    return DUMMY;
}

