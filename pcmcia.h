#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define LEWord(P) (*(P)|(*((P)+1)<<8))

#define PCMCIA_PRIORITY 20
#define MAX_TUPLE_SIZE 0xff
#define TUPLE_BUFFER_SIZE (MAX_TUPLE_SIZE + 8)

#define CISTPL_VERS_1  0x15
#define CISTPL_MANFID 0x20
#define CISTPL_FUNCID 0x21

#define CISTPL_FUNCID_MULTI  0x00
#define CISTPL_FUNCID_MEMORY    0x01
#define CISTPL_FUNCID_SERIAL 0x02
#define CISTPL_FUNCID_PARALLEL  0x03
#define CISTPL_FUNCID_FIXED  0x04
#define CISTPL_FUNCID_VIDEO  0x05
#define CISTPL_FUNCID_NETWORK  0x06
#define CISTPL_FUNCID_AIMS   0x07
#define CISTPL_FUNCID_SCSI   0x08

BOOL search_pcmcia(char option, BOOL testExpected, char *testValue);

