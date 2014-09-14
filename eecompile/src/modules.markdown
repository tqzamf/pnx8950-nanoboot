| address | size | module id | module name | module description | PLL/DDS | clock control |
|---|---|---|---|---|---|---|
| — | — | — | — | MIPS CPU | `000` | `100` |
| — | — | — | PMAN | Pipelined Memory Access Network / memory bus; only one | — | — |
| — | — | — | DCSN | Device, Control and Status Network / control bus; one for MIPS, one for TM32 | — | — |
| `000000-03dfff` | 248k | — | — | — | — | — |
| `03e000-03efff` | 4k | `1106` v1.0 | `mpic` | MIPS PIC (big) | ??? | ??? |
| `03f000-03ffff` | 4k | `1106` v1.0 | — | (small PIC) | ??? | ??? |
| `040000-040fff` | 4k | `a051` v0.0 | `pci-xio` | PCI / XIO | ??? | ??? |
| `041000-041fff` | 4k | `0104` v1.0 | `mips-jtag` | MIPS EJTAG | ??? | ??? |
| `042000-042fff` | 4k | — | — | — | — | — |
| `043000-043fff` | 4k | `0106` v2.1 | `sc1` | Smartcard 1 | ??? | `728` |
| `044000-044fff` | 4k | `0106` v2.1 | `sc2` | Smartcard 2 | ??? | `72c` |
| `045000-045fff` | 4k | `3203` v2.15 | `i2c-hp1` | I²C HP 1 (i2c-0) | ??? | `714` |
| `046000-046fff` | 4k | `3203` v2.15 | `i2c-hp2` | I²C HP 2 (i2c-1) | ??? | `714` |
| `047000-047fff` | 4k | `0108` v2.0 | `clock` | clock module | ??? | ??? |
| `048000-048fff` | 4k | `0109` v1.0 | `ohci` | USB 1.1 OHCI | ??? | `710` |
| `049000-049fff` | 4k | — | — | — | — | — |
| `04a000-04afff` | 4k | `0107` v1.0 | `uart1` | UART 1 | ??? | `71c` |
| `04b000-04bfff` | 4k | `0107` v1.0 | `uart2` | UART 2 | ??? | `720` |
| `04c000-04cfff` | 4k | `0105` v0.3 | `i2c-fast2` | I²C Fast 2 (i2c-3) | ??? | `718` |
| `04d000-04dfff` | 4k | `0128` v4.1 | `global2` | Global2 | ??? | ??? |
| `04e000-04efff` | 4k | `a047` v0.0 | `mips-dcsnc` | MIPS DCSN controller | ??? | ??? |
| `04f000-05ffff` | 68k | `0117` v2.0 | `2dde` | 2D drawing engine | ??? | ??? |
| `060000-060fff` | 4k | `0123` v0.0 | `reset` | reset module | ??? | ??? |
| `061000-061fff` | 4k | `0127` v0.0 | `tm32-jtag1` | TM32 JTAG | ??? | ??? |
| `062000-062fff` | 4k | `0127` v0.0 | `tm32-jtag2` | TM32 JTAG | ??? | ??? |
| `063000-063fff` | 4k | `0126` v1.0 | `global1` | Global1 | ??? | ??? |
| `064000-064fff` | 4k | `1010` v0.0 | `arbiter` | PMAN Arbiter | ??? | ??? |
| `065000-065fff` | 4k | `2031` v1.0 | `dram` | main memory interface (DRAM controller) | `014` | ??? |
| `066000-066fff` | 4k | `a056` v0.0 | `security` | PMAN security | ??? | ??? |
| `067000-067fff` | 4k | `a066` v1.0 | `monitor` | PMAN monitor (PNX8550 compatible) | ??? | ??? |
| `068000-068fff` | 4k | `a058` v0.0 | `mips-dcsns` | (MIPS DCSN security) | ??? | ??? |
| `069000-069fff` | 4k | `0105` v0.3 | `i2c-fast1` | I²C Fast 1 (i2c-2) | ??? | `718` |
| `06a000-0fffff` | 600k | — | — | — | — | — |
| `100000-101fff` | 8k | — | — | — | — | — |
| `102000-102fff` | 4k | `1106` v1.0 | — | (big PIC) | ??? | ??? |
| `103000-103fff` | 4k | `a048` v0.0 | `tm32-dcsnc` | TM32 DCSN controller | ??? | ??? |
| `104000-104fff` | 4k | `010f` v1.0 | `gpio` | GPIO | ??? | ??? |
| `105000-105fff` | 4k | `0100` v2.0 | `vmpg` | MPEG video decoder | ??? | `400` |
| `106000-106fff` | 4k | `011a` v3.0 | `vip1` | video input processor 1 | ??? | `600` |
| `107000-107fff` | 4k | `011a` v3.0 | `vip2` | video input processor 2 | ??? | `604` |
| `108000-108fff` | 4k | `014d` v0.0 | `vld` | Variable Length Decoder (MPEG slicer) | ??? | `404` |
| `109000-109fff` | 4k | `0121` v0.1 | `spdo` | SPDIF out | `03c` | `b00` |
| `10a000-10afff` | 4k | `0110` v0.1 | `spdi1` | SPDIF in 1 | `03c`?? | ??? |
| `10b000-10bfff` | 4k | `a005` v1.0 | `dvd-css` | DVD CSS (yay!) | ??? | `b3c` |
| `10c000-10cfff` | 4k | `0119` v2.0 | `mbs1` | memory-based scaler 1 | ??? | `500` |
| `10d000-10dfff` | 4k | `a02a` v0.0 | `mbs3` | memory-based scaler 3 (called "QNTR" in IRQ table; looks like an MBS) | ??? | `504` |
| `10e000-10efff` | 4k | `a014` v1.0 | `qvcp1` | AICP / QVCP 1 (framebuffer) | `008`, `01c` | `a00`, `a04`, `a18` |
| `10f000-10ffff` | 4k | `a019` v1.0 | `qvcp2` | AICP / QVCP 2 | `00c`, `020` | `a08`, `a0c`, `a1c` |
| `110000-110fff` | 4k | `0120` v0.1 | `ao1` | audio out 1 | `02c` | `b08`, `b1c` |
| `111000-111fff` | 4k | `010d` v1.0 | `ai1` | audio in 1 | `028`? | `b04`?, `b18`? |
| `112000-112fff` | 4k | `0120` v0.1 | `ao2` | audio out 2 | `034`? | `b10`?, `b24`? |
| `113000-113fff` | 4k | `010d` v1.0 | `ai2` | audio in 2 | `030`? | `b0c`?, `b20`? |
| `114000-114fff` | 4k | `a006` v1.0 | `edma` | EDMA (DMA Controller) | ??? | ??? |
| `115000-115fff` | 4k | `a059` v0.0 | `tm32-dcsns` | (TM32 DCSN security) | ??? | ??? |
| `116000-116fff` | 4k | `0125` v3.0 | `tsdma` | TSDMA | `040`? | `b34` |
| `117000-117fff` | 4k | `a04f` v0.0 | `qvcp2-dac` | QVCP2 DAC, PNX8510 compatible | (`00c`, `020`) | (`a08`) |
| `118000-11ffff` | 32k | `010e` v8.1 | `msp1` | MPEG system processor 1 | ??? | `800` |
| `120000-127fff` | 32k | `010e` v8.1 | `msp2` | MPEG system processor 2 | ??? | `804` |
| `128000-13ffff` | 96k | — | — | — | — | — |
| `140000-141fff` | 8k | `2b80` v4.0 | `tm32-1` | TM32 CPU 1 | `004` | `204` |
| `142000-15ffff` | 120k | — | — | — | — | — |
| `160000-161fff` | 8k | `2b80` v4.0 | `tm32-2` | TM32 CPU 2 | `048` | `208` |
| `162000-16ffff` | 56k | — | — | — | — | — |
| `170000-170fff` | 4k | `1106` v1.0 | — | (big PIC) | ??? | ??? |
| `171000-171fff` | 4k | `1106` v1.0 | — | (small PIC) | ??? | ??? |
| `172000-172fff` | 4k | `1106` v1.0 | — | (small PIC) | ??? | ??? |
| `173000-173fff` | 4k | `a06a` v0.0 | — | ??? | ??? | ??? |
| `174000-174fff` | 4k | `0119` v3.0 | `mbs2` | memory-based scaler 2 | ??? | `750` |
| `175000-175fff` | 4k | `0110` v0.1 | `spdi2` | SPDIF in 2 | `03c`?? | ??? |
| `176000-17efff` | 36k | — | — | — | — | — |
| `17f000-17ffff` | 4k | `a04a` v0.0 | `tunnel` | CTL12 North Tunnel (UM10113, p522, p528; probably unused) | ??? | ??? |
| `180000-1fffff` | 512k | — | — | (probably used for PNX2015 extension MMIO) | — | — |