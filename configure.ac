#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# prereq & init

AC_PREREQ(2.60)
AC_INIT([gdEngine], [0.1], [john@domainofchildhood.com])
AC_CONFIG_SRCDIR([engine/main.cpp])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE([gdEngine], [0.5])

# ----------------------------------------------------------------------

# test the build environment. These vars are used in Makefile.am during
# the linking of the libraries.
# Usage: ./configure --target=[windows | linux | osx]

if test "$target" = ""; then
	AC_MSG_ERROR(["target OS not specified. Please run ./configure --target=<windows | linux | osx>"])
fi

case "$target" in
  linux)
    os=linux
    ;;
  windows)
    os=windows
    ;;
  osx)
    os=osx
    ;;
  *)
    AC_MSG_ERROR(["Unrecognised target OS: $target"])
    ;;
esac
AM_CONDITIONAL(LINUX,   test "x$os" = "xlinux")
AM_CONDITIONAL(WINDOWS, test "x$os" = "xwindows")
AM_CONDITIONAL(OSX,     test "x$os" = "xosx")

# ----------------------------------------------------------------------

# Check for programs.

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# ----------------------------------------------------------------------

# Check for libraries.

AC_CHECK_LIB(
	[pthread],
	[pthread_exit],
	[],
	[AC_MSG_ERROR([error: library 'pthread' not found!])]
)

# ----------------------------------------------------------------------

# Check for generic headers (fltk, rtaudio and libsndfile are static,
# we ask if headers are available)

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[FL/Fl.H],
	[],
	[AC_MSG_ERROR([library 'fltk' not found!])]
)
AC_LANG_POP

#AC_LANG_PUSH([C++])
#AC_CHECK_HEADER(
#	[RtMidi.h],
#	[],
#	[AC_MSG_ERROR([library 'RtMidi' not found!])]
#)
#AC_LANG_POP

#AC_LANG_PUSH([C++])
#AC_CHECK_HEADER(
#	[RtAudio.h],
#	[],
#	[AC_MSG_ERROR([library 'RtAudio' not found!])]
#)
#AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[sndfile.h],
	[],
	[AC_MSG_ERROR([library 'libsndfile' not found!])]
)
AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[samplerate.h],
	[],
	[AC_MSG_ERROR([library 'samplerate' not found!])]
)
AC_LANG_POP

# ----------------------------------------------------------------------

# Check for linux header files.

if test "x$os" = "xlinux"; then

	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[X11/xpm.h],
		[],
		[AC_MSG_ERROR([missing xpm.h, maybe you need to install the libxpm-dev package?])]
	)
	AC_LANG_POP
fi

# ----------------------------------------------------------------------

# finalizing
AC_CONFIG_SUBDIRS([rtaudio rtmidi])
AC_CONFIG_FILES([Makefile engine/Makefile interfaces/Makefile interfaces/fltk/Makefile])

AC_OUTPUT
