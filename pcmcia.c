#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/cardres.h>
#include <resources/card.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "pcmcia.h"

struct Library *CardResource = NULL;

void DummyInterruptHandler(void) {}

void CleanupCard(struct CardHandle *cardHandle) {
  CardMiscControl(cardHandle, 0);
  CardResetCard(cardHandle);
  ReleaseCard(cardHandle, CARDF_REMOVEHANDLE);
}

BOOL search_pcmcia(char option, BOOL testExpected, char *testValue) {
  BOOL returnCode = FALSE;
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

  BOOL success;

  /* Open card.resource */
  CardResource = OpenResource(CARDRESNAME);
  if (!CardResource) {
    debug("Unable to open card.resource");
    return(FALSE);
  }

  /* This is a fairly pointless test, but why not */
  if (CardInterface() != CARD_INTERFACE_AMIGA_0) {
    debug("cardInfo only works on classic Amiga PCMCIA interfaces");
    return(FALSE);
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
    debug("No PCMCIA card is present");
    return(FALSE);
  } else {
    printf("PCMCIA card already owned by: %s\n", ownCard->cah_CardNode.ln_Name);
    goto cleanup;
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
        if (stringTest(manufacturer, testValue)) returnCode = TRUE;
        break;
      case 'p':
        if (stringTest(product, testValue)) returnCode = TRUE;
        break;
      case 't':
        if (stringTest(type, testValue)) returnCode = TRUE;
        break;
      case '1':
        if (stringTest(infoString1, testValue)) returnCode = TRUE;
        break;
      case '2':
        if (stringTest(infoString2, testValue)) returnCode = TRUE;
        break;
      case '3':
        if (stringTest(infoString3, testValue)) returnCode = TRUE;
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

cleanup:
  if (infoString1 != NULL)
    free(infoString1);
  if (infoString2 != NULL)
    free(infoString2);
  if (infoString3 != NULL)
    free(infoString3);

  CleanupCard(&cardHandle);

  return(returnCode);
}
