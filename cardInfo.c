#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "pcmcia.h"
#include "zorro.h"
/* #include "pci.h" */

int main(int argc, char *argv[]) {
  char option = '*';
  char *testValue = NULL;
  BOOL testExpected = FALSE;

  if (argc == 2 && !(strncmp(argv[1], "?", 1))) {
    printf("Usage: %s [v|p|t|b|1|2|3] [testValue]\n", argv[0]);
    printf("  If no options are specified, all card information will be printed, separated by colons\n");
    printf("  To display only one piece of information, add one of the following options:\n");
    printf("    v - Vendor ID\n");
    printf("    p - Product ID\n");
    printf("    t - Card type\n");
    printf("    1 - Card info string 1 (PCMCIA only)\n");
    printf("    2 - Card info string 2 (PCMCIA only)\n");
    printf("    3 - Card info string 3 (PCMCIA only)\n");
    printf("  For use in scripts, specify one of those options and the value you expect.\n");
    printf("  This will cause %s to print nothing, and exit with a return code indicating whether\n", argv[0]);
    printf("   the value was found (0 if it was found, 10 if not).\n");
    printf("   Example:\n");
    printf("    cardInfo p 1024\n");
    printf("    IF WARN\n");
    printf("      Echo \"Card not found\"\n");
    printf("    ELSE\n");
    printf("      Echo \"Card found\"\n");
    printf("    ENDIF\n");
    exit(0);
  } else if (argc >= 2) {
    option = argv[1][0];
  }

  if (argc == 3) {
    testExpected = TRUE;
    testValue = argv[2];
  }

  if (search_pcmcia(option, testExpected, testValue) && testExpected)
      goto exit_found;

  if (search_zorro(option, testExpected, testValue) && testExpected)
      goto exit_found;

  /*
  if (search_pci(option, testExpected, testValue) && testExpected)
    goto exit;
    */

  // We made it here, so if we were testing for something, we failed
  if (testExpected)
    return(EXIT_WARN);

exit_found:
  exit(EXIT_SUCCESS);
}
