#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "expect.h"

#define BUFSIZE 4096

static uint8_t buf[BUFSIZE];

int main(int argc, char *argv[]) {
	if (unlikely(argc != 5)) {
		fputs("Not enough arguments!\n", stderr);
		return EXIT_FAILURE;
	}

	/* Check number of input objects */
	unsigned long int argn = strtoul(argv[4], (char **) 0, 16);
	if (unlikely(argn != 1)) {
		fputs("Invalid number of input objects!\n"
			"The identity transformation takes exactly one object!\n", stderr);
		return EXIT_FAILURE;
	}

	ssize_t in, out;

	do {
		in = read(0, buf, BUFSIZE);
		if (unlikely(in < 0)) {
			perror("Read error");
			return EXIT_FAILURE;
		}

		out = write(1, buf, in);
		if (unlikely(out < 0)) {
			perror("Write error");
			return EXIT_FAILURE;
		}

		else if (unlikely(out < in)) {
			fputs("Write error!\n", stderr);
			return EXIT_FAILURE;
		}
	} while (likely(in > 0));

	return EXIT_SUCCESS;
}
