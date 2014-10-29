# Windows CE loader for U-Boot

## Usage

* obtain `03C00000_04000000__wince0.bin` and copy it to `wince0.bin`.
* optionally: obtain `complete_nand_layout.rar`. extract and copy `2_WinCe1.nb0` to `wince1.bin`.
* `make clean all` to build everything. `make clean wince0.uImage` to build just WinCE0.
* in U-Boot, load `wince0.uImage` (or `wince1.uImage`) using one of the available methods, eg. `loady` for Xmodem.
* boot the image with `bootm`. Windows CE should start up.

note that you don't need valid `1ECW`, `2ECW` or `3ECW` partitions or the info
page unless you actually intend to use the DSPs. however, WinCE tries to
be clever and creates an info page, so you should at least reserve the
space for it, and probably also create a semi-valid one.

not even WinCE1 needs the DSP images to run successfully. you will however
get the following (harmless) error message in the log:

```
Loading DSP Image...
+TmLoadDspImageDefault() Image='\Windows\phStbRootApp.mi'
TmLoadDspImageFromFile: '\Windows\phStbRootApp.mi'
DSP image file '\Windows\phStbRootApp.mi' not found!
Hynix, 8bit, 64MB
No InfoBlock found - creating one
DSP Image not found in flash!

TmLoadDspImageDefault() returns 0
!!!!!!!!!!!!!!!!!!!!!!!!!!
ERROR: DSP software download failed.
There was no DSP image found in the CE image or on flash!
Please update either the flash or include the DSP image into the CE image
!!!!!!!!!!!!!!!!!!!!!!!!!!
```

## Limitations

* it isn't autonomous. it needs U-Boot to load it into memory.

* with `loadaddr=0x82000000`, it is probably limited to ~31MB uncompressed image size.

  workaround: `setenv loadaddr 0x85000000`, for ~120MB uncompressed, 72MB compressed max.

* it cannot load the FlashReader. but then again, why should it, when it can boot WinCE directly.

* it cannot provide the DSP images. either build them into the WinCE image (`\Windows\phStbRootApp.mi`, probably), or have them in flash (uncompressed, and with WinCE ECC format) with a valid info page.
