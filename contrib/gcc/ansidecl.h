/* ANSI and traditional C compatability macros
   Copyright 1991, 1992, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* ANSI and traditional C compatibility macros

   ANSI C is assumed if __STDC__ is #defined.

   Macro	ANSI C definition	Traditional C definition
   -----	---- - ----------	----------- - ----------
   PTR		`void *'		`char *'
   LONG_DOUBLE	`long double'		`double'
   VOLATILE	`volatile'		`'
   SIGNED	`signed'		`'
   PTRCONST	`void *const'		`char *'
   ANSI_PROTOTYPES  1			not defined

   CONST is also defined, but is obsolete.  Just use const.

   obsolete --     DEFUN (name, arglist, args)

	Defines function NAME.

	ARGLIST lists the arguments, separated by commas and enclosed in
	parentheses.  ARGLIST becomes the argument list in traditional C.

	ARGS list the arguments with their types.  It becomes a prototype in
	ANSI C, and the type declarations in traditional C.  Arguments should
	be separated with `AND'.  For functions with a variable number of
	arguments, the last thing listed should be `DOTS'.

   obsolete --     DEFUN_VOID (name)

	Defines a function NAME, which takes no arguments.

   obsolete --     EXFUN (name, (prototype))	-- obsolete.

	Replaced by PARAMS.  Do not use; will disappear someday soon.
	Was used in external function declarations.
	In ANSI C it is `NAME PROTOTYPE' (so PROTOTYPE should be enclosed in
	parentheses).  In traditional C it is `NAME()'.
	For a function that takes no arguments, PROTOTYPE should be `(void)'.

   obsolete --     PROTO (type, name, (prototype)    -- obsolete.

	This one has also been replaced by PARAMS.  Do not use.

   PARAMS ((args))

	We could use the EXFUN macro to handle prototype declarations, but
	the name is misleading and the result is ugly.  So we just define a
	simple macro to handle the parameter lists, as in:

	      static int foo PARAMS ((int, char));

	This produces:  `static int foo();' or `static int foo (int, char);'

	EXFUN would have done it like this:

	      static int EXFUN (foo, (int, char));

	but the function is not external...and it's hard to visually parse
	the function name out of the mess.   EXFUN should be considered
	obsolete; new code should be written to use PARAMS.

   DOTS is also obsolete.

   Examples:

	extern int printf PARAMS ((const char *format, ...));
*/

#ifndef	_ANSIDECL_H

#define	_ANSIDECL_H	1


/* Every source file includes this file,
   so they will all get the switch for lint.  */
/* LINTLIBRARY */


#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(_WIN32)
/* All known AIX compilers implement these things (but don't always
   define __STDC__).  The RISC/OS MIPS compiler defines these things
   in SVR4 mode, but does not define __STDC__.  */

#define	PTR		void *
#define	PTRCONST	void *CONST
#define	LONG_DOUBLE	long double

#ifndef IN_GCC
#define	AND		,
#define	NOARGS		void
#define	VOLATILE	volatile
#define	SIGNED		signed
#endif /* ! IN_GCC */

#define PARAMS(paramlist)		paramlist
#define ANSI_PROTOTYPES			1

#define VPARAMS(ARGS)			ARGS
#define VA_START(va_list,var)		va_start(va_list,var)

/* These are obsolete.  Do not use.  */
#ifndef IN_GCC
#define CONST				const
#define DOTS				, ...
#define PROTO(type, name, arglist)	type name arglist
#define EXFUN(name, proto)		name proto
#define DEFUN(name, arglist, args)	name(args)
#define DEFUN_VOID(name)		name(void)
#endif /* ! IN_GCC */

#else	/* Not ANSI C.  */

#define	PTR		char *
#define	PTRCONST	PTR
#define	LONG_DOUBLE	double

#ifndef IN_GCC
#define	AND		;
#define	NOARGS
#define	VOLATILE
#define	SIGNED
#endif /* !IN_GCC */

#ifndef const /* some systems define it in header files for non-ansi mode */
#define	const
#endif

#define PARAMS(paramlist)		()

#define VPARAMS(ARGS)			(va_alist) va_dcl
#define VA_START(va_list,var)		va_start(va_list)

/* These are obsolete.  Do not use.  */
#ifndef IN_GCC
#define CONST
#define DOTS
#define PROTO(type, name, arglist)	type name ()
#define EXFUN(name, proto)		name()
#define DEFUN(name, arglist, args)	name arglist args;
#define DEFUN_VOID(name)		name()
#endif /* ! IN_GCC */

#endif	/* ANSI C.  */

#endif	/* ansidecl.h	*/
