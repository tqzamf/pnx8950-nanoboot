#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define BOOT_BUS "/dev/i2c-2"
#define BOOT_ADDR 0x50

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: ./install nanoboot.img\n");
		return -1;
	}
	
	char *file = argv[1];
	int fd = open(file, O_RDONLY);
	if (fd < 0)
		err(1, "cannot open %s", file);
	int bus = open(BOOT_BUS, O_RDWR);
	if (bus < 0)
		err(1, "cannot open bus %s", BOOT_BUS);

	// check whether the chip is actually present and responding. this
	// ensures that if it isn't, the installer fails BEFORE printing
	// "writing xyz". the user can thus use the "writing xyz" indication
	// to determine whether anything has been written, and thus whether
	// the EEPROM might be in an inconsistent state.
	if (ioctl(bus, I2C_SLAVE, BOOT_ADDR) < 0)
		err(1, "cannot set chip address %02x", BOOT_ADDR);
	char zero = 0x00;
	int res = write(bus, &zero, 1);
	if (res < 0)
		err(1, "cannot set address %02x:%02x", BOOT_ADDR, 0);

	int chip_addr = BOOT_ADDR;
	unsigned char addr = 0x00;
	char buffer[17];
	int len;
	do {
		buffer[0] = addr;
		len = read(fd, &buffer[1], 16);
		if (len < 0)
			err(1, "read from %s", file);

		printf("\rwriting %02x:%02x  ", chip_addr, addr);
		fflush(stdout);
		if (ioctl(bus, I2C_SLAVE, chip_addr) < 0) {
			printf("\n");
			fflush(stdout);
			err(1, "cannot set chip address %02x", chip_addr);
		}
		int num = write(bus, buffer, len + 1);
		if (num < 0) {
			printf("\n");
			fflush(stdout);
			err(1, "write %02x:%02x (%d bytes) failed", chip_addr, addr,
					len);
		}
		// give chip some time to perform the program operation. 24LC16B
		// datasheet says: write cycle time is 5ms max. make it 10ms
		// because it doesn't hurt much and adds some extra safety margin.
		// don't poll because if the chip isn't ready yet, the command will
		// will take ~5 seconds (!) before it times out. if the delay is
		// ever too short for some reason, the next page written will not
		// be acknowledged and eventually time out, preventing an incomplete
		// flash from going unnoticed.
		usleep(10000);

		addr += 16;
		if (addr == 0x00)
			chip_addr++;
	} while (len == 16);
	int num = ((chip_addr - BOOT_ADDR) << 8) + (addr - 16) + len;
	printf("\rok (%d bytes)  \n", num);
	
	close(fd);
	close(bus);
	return 0;
}
