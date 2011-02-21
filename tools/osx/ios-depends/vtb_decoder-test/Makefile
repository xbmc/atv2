#
#  Apple VideoToolBox.framework test project.
#  Usage:
#    ./vtb_decoder --input media/SpeedRacer.mov
#

include ../config.mk

# FFmpeg and libswscale are paired yet come from different locations
# a normal FFmpeg checkout will grab the most recent libswscale so if
# you want to checkout a specific FFmpeg version, need to know an use
# the matching libswscale version. There is no automated way to do this.
FFMPEG_VERS = 23596
SWSCALE_VERS = 31391

BIN = vtb_decoder

INCLUDES+= -I./ffmpeg

LDFLAGS+= -lbz2 -lz -lc
LDFLAGS+= -F$(SDKROOT)/System/Library/Frameworks
LDFLAGS+= -framework CoreFoundation -framework CoreVideo -framework CoreMedia
LDFLAGS+= -F$(SDKROOT)/System/Library/PrivateFrameworks
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
	rm -rf ffmpeg
	svn export --revision $(FFMPEG_VERS) --ignore-externals svn://svn.ffmpeg.org/ffmpeg/trunk ffmpeg
	cd ffmpeg && svn export --revision $(SWSCALE_VERS) svn://svn.ffmpeg.org/mplayer/trunk/libswscale libswscale
	cd ffmpeg && patch -p1 <../03_mpeg4_video_to_elementary_stream.patch
	cd ffmpeg && patch -p1 <../04_vc1_bsfs.patch
	cd ffmpeg && ./configure \
      --extra-cflags="$(CFLAGS) -march=armv7-a -w -D_DARWIN_C_SOURCE -Dclosesocket=close -Dattribute_deprecated=" \
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
      --cc=$(CC) \
      --as="gas-preprocessor.pl $(CC)"

ffmpeg-clean:
	make -C ffmpeg clean

clean::
	rm -f $(BIN) $(OBJFILES)

distclean:: clean
	rm -rf ffmpeg	



