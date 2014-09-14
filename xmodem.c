#include <stdint.h>
#include "hw.h"

#define XMODEM_TIMEOUT TIMER_TIMEOUT(1000)
#define ACK 0x06
#define NAK 0x15
#define EOT 0x04
#define STX 0x02
#define SOH 0x01
#define CRC 'C'

static uint8_t xmodem_ack, xmodem_block;

// ~32 bytes
export void xmodem_init(void) {
	xmodem_ack = NAK;
	xmodem_block = 0;
}

// ~380 bytes
export int16_t xmodem_get_block(uint8_t *base) {
	UART_TX(xmodem_ack);
	xmodem_ack = NAK;
	register uint32_t timeout_start = TIMER_START();
	
	// OUCH. but it does get rid of the "uninitialized" warning.
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
		// wrong block number
		return -1;

	if (checksum != 0)
		// incorrect checksum. retry.
		return -1;
	
	// correct block received; next one please
	xmodem_block = block;
	xmodem_ack = ACK;
	return 128;
}
