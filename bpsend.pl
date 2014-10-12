#!/usr/bin/perl -wp
use strict;
use Time::HiRes qw(usleep);

## usage:
# screen /dev/ttyUSBx 115200
# Ctrl-A :
# exec .!. ./bpsend.pl somefile.bp

# implicit main loop. noting to do except delay the lines we're sending
# so we don't overwhelm the bus pirate. this way the reprogramming is
# slower, but at least it works.
# a 10ms delay is enough because the EEPROM needs at most 5ms to program
# a page.
usleep 10000;
