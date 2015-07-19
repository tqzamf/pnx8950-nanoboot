OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 __load_base = 0x80100000 - 512;
 __dram_base = 0x80000000;
 .text __load_base - 512 : AT (__load_base - 512) {
   LONG(0x57434530)
   LONG(__image_end - __load_base)
   LONG(0x57434530)
   LONG(__load_base - __dram_base)
   . = ALIGN(512);
   __image_start = .;
   KEEP(*(.start))
   KEEP(*(.text*))
   KEEP(*(.data*))
   KEEP(*(.bss*))
   KEEP(*(.rodata*))
   . = ALIGN(512);
   KEEP(*(.image*))
   __image_end = .;
 }
}
