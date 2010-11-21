include config.mk

DEPLIBS = \
	pcre \
	expat \
	gettext \
	readline

# liblzo liblzo2
# freetype2 fontconfig
# boost curl
# jpeg tiff libpng libogg
# libvorbis libmad fribidi
# glib2 libmms
# libwavpack libmpeg2
# libcdio openssl libssh libsamplerate

#	samba
#	smbclient
#	mysqlclient
#	python5

.PHONY: $(DEPLIBS)

pcre:
	make -C pcre
	make -C pcre install

expat:
	make -C expat
	make -C expat install

gettext:
	make -C gettext
	make -C gettext install

readline:
	make -C readline
	make -C readline install

#clean:
#for d in $(DEPLIBS); do if test -f $$d/Makefile; then $(MAKE) -C $$d clean; fi; done
#rm -rf ../build/xbmc-ios-4.1

#distclean::
#	rm -rf $(SOURCE)




