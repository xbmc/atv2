#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

SOURCE=python_2.5.1-5_iphoneos-arm
# download location and format
# wget http://apt.saurik.com/debs/python_2.5.1-5_iphoneos-arm.deb
BASE_URL=http://apt.saurik.com/debs

RETRIEVE_TOOL=/usr/bin/curl
RETRIEVE_TOOL_FLAGS=-Ls --output $(ARCHIVE)

ARCHIVE=$(SOURCE).deb
ARCHIVE_TOOL=ar
ARCHIVE_TOOL_FLAGS=-x

all: $(SOURCE)

$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE); mkdir $(SOURCE)
	echo $(ARCHIVE) > .gitignore
	echo $(SOURCE) >> .gitignore
	cd $(SOURCE); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) ../$(ARCHIVE)
	cd $(SOURCE); mkdir data; tar -xzf data.tar.gz -C data

install:
	cp -rf $(SOURCE)/data/usr/* $(PREFIX)/
	chmod +w $(PREFIX)/lib/libpython2.5.dylib
	install_name_tool -id $(PREFIX)/lib/libpython2.5.dylib.1.0 $(PREFIX)/lib/libpython2.5.dylib

clean:

distclean::
	rm -rf $(SOURCE)

