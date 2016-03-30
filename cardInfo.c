#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/cardres.h>
#include <resources/card.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEWord(P) (*(P)|(*((P)+1)<<8))

#define PCCARD_PRIORITY 20

#define MAX_TUPLE_SIZE 0xff
#define TUPLE_BUFFER_SIZE (MAX_TUPLE_SIZE + 8)

#define PCCARD_TPL_VERS1  0x15
#define PCCARD_TPL_MANFID 0x20
#define PCCARD_TPL_FUNCID 0x21

#define PCCARD_FUNC_MULTI  0x00
#define PCCARD_FUNC_MEM    0x01
#define PCCARD_FUNC_SERIAL 0x02
#define PCCARD_FUNC_PARRL  0x03
#define PCCARD_FUNC_FIXED  0x04
#define PCCARD_FUNC_VIDEO  0x05
#define PCCARD_FUNC_NETWK  0x06
#define PCCARD_FUNC_AIMS   0x07
#define PCCARD_FUNC_SCSI   0x08

struct Library *CardResource;

void debug(char *msg) {
  printf("DEBUG: %s\n", msg);
  return;
}

void error(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
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
  UWORD manufacturer = 0;
  UWORD product = 0;
  char *infoString = NULL;
  char *type = "Unknown";
  char option = '*';

  BOOL success;

  if (argc == 2 && !(strncmp(argv[1], "?", 1))) {
    printf("Usage: %s [v|p|t|n]\n", argv[0]);
    printf("  v - Vendor ID\n");
    printf("  p - Product ID\n");
    printf("  t - Card type\n");
    printf("  n - Card name\n");
    printf("If no options are specified, all the information will be printed, separated by colons\n");
    exit(0);
  } else if (argc == 2) {
    option = argv[1][0];
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
  cardHandle.cah_CardNode.ln_Pri  = PCCARD_PRIORITY;
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
  CardMiscControl(&cardHandle, CARD_DISABLEF_WP|CARD_ENABLEF_DIGAUDIO);

  /* Get Manufacturer/Product data, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, PCCARD_TPL_MANFID, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == PCCARD_TPL_MANFID) {
    manufacturer = LEWord(tupleBuffer + 2);
    product = LEWord(tupleBuffer + 4);
  }

  /* Get product name string, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, PCCARD_TPL_VERS1, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == PCCARD_TPL_VERS1) {
    char *p = &tupleBuffer[4];
    p += strlen(p) + 1;
    p += strlen(p) + 1;
    infoString = malloc(strlen(p) + 1);
    strncpy(infoString, p, strlen(p));
  }

  /* Get card type, if available */
  success = CopyTuple(&cardHandle, tupleBuffer, PCCARD_TPL_FUNCID, MAX_TUPLE_SIZE);
  if (success && tupleBuffer[0] == PCCARD_TPL_FUNCID) {
    switch (tupleBuffer[2]) {
      case PCCARD_FUNC_MULTI:
        type = "Multifunction";
        break;
      case PCCARD_FUNC_MEM:
        type = "Memory";
        break;
      case PCCARD_FUNC_SERIAL:
        type = "Serial";
        break;
      case PCCARD_FUNC_PARRL:
        type = "Parallel";
        break;
      case PCCARD_FUNC_FIXED:
        type = "Fixed";
        break;
      case PCCARD_FUNC_VIDEO:
        type = "Video";
        break;
      case PCCARD_FUNC_NETWK:
        type = "Network";
        break;
      case PCCARD_FUNC_AIMS:
        type = "AIMS";
        break;
      case PCCARD_FUNC_SCSI:
        type = "SCSI";
        break;
      default:
        break;
    }
  }

  switch ((unsigned int)option) {
    case 'v':
      printf("%d\n", manufacturer);
      break;
    case 'p':
      printf("%d\n", product);
      break;
    case 't':
      printf("%s\n", type);
      break;
    case 'n':
      printf("%s\n", infoString);
      break;
    default:
      printf("%d:%d:%s:%s\n", manufacturer, product, type, infoString);
      break;
  }

  free(infoString);
  CleanupCard(&cardHandle);

  return 0;
}
