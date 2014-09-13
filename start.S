	.set reorder
	.globl _start
	.type _start, @function
	.section .init
_start:
	li $sp, 0x84010000
	# the rest can be done in C. fall through to function _init.
