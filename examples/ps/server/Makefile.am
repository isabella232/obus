## Process this file with automake to produce Makefile.in
###############################################################################
# Makefile.am for examples automake build system
###############################################################################

AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = psd

psd_SOURCES = \
	psd.c \
	generated/ps_bus.c \
	generated/ps_bus.h \
	generated/ps_process.c \
	generated/ps_process.h \
	generated/ps_summary.c
	generated/ps_summary.h

psd_CPPFLAGS = -I$(srcdir)/generated -I$(top_srcdir)/src/libobus/include
psd_LDADD = $(top_srcdir)/src/libobus/libobus.la

MAINTAINERCLEANFILES = Makefile.in
