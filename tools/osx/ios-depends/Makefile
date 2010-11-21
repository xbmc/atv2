include config.mk

SUBDIRS = \
	pcre expat gettext readline \
	liblzo liblzo2 freetype2 fontconfig \
	boost curl \
	jpeg tiff libpng libogg \
	libvorbis libmad fribidi \
	glib2 libmms \
	libwavpack libmpeg2 \
	libcdio openssl libssh libsamplerate

#	samba
#	smbclient
#	mysqlclient
#	python5

.PHONY: setup subdirs $(SUBDIRS)

all: $(SUBDIRS)

subdirs:
	$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
	$(MAKE) -C $@ install

gettext: expat
fontconfig: freetype2


clean:
	for d in $(DEPLIBS); $(MAKE) -C $$d clean; done

distclean::
	for d in $(DEPLIBS); rm -rf $(SOURCE); done
	rm -rf build




