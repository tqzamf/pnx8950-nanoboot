CC=mipsel-linux-gnu-gcc
CFLAGS=-Wall -Werror -std=gnu99 -g -Os -mips32 -mno-shared -mno-gpopt \
	-G0 -fpic -fomit-frame-pointer -mno-interlink-mips16 -mno-abicalls \
	-mabi=eabi -mgp32 -mno-check-zero-division
AS=mipsel-linux-gnu-as
ASFLAGS=--fatal-warnings -mips32 -mabi=eabi -mgp32 -mno-shared -G0
LD=mipsel-linux-gnu-ld
LDFLAGS=-static -nostdlib -G0
OC=mipsel-linux-gnu-objcopy
NM=mipsel-linux-gnu-nm
MKIMAGE=mkimage
TARGETS=boot.elf

all: $(TARGETS)

clean:
	rm -f *.o *~ $(TARGETS)

uImage: boot.bin
	$(MKIMAGE) -e $$($(NM) boot.elf | awk -f get-entry-point.awk) \
		-A mips -O linux -T kernel -C none -n 'NanoBoot' -a 0xA4010000 \
		-d boot.bin $@

boot.bin: boot.elf
	$(OC) -j .text -O binary $< $@

boot.elf: start.o init.o main.o
	$(LD) $(LDFLAGS) -o $@ -T boot.x $^

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
