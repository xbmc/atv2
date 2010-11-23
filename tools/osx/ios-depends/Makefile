include config.mk

SUBDIRS = \
	Backrow \
	gas-preprocessor \
	pcre expat gettext readline \
	liblzo liblzo2 freetype2 fontconfig \
	openssl libssh2 curl \
	jpeg tiff libpng libogg \
	libvorbis libmad fribidi \
	glib2 libwavpack libmpeg2 \
	libcdio libsamplerate \
	boost \
	mysqlclient smbclient python5 \
	

.PHONY: setup subdirs $(SUBDIRS)

all: $(SUBDIRS)

subdirs:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/slib
	mkdir -p build/include
	$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

libmms: glib2
gettext: expat
fontconfig: freetype2
libmad: gas-preprocessor


clean:
	for d in $(DEPLIBS); $(MAKE) -C $$d clean; done

distclean::
	for d in $(DEPLIBS); rm -rf $(SOURCE); done
	rm -rf build




