#include <exec/types.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

#include "zorro.h"

struct Library *ExpansionBase;

int search_zorro(char option, BOOL testExpected, char *testValue) {
  BOOL returnCode = FALSE;
  struct ConfigDev *myCD = NULL;

  UWORD m,t;
  UBYTE p;

  char manufacturer[8] = "0";
  char product[8] = "0";
  char type[20] = "Unknown";

  if((ExpansionBase = OpenLibrary("expansion.library", 0L)) == NULL)
    return FALSE;

  while((myCD = FindConfigDev(myCD, -1L, -1L))) { /* search for all ConfigDevs */
    m = myCD->cd_Rom.er_Manufacturer;
    p = myCD->cd_Rom.er_Product;
    t = myCD->cd_Rom.er_Type;

    snprintf(manufacturer, 8, "%ld", m);
    snprintf(product, 8, "%ld", p);
    snprintf(type, 8, "%ld", t);

    if (testExpected) {
      switch ((unsigned int)option) {
        case 'v':
          if (stringTest(manufacturer, testValue)) returnCode = TRUE;
          break;
        case 'p':
          if (stringTest(product, testValue)) returnCode = TRUE;
          break;
        case 't':
          if (stringTest(type, testValue)) returnCode = TRUE;
          break;
        case '1':
        case '2':
        case '3':
          break;
        default:
          error("Invalid option");
          exit(EXIT_ERROR);
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
          printf("\n");
          break;
        case '2':
          printf("\n");
          break;
        case '3':
          printf("\n");
          break;
        default:
          printf("ZORRO:%s:%s:%s:::\n", manufacturer, product, type);
          break;
      }
    }
  }
  CloseLibrary(ExpansionBase);

  return returnCode;
}
