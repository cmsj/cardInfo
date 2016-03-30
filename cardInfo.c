#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/cardres.h>
#include <resources/card.h>
#include <stdio.h>
#include <stdlib.h>

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
debug("Resetting card");
  CardMiscControl(cardHandle, 0);
  CardResetCard(cardHandle);
debug("Releasing card");
  ReleaseCard(cardHandle, CARDF_REMOVEHANDLE);
}

int main(int argc, char *argv[]) {
  struct CardHandle cardHandle;
  struct CardHandle *ownCard;

  static struct Interrupt cardRemovedInt  = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};
  static struct Interrupt cardInsertedInt = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};
  static struct Interrupt cardStatusInt   = {{NULL, NULL, NT_INTERRUPT, 0, NULL}, (APTR)1, DummyInterruptHandler};

  static UBYTE tupleBuffer[TUPLE_BUFFER_SIZE];
  UBYTE tuplePart;
  UWORD manufacturer = 0;
  UWORD product = 0;

  BOOL success;

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

  success = CopyTuple(&cardHandle, tupleBuffer, PCCARD_TPL_MANFID, MAX_TUPLE_SIZE);
  if (!success) {
    CleanupCard(&cardHandle);
    error("MANFID tuple not found");
  } else {
debug("Testing tuple type");
    tuplePart = tupleBuffer[0];
    if (tuplePart == PCCARD_TPL_MANFID) {
debug("Reading 1st tuple value");
	manufacturer = LEWord(tupleBuffer + 2);
debug("Reading 2nd tuple value");
        product = LEWord(tupleBuffer + 4);
    } 
  }

debug("Printing tuple data");
  printf("Found manufacturer: %d\n", manufacturer);
  printf("Found product: %d\n", product);

  CleanupCard(&cardHandle);

  return 0;
}
