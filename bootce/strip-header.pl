#!/usr/bin/perl -w
use strict;

# check for header. if it doesn't look like a *ECW header, then there
# probably isn't any. the reverse isn't necessarily true...
my $buffer;
read STDIN, $buffer, 16;
my ($minsig, $maxsig) = unpack "VV", "0ECW9ECW";
my ($sig1, $size, $sig2, $entry) = unpack "VVVV", $buffer;
if ($sig1 == $sig2 && $sig1 >= $minsig && $sig1 <= $maxsig) {
	printf STDERR "stripping %s header: %s %08x %s %08x\n",
		substr($buffer, 0, 4),
		pack("V", $sig1), $size, pack("V", $sig2), $entry;
	read STDIN, $buffer, 512 - 16;
} else {
	print $buffer;
}

# copy the rest
while (!eof(STDIN)) {
	my $len = read STDIN, $buffer, 1024*1024;
	print STDOUT substr($buffer, 0, $len);
}
