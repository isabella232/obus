## Process this file with automake to produce Makefile.in
###############################################################################
# Makefile.am for examples automake build system
###############################################################################

AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = netclient

netclient_SOURCES = \
	netclient.c \
	generated/net_bus.c \
	generated/net_bus.h \
	generated/net_interface.c \
	generated/net_interface.h

netclient_CPPFLAGS = -I$(srcdir)/generated -I$(top_srcdir)/src/libobus/include
netclient_LDADD = $(top_srcdir)/src/libobus/libobus.la

MAINTAINERCLEANFILES = Makefile.in
