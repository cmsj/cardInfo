#include <exec/types.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int search_zorro(char option, BOOL testExpected, char *testValue);
