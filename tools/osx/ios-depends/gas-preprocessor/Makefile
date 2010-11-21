#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

GASBIN=$(PREFIX)/bin/gas-preprocessor.pl

all:

install:
	cp gas-preprocessor.pl $(GASBIN)

clean:
	rm $(GASBIN)

distclean::
	rm $(GASBIN)
