#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "expect.h"

#define BUFSIZE 4096

static uint8_t buf[BUFSIZE];

int main(int argc, char *argv[]) {
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
