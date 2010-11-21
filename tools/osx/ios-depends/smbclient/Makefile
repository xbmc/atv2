#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

SOURCE=samba_3.2.7-4_iphoneos-arm
# download location and format
# wget http://apt.saurik.com/debs/samba_3.2.7-4_iphoneos-arm.deb

ARCHIVE=$(SOURCE).deb

ARCHIVE_TOOL=ar
ARCHIVE_TOOL_FLAGS=-x

all: $(SOURCE)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE); mkdir $(SOURCE)
	cd $(SOURCE); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) ../$(ARCHIVE)
	cd $(SOURCE); mkdir data; tar -xzf data.tar.gz -C data

install:
	cp -rf $(SOURCE)/data/usr/* /usr/local/xbmc-ios-4.1/ 
	install_name_tool -change bin/libtalloc.dylib.1 $(PREFIX)/lib/libtalloc.dylib.1 \
		-id $(PREFIX)/lib/libsmbclient.dylib $(PREFIX)/lib/libsmbclient.dylib
        install_name_tool -change bin/libtdb.dylib.1 $(PREFIX)/lib/libtdb.dylib.1 \
                -id $(PREFIX)/lib/libsmbclient.dylib $(PREFIX)/lib/libsmbclient.dylib
clean:

distclean::
	rm -rf $(SOURCE)

