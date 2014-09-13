CC=mipsel-linux-gnu-gcc
CFLAGS=-Wall -Werror -std=gnu99 -g -Os -mips32 -mno-shared -mno-gpopt \
	-G0 -fpic -fomit-frame-pointer -mno-interlink-mips16 -mno-abicalls \
	-mabi=eabi -mgp32 -mno-check-zero-division
AS=mipsel-linux-gnu-as
ASFLAGS=-W -g -mips32 -mno-shared -G0 -fomit-frame-pointer
LD=mipsel-linux-gnu-gcc
LDFLAGS=-static -nostdlib
TARGETS=boot

all: $(TARGETS)

clean:
	rm -f *.o *~ $(TARGETS)

boot: boot.lds start.o init.o main.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
