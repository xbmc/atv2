#  A quick and dirty Makefile to configure xbmc
#

include ../config.mk

SOURCE=../../../../
# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
  --with-arch=arm --with-cpu=cortex-a8 --enable-external-python --enable-gles \
  --disable-optical-drive --disable-dvdcss --disable-webserver --disable-optimizations \
  PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
  ac_cv_lib_smbclient_main=yes ac_cv_lib_python2_5_main=yes

all: configure

configure:
	cd $(SOURCE); ./bootstrap
	cd $(SOURCE); $(CONFIGURE)
	#cd $(SOURCE); make xcode_depends
