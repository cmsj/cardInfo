CC = m68k-amigaos-gcc
INCDIR = /opt/m68k-amigaos/os-include
LIBDIR = /opt/m68k-amigaos/os-lib
CCOPTS = -noixemul

cardInfo:
	$(CC) -I $(INCDIR) -L $(LIBDIR) $(CCOPTS) cardInfo.c -o cardInfo
