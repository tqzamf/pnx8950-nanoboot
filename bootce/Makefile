CC=mipsel-linux-gnu-gcc
CFLAGS=-Wall -Werror -std=gnu99 -g -Os -mips32 -mno-shared -mno-gpopt \
	-G0 -fpic -fomit-frame-pointer -mno-interlink-mips16 -mno-abicalls \
	-mabi=eabi -mgp32 -mno-check-zero-division
AS=mipsel-linux-gnu-as
ASFLAGS=--fatal-warnings -mips32 -mabi=eabi -mgp32 -mno-shared -G0
LD=mipsel-linux-gnu-ld
LDFLAGS=-static -G0
CPP=mipsel-linux-gnu-cpp
OC=mipsel-linux-gnu-objcopy
NM=mipsel-linux-gnu-nm
MKIMAGE=mkimage
TARGETS=wince0.elf wince0.ecw wince0.uImage \
	wince1.elf wince1.ecw wince1.uImage

all: $(TARGETS)

clean:
	rm -f *.o *.elf *.bp *.gz *.s *~ $(TARGETS)

%.elf: %.o init.o
	$(LD) $(LDFLAGS) -o $@ -T wince.x -nostdlib $^

%.ecw: %.elf
	$(OC) -j .text -O binary $< $@

%.gz %.uImage: %.ecw
	gzip --best <$*.ecw >$*.gz
	$(MKIMAGE) -A mips -O linux -T kernel -C gzip -n "$*" \
		-a $$($(NM) $*.elf | awk -f get-entry-point.awk) \
		-e $$($(NM) $*.elf | awk -f get-entry-point.awk) \
		-d $*.gz $@

%.o: %.bin
	perl strip-header.pl <$*.bin >$*.tmp
	$(OC) -I binary -O elf32-tradlittlemips -B mips:isa32 \
		--change-addresses 0x80100000 --rename-section .data=.image \
		--set-section-flags .image=contents,alloc,load,readonly,code \
		--redefine-sym _binary_$*_tmp_start=image_main \
		$*.tmp $@
	rm $*.tmp

info.bin: info.pl
	perl $< >$@

%.o: %.S
	$(CPP) -o $*.s $^
	$(AS) $(ASFLAGS) -o $@ $*.s

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
