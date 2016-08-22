#include <proto/dos.h>

#define EXIT_SUCCESS 0
#define EXIT_WARN 5
#define EXIT_ERROR 10

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void debug(char *msg);
void error(char *msg);
BOOL stringTest(char *actual, char *expected);
