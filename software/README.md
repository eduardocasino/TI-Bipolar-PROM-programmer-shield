# prom - National/TI Bipolar PROM programmer utility

Copyright (C) 2024 Eduardo Casino (https://github.com/eduardocasino) under the MIT License. See the [LICENSE.md](https://github.com/eduardocasino/TI-Bipolar-PROM-programmer-shield/blob/main/software/LICENSE.md) file for details.

## Build instructions

### Notice

I've only built it on a Debian Linux and on WSL with Ubuntu. I can't give support for other systems, sorry.

### Prerequisites

Any modern Linux distribution (tested on `Debian 12.6` and `Ubuntu 24.04`) with the GCC toolchain, `binutils` and `make`.

### Build

Just go to the `software` directory and build it with `make`:
```console
$ cd software
$ make
```

## Usage

### General

On WSL, first make sure that you [attach the USB device to the Linux susbsystem](https://learn.microsoft.com/en-us/windows/wsl/connect-usb#attach-a-usb-device).

```text
Usage: prom [-h]
       prom DEVICE [-c NUM] -b
       prom DEVICE [-c NUM] -r [ADDRESS | -o FILE [-f FORMAT]]
       prom DEVICE [-c NUM] {-w|-s|-v} {ADDRESS -d BYTE | -i FILE [-f FORMAT]}

Arguments:
    DEVICE                  Serial device.

Options:
    -h[elp]                 Show this help message and exit.
    -c[hip]     NUM         Chip to program: 0 == 74s471 (default), 1 == 74s472.
    -b[lank]                Do a whole chip blank test.
    -r[ead]     [ADDRESS]   Read chip. If ADDRESS is specified, read just that
                            byte. If not, do a hexdump of the chip contents to
                            the screen or, if an output file name is provided,
                            dump the contents with the format specified by the
                            -format option.
    -w[rite]    [ADDRESS]   Program chip. If ADDRESS is provided, program just
                            that location with data specified with the -data
                            option. If not, an input filename must be specified.
    -s[imulate] [ADDRESS]   Program simulation. Will success of fail as the write
                            command, but without actually burning the chip.
    -v[erify]   [ADDRESS]   Verify data. Same options as for -write|-simulate.
    -d[ata]     BYTE        Byte to program, simulate or verify.
    -i[nput]    FILE        File to read the data from.
    -o[utput]   FILE        File to save the data to.
    -f[ormat]   {bin,ihex}  File format. Defaults to bin.

Note: Long and short options, with single or dual '-' are supported
```

### Blank test command

Makes a quick blank test of the whole chip.

Example:

```bash
$ prom /dev/ttyUSB0 -b
Connected to programmer, firmware V01.00.00.
Chip is not blank. Found non-zero data at address 0x0.
```

### Read command

`prom DEVICE -r`, without any more arguments, does a hexadecimal dump to the screen:

```bash
$ ./prom /dev/ttyUSB0 -r
Connected to programmer, firmware V01.00.00.
Reading
.........................................................................
.........................................................................
.........................................................................
.....................................
Success.
000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
010  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
020  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
030  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
040  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
050  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
060  01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
090  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
0A0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
0B0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
0C0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
0D0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
0E0  00 00 00 00 00 00 00 00  ff ff ff ff ff ff ff ff  |................|
0F0  ff ff ff ff ff ff ff ff  ff ff 1c 1c 00 ff 1f 1c  |................|
100
```

If an address is provided, the content of that cell is printed to the screen:

```bash
$ ./prom /dev/ttyUSB0 -r 0xec
Connected to programmer, firmware V01.00.00.
Reading
.
Success.
0EC  ff                                                |.               |
```

Finally, it is also possible to make a dump in binary or Intel HEX format with the `-o FILE` and `-f {bin|ihex}` options, If not specified, `bin` is the default:

```bash
$ ./prom /dev/ttyUSB0 -r -f ihex -o test.hex
Connected to programmer, firmware V01.00.00.
Reading
.........................................................................
.........................................................................
.........................................................................
.....................................
Success.

$ cat test.hex
:20000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
:20002000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE0
:20004000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
:2000600001000000000000000000000000000000000000000000000000000000000000007F
:20008000000000000000000000000000000000000000000000000000000000000000000060
:2000A000000000000000000000000000000000000000000000000000000000000000000040
:2000C000000000000000000000000000000000000000000000000000000000000000000020
:2000E0000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1C1C00FF1F1CA0
:00000001FF
```

### Write and Simulate Write commands

`prom DEVICE -w ADDRESS -d DATA` programs the cell at address `ADDRESS` with the `DATA` byte:

```bash
$ ./prom /dev/ttyUSB0 -w 0x60 -d 1
Connected to programmer, firmware V01.00.00.
WARNING: Programming is irreversible. Are you sure? Type YES to confirm
YES
Writing
.
Success.
```

`prom DEVICE -w -f {bin|ihex} -i FILE` programs the chip with the contents of the `FILE` file. Working with `bin` and `ihex` have different implications:

* If a binary file is specified, the chip is programmed starting at address `0x000` up to the size of the file or the chip, whichever is smaller.
* With an Intel HEX file, only the addresses specified in the file are programmed.

```bash
$ ./prom /dev/ttyUSB0 -w -i test.bin
Connected to programmer, firmware V01.00.00.
WARNING: Programming is irreversible. Are you sure? Type YES to confirm
YES
Writing
.........................................................................
.......................
Success.
```

Command `-s` works exactly the same as `-w`, but without burning the chip. It is strongly suggested to execute first a simulation as it helps to catch errors beforehand:

```bash
$ ./prom /dev/ttyUSB0 -s -f bin -i test2.bin
Connected to programmer, firmware V01.00.00.
Performing a write simulation
..........................................................
Error writing (simulated) to prom address 0x039: Read == 0xff, expected == 0x03
```

### Verify command

The verify command just checks the contents of the PROM against the data provided. It has the same options as the Write and Simulate write commands:

```bash
$ ./prom /dev/ttyUSB0 -v -f bin -i test2.bin
Connected to programmer, firmware V01.00.00.
Verifying
..........................................................
Error verifying prom address 0x039: Read == 0xff, expected == 0x03

$ ./prom /dev/ttyUSB0 -v -f ihex -i test.ihex
Connected to programmer, firmware V01.00.00.
Verifying
.........................................................................
.........................................................................
.........................................................................
.....................................
Success.
```
