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

struct Library *ExpansionBase;

int search_zorro(char option, BOOL testExpected, char *testValue) {
  struct ConfigDev *myCD;

  UWORD m,t;
  UBYTE p;

  char manufacturer[8] = "0";
  char product[8] = "0";
  char type[20] = "Unknown";

  /*
  if (testExpected) {
    switch ((unsigned int)option) {
      case 'v':
        if (!stringTest(manufacturer, testValue)) exitCode = EXIT_WARN;
        break;
      case 'p':
        if (!stringTest(product, testValue)) exitCode = EXIT_WARN;
        break;
      case 't':
        if (!stringTest(type, testValue)) exitCode = EXIT_WARN;
        break;
      case '1':
        if (!stringTest(infoString1, testValue)) exitCode = EXIT_WARN;
        break;
      case '2':
        if (!stringTest(infoString2, testValue)) exitCode = EXIT_WARN;
        break;
      case '3':
        if (!stringTest(infoString3, testValue)) exitCode = EXIT_WARN;
        break;
      default:
        error("Invalid option");
        break;
    }
  } else {
    switch ((unsigned int)option) {
      case 'v':
        printf("%s\n", manufacturer);
        break;
      case 'p':
        printf("%s\n", product);
        break;
      case 't':
        printf("%s\n", type);
        break;
      case '1':
        printf("%s\n", infoString1);
        break;
      case '2':
        printf("%s\n", infoString2);
        break;
      case '3':
        printf("%s\n", infoString3);
        break;
      default:
        printf("PCMCIA:%s:%s:%s:%s:%s:%s\n", manufacturer, product, type, infoString1, infoString2, infoString3);
        break;
    }
  }
  */

  if((ExpansionBase = OpenLibrary("expansion.library", 0L)) == NULL)
    return FALSE;

    myCD = NULL;
    while((myCD = FindConfigDev(myCD, -1L, -1L))) { /* search for all ConfigDevs */
      m = myCD->cd_Rom.er_Manufacturer;
      p = myCD->cd_Rom.er_Product;
      t = myCD->cd_Rom.er_Type;

      snprintf(manufacturer, 8, "%ld", m);
      snprintf(product, 8, "%ld", p);
      snprintf(type, 8, "%ld", t);

      printf("ZORRO:%s:%s:%s:::\n", manufacturer,
                                    product,
                                    type);
      }
    CloseLibrary(ExpansionBase);

  return FALSE; // This is wrong
}
