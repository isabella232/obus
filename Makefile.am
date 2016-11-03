## Process this file with automake to produce Makefile.in
SUBDIRS = src examples

pkgconfig_DATA= obus.pc
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = autogen.sh obus.pc.in

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = \
		aclocal.m4 compile config.guess config.sub configure \
		depcomp install-sh ltmain.sh Makefile.in missing config.h.in

ACLOCAL_AMFLAGS = -I m4

