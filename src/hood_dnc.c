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
#define _POSIX_C_SOURCE 200809L

#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serial/serial.h"

#define SEND 's'
#define RECEIVE 'r'
#define FAIL 'f'

int nc_send(int serial_fd, char *file_path);
int nc_receive(int serial_fd, char *file_path);
int serial_dump(int serial_fd, char *file_path);

/**
 * chad comment
 */
int main(int argc, char *argv[]) {
	/* freaky symbols */
	struct termios backup_config = {0};
	char *serial_path = NULL;
	char *file_path = NULL;
	speed_t baud_rate = -1; /* try (15 = B38400), (4098 = B115200), or (13 = B9600) */
	int serial_fd = -1;
	int helper_return = -1;
	char mode = -1;

	/* check the params; this is a "no stdin" household */
	if (argc != 5) {
		(void)fprintf(stderr,
					  "Usage: %s [baud rate] [serial port] [file] ['s'end/'r'eceive]\n",
					  PROG);
		return 1;
	}

	/* grab the params */
	baud_rate = (speed_t)atoi(argv[1]);
	serial_path = argv[2];
	file_path = argv[3];
	mode = argv[4][0];

	/* sanitize params */
	if (baud_rate == 0) {
		ERROR("atoi failed; try a real number for baud rate");
		return 2;
	}
	if (mode != SEND && mode != RECEIVE) {
		ERROR("file mode MUST be 's'end or 'r'eceive");
		return 3;
	}

	/* open and configure the serial port */
	serial_fd = serial_open(serial_path);
	if (serial_fd == -1) {
		ERROR("failed to open the mf serial port");
		return 4;
	}
	if ((helper_return = serial_configure(serial_fd, baud_rate, &backup_config))) {
		ERROR("failed to configure the mf serial port");
		mode = FAIL;
	}

	/* yay branching */
	switch (mode) {
	case SEND: {
		helper_return = nc_send(serial_fd, file_path);
		break;
	}
	case RECEIVE: {
		helper_return = nc_receive(serial_fd, file_path);
		break;
	}
	default:
		ERROR("program control flow failed; exiting");
		break;
	}

	/* close the serial port */
	if (serial_close(serial_fd, &backup_config)) {
		ERROR("failed to close the mf serial port");
		return 7;
	}
	return helper_return;
}

/**
 * Open a file and write it to the serial port line-by-line.
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
	ssize_t bytes_read = 0;
	size_t bytes_alloc = 0;
	int status = 0;

	/* open and read from the file */
	nc_file = fopen(file_path, "r");
	if (nc_file == NULL) {
		IO_ERROR("error opening NC file", file_path, errno);
		return 30;
	}

	/* write contents to the serial port line-by-line they are read from the file */
	while ((bytes_read = getline(&data_buffer, &bytes_alloc, nc_file)) != -1) {
		/* queue the line for transmission */
		if (write(serial_fd, data_buffer, bytes_read) == -1) {
			IO_ERROR("error writing to", "serial port", errno);
			status = 31;
			break;
		}

		/* ask termios to actually put it on the wire */
		if (tcdrain(serial_fd)) {
			IO_ERROR("error syncing", "serial port", errno);
			status = 32;
			break;
		}
	}

	/* error "handling" */
	if (ferror(nc_file)) {
		IO_ERROR("error reading NC file", file_path, errno);
		status = 33;
	}

	/* close file and free buffers */
	if (fclose(nc_file)) {
		IO_ERROR("error closing NC file", file_path, errno);
		status = 34;
	}
	free(data_buffer);
	data_buffer = NULL;
	nc_file = NULL;
	return status;
}

/**
 * Read the serial port and dump the content to a file.
 *
 * @param serial_fd serial port file descriptor
 * @param file_path path to file to dump
 *
 * @return zero on success, nonzero if anything went wrong
 */
int nc_receive(int serial_fd, char *file_path) {
	/* symbols 2.0 */
	FILE *nc_file = NULL;
	char *data_buffer = NULL;
	ssize_t bytes_read = 0;
	size_t bytes_written = 0;
	int status = 0;

	/* allocate space; BUFSIZ is too much, it seems */
	data_buffer = alloca(sizeof(char) * MAX_SERIAL_READ);

	/* open file for writing */
	nc_file = fopen(file_path, "w");
	if (nc_file == NULL) {
		IO_ERROR("error opening NC file", file_path, errno);
		return 20;
	}

	/* this is "read with timeout" according to man termios */
	while ((bytes_read = read(serial_fd, data_buffer, MAX_SERIAL_READ))) {
		/* check for fumble */
		if (bytes_read == -1) {
			IO_ERROR("error reading from", "serial port", errno);
			status = 21;
			break;
		}

		/* dump content to disk */
		if (!(bytes_written = fwrite(data_buffer, sizeof(char), bytes_read, nc_file))) {
			if (ferror(nc_file)) {
				IO_ERROR("error writing to", file_path, errno);
				status = 22;
				break;
			} else {
				ERROR("successfully wrote zero bytes?");
				status = 23;
				break;
			}
		}
	}

	/* flush contents; close files */
	if (fflush(nc_file)) {
		IO_ERROR("error flushing", file_path, errno);
		status = 24;
	}
	if (fclose(nc_file)) {
		IO_ERROR("error closing NC file", file_path, errno);
		status = 25;
	}
	data_buffer = NULL;
	nc_file = NULL;
	return status;
}

/**
 * Open a file and dump it to the serial port.
 * Doesn't work for CNC machines lmfao.
 *
 * @param serial_fd serial port file descriptor
 * @param file_path path to file to dump
 *
 * @return zero on success, nonzero if anything went wrong
 */
int serial_dump(int serial_fd, char *file_path) {
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
		IO_ERROR("error opening NC file", file_path, errno);
		return 10;
	}

	/* write contents to the serial port as they are read from the file */
	while ((bytes_read = fread(data_buffer, sizeof(char), BUFSIZ, nc_file))) {
		if (write(serial_fd, data_buffer, bytes_read) == -1) {
			IO_ERROR("error writing to", "serial port", errno);
			status = 11;
			break;
		}
	}

	/* error "handling" */
	if (ferror(nc_file)) {
		IO_ERROR("error reading NC file", file_path, errno);
		status = 12;
	}
	if (fclose(nc_file)) {
		IO_ERROR("error closing NC file", file_path, errno);
		status = 13;
	}
	data_buffer = NULL;
	nc_file = NULL;
	return status;
}