## Process this file with automake to produce Makefile.in
###############################################################################
# Makefile.am for libobus automake build system
###############################################################################
AUTOMAKE_OPTIONS = subdir-objects

# include libobus common makefile
include $(srcdir)/libobus.mk

# declare library
lib_LTLIBRARIES = libobus.la

# get include dirs
INC_DIRS = $(addprefix -I$(srcdir)/,$(sort $(dir $(LIBOBUS_PUBLIC_HEADER_FILES))))
INC_DIRS += $(addprefix -I$(srcdir)/,$(sort $(dir $(LIBOBUS_HEADER_FILES))))

# add library flags
libobus_la_CPPFLAGS = $(INC_DIRS) $(LIBOBUS_CFLAGS)
libobus_la_CFLAGS = $(LIBOBUS_CFLAGS)
libobus_la_LDFLAGS = $(LIBOBUS_LDFLAGS)
libobus_la_SOURCES = $(LIBOBUS_HEADER_FILES) $(LIBOBUS_SOURCE_FILES)
include_HEADERS = $(LIBOBUS_PUBLIC_HEADER_FILES)

MAINTAINERCLEANFILES = Makefile.in
