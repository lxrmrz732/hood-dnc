/**
 * Serial interface code to make hood_dnc more simple.
 * This uses an RS-232 serial port on your system!! (please be in the "dialout" group)
 *
 * Input uses "Canonical Input Processing" because NC code always begins with '%' and ends with '%'.
 * Output is undecided because I have no idea.
 *
 * Sources:
 * https://tldp.org/HOWTO/Serial-Programming-HOWTO/
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 */
#include "serial.h"
#include <asm-generic/termbits-common.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* baudrate settings are defined in <asm/termbits.h>, which is included by <termios.h> */
#define BAUDRATE B38400

volatile int STOP = 0;

int serial_open(char *path_to_port) {
	/* symbols */
	int fd;

	/* straight from source #1 */
	fd = open(path_to_port, O_RDWR | O_NOCTTY);

	/* error check file descriptor */
	if (fd < 0) {
		perror(path_to_port);
	}

	/* yay */
	return fd;
}

struct termios *serial_configure(int unknown) {
	(void)printf("%d", unknown);
	return NULL;
}

int not_main(int fd) {
	char buf[355];
	int res;
	struct termios oldtio = {0};
	struct termios newtio = {0};
	tcgetattr(fd, &oldtio);				/* save current serial port settings */
	memset(&newtio, 0, sizeof(newtio)); /* clear struct for new port settings */

	/*
	  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	  CRTSCTS : output hardware flow control (only used if the cable has
				all necessary lines. See sect. 7 of Serial-HOWTO)
	  CS8     : 8n1 (8bit,no parity,1 stopbit)
	  CLOCAL  : local connection, no modem contol
	  CREAD   : enable receiving characters
	*/
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

	/*
	  IGNPAR  : ignore bytes with parity errors
	  ICRNL   : map CR to NL (otherwise a CR input on the other computer
				will not terminate input)
	  otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR | ICRNL;

	/*
	 Raw output.
	*/
	newtio.c_oflag = 0;

	/*
	  ICANON  : enable canonical input
	  disable all echo functionality, and don't send signals to calling program
	*/
	newtio.c_lflag = ICANON;

	/*
	  initialize all control characters
	  default values can be found in /usr/include/termios.h, and are given
	  in the comments, but we don't need them here
	*/
	newtio.c_cc[VINTR] = 0;	   /* Ctrl-c */
	newtio.c_cc[VQUIT] = 0;	   /* Ctrl-\ */
	newtio.c_cc[VERASE] = 0;   /* del */
	newtio.c_cc[VKILL] = 0;	   /* @ */
	newtio.c_cc[VEOF] = 4;	   /* Ctrl-d */
	newtio.c_cc[VTIME] = 0;	   /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;	   /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC] = 0;	   /* '\0' */
	newtio.c_cc[VSTART] = 0;   /* Ctrl-q */
	newtio.c_cc[VSTOP] = 0;	   /* Ctrl-s */
	newtio.c_cc[VSUSP] = 0;	   /* Ctrl-z */
	newtio.c_cc[VEOL] = 0;	   /* '\0' */
	newtio.c_cc[VREPRINT] = 0; /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0; /* Ctrl-u */
	newtio.c_cc[VWERASE] = 0;  /* Ctrl-w */
	newtio.c_cc[VLNEXT] = 0;   /* Ctrl-v */
	newtio.c_cc[VEOL2] = 0;	   /* '\0' */

	/*
	  now clean the modem line and activate the settings for the port
	*/
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	/*
	  terminal settings done, now handle input
	  In this example, inputting a 'z' at the beginning of a line will
	  exit the program.
	*/
	while (STOP == 0) { /* loop until we have a terminating condition */
		/* read blocks program execution until a line terminating character is
		   input, even if more than 255 chars are input. If the number
		   of characters read is smaller than the number of chars available,
		   subsequent reads will return the remaining chars. res will be set
		   to the actual number of characters actually read */
		res = read(fd, buf, 255);
		buf[res] = 0; /* set end of string, so we can printf */
		printf(":%s:%d\n", buf, res);
		if (buf[0] == 'z')
			STOP = 1;
	}
	/* restore the old port settings */
	tcsetattr(fd, TCSANOW, &oldtio);

	return 9;
}