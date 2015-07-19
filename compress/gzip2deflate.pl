#!/usr/bin/perl -w
use strict;
use String::CRC32;

my $buffer;
read STDIN, $buffer, 10;
my ($id1, $id2, $cm, $flg, $mtime, $xfl, $os) = unpack "ccccVcc", $buffer;
die "not a GZIP file" unless $id1 == 31 and $id2 == -117;
die "not DEFLATE compressed" unless $cm == 8;

# FTEXT ignored
my $fhcrc    = $flg & 2;
my $fextra   = $flg & 4;
my $fname    = $flg & 8;
my $fcomment = $flg & 16;
die "unsupported flags $flg" if $flg & 0xe0;

if ($fextra) {
	# skip extra data
	read STDIN, $buffer, 2;
	my $xlen = unpack "v", $buffer;
	read STDIN, $buffer, $xlen;
}

sub readz() {
	my $name = "";
	read STDIN, $buffer, 1;
	while (ord $buffer != 0) {
		$name .= $buffer;
		read STDIN, $buffer, 1;
	}
	return $name;
}
printf STDERR "filename: %s\n", readz() if $fname;
printf STDERR "comment:  %s\n", readz() if $fcomment;
printf STDERR "modified: %s\n", scalar localtime $mtime;
if ($fhcrc) {
	read STDIN, $buffer, 2;
	my $hcrc = unpack "v", $buffer;
	printf STDERR "header crc: %04x, probably correct\n", $hcrc;
}

# copy compressed data, including CRC32 and uncompressed size
my $crc = 0;
while (!eof(STDIN)) {
	my $len = read STDIN, $buffer, 1024*1024;
	$buffer = substr($buffer, 0, $len);
	print STDOUT $buffer;
	$crc = crc32 $buffer, $crc;
}
# append CRC32 of source data to detect corrupted input before
# attempting to decompress it.
print STDOUT pack "V", $crc;
