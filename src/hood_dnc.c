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
#include <stdio.h>
#include "serial/serial.h"

int main(int argc, char *argv[]) {
	(void)serial_open(argv[2]);
	(void)puts(argv[argc - 1]);
	return 0;
}