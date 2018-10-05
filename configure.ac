dnl Standard configure.ac
AC_PREREQ(2.53)
AC_INIT([wm], [0.1.0], [manuel.cano@amadeus.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([dist-bzip2])
AM_CONFIG_HEADER([config.h:config.in])
AC_CONFIG_SRCDIR([src/main.cc])

dnl Checks for programs.
AC_PROG_CXX
dnl AC_PROG_CC
dnl AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
dnl AC_HEADER_STDC
dnl AC_CHECK_HEADERS([sys/types.h sys/socket.h stdlib.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_ARG_WITH([log],
 	AC_HELP_STRING([--with-log],
 	[Build with log. @<:@default=no@:>@]),
 	[with_log=$withval])

AS_IF([test "x$with_log" == xyes],
      [AC_DEFINE([WITH_LOG], [1],
[Defined if you want output log.])])

dnl AM_CONDITIONAL(FEATURE, [test x"$the_feature" != x])
dnl AC_DEFUN(AC_MSG_SUMMARY, [echo $1])
dnl AC_DEFUN(WITH,dnl [
dnl if test ${2_FALSE} ; then
dnl 	echo $1: enabled
dnl else
dnl 	echo $1: disabled
dnl fi
dnl ])
dnl
dnl AC_MSG_SUMMARY(------------------)
dnl AC_MSG_SUMMARY(  FEATURES:)
dnl AC_MSG_SUMMARY(------------------)
dnl WITH(feature, FEATURE)
dnl AC_MSG_SUMMARY(------------------)
dnl
dnl Checks for library functions.

dnl AC_SUBST(CPPFLAGS)
dnl AC_SUBST(LDFLAGS)
CFLAGS="-Wall -g $CFLAGS"

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
