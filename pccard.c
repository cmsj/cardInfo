#include <proto/exec.h>
#include <proto/card.h>
#include <stdio.h>

struct Library *pccardLib;

void error(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

int main(int argc, char *argv) {
  pccardLib = OpenLibrary("pccard.library", 0);

  if (!pccardLib) {
    error("Unable to open pccard.library");
  }
 
  return 0;
}
