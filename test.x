OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 .text 0x81000000 : AT (0x01000000) {
   KEEP(*(.header))
   . = ALIGN(512);
   KEEP(*(.text))
   KEEP(*(.data));
   KEEP(*(.bss));
   . = ALIGN(512);
   _edata = .;
 }
}
