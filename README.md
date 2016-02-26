# gpiotime
CHIP program to do a timing test of a gpio I/O line.

## License

Copyright 2016 Steven Ford http://geeky-boy.com and licensed
"public domain" style under
[CC0](http://creativecommons.org/publicdomain/zero/1.0/): 
![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png "CC0")

To the extent possible under law, the contributors to this project have
waived all copyright and related or neighboring rights to this work.
In other words, you can use this code for any purpose without any
restrictions.  This work is published from: United States.  The project home
is https://github.com/fordsfords/gpiotime/tree/gh-pages

To contact me, Steve Ford, project owner, you can find my email address
at http://geeky-boy.com.  Can't see it?  Keep looking.

## Introduction

The [CHIP](http://getchip.com/) single-board computer has two sets of general-purpose I/O lines: 8 "XIO" lines and 8 "CSI" lines.  The "CSI" lines are directly handled by the CPU chip; the "XIO" lines are handled by a separate chip which communicates with the CPU over an I2C serial bus.  This project contains a C program which measures the speed that the I/O lines can be manipulated in software using the Sysfs driver ("/sys/class/gpio").

The time measurements show noticable variance from run to run.  Linux is generally not very deterministic with timing due to the unpredictable nature of interrupts, cron jobs, dynamic power adjustment, etc.  If very accurate timing is required, a more-simple microcontroller such as an Arduino.

### XIO Measurements

* input: 127 microseconds (lseek/read) - sample rate
* output: 122 microseconds (write) - 4.1 KHz square wave

### CSI Measurements

* input: 6.7 microseconds (lseek/read) - sample rate of 149 KHz
* output: 2.8 microseconds (write) - 178 KHz

You can find gpiotime at:

* User documentation (this README): https://github.com/fordsfords/gpiotime/tree/gh-pages

Note: the "gh-pages" branch is considered to be the current stable release.  The "master" branch is the development cutting edge.

## Quick Start

1. If you haven't set up your CHIP to be able to compile C programs, perform [these instructions](http://wiki.geeky-boy.com/w/index.php?title=CHIP_do_once) up to and including installing gcc.

2. Get the files onto CHIP:

        mkdir gpiotime
        cd gpiotime
        wget http://fordsfords.github.io/gpiotime/gpiotime.c
        wget http://fordsfords.github.io/gpiotime/bld.sh

3. Build the package:

        ./bld.sh

4. Test the package:

        sudo ./gpiotime 132  # test CSID0
        sudo ./gpiotime 408  # test XIO-P0

## Release Notes

* 1.0 (26-Feb-2016)

    Initial release.
