#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/cardres.h>
#include <pragmas/cardres_pragmas.h>
#include <exec/memory.h>
#include <resources/card.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef REG
#ifdef __mc68000
#define _REG(A, B) B __asm(#A)
#define REG(A, B) _REG(A, B)
#else
#define REG(A, B) B
#endif
#endif

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

typedef struct {
  int empty;
} dummyIntData;

void debug(char *msg) {
  printf("DEBUG: %s\n", msg);
  return;
}

void error(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

static VOID DummyInterruptHandler(REG(a1, dummyIntData *intData), REG(a6,APTR intCode)) {
  return;
}

static UBYTE DummyStatusHandler(REG(d0, UBYTE mask), REG(a1, dummyIntData *intData), REG(a6, APTR intCode)) {
  return mask;
}

int main(int argc, char *argv[]) {
  struct CardHandle *cardHandle = NULL;
  struct CardHandle *ownCard;

  dummyIntData *intData;
  struct Interrupt *cardRemovedInt, *cardInsertedInt, *cardStatusInt;

  unsigned long controlBits = CARD_DISABLEF_WP|CARD_ENABLEF_DIGAUDIO;

  UBYTE *tupleBuffer;
  UBYTE tuplePart;
  UWORD manufacturer = 0;
  UWORD product = 0;

  BOOL success;

debug("Opening card.resource");
  /* Open card.resource */
  CardResource = OpenResource(CARDRESNAME);
  if (!CardResource) {
    error("Unable to open card.resource");
  }
debug("Testing PCMCIA interface");
  /* This is a fairly pointless test, but why not */
  if (CardInterface() != CARD_INTERFACE_AMIGA_0) {
    error("cardInfo only works on classic Amiga PCMCIA interfaces");
  }
debug("Allocating interrupt handler RAM");
  intData         = AllocMem(sizeof(dummyIntData),     MEMF_PUBLIC | MEMF_CLEAR);
  cardRemovedInt  = AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR);
  cardInsertedInt = AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR);
  cardStatusInt   = AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR);
  
  if (!intData || !cardRemovedInt || !cardInsertedInt || !cardStatusInt) {
    error("Unable to allocate memory for interrupt handlers");
  }

  /* Add our dummy data struct to the interrupt handlers */
  cardRemovedInt->is_Code  = DummyInterruptHandler;
  cardRemovedInt->is_Data  = intData;
  cardInsertedInt->is_Code = DummyInterruptHandler;
  cardInsertedInt->is_Data = intData;
  cardStatusInt->is_Code   = (void *)DummyStatusHandler;
  cardStatusInt->is_Data   = intData;

  cardHandle->cah_CardNode.ln_Pri = PCCARD_PRIORITY;
  cardHandle->cah_CardNode.ln_Type = 0;
  cardHandle->cah_CardNode.ln_Name = "cardInfo";
  cardHandle->cah_CardRemoved = cardRemovedInt;
  cardHandle->cah_CardInserted = cardInsertedInt;
  cardHandle->cah_CardStatus = cardStatusInt;
  cardHandle->cah_CardFlags = CARDF_IFAVAILABLE;

debug("Claiming card");
  /* Claim ownership of the card */
  ownCard = OwnCard(cardHandle);
  if ((int)ownCard == 0) {
    /* success */
debug("We own the card");
  } else if ((int)ownCard == -1) {
    error("No PCMCIA card is present");
  } else {
    printf("PCMCIA card owned by: %s\n", ownCard->cah_CardNode.ln_Name);
    error("Unable to claim card");
  }

debug("Setting control bits");
  /* Set control bits for the card */
  CardMiscControl(cardHandle, controlBits);

debug("Allocating tuple buffer");
  tupleBuffer = (UBYTE *)AllocVec(TUPLE_BUFFER_SIZE, MEMF_PUBLIC);
  if (!tupleBuffer) {
    ReleaseCard(cardHandle, NULL);
    error("Unable to allocate memory for tuple buffer");
  }

debug("Copying MANFID tuple");
  success = CopyTuple(cardHandle, tupleBuffer, PCCARD_TPL_MANFID, MAX_TUPLE_SIZE);
  if (success) {
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
  printf("Found manufacturer: %d", manufacturer);
  printf("Found product: %d", product);

debug("Freeing tuple buffer");
  FreeVec(tupleBuffer);

debug("Resetting card");
  CardMiscControl(cardHandle, 0);
  CardResetCard(cardHandle);
debug("Releasing card");
  ReleaseCard(cardHandle, CARDF_REMOVEHANDLE);
debug("Releasing memory");
  FreeVec(cardHandle->cah_CardStatus);
  FreeVec(cardHandle->cah_CardInserted);
  FreeVec(cardHandle->cah_CardRemoved);
  FreeVec(tupleBuffer);
  FreeMem(cardHandle, sizeof(struct CardHandle));
  FreeMem(intData, sizeof(dummyIntData));

  return 0;
}
