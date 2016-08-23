#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void debug(char *msg) {
  printf("DEBUG: %s\n", msg);
  return;
}

void error(char *msg) {
  printf("ERROR: %s\n", msg);
}

BOOL stringTest(char *actual, char *expected) {
  return (0 == strncmp(actual, expected, MIN(strlen(actual), strlen(expected))));
}
