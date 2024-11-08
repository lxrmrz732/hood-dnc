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
#include <alloca.h>
#include <asm-generic/termbits-common.h>  // needed to c89
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "serial.h"

int serial_open(char *path_to_port) {
    /* symbols */
    int fd = -1;

    /* straight from source #1 */
    fd = open(path_to_port, O_RDWR | O_NOCTTY);

    /* error check file descriptor */
    if (fd < 0) {
        perror(path_to_port);
    }

    /* yay */
    return fd;
}

int serial_configure(int fd, speed_t rate, struct termios *old_config) {
    /* straight from source #2 */
    struct termios new_config = {0};

    /* acquire and backup the current attributes*/
    if (tcgetattr(fd, &new_config) < 0) {
        (void)printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }
    (void)memcpy(old_config, &new_config, sizeof(struct termios));

    /* set baud rate */
    if (cfsetospeed(&new_config, (speed_t)rate) || cfsetispeed(&new_config, (speed_t)rate)) {
        (void)printf("Error setting baud rate: %s\n", strerror(errno));
        return -1;
    }

    /* weirdo settings */
    new_config.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
    new_config.c_cflag &= ~CSIZE;
    new_config.c_cflag |= CS8; /* 8-bit characters */
    // new_config.c_cflag &= ~PARENB;  /* no parity bit */ // should turn on instead
    new_config.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
    new_config.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    new_config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    new_config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    new_config.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    new_config.c_cc[VMIN] = 1;
    new_config.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &new_config) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}