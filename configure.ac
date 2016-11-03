AC_PREREQ([2.68])

AC_INIT([obus],
	[1.0.0],
	[jean-baptiste@parrot.com])

AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability foreign])

AC_CONFIG_SRCDIR([src/libobus/src/obus_bus.c])

AC_SUBST(VERSION)

AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_PROG_CC

AM_PATH_PYTHON([2.7])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT

AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_HEADERS([config.h])

AC_PROG_LN_S
AC_PROG_MKDIR_P

AC_CONFIG_FILES([Makefile src/Makefile obus.pc]
		[src/libobus/Makefile src/obusgen/Makefile src/obusgen/c/Makefile src/obusgen/java/Makefile src/obusgen/vala/Makefile]
		[examples/Makefile examples/net/Makefile examples/net/server/Makefile examples/net/client/Makefile examples/ps/Makefile examples/ps/server/Makefile examples/ps/client/Makefile])

# signalfd & timerfd
AC_CHECK_HEADERS_ONCE([sys/signalfd.h sys/timerfd.h sys/eventfd.h])

# Epoll check. If not present, the build will fallback on poll() API
AX_HAVE_EPOLL(
	[AX_CONFIG_FEATURE_ENABLE(epoll)],
	[AX_CONFIG_FEATURE_DISABLE(epoll)]
)

AX_CONFIG_FEATURE(
	[epoll], [This platform supports epoll(7)],
	[HAVE_EPOLL], [This platform supports epoll(7).]
)

AC_OUTPUT
