## TI/National Bipolar PROM Programmer

![picture](https://github.com/eduardocasino/TI-Bipolar-PROM-programmer-shield/blob/main/hardware/images/TI-Bipolar-PROM-programmer-shield-actual.jpg?raw=true)
![picture](https://github.com/eduardocasino/TI-Bipolar-PROM-programmer-shield/blob/main/hardware/images/TI-Bipolar-PROM-programmer-shield-front.png?raw=true)
![picture](https://github.com/eduardocasino/TI-Bipolar-PROM-programmer-shield/blob/main/hardware/images/TI-Bipolar-PROM-programmer-shield-back.png?raw=true)

This is a poor man's TI/National Semiconductor TTL PROM programmer shield for the Arduino Mega, based on [this design of a PROM programmer](https://theoddys.com/acorn/replica_boards/tesla_prom_programmer_board/tesla_prom_programmer_board.html) for the ACORN by Chris Oddy. It is able to program the 74s471 (256x8 bit) I've just acquired for my [K-1013 FDC Replica](https://github.com/eduardocasino/k-1013-floppy-disk-controller-replica), as well as other 256x8 and 512x8 bit PROMs from TI and Tesla.

**IMPORTANT NOTE**: Programming these chips requiere a fair amount of power, so you need to power up the Arduino with an external adapter capable of about 15W. I'm using a 12V 1A I had lying around and seems to be stable.

[This post](https://eduardocasino.es/posts/TI-National-Bipolar-PROM-Programmer/) have some additional info about the design and how it works.

### Contents

The `hardware` folder contains the KiCad project with all the necessary to generate the gerber files to fabricate your own boards.

The `firmware` folder contains the skecth you need to upload into your Arduino.

The `software` folder contains the sources of a command line utility that interfaces with the programmer. Tested only with recent Ubuntu/Debians, WSL included.

### Licensing

This is a personal project that I am sharing in case it is of interest to any retrocomputing enthusiast and all the information in this repository is provided "as is", without warranty of any kind. I am not liable for any damages that may occur, whether it be to individuals, objects, computers, kittens or any other pets. It should also be noted that everything in this repository is a work in progress, may have not been thoroughly tested and may contain errors, therefore anyone who chooses to use it does so at their own risk.

Unless otherwise noted, all the content under the `hardware` folder is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/). Check the LICENSE.md file in each directory for exceptions or third party content licensing.

[![license](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-sa/4.0/)

The code in the `firmware` and `software` folders is under the MIT License. See the LICENSE.md files for details.

### Acknowlengements

[Chris Oddy](https://theoddys.com/acorn/replica_boards/tesla_prom_programmer_board/tesla_prom_programmer_board.html) for the original Tesla Programmer.