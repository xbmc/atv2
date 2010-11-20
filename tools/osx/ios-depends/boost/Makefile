#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

LIBNAME=boostoniphone
VERSION=1.44.0
SOURCE=$(LIBNAME)-$(VERSION)
# download location and format
# git clone git://gitorious.org/boostoniphone/boostoniphone.git
ARCHIVE=$(SOURCE).tar.gz
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

LIBDYLIB=$(SOURCE)/prefix/lib/libboost_system.a

CLEAN_FILES=$(SOURCE)

all: $(LIBDYLIB)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	cd $(SOURCE); patch -p0 <../01-target-ios-4.1.patch

$(LIBDYLIB): $(SOURCE)
	cd $(SOURCE); ./boost.sh

install:
	sudo cp -rf $(SOURCE)/prefix/* $(PREFIX) 

clean:
distclean::
	rm -rf $(SOURCE)

