#include <stdint.h>
#include "hw.h"

#define XMODEM_TIMEOUT TIMER_TIMEOUT(1000)
#define XMODEM_LONG_TIMEOUT TIMER_TIMEOUT(10000)
#define ACK 0x06
#define NAK 0x15
#define EOT 0x04
#define STX 0x02
#define SOH 0x01
#define CAN 0x18
#define CRC 'C'

static uint8_t xmodem_ack uninitialized;
static uint8_t xmodem_block uninitialized;
static uint32_t xmodem_timeout uninitialized;

export void xmodem_init(void) {
	xmodem_ack = NAK;
	xmodem_block = 0;
	// no need to initialize xmodem_timeout here. it's ignored while in
	// the header, and it takes a valid block to get out of the header.
	// xmodem_timeout IS initialized (reset actually) when a valid block
	// is received.
}

export int16_t xmodem_get_block(uint8_t *base, int expect_header) {
	// if there was no useable block for 10s, give up and try again.
	// don't do that while waiting for the header, though.
	if (!expect_header && TIMER_TIMED_OUT(xmodem_timeout, XMODEM_LONG_TIMEOUT))
		return 0;

	// no need to flush here. the RX will wait until the sender is
	// actually sending something, which it won't do before having
	// received the ACK. there won't be an overflow either because
	// ACKs are sent only every 132 bytes, and NAKs only once a second.
	UART_TX(xmodem_ack);
	xmodem_ack = NAK;
	register uint32_t timeout_start = TIMER_START();
	
	// self-initialization. OUCH. but it does get rid of the
	// "uninitialized variable" warning.
	uint8_t block = block, invblock = invblock;
	uint8_t checksum = 0;
	for (uint8_t pos = 0; pos <= 128 + 3; pos++) {
		uint8_t ch = UART_RX_TIMEOUT(timeout_start, XMODEM_TIMEOUT);
		
		switch (pos) {
		case 0:
			if (ch == EOT) {
				// there'll be no more blocks; signal EOF. try to ack the
				// packet here, but we only have a single attempt at that.
				// there is no possibility to retry if that ACK is lost:
				// if all goes well, the received image will be running
				// by the time any retried EOTs arrive!
				// we don't flush the UART. if the image immediately
				// reconfigures it to something incompatible, the ACK
				// will get lost and the Xmodem client will keep retrying
				// for a while. however there is some disruption to be
				// expected in that case anyway, because the client needs
				// to switch his UART config as well. flushing needs a
				// considerable amount of space, so there is no point
				// adding it just for an obscure boundary case.
				UART_TX(ACK);
				return 0;
			}
			if (ch != SOH)
				// wrong header, or 1k blocks
				return -1;
			break;
		case 1:
			block = ch;
			break;
		case 2:
			invblock = ch;
			break;
		default:
			base[pos - 3] = ch;
			checksum += ch;
			break;
		case 128 + 3:
			checksum -= ch;
			break;
		}
	}
	
	if (block != (uint8_t) ~invblock)
		// block number corrupted
		return -1;
	if (block == xmodem_block) {
		// previous block was re-sent. acknowledge to client, but we'll
		// need to retry for the current block.
		xmodem_ack = ACK;
		return -1;
	}
	if (block != (uint8_t) (xmodem_block + 1))
		// wrong block number.
		return -1;

	if (checksum != 0)
		// incorrect checksum. retry.
		return -1;
	
	if (expect_header && !is_header(base)) {
		// expecting a header block, but didn't get one. cancel
		// transmission; retrying won't help for an invalid image.
		// minimum of 2 CANs required to pevent line noise from aborting
		// transfers.
		UART_TX(CAN);
		UART_TX(CAN);
		return 0;
	}

	// correct block received; next one please
	xmodem_block = block;
	xmodem_ack = ACK;
	xmodem_timeout = TIMER_START();
	return 128;
}
