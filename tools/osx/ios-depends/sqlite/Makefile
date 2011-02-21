#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

PKGFILE=$(PREFIX)/lib/pkgconfig/sqlite3.pc

all: .installed

.installed:
	mkdir -p $(PREFIX)/lib/pkgconfig
	sqlite3.pc $(PKGFILE)
	touch $@

clean:
distclean::
	rm $(PKGFILE) .installed
