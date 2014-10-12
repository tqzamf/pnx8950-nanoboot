OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 .text 0x84010000 : AT (0x04010000) {
   KEEP(start.o(.init))
   KEEP(init.o(.init))
   KEEP(*(.init))
   KEEP(*(.text))
   KEEP(*(.data));
   KEEP(*(.bss));
   . = ALIGN(4);
   _edata = .;
 }
 .uninitialized : {
   KEEP(*(.uninitialized));
 }
}
