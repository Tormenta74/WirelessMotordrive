/* connect.cpp
 * Author: Diego Sáinz de Medrano <diego.sainzdemedrano@gmail.com>
 * Contents based on the work of Antonio Carlos Domínguez <adominguez@iusiani.ulpgc.es>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xbee.h>

#include "connect.h"

// "module" global variables
struct xbee *xbee;
struct xbee_con *con;
struct xbee_conAddress address;
struct xbee_pkt* pkt;

// call to set the router address
void setup_conn() {
    memset(&address, 0, sizeof(address));
    address.addr64_enabled = 1;
    // ROUTER1
    address.addr64[0] = 0x00;
    address.addr64[1] = 0x13;
    address.addr64[2] = 0xA2;
    address.addr64[3] = 0x00;
    address.addr64[4] = 0x40;
    address.addr64[5] = 0xAD;
    address.addr64[6] = 0x72;
    address.addr64[7] = 0x42;
}

// establish the connection (I have no idea)
int connect(char *port) {
    xbee_err ret=xbee_setup(&xbee,"xbeeZB",port,38400);
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_setup() failed (%s:%i)\n\t-> %i - %s\n",
                __FILE__,__LINE__,(int)ret,xbee_errorToStr(ret));
         return (int)ret;
    }
    ret=xbee_conNew(xbee,&con,"Data",&address);
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_conNew() failed (%s:%i)\n\t-> %i - %s\n",
                __FILE__,__LINE__,(int)ret,xbee_errorToStr(ret));
        return (int)ret;
    }
    return 0;
}

// try a transmission
int send(char *msg, int length) {
    xbee_err ret=xbee_connTx(
            con,
            NULL,
            (const unsigned char*)msg,
            (length < MAX_STR_LENGTH) ?
            length: MAX_STR_LENGTH
            );
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_connTx() failed (%s:%i)\n\t-> %i - %s\n",
                __FILE__,__LINE__,(int)ret,xbee_errorToStr(ret));
        return (int)ret;
    }
    return 0;
}

// try a reception, waiting 1s
char *receive() {
    char *msg=NULL;
    xbee_err ret=xbee_conRxWait(con,&pkt,NULL);
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_conRxWait() failed (%s:%i)\n\t-> %i - %s\n",
                __FILE__,__LINE__,(int)ret,xbee_errorToStr(ret));
         goto end;
    }
    if((msg=(char*)malloc((sizeof(char))*(pkt->dataLen+1))) == NULL) {
        fprintf(stderr,"malloc error (%s,%i)\n",
                __FILE__,__LINE__);
        goto end;
    }
    memcpy(msg,pkt->data,pkt->dataLen);
end:
    return msg;
}
