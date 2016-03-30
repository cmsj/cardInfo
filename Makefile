CC = m68k-amigaos-gcc -noixemul -s
CFLAGS = -Os -Wall -fomit-frame-pointer -msmall-code -m68000

cardInfo: cardInfo.c

clean:
	rm -f cardInfo
