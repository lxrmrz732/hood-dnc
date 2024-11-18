/**
 * Headers for serial.c and hood_dnc.c along with some helpful macros.
 */
#include <termios.h>

#define PROG "hood_dnc"
#define READ_TIMEOUT_SEC 20
#define MAX_SERIAL_READ __UINT8_MAX__
#define ERROR(MSG) (void)fprintf(stderr, "%s: %s\n", PROG, MSG)
#define IO_ERROR(MSG, FNAME, ERRNO) (void)fprintf(stderr, "%s: %s %s: %s\n", PROG, MSG, FNAME, strerror(ERRNO));

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
 * @param old_config pointer to backup old configuration
 *
 * @return 0 if the port was configured successfully, -1 otherwise
 */
int serial_configure(int fd, speed_t rate, struct termios *old_config);

/**
 * Close the serial port in use with Hood DNC.
 * Restore the old configuration and release the file descriptor.
 *
 * @param fd file descriptor to open serial port
 * @param old_config pointer to backup old configuration
 *
 * @return 0 if the port was configured successfully, -1 otherwise
 */
int serial_close(int fd, struct termios *old_config);
