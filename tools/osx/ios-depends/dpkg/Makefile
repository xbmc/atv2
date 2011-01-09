#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

# lib name, version
APPNAME=dpkg
VERSION=1.14.30
SOURCE=$(APPNAME)_$(VERSION)
SOURCE_DEBIAN=$(APPNAME)-$(VERSION)
# download location and format
BASE_URL=http://ftp.de.debian.org/debian/pool/main/d/dpkg/
ARCHIVE=$(SOURCE).tar.gz

RETRIEVE_TOOL=/usr/bin/curl
RETRIEVE_TOOL_FLAGS=-Ls --output $(ARCHIVE)
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

export LIBTOOL=builds/unix/libtool
CONFIGURE=./configure --prefix=$(PREFIX) --enable-static --disable-shared

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(APPNAME) .installed

$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)
	echo $(ARCHIVE) > .gitignore

$(SOURCE_DEBIAN): $(ARCHIVE)
	-rm -rf $(SOURCE_DEBIAN)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	echo $(SOURCE_DEBIAN) >> .gitignore
	cd $(SOURCE_DEBIAN); patch -p1 -i ../dpkg-deb.patch; $(CONFIGURE)

$(APPNAME): $(SOURCE_DEBIAN)
	make -C $(SOURCE_DEBIAN)/lib CFLAGS="" LDFLAGS=""
	make -C $(SOURCE_DEBIAN)/libcompat CFLAGS="" LDFLAGS=""
	make -C $(SOURCE_DEBIAN)/src CFLAGS="" LDFLAGS=""
	make -C $(SOURCE_DEBIAN)/dpkg-deb CFLAGS="" LDFLAGS=""

.installed:
	#make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm  .installed

distclean::
	rm -rf $(SOURCE) .installed

export COPYFILE_DISABLE
export COPY_EXTENDED_ATTRIBUTES_DISABLE

package:
	sudo chmod -R 0755 ${PWD}/XBMC/DEBIAN
	sudo chown -R root:wheel ${PWD}/XBMC/DEBIAN
	sudo chmod -R 775 ${PWD}/XBMC/Applications/XBMC.app
	sudo chown -R root:wheel ${PWD}/XBMC/Applications/XBMC.app
	sudo ./$(SOURCE_DEBIAN)/dpkg-deb/dpkg-deb -Zgzip -b XBMC

