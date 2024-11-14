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

#define SEND 's'
#define RECEIVE 'r'
#define FAIL 'f'

int nc_send(int serial_fd, char *file_path);

/**
 * chad comment
 */
int main(int argc, char *argv[]) {
	/* freaky symbols */
	struct termios backup_config = {0};
	char *serial_path = NULL;
	char *file_path = NULL;
	speed_t baud_rate = -1; /* use 4097 for B57600; try 4102 */
	int serial_fd = -1;
	int helper_return = -1;
	char mode = -1;

	/* chack the params, FUCK stdin */
	if (argc != 5) {
		(void)fprintf(stderr, "Usage: ./%s [baud rate] [serial port] [file to send/receive] ['s'end/'r'eceive]\n", argv[0]);
		return 1;
	}

	/* grab the params */
	baud_rate = (speed_t)atoi(argv[1]);
	serial_path = argv[2];
	file_path = argv[3];
	mode = argv[4][0];

	/* sanitize params */
	if (baud_rate == 0) {
		(void)fprintf(stderr, "bro atoi failed; use a real number\n");
		return 2;
	}
	if (mode != SEND && mode != RECEIVE) {
		(void)fprintf(stderr, "file mode MUST be 's'end or 'r'eceive\n");
		return 3;
	}

	/* open and configure the serial port */
	serial_fd = serial_open(serial_path);
	if (serial_fd == -1) {
		(void)fprintf(stderr, "failed to open the mf serial port\n");
		return 4;
	}
	if ((helper_return = serial_configure(serial_fd, baud_rate, &backup_config))) {
		(void)fprintf(stderr, "failed to configure the mf serial port\n");
		mode = FAIL;
	}

	/* yay branching */
	switch (mode) {
	case SEND: {
		helper_return = nc_send(serial_fd, file_path);
		break;
	}
	case RECEIVE: {
		(void)fprintf(stderr, "nc receive not implemented\n");
		break;
	}
	default:
		(void)fprintf(stderr, "mode set to %c, bailing", mode);
	}

	/* close the serial port */
	if (serial_close(serial_fd, &backup_config)) {
		(void)fprintf(stderr, "failed to close the mf serial port\n");
		return 7;
	}
	return helper_return;
}

/**
 * Open a file and dump it to the serial port.
 *
 * @param serial_fd serial port file descriptor
 * @param file_path path to file to dump
 *
 * @return zero on success, nonzero if anything went wrong
 */
int nc_send(int serial_fd, char *file_path) {
	/* symbols */
	FILE *nc_file = NULL;
	char *data_buffer = NULL;
	size_t bytes_read = 0;
	int status = 0;

	/* allocate space */
	data_buffer = alloca(sizeof(char) * BUFSIZ);

	/* open and read from the file */
	nc_file = fopen(file_path, "r");
	if (nc_file == NULL) {
		(void)fprintf(stderr, "Error opening NC file %s: %s\n", file_path, strerror(errno));
		return 10;
	}

	/* write contents to the serial port as they are read from the file */
	while ((bytes_read = fread(data_buffer, 1, BUFSIZ, nc_file))) {
		if (write(serial_fd, data_buffer, bytes_read) == -1) {
			(void)fprintf(stderr, "Writing to serial port fucked up: %s\n", strerror(errno));
			status = 11;
			break;
		}
	}

	/* error "handling" */
	if (ferror(nc_file)) {
		(void)fprintf(stderr, "Error reading NC file: %s\n", strerror(errno));
		status = 12;
	}
	if (fclose(nc_file)) {
		(void)fprintf(stderr, "Error closing NC file: %s\n", strerror(errno));
		status = 13;
	}
	return status;
}