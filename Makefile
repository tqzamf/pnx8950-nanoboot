CC=mipsel-linux-gnu-gcc
CFLAGS=-Wall -Werror -std=gnu99 -g -Os -mips32 -mno-shared -mno-gpopt \
	-G0 -fpic -fomit-frame-pointer -mno-interlink-mips16 -mno-abicalls \
	-mabi=eabi -mgp32 -mno-check-zero-division
AS=mipsel-linux-gnu-as
ASFLAGS=--fatal-warnings -mips32 -mabi=eabi -mgp32 -mno-shared -G0
LD=mipsel-linux-gnu-ld
LDFLAGS=-static -nostdlib -G0
TARGETS=boot.elf

all: $(TARGETS)

clean:
	rm -f *.o *~ $(TARGETS)

boot.elf: start.o init.o main.o
	$(LD) $(LDFLAGS) -o $@ -T boot.x $^

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
