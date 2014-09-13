OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradbigmips", "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 . = 0x84010000;
 . = ALIGN(4);
 .text 0x84010000 :
 {
   KEEP(start.o(.init))
   KEEP(*(.init))
   KEEP(*(.text))
   _data = .;
   KEEP(*(.data));
   KEEP(*(.bss));
 }
 _etext = .;
}
