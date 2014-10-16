## Installing nanoboot on the Pollinux board

* build nanoboot (obviously). this is just `make clean all`.
* solder pin headers onto U11.
* connect bus pirate. pin 4 is GND, 5 is SDA, 6 is SCL.
* connect to bus pirate: `screen /dev/ttyUSBx 115200`.
* switch bus pirate to I²C: press `m` to select mode, then follow menu. speed is irrelevant.
* upload nanoboot: press Ctrl-A `:` then type `exec .!. ./bpsend.pl nanoboot.bp`.
* use a jumper to short U11 pins 7/8. this prevents accidental reprogramming, and keeps the board from wearing out the first EEPROM page.
* press the reset button

the board is now unbrickable (but not indestructible). the steps above can be repeated easily if the EEPROM does get corrupted. if any further step in the boot chain fails, switch SW1.1 to ON and upload something (probably U-Boot) over Xmodem.

if the bus pirate shows errors during reprogramming, then probably the boot module hangs and thus the I²C bus is blocked. just hold the reset button during the entire programming process programming and things should work.

## Installing U-Boot using nanoboot

* connect to nanoboot: `screen /dev/ttyUSBx 38400`.
* switch SW1.1 to ON and press reset. nanoboot should eventually respond with an `X` prompt.
* upload U-Boot: press Ctrl-A `:` then type `exec !! sx uboot.bin`. you'll want to use the one with Linux ECC and proper length in the header.
* cancel autoboot. if you miss it and it fails to boot, that's fine but don't do a `saveenv` without resetting again.
* do `mtdparts default` if necessary. it *is* necessary when converting from Windows CE.
* edit `mtdparts` to make the `U-Boot` partition writeable, ie. change `512k(U-Boot)ro` to `512k(U-Boot)`: `editenv mtdparts`.
* erase the `U-Boot` partition: `nand erase.part U-Boot`.
* load the U-Boot image from somewhere. easiest but slowest solution is Xmodem, ie. `loady` and then Ctrl-A `:` `exec !! sx uboot.bin` again.
* write the image to flash: `nand write $(loadaddr) U-Boot 0x00040000`. the size (last value) may need adjusting.
* switch SW1.1 to OFF and press reset.

U-Boot is now installed and should be loaded on boot. if it shows errors, initialize its environment (see below).

## Initializing U-Boot environment

this is primarily necessary if converting from Windows CE.

* reset the board and cancel autoboot, so it doesn't mess up the environment.
* do `mtdparts default` again. edit it if desired, but keep U-Boot where it is.
* `nand erase.part Env` to convert the environment to Linux ECC layout.
* `saveenv` to persist changes.
* `nand scrub.part bbt` to convert the bad block table to Linux ECC layout.
* the board now probably hangs and needs to be reset.
* convert other partitions; usually `nand erase.part Linux` and `nand erase.part ROMFS`.

## Installing Linux to flash

* load a kernel from somewhere (SATA, TFTP or NFS, or good old slow Xmodem) and boot it.
* in Linux, write the kernel to flash: `nandwrite -p /dev/mtd2 uImage`.
	* if necessary, erase partition first: `flash_eraseall /dev/mtd2`.

that's it. you can also configure U-Boot to load the kernel from somewhere else, eg. from `/boot/linux` on SATA. this is slower, but easier to recover from: just connect the SATA device to a Linux PC and change the file.
