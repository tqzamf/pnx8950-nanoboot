OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 .text 0x80100000 - 4096 : AT (0x80100000 - 4096) {
   KEEP(*(.start))
   KEEP(*(.text*))
   KEEP(*(.data*))
   KEEP(*(.bss*))
   KEEP(*(.rodata*))
   . = ALIGN(4096);
   KEEP(*(.image*))
   __image_end = .;
 }
}
