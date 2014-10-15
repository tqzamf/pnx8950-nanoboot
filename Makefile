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
TARGETS=boot.bin uImage eecompile.jar nanoboot.bp

all: $(TARGETS)

clean:
	rm -f *.o *~ $(TARGETS)

eecompile.jar: eecompile/src/*.java eecompile/build.xml
	(cd eecompile && ant clean build)
	cp eecompile/eecompile.jar $@
	chmod +x $@

ecctester: ecctester.c
	$(CC) $(CFLAGS) $(LDFLAGS) -mabicalls -mabi=32 -o $@ $^

uImage: boot.bin
	$(MKIMAGE) -e $$($(NM) boot.elf | awk -f get-entry-point.awk) \
		-A mips -O linux -T kernel -C none -n 'NanoBoot' -a 0xA4010000 \
		-d boot.bin $@

%.bp: %.ee boot.bin eecompile.jar
	./eecompile.jar -O1 -fbp -o $@ -v $<

%.bin: %.elf
	$(OC) -j .text -O binary $< $@

boot.elf: start.o init.o main.o
	$(LD) $(LDFLAGS) -o $@ -T boot.x -nostdlib $^

%.elf: %.o appstart.o
	$(LD) $(LDFLAGS) -o $@ -T app.x -nostdlib $^

%.o: %.S
	$(CPP) -o $*.s $^
	$(AS) $(ASFLAGS) -o $@ $*.s

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: all clean
