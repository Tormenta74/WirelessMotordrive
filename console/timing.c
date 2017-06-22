
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "timing.h"

char *time_str() {
    time_t now = time(NULL);
    char *string = (char*)malloc(MAX_TIME_STR+1);
    if(sprintf(string,"%lu",now) < 1) {
        return NULL;
    }
    return string;
}