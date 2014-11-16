#include <stdint.h>
#include <stdarg.h>
#include <hw.h>

static void _put_char(uint8_t ch) {
	UART_TX(ch);
}

static void _put_flush(void) {
	UART_FLUSH();
}

static int putchar(int ch) {
	_put_char((uint8_t) ch);
	_put_flush();
	return 1;
}

static int puts(char *string) {
	int len = 0;
	while (*string) {
		putchar(*string++);
		len++;
	}
	return len;
}

static void _fmt_hex(char *buffer, uint32_t val) {
	buffer[8] = 0;
	for (int i = 7; i >= 0; i--) {
		uint32_t nybble = val & 15;
		if (nybble <= 9)
			buffer[i] = nybble + '0';
		else
			buffer[i] = nybble + 'a' - 10;
		val >>= 4;
	}
}

static void _fmt_dec(char *buffer, uint32_t val) {
	buffer[10] = 0;
	for (int i = 9; i >= 0; i--) {
		uint32_t digit = val % 10;
		buffer[i] = digit + '0';
		val /= 10;
	}
}

static char *_strip_lz(char *buffer) {
	while (buffer[1] && buffer[0] == '0')
		buffer++;
	return buffer;
}

static char *_replace_lz(char *buffer) {
	char *str = buffer;
	while (buffer[1] && buffer[0] == '0') {
		buffer[0] = ' ';
		buffer++;
	}
	return str;
}

static void _pad(int digits) {
	while (digits--)
		putchar(' ');
}

static int _put_str(char *str, int digits) {
	int len = 0;
	for (char *temp = str; *temp; temp++)
		len++;
	if (len > digits)
		str += len - digits;
	return puts(str);
}

static int printf(const char *format, ...) __attribute__((unused));
static int printf(const char *format, ...) {
	char buffer[12];
	va_list argp;
	va_start(argp, format);

	while (*format) {
		if (*format == '%') {
			format++;
			
			int lz = 0;
			if (*format == '0') {
				lz = 1;
				format++;
			}
			
			int trail = 0;
			if (*format == '-') {
				trail = 1;
				lz = 0;
				format++;
			}
			
			int digits = -1;
			while (*format >= '0' && *format <= '9') {
				if (digits < 0)
					digits = 0;
				digits *= 10;
				digits += *format - '0';
				format++;
			}
			
			char *str = buffer;
			switch (*format) {
			case '%':
				str = "%";
				break;
			case 's':
				str = va_arg(argp, char *);
				break;
			case 'd':
				_fmt_dec(buffer, va_arg(argp, int32_t));
				break;
			case 'x':
				_fmt_hex(buffer, va_arg(argp, uint32_t));
				break;
			default:
				buffer[0] = '%';
				buffer[1] = *format;
				buffer[2] = 0;
				break;
			}
			
			if (!lz) {
				if (trail || digits < 0)
					str = _strip_lz(str);
				else
					str = _replace_lz(str);
			}
			if (digits >= 0) {
				digits -= _put_str(str, digits);
				if (trail)
					_pad(digits);
			} else
				puts(str);
		} else if (*format == '\n')
			puts("\r\n");
		else
			putchar(*format);
		format++;
	}
	
	return 0;
}
