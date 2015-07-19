#include <stdint.h>

#define _GPIO_BASE 0xBBF04000
#define _GPIO_TIMER (*((volatile uint32_t *) (_GPIO_BASE + 0x0C0)))
#define _GPIO_LEDS2 (*((volatile uint32_t *) (_GPIO_BASE + 0x018)))
#define _GPIO_LEDS1 (*((volatile uint32_t *) (_GPIO_BASE + 0x01C)))
#define _GPIO_SW11  (*((volatile uint32_t *) (_GPIO_BASE + 0x01C)))
#define _UART_BASE 0xBBE4A000
#define _UART_CONTROL  (*((volatile uint32_t *) (_UART_BASE + 0x000)))
#define _UART_STATUS   (*((volatile uint32_t *) (_UART_BASE + 0x028)))
#define _RESET_BASE 0xBBE60000
#define _RESET_CONTROL (*((volatile uint32_t *) (_RESET_BASE + 0x000)))
#define _XIO_BASE 0xBBE40000
#define _XIO_FLASH_CONTROL (*((volatile uint32_t *) (_XIO_BASE + 0x830)))
#define _XIO_FLASH_COMMAND (*((volatile uint16_t *) (_XIO_BASE + 0x830)))
#define _XIO_FLASH_OPER    (*((volatile uint16_t *) (_XIO_BASE + 0x832)))
#define FLASH_BASE ((volatile uint32_t *) 0xB0000000)
#define FLASH_END  ((volatile uint32_t *) 0xB3FFFFFF)

/****** timeout timer using GPIO timestamp timer ******/
/**
 * get value marking timer starting point.
 * 
 * @return an uint32_t starting point
 */
#define TIMER_START() (_GPIO_TIMER)

/**
 * convert timeout from milliseconds to ticks. this is rounded to a value
 * that can be loaded with a single lui instruction in TIMER_TIMED_OUT.
 * 
 * @param msec timeout in milliseconds
 * @return timeout in timer ticks
 */
#define TIMER_TIMEOUT(msec) \
	(((27000 * (msec)) / 8 + 0x4000) & 0xffff8000)

/**
 * check if the timer has exceeded the timeout since the time it has been
 * started.
 * 
 * @param start timer starting point, as returned by TIMER_START()
 * @param timeout timeout in ticks, as returned by TIMER_TIMEOUT()
 * @return true if the timer has timed out; zero otherwise
 */
#define TIMER_TIMED_OUT(start, timeout) \
	(((_GPIO_TIMER - (start)) << 1) > ((timeout) << 1))


/****** serial interface using the first UART ******/
/**
 * transmit a single character over the serial line.
 * 
 * @param ch the character to send
 */
#if 1
#define UART_TX(ch) \
	do { \
		_UART_STATUS = ch; \
	} while (0)
#else
static void UART_TX(uint8_t ch) __attribute__((noinline));
static void UART_TX(uint8_t ch) {
	_UART_STATUS = ch;
}
#endif

/**
 * waits until the UART output buffer is empty.
 */
#if 1
#define UART_FLUSH(ch) \
	while (((_UART_STATUS >> 16) & 31) != 0)
#else
static void UART_FLUSH(uint8_t ch) __attribute__((noinline));
static void UART_FLUSH(uint8_t ch) {
	while (((_UART_STATUS >> 16) & 31) != 0);
}
#endif

/**
 * receive a single character from the serial line. if the given timeout
 * is exceeded, this macro performs a "return" from the calling function!
 * 
 * @param start start point of the timeout
 * @param timeout timeout in timer ticks
 * @param retval value to return out of the calling function on timeout
 * @return the character that was read
 */
#define UART_RX_TIMEOUT(start, timeout, retval) \
	({ \
		do { \
			if (TIMER_TIMED_OUT(start, timeout)) \
				return retval; \
		} while (((_UART_STATUS >> 8) & 31) == 0); \
		register uint8_t ch = _UART_STATUS; \
		_UART_CONTROL = (1 << 16) | (1 << 24); \
		ch; \
	})


/****** LED control / switch status using GPIO ******/
/**
 * controls the state of the CPU status LEDs.
 * 
 * @param red if 1, the red LED is turned on; else it is turned off
 * @param green if 1, the greeen LED is turned on; else it is turned off
 */
#define LEDS_SET(red, green) \
	do { \
		_GPIO_LEDS1 = 0x11000000 | (green ? 0 : 0x1000) | (red ? 0 : 0x100); \
	} while (0)

/**
 * reads the state of DIP switch SW1, position 1 (left switch).
 *
 * @return 0 if the switch is off, 1 if it is on
 */
#define SW11_STATUS() \
	(_GPIO_SW11 & 0x10)


/****** XIO NAND flash interface ******/
/**
 * selects OOB read, and configures the XIO interface to perform full
 * reads with 1 command and 4 address cycles.
 */
#define XIO_SELECT_OOB() \
	do { \
		_XIO_FLASH_CONTROL = 0x00370050; \
	} while (0)

/**
 * selects main memory read of the given half-page, and configures the
 * XIO interface to perform full reads with 1 command and 4 address
 * cycles.
 *
 * @param x address bit A8 (0x100)
 */
#define XIO_SELECT_BLOCK(x) \
	do { \
		_XIO_FLASH_CONTROL = 0x00370000 + ((x) & 1); \
	} while (0)

/**
 * reconfigures the XIO interface to perform short read cycles without
 * any command or address cycles. the flash will respond with sequential
 * bytes until the end of the current page.
 */
 #define XIO_SELECT_SEQUENTIAL() \
	do { \
		_XIO_FLASH_CONTROL = 0x00300000; \
	} while (0)
