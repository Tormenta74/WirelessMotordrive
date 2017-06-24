
#include <stdio.h>
#include <xbee.h>

#include "connect.h"

struct xbee *xbee;
struct xbee_con *con;
struct xbee_conAddress address;
struct xbee_pkt* pkt;

void setup() {
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

int connect(char *port) {
    xbee_err ret=xbee_setup(&xbee,"xbeeZB",port,38400);
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_setup() failed -> %i (%s:%i)\n",
                (int)ret, __FILE__,__LINE__);
        return (int)ret;
    }
    ret=xbee_conNew(xbee,&con,"Data",&address);
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_conNew() failed -> %i (%s:%i)\n",
                (int)ret, __FILE__,__LINE__);
        return (int)ret;
    }
    return 0;
}

int send(char *msg, int length) {
    xbee_err ret=xbee_connTx(
            con,
            NULL,
            (const char*)msg,
            (length < MAX_STR_LENGTH) ?
            lenght : MAX_STR_LENGTH
            );
    if(ret!=XBEE_ENONE) {
        fprintf(stderr,"xbee_connTx() failed -> %i (%s:%i)\n",
                (int)ret, __FILE__,__LINE__);
        return (int)ret;
    }
    return 0;
}

char *receive() {
    char *msg=NULL;
    xbee_err ret=xbee_conRx(con,&pkt,NULL);
    if( (ret!=XBEE_ENONE) && (ret!=XBEE_ENOTEXISTS) ) {
        fprintf(stderr,"xbee_conRx() failed -> %i (%s,%i)\n",
                (int)ret,__FILE__,__LINE__);
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
