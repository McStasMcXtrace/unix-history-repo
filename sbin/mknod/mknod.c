/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kevin Fall.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)mknod.c	8.1 (Berkeley) 6/5/93";
#else
static const char rcsid[] =
    "$Id: mknod.c,v 1.3 1996/07/30 17:43:21 bde Exp $";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	dev_t dev;
	char *endp;
	long major, minor;
	mode_t mode;
	int range_error;

	if (argc != 5) {
		(void)fprintf(stderr,
		    "usage: mknod name [b | c] major minor\n");
		exit(1);
	}

	mode = 0666;
	if (argv[2][0] == 'c')
		mode |= S_IFCHR;
	else if (argv[2][0] == 'b')
		mode |= S_IFBLK;
	else {
		(void)fprintf(stderr,
		    "mknod: node must be type 'b' or 'c'\n");
		exit(1);
	}

	errno = 0;
	major = (long)strtoul(argv[3], &endp, 0);
	if (endp == argv[3] || *endp != '\0') {
		(void)fprintf(stderr,
		    "mknod: %s: non-numeric major number\n", argv[3]);
		exit(1);
	}
	range_error = errno;
	errno = 0;
	minor = (long)strtoul(argv[4], &endp, 0);
	if (endp == argv[4] || *endp != '\0') {
		(void)fprintf(stderr,
		    "mknod: %s: non-numeric minor number\n", argv[4]);
		exit(1);
	}
	range_error |= errno;
	dev = makedev(major, minor);
	if (range_error || major(dev) != major || minor(dev) != minor) {
		(void)fprintf(stderr,
		    "mknod: major or minor number too large\n");
		exit(1);
	}

	if (mknod(argv[1], mode, dev) != 0) {
		(void)fprintf(stderr,
		    "mknod: %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	exit(0);
}
