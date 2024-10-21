/* $Id: le32hex2bin.c,v 1.3 2008/12/11 12:18:17 ecd Exp $
 *
 * $Log: le32hex2bin.c,v $
 * Revision 1.3  2008/12/11 12:18:17  ecd
 * major rework to qemu
 *
 * Revision 1.1  2006-08-20 17:26:07  ecd
 * Modularize most stuff, IO_PORT, INTC, ARM, and MMU still missing.
 *
 * Revision 1.1  2002/06/19 08:19:39  ecd
 * Add Lattice/Xilinx tools
 *
 * Revision 1.1  2002/01/04 10:46:48  ecd
 * initial import
 *
 * Revision 1.1  1999/12/09 10:53:33  ecd
 * add Xilinx conversion tool
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>

int
main(int argc, char **argv)
{
	unsigned char *input, *p;
	unsigned char *memory = NULL;
	size_t size;
	int in, out;
	int i;

	if (argc < 3) {
		fprintf(stderr, "usage: %s <infile> <outfile>\n", argv[0]);
		exit(1);
	}

	if (!strcmp(argv[1], "-"))
		in = 0;
	else {
		in = open(argv[1], O_RDONLY);
		if (in < 0) {
			perror(argv[1]);
			exit(1);
		}
	}

	if (!strcmp(argv[2], "-"))
		out = 1;
	else {
		out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);
		if (out < 0) {
			perror(argv[2]);
			exit(1);
		}
	}

	size = lseek(in, 0, SEEK_END);
	lseek(in, 0, SEEK_SET);

	input = (unsigned char *)malloc(size);
	if (!input) {
		fprintf(stderr, "%s: out of memory\n", argv[0]);
		exit(1);
	}

	if (read(in, input, size) != size) {
		perror("read");
		exit(1);
	}

	close(in);

	memory = malloc(size >> 1);
	if (!memory) {
		fprintf(stderr, "%s: out of memory\n", argv[0]);
		exit(1);
	}

	p = input;
	for (i = 0; i < (size >> 1); i++) {
		if ('0' <= *p && *p <= '9')
			memory[(i & ~3) + 3 - (i & 3)] = (*p - '0') << 0;
		else if ('a' <= *p && *p <= 'f')
			memory[(i & ~3) + 3 - (i & 3)] = (*p - 'a' + 10) << 0;
		else if ('A' <= *p && *p <= 'F')
			memory[(i & ~3) + 3 - (i & 3)] = (*p - 'A' + 10) << 0;
		else {
			fprintf(stderr, "%s: parse error at byte %d\n",
				argv[0], i);
			exit(1);
		}
		p++;
		if ('0' <= *p && *p <= '9')
			memory[(i & ~3) + 3 - (i & 3)] |= (*p - '0') << 4;
		else if ('a' <= *p && *p <= 'f')
			memory[(i & ~3) + 3 - (i & 3)] |= (*p - 'a' + 10) << 4;
		else if ('A' <= *p && *p <= 'F')
			memory[(i & ~3) + 3 - (i & 3)] |= (*p - 'A' + 10) << 4;
		else {
			fprintf(stderr, "%s: parse error at byte %d\n",
				argv[0], i);
			exit(1);
		}
		p++;
	}

	write(out, memory, size >> 1);
	close(out);

	return 0;
}
