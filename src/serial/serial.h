/**
 * Headers for serial.c
 */

/**
 * Initialize the serial port for use with Hood DNC.
 * In other words, this opens the port as a file descriptor.
 *
 * @param path_to_port path to the serial port
 *
 * @return file descriptor to serial port
 */
int serial_open(char *path_to_port);

// struct termios *serial_configure(int unknown);

/**
 * Close the serial port in use with Hood DNC.
 * Restore the old configuration and release the file descriptor.
 *
 * @param a ligma
 *
 * @return file descriptor to serial port
 */
int serial_close(void);
