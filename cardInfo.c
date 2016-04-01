#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/cardres.h>
#include <resources/card.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define LEWord(P) (*(P)|(*((P)+1)<<8))

#define PCMCIA_PRIORITY 20
#define EXIT_SUCCESS 0
#define EXIT_WARN 5
#define EXIT_ERROR 10

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

struct Library *CardResource;

void debug(char *msg) {
  printf("DEBUG: %s\n", msg);
  return;
}

void error(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(EXIT_ERROR);
}

BOOL stringTest(char *actual, char *expected) {
  return (0 == strncmp(actual, expected, MIN(strlen(actual), strlen(expected))));
}

void DummyInterruptHandler(void) {}

void CleanupCard(struct CardHandle *cardHandle) {
  CardMiscControl(cardHandle, 0);
  CardResetCard(cardHandle);
  ReleaseCard(cardHandle, CARDF_REMOVEHANDLE);
}

int main(int argc, char *argv[]) {
  struct CardHandle cardHandle;
  struct CardHandle *ownCard;

  static struct Interrupt cardRemovedInt  = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};
  static struct Interrupt cardInsertedInt = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};
  static struct Interrupt cardStatusInt   = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};

  static UBYTE tupleBuffer[TUPLE_BUFFER_SIZE];
  char manufacturer[6] = "0";
  char product[6] = "0";
  char *infoString1 = NULL;
  char *infoString2 = NULL;
  char *infoString3 = NULL;
  char *type = "Unknown";
  char option = '*';
  char *testValue = NULL;

  BOOL testExpected = FALSE;
  BOOL success;

  int exitCode = EXIT_SUCCESS;

  if (argc == 2 && !(strncmp(argv[1], "?", 1))) {
    printf("Usage: %s [v|p|t|1|2|3] [testValue]\n", argv[0]);
    printf("  If no options are specified, all card information will be printed, separated by colons\n");
    printf("  To display only one piece of information, add one of the following options:\n");
    printf("    v - Vendor ID\n");
    printf("    p - Product ID\n");
    printf("    t - Card type\n");
    printf("    1 - Card info string 1\n");
    printf("    2 - Card info string 2\n");
    printf("    3 - Card info string 3\n");
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

  /* Open card.resource */
  CardResource = OpenResource(CARDRESNAME);
  if (!CardResource) {
    error("Unable to open card.resource");
  }

  /* This is a fairly pointless test, but why not */
  if (CardInterface() != CARD_INTERFACE_AMIGA_0) {
    error("cardInfo only works on classic Amiga PCMCIA interfaces");
  }

  /* Add our dummy data struct to the interrupt handlers */
  cardHandle.cah_CardNode.ln_Pri  = PCMCIA_PRIORITY;
  cardHandle.cah_CardNode.ln_Type = 0;
  cardHandle.cah_CardNode.ln_Name = "cardInfo";
  cardHandle.cah_CardRemoved  = &cardRemovedInt;
  cardHandle.cah_CardInserted = &cardInsertedInt;
  cardHandle.cah_CardStatus   = &cardStatusInt;
  cardHandle.cah_CardFlags    = CARDF_IFAVAILABLE;

  /* Claim ownership of the card */
  ownCard = OwnCard(&cardHandle);
  if ((int)ownCard == 0) {
    /* success */
  } else if ((int)ownCard == -1) {
    error("No PCMCIA card is present");
  } else {
    printf("PCMCIA card owned by: %s\n", ownCard->cah_CardNode.ln_Name);
    error("Unable to claim card");
  }

  /* Set control bits for the card */
  CardMiscControl(&cardHandle, CARD_DISABLEF_WP | CARD_ENABLEF_DIGAUDIO);

  /* Get Manufacturer/Product data, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, CISTPL_MANFID, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == CISTPL_MANFID) {
    sprintf(manufacturer, "%hu", LEWord(tupleBuffer + 2));
    sprintf(product, "%hu", LEWord(tupleBuffer + 4));
  }

  /* Get device info strings, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, CISTPL_VERS_1, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == CISTPL_VERS_1) {
    char *p = &tupleBuffer[4];
    infoString1 = malloc(strlen(p) + 1);
    strncpy(infoString1, p, strlen(p) + 1);
    p += strlen(p) + 1;

    infoString2 = malloc(strlen(p) + 1);
    strncpy(infoString2, p, strlen(p) + 1);
    p += strlen(p) + 1;

    infoString3 = malloc(strlen(p) + 1);
    strncpy(infoString3, p, strlen(p) + 1);
  }

  /* Get card type, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, CISTPL_FUNCID, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == CISTPL_FUNCID) {
    switch (tupleBuffer[2]) {
      case CISTPL_FUNCID_MULTI:
        type = "Multifunction";
        break;
      case CISTPL_FUNCID_MEMORY:
        type = "Memory";
        break;
      case CISTPL_FUNCID_SERIAL:
        type = "Serial";
        break;
      case CISTPL_FUNCID_PARALLEL:
        type = "Parallel";
        break;
      case CISTPL_FUNCID_FIXED:
        type = "Fixed";
        break;
      case CISTPL_FUNCID_VIDEO:
        type = "Video";
        break;
      case CISTPL_FUNCID_NETWORK:
        type = "Network";
        break;
      case CISTPL_FUNCID_AIMS:
        type = "AIMS";
        break;
      case CISTPL_FUNCID_SCSI:
        type = "SCSI";
        break;
      default:
        break;
    }
  }

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
        printf("%s:%s:%s:%s:%s:%s\n", manufacturer, product, type, infoString1, infoString2, infoString3);
        break;
    }
  }

  free(infoString1);
  free(infoString2);
  free(infoString3);
  CleanupCard(&cardHandle);

  exit(exitCode);
}
