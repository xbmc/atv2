#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

LIBNAME=mysql
VERSION=5.0.89
SOURCE=$(LIBNAME)-$(VERSION)
# download location and format
# http://mysql.mirrors.pair.com/Downloads/MySQL-5.0
# build native on atv2 using iphone-gcc
# ./configure --build=arm-apple-darwin10.3.1 --without-server --enable-thread-safe-client -with-extra-charsets=complex --without-bench --without-docs --without-man
# In “my_net.h”, comment out line 47 and 49
#  //#include <netinet/in_systm.h>
#  #include <netinet/in.h>
#  //#include <netinet/ip.h>
# In “my_global.h”, change the line 129 to
#  #  if defined(__i386__) || defined(__ppc__) || defined(__arm__)
# In “my_config.h”, undef HAVE_FDATASYNC at line 63
#  /* #define HAVE_FDATASYNC 1 */

ARCHIVE=$(SOURCE)-ios.tar.gz
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

all: $(SOURCE) .installed

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	echo $(SOURCE) > .gitignore

.installed:
	cp -rf $(SOURCE)/* $(PREFIX)
	install_name_tool -id $(PREFIX)/lib/mysql/libmysqlclient.15.dylib $(PREFIX)/lib/mysql/libmysqlclient.dylib
	touch $@

clean:
distclean::
	rm -rf $(SOURCE) .installed

