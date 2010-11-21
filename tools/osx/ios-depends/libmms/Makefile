#  A quick and dirty Makefile to download/build and install
#
#  Usage:
#    make
#    sudo make install

include ../config.mk

# lib name, version
LIBNAME=libmms
VERSION=0.6
SOURCE=$(LIBNAME)-$(VERSION)
# download location and format
BASE_URL=http://softlayer.dl.sourceforge.net/project/libmms/libmms/0.6
ARCHIVE=$(SOURCE).tar.gz

RETRIEVE_TOOL=/usr/bin/curl
RETRIEVE_TOOL_FLAGS=-Ls --output $(ARCHIVE)
ARCHIVE_TOOL=tar
ARCHIVE_TOOL_FLAGS=xf

# configuration settings
export LDFLAGS+=-Wl,-read_only_relocs,suppress -Wl,-undefined,dynamic_lookup
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --enable-shared=yes lt_cv_sys_lib_dlsearch_path_spec="$(SDKROOT)/usr/lib $(DEVROOT)/usr/lib" lt_cv_sys_lib_search_path_spec=$(SDKROOT)System/Library/Frameworks

LIBDYLIB=$(SOURCE)/.libs/$(LIBNAME).dylib

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB)

$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE)
	cd $(SOURCE); patch -p1 < ../01-libmms-06-off_t.patch
	echo $(ARCHIVE) > .gitignore
	echo $(SOURCE) >> .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)

install:
	make -C $(SOURCE) install

clean:
	make -C $(SOURCE) clean

distclean::
	rm -rf $(SOURCE)
