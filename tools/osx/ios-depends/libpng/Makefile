#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

# lib name, version
LIBNAME=libpng
#VERSION=1.4.4
VERSION=1.2.38
SOURCE=$(LIBNAME)-$(VERSION)
# download location and format
BASE_URL=ftp://ftp.simplesystems.org/pub/libpng/png/src
ARCHIVE=$(SOURCE).tar.gz

RETRIEVE_TOOL=/usr/bin/curl
RETRIEVE_TOOL_FLAGS=-Ls --output $(ARCHIVE)
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

# configuration settings
export CFLAGS+=-undefined dynamic_lookup
export CXXFLAGS+=-undefined dynamic_lookup
export LDFLAGS+=-Wl,-read_only_relocs,suppress
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --enable-shared=yes 

LIBDYLIB=$(SOURCE)/.libs/$(LIBNAME).dylib

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB)

$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	echo $(ARCHIVE) > .gitignore
	echo $(SOURCE) >> .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)

install:
	make -C $(SOURCE) install

clean:
	make -C $(SOURCE) clean

distclean::
	rm -rf $(SOURCE)
