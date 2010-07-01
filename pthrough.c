#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "expect.h"

int main(int argc, char *argv[]) {
	/* Check number of arguments */
	if (unlikely(argc != 5)) {
		fputs("Invalid number of arguments!\n", stderr);
		return EXIT_FAILURE;
	}

	/* Change to directory where the transformation resides */
	if (unlikely(chdir(argv[1]))) {
		perror("Unable to change to transformation directory");
		return EXIT_FAILURE;
	}

	/* Despatch transformation */
	if (unlikely(execl("transform", "transform",
		argv[1], argv[2], argv[3], argv[4], (char *) 0))) {
		perror("Failed to despatch transformation");
		return EXIT_FAILURE;
	}
}
