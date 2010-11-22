#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

SOURCE=samba_3.2.7-4_iphoneos-arm
# download location and format
# wget http://apt.saurik.com/debs/samba_3.2.7-4_iphoneos-arm.deb
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
	cp -rf $(SOURCE)/data/usr/include/* $(PREFIX)/include/
	cp -rf $(SOURCE)/data/usr/samba/bin/* $(PREFIX)/bin
	cp -rf $(SOURCE)/data/usr/samba/lib/* $(PREFIX)/lib
	cp -rf $(SOURCE)/data/usr/samba/sbin/* $(PREFIX)/lib
	ln -s $(PREFIX)/lib/libsmbclient.dylib.0 $(PREFIX)/lib/libsmbclient.dylib
	install_name_tool -change bin/libtalloc.dylib.1 $(PREFIX)/lib/libtalloc.dylib.1 \
		-id $(PREFIX)/lib/libsmbclient.dylib $(PREFIX)/lib/libsmbclient.dylib.0
	install_name_tool -change bin/libtdb.dylib.1 $(PREFIX)/lib/libtdb.dylib.1 \
                -id $(PREFIX)/lib/libsmbclient.dylib $(PREFIX)/lib/libsmbclient.dylib.0
clean:

distclean::
	rm -rf $(SOURCE)

