OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 /* hardcoded values for U-Boot: load decompressor directly above boot
    loader, unpack U-Boot to 0x80100000 and jump there. */
 image_load_base = 0x80100000;
 image_main = image_load_base;
 _load_start = 0x84020000;
 
 .text _load_start - 512 : AT (_load_start - 512) {
   /* 0ECW header */
   LONG(0x57434530)
   LONG(__image_end - __image_start)
   LONG(0x57434530)
   LONG(_start - 0x80000000)
   . = ALIGN(512);

   /* decompressor */
   __image_start = .;
   KEEP(*(.start))
   KEEP(*(.text*))
   KEEP(*(.data*))
   KEEP(*(.rodata*))
   
   /* compressed data */
   packed_base = .;
   KEEP(*(.packed*))
   packed_end = .;
   . = ALIGN(512); /* for xmodem and flash */
   __image_end = .;
 }
 
 .bss : {
   __bss_start = .;
   KEEP(*(.bss*))
   . = ALIGN(16); 
   __bss_end = .;
 }
}
