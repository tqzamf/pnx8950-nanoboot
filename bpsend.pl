#!/usr/bin/perl -p
use Time::HiRes qw(usleep);

## usage:
# screen /dev/ttyUSBx 115200
# Ctrl-A :
# exec .!. ./bpsend.pl somefile.bp

# implicit main loop. noting to do except delay the lines we're sending
# so we don't overwhelm the bus pirate. this way the reprogramming is
# slower, but at least it works.
# a 50ms delay seems to be enough for the bus pirate to keep up with our
# commands.
usleep 50000;
print STDERR "\r" . $n++;
