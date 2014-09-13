#include <stdint.h>

#define _GPIO_BASE 0xBBF04000
#define _GPIO_TIMER (*((volatile uint32_t *) (_GPIO_BASE + 0x0C0)))
#define _UART_BASE 0xBBE4A000
#define _UART_CONTROL  (*((volatile uint32_t *) (_UART_BASE + 0x000)))
#define _UART_STATUS   (*((volatile uint32_t *) (_UART_BASE + 0x028)))
#define _RESET_BASE 0xBBE60000
#define _RESET_CONTROL (*((volatile uint32_t *) (_RESET_BASE + 0x000)))

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
	{ \
		_UART_STATUS = ch; \
		while (((_UART_STATUS >> 16) & 31) != 0); \
	}
#else
static void UART_TX(uint8_t ch) __attribute__((noinline));
static void UART_TX(uint8_t ch) {
	_UART_STATUS = ch;
	while (((_UART_STATUS >> 16) & 31) != 0);
}
#endif

/**
 * receive a single character from the serial line. if the given timeout
 * is exceeded, this macro performs a "return -1" from the calling
 * function.
 * 
 * @param start start point of the timeout
 * @param timeout timeout in timer ticks
 * @return the character that was read
 */
#define UART_RX_TIMEOUT(start, timeout) \
	({ \
		do {\
			if (TIMER_TIMED_OUT(start, timeout)) \
				return -1; \
		} while (((_UART_STATUS >> 8) & 31) == 0); \
		register uint8_t ch = _UART_STATUS; \
		_UART_CONTROL = (1 << 16) | (1 << 24); \
		ch; \
	})
