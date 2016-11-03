## Process this file with automake to produce Makefile.in
###############################################################################
# Makefile.am for obusgen automake build system
###############################################################################
AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = c java vala

obusgendir=$(pythondir)/obusgen

obusgen_PYTHON = obusparser.py
obusgen_SCRIPTS = obusgen.py

CLEANFILES = $(bindir)/obusgen

install-exec-hook:
	$(MKDIR_P) $(bindir)
	$(LN_S) $(pythondir)/obusgen/obusgen.py $(bindir)/obusgen

uninstall-hook:
	rm -f $(bindir)/obusgen
	rm -rf $(pythondir)/obusgen
