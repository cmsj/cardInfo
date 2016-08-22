CC = m68k-amigaos-gcc -noixemul -s
CFLAGS = -Os -Wall -fomit-frame-pointer -msmall-code -m68000

cardInfo: cardInfo.c pcmcia.c zorro.c util.c
lha: ../cardInfo.lha cardInfo
../cardInfo.lha:
	./make-lha.sh

clean:
	rm -f cardInfo

dist-clean:
	rm -f ../cardInfo.lha
