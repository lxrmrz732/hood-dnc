/**
 * hood_dnc
 *
 * Proof-of-concept DNC program based on behavior seen in Cadem NCNet Lite 7.
 * Only RS-232 for Haas CNC machines is supported.
 * Make sure you're in the dialout group!
 *
 * Sources:
 * https://tldp.org/HOWTO/Serial-Programming-HOWTO/
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 */
#include "serial/serial.h"
#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * chad comment
 */
int main(int argc, char *argv[]) {
	/* freaky */
	char *serial_path = NULL;
	char *file_path = NULL;
	FILE *nc_file = NULL;
	char *data_buffer = NULL;
	size_t bytes_read = 0;
	struct termios backup_config = {0};
	speed_t baud_rate = -1; /* use 4097 for B57600 */
	int serial_fd = -1;

	/* chack the params, FUCK stdin */
	if (argc != 4) {
		(void)fprintf(stderr, "Usage: ./%s [baud rate] [serial port] [file to transfer]\n", argv[0]);
		return 1;
	}

	/* grab the params */
	baud_rate = (speed_t)atoi(argv[1]);
	serial_path = argv[2];
	file_path = argv[3];

	/* sanitize params */
	if (baud_rate == 0) {
		(void)fprintf(stderr, "bro atoi failed; use a real number\n");
		return 2;
	}

	/* open the serial port */
	serial_fd = serial_open(serial_path);
	if (serial_fd == -1) {
		(void)fprintf(stderr, "failed to open the mf serial port\n");
		return 3;
	}

	/* configure the serial port */
	if (serial_configure(serial_fd, baud_rate, &backup_config)) {
		(void)fprintf(stderr, "failed to configure the mf serial port\n");
		return 4;
	}

	/* open the file */
	nc_file = fopen(file_path, "r");
	if (nc_file == NULL) {
		(void)fprintf(stderr, "Error opening NC file: %s\n", strerror(errno));
		(void)serial_close(serial_fd, &backup_config);
		return 5;
	}

	/* read from the file */
	data_buffer = alloca(sizeof(char) * BUFSIZ);
	while ((bytes_read = fread(data_buffer, 1, BUFSIZ, nc_file))) {
		/* use the serial port */
		if (write(serial_fd, data_buffer, bytes_read) == -1) {
			(void)fprintf(stderr, "Writing to serial port fucked up: %s\n", strerror(errno));
		}
	}
	if (ferror(nc_file)) {
		(void)fprintf(stderr, "Error reading NC file: %s\n", strerror(errno));
	}

	/* close the serial port */
	if (serial_close(serial_fd, &backup_config)) {
		(void)fprintf(stderr, "failed to close the mf serial port\n");
		return 6;
	}

	if (fclose(nc_file)) {
		(void)fprintf(stderr, "Error closing NC file: %s\n", strerror(errno));
	}
	return 0;
}