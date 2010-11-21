include config.mk

SUBDIRS = \
	gas-preprocessor \
	pcre expat gettext readline \
	liblzo liblzo2 freetype2 fontconfig \
	jpeg tiff libpng libogg \
	libvorbis libmad fribidi \
	curl glib2 libwavpack libmpeg2 \
	libcdio openssl libsamplerate \
	boost \
	mysqlclient smbclient python5 \
	

.PHONY: setup subdirs $(SUBDIRS)

all: $(SUBDIRS)

subdirs:
	$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
	$(MAKE) -C $@ install

libmms: glib2
gettext: expat
fontconfig: freetype2
libmad: gas-preprocessor


clean:
	for d in $(DEPLIBS); $(MAKE) -C $$d clean; done

distclean::
	for d in $(DEPLIBS); rm -rf $(SOURCE); done
	rm -rf build




