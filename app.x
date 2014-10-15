OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 .text 0x80100000 - 512 : AT (0x80100000 - 512) {
   LONG(0x57434530)
   LONG(__image_end - 0x80100000)
   LONG(0x57434530)
   LONG(0)
   . = ALIGN(512);
   KEEP(*(.start))
   KEEP(*(.text*))
   KEEP(*(.data*))
   KEEP(*(.bss*))
   KEEP(*(.rodata*))
   . = ALIGN(512);
   __image_end = .;
 }
}
