## Installing nanoboot on the Pollinux board

* build nanoboot (obviously). this is just `make clean all`.
* solder pin headers onto U11. pins 5 .. 8 should be enough; GND (pin 5) requires a high-power soldering iron.
* connect bus pirate. pin 4 is GND, 5 is SDA, 6 is SCL. there's an alternative GND next to the SCARTs.
* connect to bus pirate: `screen /dev/ttyUSBx 115200`.
* switch bus pirate to I²C: press `m` to select mode, then follow menu. speed is irrelevant; 400kHz works fine.
* upload nanoboot: press Ctrl-A `:` then type `exec .!. ./bpsend.pl nanoboot.bp`. nonzero retries are fine if everything else works.
* use a jumper to short U11 pins 7/8. this prevents accidental reprogramming, and keeps the board from wearing out the first EEPROM page whenever it boots.
* press the reset button

the board is now unbrickable (but not indestructible). the steps above can be repeated easily if the EEPROM does get corrupted. if any further step in the boot chain fails, switch SW1.1 to ON and upload something (probably U-Boot) over Xmodem.

if the bus pirate shows errors during reprogramming, then probably the boot module hangs and thus the I²C bus is blocked. just hold the reset button during the entire programming process and things should work. if holding reset powers down the board, power it up while holding reset and program it while keeping reset pressed.

## Installing U-Boot using nanoboot

* connect to nanoboot: `screen /dev/ttyUSBx 38400`.
* switch SW1.1 to ON and press reset. nanoboot should quickly respond with an `!` prompt.
* upload U-Boot: press Ctrl-A `:` then type `exec !! sx u-boot.ecw`. you'll want to use the one with Linux ECC and proper length in the header.
* cancel autoboot. if you miss it and it fails to boot, that's fine but don't do a `saveenv` without resetting again.
* do `mtdparts default` if necessary. it *is* necessary when converting from Windows CE.
* erase the `U-Boot` partition: `nand erase.part U-Boot`.
* load the U-Boot image from somewhere. easiest but slowest solution is Xmodem, ie. `loady` and then Ctrl-A `:` `exec !! sx u-boot.ecw` again.
* write the image to flash: `nand write $(loadaddr) U-Boot 0x00040000`. the size (last value) may need adjusting.
* switch SW1.1 to OFF and press reset.

U-Boot is now installed and should be loaded on boot. if it shows errors, initialize its environment (see below).

for extra failsafe-ness, concatenate two U-Boot `.ecw` images and write the result to the U-Boot partition. should the first copy get corrupted, nanoboot will just load the second copy instead.

## Initializing U-Boot environment

this is primarily necessary if converting from Windows CE.

* reset the board and cancel autoboot, so it doesn't mess up the environment.
* do `mtdparts default` again. edit it if desired, but keep `U-Boot`, `Env` and `bbt` where they are.
* `nand erase.part Env` to convert the environment to Linux ECC layout.
* `saveenv` to persist changes.
* `nand scrub.part bbt` to convert the bad block table to Linux ECC layout.
* the board now probably hangs and needs to be reset.
* convert other partitions; usually `nand erase.part Linux` and `nand erase.part ROMFS`.

## Installing Linux to flash

* load a kernel from somewhere (SATA, TFTP or NFS, or good old slow Xmodem) and boot it.
	* loading the kernel over Xmodem takes ~30 minutes; use something faster.
* in Linux, find the correct flash partition: `cat /proc/mtd`.
* in Linux, write the kernel to flash: `nandwrite -p /dev/mtdX uImage`.
	* if necessary, erase partition first: `flash_eraseall /dev/mtdX`.

that's it. you can also configure U-Boot to load the kernel from somewhere else, eg. from `/boot/linux` on SATA. this is slower, but makes it easier to recover from trouble: just connect the SATA device to a Linux PC and change the file.

## Recommended Flash partition tables

### Without any Windows CE

```
setenv mtdparts mtdparts=nxp-0:528k(U-Boot)ro,32k(Env),32k(bbt),4512k(Linux),59M(ROMFS),16k(info)
```

keeping the info partition is strongly recommended because Windows CE will blindly overwrite that page. it doesn't have to contain anything valid; it's just there to prevent Windows from corrupting data if Windows CE is ever booted.

### With WinCE0 (recovery) support

```
setenv mtdparts mtdparts=nxp-0:16k(FlashReader)ro,512k(U-Boot)ro,32k(Env),32k(bbt),4512k(Linux),56M(ROMFS),3M(WinCE0),16k(info)
```

note that while the WinCE0 image is ~4MB, in U-Boot compressed format it is actually just slightly larger than 2MB.

### With WinCE1 (Desktop) support

```
setenv mtdparts mtdparts=nxp-0:16k(FlashReader)ro,512k(U-Boot)ro,32k(Env),32k(bbt),4512k(Linux),45M(ROMFS),14M(WinCE1),16k(info)
```

note that while the WinCE1 image is ~26MB, it will be compressed down to <13MB when loading it from the U-Boot image.

### With both Windows CE images

```
setenv mtdparts mtdparts=nxp-0:16k(FlashReader)ro,512k(U-Boot)ro,32k(Env),32k(bbt),4512k(Linux),42M(ROMFS),14M(WinCE1),3M(WinCE0),16k(info)
```

you get the idea. just subtract whatever additional size you need from the `ROMFS` partition.
