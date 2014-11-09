#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define RESET_CONTROL *((volatile uint32_t *) 0xbbe60000)
#define ASSERT_MIPS_SYS_RESET 5

int main(void) {
	puts("\r\n*** software reset ***\r\n");
	RESET_CONTROL = ASSERT_MIPS_SYS_RESET;
	while (1);
}
