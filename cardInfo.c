#include <proto/exec.h>
#include <proto/card.h>
#include <resources/card.h>
#include <stdio.h>

#define LEWord(P) (*(P)|(*((P)+1)<<8))

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

APTR cardRes;

void error(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  struct CardHandle *cardHandle = NULL;
  /*UBYTE controlBits = CARD_DISABLEF_WP|CARD_ENABLEF_DIGAUDIO; */
  UBYTE *tupleBuffer;
  UBYTE tuplePart;
  UWORD manufacturer = 0;
  UWORD product = 0;
  BOOL success = FALSE;

  /* Open card.resource */
  cardRes = OpenResource(CARDRESNAME);
  if (!cardRes) {
    error("Unable to open card.resource");
  }

  /* This is a fairly pointless test, but why not */
  if (CardInterface() != CARD_INTERFACE_AMIGA_0) {
    error("cardInfo only works on classic Amiga PCMCIA interfaces");
  }

  cardHandle->cah_CardNode.ln_Pri = 20;
  cardHandle->cah_CardNode.ln_Type = 0;
  cardHandle->cah_CardNode.ln_Name = NULL;
  cardHandle->cah_CardRemoved = NULL;
  cardHandle->cah_CardInserted = NULL;
  cardHandle->cah_CardStatus = NULL;
  cardHandle->cah_CardFlags = CARDF_IFAVAILABLE;

  /* Claim ownership of the card */
  if (OwnCard(cardHandle) != 0) {
    /* FIXME: OwnCard() returns -1 on failure, or a pointer, from which we could get the ln_Name of the owner and print it? */
    error("Unable to take ownership of PCMCIA card");
  }

  /* Set control bits for the card
  // FIXME: This probably isn't necessary since we're not controlling the card?
  //CardMiscControl(cardHandle, controlBits) != controlBits);
  */

  tupleBuffer = AllocVec(TUPLE_BUFFER_SIZE, MEMF_PUBLIC);
  if (!tupleBuffer) {
    error("Unable to allocate memory for tuple buffer");
  }

  success = CopyTuple(cardHandle, tupleBuffer, PCCARD_TPL_MANFID, MAX_TUPLE_SIZE);
  if (success) {
    tuplePart = tupleBuffer[0];
    if (tuplePart == PCCARD_TPL_MANFID) {
	manufacturer = LEWord(tupleBuffer + 2);
        product = LEWord(tupleBuffer + 4);
    } 
  }

  printf("Found manufacturer: %d", manufacturer);
  printf("Found product: %d", product);

  FreeVec(tupleBuffer);
  
 /* FIXME: ReleaseCard() - maybe with a CardResetCard() first? */
  return 0;
}
