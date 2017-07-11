/* timing.cpp
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
#include <stdlib.h>
#include <time.h>

#include "timing.h"

char *time_str() {
    time_t now = time(NULL);
    char *string = (char*)malloc(MAX_TIME_STR+2);
    if(sprintf(string,"T%lu",now) < 1) {
        return NULL;
    }
    return string;
}
