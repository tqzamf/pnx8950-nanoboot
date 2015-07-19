CC=mipsel-linux-gnu-gcc
CFLAGS=-Wall -Werror -std=gnu99 -g -Os -mips32 -mno-shared -mno-gpopt \
	-G0 -fpic -fomit-frame-pointer -mno-interlink-mips16 -mno-abicalls \
	-mabi=32 -mgp32 -mno-check-zero-division -I. -I.. -I../apps
AS=mipsel-linux-gnu-as
ASFLAGS=--fatal-warnings -mips32 -mabi=eabi -mgp32 -mno-shared -G0
LD=mipsel-linux-gnu-ld
LDFLAGS=-static -G0
CPP=mipsel-linux-gnu-cpp
OC=mipsel-linux-gnu-objcopy
NM=mipsel-linux-gnu-nm
MKIMAGE=mkimage
TARGETS=u-boot.ecw u-boot.elf empty.ecw empty.elf

all: $(TARGETS)

clean:
	rm -f *.o *.elf *~ $(TARGETS)

%.elf: %.o crt.o decompress.o puff.o crc32.o
	$(LD) $(LDFLAGS) -o $@ -T compressed.x -nostdlib $^

%.ecw: %.elf
	$(OC) -j .text -O binary $< $@

%.o: %.bin
	perl strip-header.pl < $*.bin | gzip --best | perl gzip2deflate.pl >temp.deflate
	$(OC) -I binary -O elf32-tradlittlemips -B mips:isa32 \
		--set-section-flags .image=contents,alloc,load,readonly,code \
		--redefine-sym _binary_temp_deflate_start=packed_base \
		--redefine-sym _binary_temp_deflate_size=packed_size \
		--redefine-sym _binary_temp_deflate_end=packed_end \
		--rename-section .data=.packed temp.deflate $@
	rm temp.deflate
	
# rules for C and assembly files

%.o: %.S
	$(CPP) -o $*.s $^
	$(AS) $(ASFLAGS) -o $@ $*.s

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
