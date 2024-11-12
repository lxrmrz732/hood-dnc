/**
 * hood_dnc
 *
 * Proof-of-concept DNC program based on behavior seen in Cadem NCNet Lite 7.
 * Only RS-232 for Haas CNC machines is supported.
 *
 * Sources:
 * https://tldp.org/HOWTO/Serial-Programming-HOWTO/
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 */
#include "serial/serial.h"
#include <stdio.h>

/**
 * chad comment
 */
int main(int argc, char *argv[]) {
	/* freaky */
	struct termios backup_config = {0};
	char *serial_path = NULL;
	speed_t baud_rate = -1;
	int serial_fd = -1;

	/* chack the params, FUCK stdin */
	if (argc != 3) {
		(void)fprintf(stderr, "Usage: ./%s [baud rate] [serial port]\n", argv[0]);
		return 1;
	}

	/* grab the params */
	baud_rate = (speed_t)(argv[1]);
	serial_path = argv[2];

	/* sanitize params */
	if (baud_rate == 0) {
		(void)fprintf(stderr, "bro atoi failed; use a real number\n");
		return 2;
	}

	/* open the serial port */
	serial_fd = serial_open(serial_fd);
	if (serial_fd == -1) {
		(void)fprintf(stderr, "failed to open the mf serial port\n");
		return 3;
	}

	/* configure the serial port */
	if (serial_configure(serial_fd, baud_rate, &backup_config)) {
		(void)fprintf(stderr, "failed to configure the mf serial port\n");
		return 4;
	}

	/* use the serial port */
	// put code here

	/* close the serial port */
	if (serial_close(serial_fd, &backup_config)) {
		(void)fprintf(stderr, "failed to close the mf serial port\n");
		return 5;
	}
	return 0;
}