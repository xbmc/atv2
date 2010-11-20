#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

# lib name, version
LIBNAME=libcdio
VERSION=0.82
SOURCE=$(LIBNAME)-$(VERSION)
# download location and format
BASE_URL=http://ftp.gnu.org/gnu/libcdio
ARCHIVE=$(SOURCE).tar.gz

RETRIEVE_TOOL=curl
RETRIEVE_TOOL_FLAGS=-Ls --output $(ARCHIVE)
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

# configuration settings
export CFLAGS+=-undefined dynamic_lookup
export CXXFLAGS+=-undefined dynamic_lookup
export LDFLAGS+=-Wl,-read_only_relocs,suppress
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --enable-shared=yes \
  --without-cd-drive --without-cd-info --without-cd-read --without-cd-paranoia --without-cdda-player --without-iso-info --without-iso-read

LIBDYLIB=$(SOURCE)/.libs/(LIBNAME).dylib

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB)

$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	cd $(SOURCE); autoconf
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)

configure:
	cd $(SOURCE); $(CONFIGURE)

install:
	sudo make -C $(SOURCE) install
	sudo cp $(SOURCE)/include/cdio/cdtext.h $(PREFIX)/include/cdio/

clean:
	make -C $(SOURCE) clean

distclean::
	rm -rf $(SOURCE)
