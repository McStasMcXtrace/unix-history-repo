/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sigcompat.c	8.1 (Berkeley) 6/2/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <signal.h>

sigvec(signo, sv, osv)
	int signo;
	struct sigvec *sv, *osv;
{
	int ret;

	if (sv)
		sv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
	ret = sigaction(signo, (struct sigaction *)sv, (struct sigaction *)osv);
	if (ret == 0 && osv)
		osv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
	return (ret);
}

sigsetmask(mask)
	int mask;
{
	int omask, n;

	n = sigprocmask(SIG_SETMASK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}

sigblock(mask)
	int mask;
{
	int omask, n;

	n = sigprocmask(SIG_BLOCK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}

sigpause(mask)
	int mask;
{
	return (sigsuspend((sigset_t *)&mask));
}
