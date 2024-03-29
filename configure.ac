dnl
dnl configure for Sufary
dnl $Id: configure.in,v 1.21 2000/09/10 10:34:33 tatuo-y Exp $
dnl
dnl Created  by Satoru Takabayashi <satoru-t@is.aist-nara.ac.jp> [1999-09-17]
dnl

dnl Process this file with autoconf to produce a configure script.
AC_INIT([sufary], [2.3.8])
AC_PREREQ(2.13)               dnl Minimum Autoconf version required.
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl *
dnl * libtool
dnl *
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

if test -n "$GCC"; then
    CFLAGS="-g -Wall -pedantic"
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_C_BIGENDIAN
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MMAP
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strstr)

dnl This option requires config.guess, config.sub and install-sh
dnl AC_CANONICAL_SYSTEM

dnl * 
dnl * Checks for external commands
dnl *

AC_ARG_WITH(
	perl5,
	[  --with-perl5=PATH       set perl 5.004 or later location [search path]],
	AC_MSG_RESULT(using $with_perl5 for perl5)
	PERL=$with_perl5,
	[AC_PATH_PROGS(PERL, perl5 perl, no)]
)
if test $PERL = "no" || $PERL -e 'exit ($] >= 5.004)'; then
    AC_MSG_ERROR(perl 5.004 or later required)
fi
AC_SUBST(PERL)

dnl * 
dnl * Output Files.
dnl * 
AM_CONFIG_HEADER(config.h)

AC_OUTPUT([Makefile src/Makefile lib/Makefile man/Makefile src/mkdids], 
  [chmod +x src/mkdids]) 

# Local Variables:
# mode:shell-script
# sh-indentation:2
# End:
