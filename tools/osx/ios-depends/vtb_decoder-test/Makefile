#
#  Apple Media Services test project.
#  Usage:
#    ./ams_test --input media/SpeedRacer.mov
#
# this makefile matches xcode project build (i.e. armv7 build)

include ../config.mk

BIN = vtb_decoder

INCLUDES+= -I./ffmpeg

LDFLAGS+= -lbz2 -lz
LDFLAGS+= -F/$(SDKROOT)/System/Library/Frameworks
LDFLAGS+= -framework CoreFoundation -framework CoreVideo -framework CoreMedia
LDFLAGS+= -F/$(SDKROOT)/System/Library/PrivateFrameworks
LDFLAGS+= -framework VideoToolbox
CPPFLAGS+= -g -O0 -Wall
CPPFLAGS+= ${INCLUDES}

SRCFILES = vtb_decoder.cpp \
           file_reader_util.cpp \
           bitstream_converter.cpp \
           ffmpeg_file_protocol.cpp

OBJFILES = ${SRCFILES:.cpp=.o}

FFMPEG_LIBS = ffmpeg/libavutil/libavutil.a \
              ffmpeg/libavcodec/libavcodec.a \
              ffmpeg/libavformat/libavformat.a \
              

all:: ffmpeg-libs $(BIN)

$(BIN): $(OBJFILES)
	$(CXX)  $(LDFLAGS) -o $(BIN) $(OBJFILES) $(FFMPEG_LIBS)

ffmpeg-libs: ffmpeg/config.h
	make -C ffmpeg

ffmpeg/config.h:
	svn export svn://svn.ffmpeg.org/ffmpeg/trunk ffmpeg
	cd ffmpeg && patch -p1 <../03_mpeg4_video_to_elementary_stream.patch
	cd ffmpeg && patch -p1 <../04_vc1_bsfs.patch
	cd ffmpeg && ./configure \
      --extra-cflags="$(CFLAGS) -w -D_DARWIN_C_SOURCE -Dclosesocket=close -Dattribute_deprecated=" \
      --extra-ldflags="-Wl,-undefined,dynamic_lookup" \
      --enable-cross-compile \
      --arch=arm \
      --cpu=cortex-a8 \
      --target-os=darwin \
      --disable-armv5te --disable-armv6t2 \
      --disable-amd3dnow \
      --enable-static \
      --disable-shared \
      --disable-muxers \
      --enable-muxer=spdif \
      --enable-muxer=adts \
      --disable-encoders \
      --enable-encoder=ac3 \
      --enable-encoder=aac \
      --disable-devices \
      --disable-ffplay \
      --disable-ffserver \
      --disable-ffmpeg \
      --enable-shared \
      --disable-decoder=mpeg_xvmc \
      --enable-postproc \
      --enable-gpl \
      --enable-protocol=http \
      --enable-pthreads \
      --enable-memalign-hack \
      --cc=$(CC) \
      --as="gas-preprocessor.pl $(CC)"

ffmpeg-clean:
	make -C ffmpeg clean

clean::
	rm -f $(BIN) $(OBJFILES)

distclean:: clean
	rm -rf ffmpeg	



