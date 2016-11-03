#!/bin/sh
# Run this to generate all the initial makefiles, etc.

package=obus
mkdir -p m4
autoreconf --force --install
rm -rf autom4te.cache
rm -f config.h.in~
echo ""
echo "type './configure' to configure $package."
echo "type 'make' to compile $package."
echo "type 'make install' to install $package."

