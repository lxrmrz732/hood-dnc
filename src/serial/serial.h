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

/**
 * Configure the serial port for use with Hood DNC.
 * Backup the old configuration and set a hardcoded new configuration.
 *
 * @param fd file descriptor to open serial port
 * @param rate desired baud rate of serial port
 * @param old_config pointer to a space to backup old configuration
 *
 * @return 0 if the port was configured successfully, -1 otherwise
 */
int serial_configure(int fd, speed_t rate, struct termios *old_config);

/**
 * Close the serial port in use with Hood DNC.
 * Restore the old configuration and release the file descriptor.
 *
 * @param a ligma
 *
 * @return file descriptor to serial port
 */
int serial_close(void);
