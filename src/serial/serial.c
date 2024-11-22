/**
 * Serial interface code to make hood_dnc more simple.
 * This uses an RS-232 serial port on your system!! (please be in the "dialout" group)
 *
 * Sources:
 * https://tldp.org/HOWTO/Serial-Programming-HOWTO/
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 */
#include "serial.h"

#include <alloca.h>
#include <asm-generic/termbits-common.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int serial_open(char *path_to_port) {
	/* symbols */
	int fd = -1;

	/* straight from source #1 */
	fd = open(path_to_port, O_RDWR | O_NOCTTY);

	/* error check file descriptor */
	if (fd < 0) {
		IO_ERROR("error opening serial port", path_to_port, errno);
	}

	/* yay */
	return fd;
}

int serial_configure(int fd, speed_t rate, struct termios *old_config) {
	/* straight from source #2 */
	struct termios new_config = {0};

	/* acquire and backup the current attributes*/
	if (tcgetattr(fd, &new_config) < 0) {
		IO_ERROR("error backing up configuration with", "tcgetattr", errno)
		return -1;
	}
	(void)memcpy(old_config, &new_config, sizeof(struct termios));

	/* set baud rate */
	if (cfsetospeed(&new_config, (speed_t)rate) || cfsetispeed(&new_config, (speed_t)rate)) {
		IO_ERROR("error setting baud rate with", "cfsetiospeed", errno)
		return -1;
	}

	/* weirdo settings taken verbatim from source #2 */
	new_config.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
	new_config.c_cflag &= ~CSIZE;			/* invert CSIZE to later set CS8 */
	new_config.c_cflag |= CS8;				/* 8-bit characters */
	new_config.c_cflag &= ~PARENB;			/* no parity bit */
	new_config.c_cflag &= ~CSTOPB;			/* only need 1 stop bit */
	new_config.c_cflag &= ~CRTSCTS;			/* no hardware flowcontrol */

	/* setup for non-canonical mode */
	new_config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	new_config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	new_config.c_oflag &= ~OPOST;

	/* setup for timed read */
	new_config.c_cc[VMIN] = 0;
	new_config.c_cc[VTIME] = READ_TIMEOUT_SEC;

	/* break the hivemind; set even parity and software flow control */
	new_config.c_cflag |= PARENB;
	new_config.c_iflag |= IXON | IXOFF | IXANY;

	/* actually set the attributes */
	if (tcsetattr(fd, TCSANOW, &new_config) != 0) {
		IO_ERROR("error applying configuration with", "tcsetattr", errno)
		return -1;
	}
	return 0;
}

int serial_close(int fd, struct termios *old_config) {
	/* fr don't do it */
	if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		ERROR("nah bruh don't close those");
		return -1;
	}

	/* set the old parameters bro; idgaf if it fails */
	if (old_config && tcsetattr(fd, TCSANOW, old_config)) {
		IO_ERROR("error restoring configuration with", "tcsetattr", errno)
	}

	/* close the mf port */
	return close(fd);
}
