
SUBDIRS = \
	Backrow \
	gas-preprocessor pkg-config \
	pcre expat gettext readline \
	liblzo liblzo2 freetype2 fontconfig \
	openssl libssh2 curl \
	jpeg tiff libpng libogg \
	libvorbis libmad fribidi libwavpack libmpeg2 \
	faad2 libcdio libsamplerate \
	boost \
	mysqlclient smbclient python5

	#glib2 libmms /

.PHONY: setup subdirs $(SUBDIRS)

all: $(SUBDIRS)

subdirs:
	mkdir -p $(CURDIR)/build/bin
	mkdir -p $(CURDIR)/build/lib
	mkdir -p $(CURDIR)/build/slib
	mkdir -p $(CURDIR)/build/include
	$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

libmms: glib2
gettext: expat
fontconfig: freetype2
libmad: gas-preprocessor


clean:
	for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done

distclean::
	for d in $(SUBDIRS); do $(MAKE) -C $$d distclean; done
	rm -rf build




