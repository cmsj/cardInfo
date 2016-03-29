CC = m68k-amigaos-gcc -noixemul -s -v
INCDIR = /opt/m68k-amigaos/os-include
LIBDIR = /opt/m68k-amigaos/os-lib
CFLAGS = -Os -Wall -fomit-frame-pointer -msmall-code

cardInfo: cardInfo.c
	$(CC) -I $(INCDIR) -L $(LIBDIR) $(CFLAGS) cardInfo.c -o cardInfo

clean:
	rm -f cardInfo
