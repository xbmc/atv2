// Copyright (c) 2010 TeamXBMC. All rights reserved.
// undocumented linker flag -no_compact_linkedit

#include <unistd.h>
#include <queue>
#include <vector>
#include <semaphore.h>
#include <mach/mach_time.h>

#include <CoreVideo/CoreVideo.h>
#include <CoreMedia/CoreMedia.h>

#include "ffmpeg_common.h"
#include "ffmpeg_file_protocol.h"
#include "file_reader_util.h"

//-----------------------------------------------------------------------------------
// -F/System/Library/PrivateFrameworks -framework VideoToolbox
#if defined(__cplusplus)
extern "C"
{
#endif
    
#pragma pack(push, 4)

typedef uint32_t VTFormatId;

typedef CFTypeRef VTDecompressionSessionRef;
typedef struct _VTDecompressionOutputCallback VTDecompressionOutputCallback;

typedef void (*VTDecompressionOutputCallbackFunc) (void *data, CFDictionaryRef unk1,
  OSStatus result, uint32_t unk2, CVBufferRef cvbuf);

enum _VTFormat
{
  kVTFormatH264 = 'avc1', // kCMVideoCodecType_H264
  kVTFormatJPEG = 'jpeg'  // kCMVideoCodecType_JPEG
  //kCMVideoCodecType_JPEG             = 'jpeg',
  //kCMVideoCodecType_H264             = 'avc1', // MPEG-4 Part 10, Advanced Video 
  //kCMVideoCodecType_MPEG4Video       = 'mp4v', // MPEG-4 Part 2 video format.
  //kCMVideoCodecType_MPEG2Video       = 'mp2v',
};
enum {
  kVDADecoderDecoderFlags_DontEmitFrame = 1 << 0
};
struct _VTDecompressionOutputCallback
{
  VTDecompressionOutputCallbackFunc func;
  void *data;
};

extern CFStringRef kVTVideoDecoderSpecification_EnableSandboxedVideoDecoder;

extern OSStatus VTDecompressionSessionCreate(
  CFAllocatorRef allocator,
  CMFormatDescriptionRef videoFormatDescription,
  CFTypeRef sessionOptions,
  CFDictionaryRef destinationPixelBufferAttributes,
  VTDecompressionOutputCallback *outputCallback,
  VTDecompressionSessionRef *session);

extern OSStatus VTDecompressionSessionDecodeFrame(
  VTDecompressionSessionRef session,
  CMSampleBufferRef sbuf,
  uint32_t decoderFlags, CFDictionaryRef frameInfo, uint32_t unk1);

extern void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session);
extern void VTDecompressionSessionRelease(VTDecompressionSessionRef session);
extern VTDecompressionSessionRef VTDecompressionSessionRetain(VTDecompressionSessionRef session);
extern OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session);
//-----------------------------------------------------------------------------------
// CoreMedia.framework (private)
extern OSStatus FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
  CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
  UInt32 atomId, const UInt8 *data, CFIndex len, CMFormatDescriptionRef *formatDesc);
extern void FigFormatDescriptionRelease(CMFormatDescriptionRef desc);
extern CMSampleBufferRef FigSampleBufferRetain(CMSampleBufferRef buf);
extern void FigSampleBufferRelease(CMSampleBufferRef buf);
extern void FigBlockBufferRelease(CMBlockBufferRef buf);
//-----------------------------------------------------------------------------------
#pragma pack(pop)
    
#if defined(__cplusplus)
}
#endif

// tracks a frame in and output queue in display order
typedef struct frame_queue {
  int64_t             frametime;
  CVPixelBufferRef    frame;
  struct frame_queue  *nextframe;
} frame_queue;


// AppContext - Application state
typedef struct
{
  frame_queue       *display_queue; // display-order queue - next display frame is always at the queue head
  int32_t           queue_depth;    // we will try to keep the queue depth around 10 frames
  pthread_mutex_t   queue_mutex;    // mutex protecting queue manipulation

  SInt32            sourceWidth;
  SInt32            sourceHeight;
  OSType            sourceFormat;
  FFmpegFileReader  *demuxer;
  AVCodecContext    *codec_context;
  VTFormatId        format_id;
  CMFormatDescriptionRef fmt_desc;
  VTDecompressionSessionRef session;
} AppContext;

/*
// silly ffmpeg, this should be in libavcodec.a
void av_free_packet(AVPacket *pkt) 
{ 
   if (pkt) { 
     if (pkt->destruct) pkt->destruct(pkt); 
     pkt->data = NULL; pkt->size = 0; 
   } 
}
*/

/* g_signal_abort is set to 1 in term/int signal handler */
static unsigned int g_signal_abort = 0;
static void signal_handler(int iSignal)
{
  g_signal_abort = 1;
  printf("Terminating - Program received %s signal\n", \
    (iSignal == SIGINT? "SIGINT" : (iSignal == SIGTERM ? "SIGTERM" : "UNKNOWN")));
}

uint64_t CurrentHostCounter(void)
{
  uint64_t absolute_nano;

  static mach_timebase_info_data_t timebase_info;
  if (timebase_info.denom == 0)
  {
    // Zero-initialization of statics guarantees that denom will be 0 before
    // calling mach_timebase_info.  mach_timebase_info will never set denom to
    // 0 as that would be invalid, so the zero-check can be used to determine
    // whether mach_timebase_info has already been called.  This is
    // recommended by Apple's QA1398.
    mach_timebase_info(&timebase_info);
  }

  // mach_absolute_time is it when it comes to ticks on the Mac.  Other calls
  // with less precision (such as TickCount) just call through to
  // mach_absolute_time.

  // timebase_info converts absolute time tick units into nanoseconds.  
  // to microseconds up front to stave off overflows.
  absolute_nano = (mach_absolute_time() * timebase_info.numer) / timebase_info.denom;

  // Don't bother with the rollover handling that the Windows version does.
  // With numer and denom = 1 (the expected case), the 64-bit absolute time
  // reported in nanoseconds is enough to last nearly 585 years.
  return( (uint64_t)absolute_nano);
}

uint64_t CurrentHostFrequency(void)
{
  return( (uint64_t)1000000000L );
}

void
CFDictionarySetSInt32(CFMutableDictionaryRef dict, CFStringRef key, int32_t value)
{
  CFNumberRef number;

  number = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
  CFDictionarySetValue(dict, key, number);
  CFRelease(number);
}

// example helper function that wraps a time into a dictionary
static CFDictionaryRef MakeDictionaryWithDisplayTime(int64_t inFrameDisplayTime)
{
  CFStringRef key = CFSTR("MyFrameDisplayTimeKey");
  CFNumberRef value = CFNumberCreate(
    kCFAllocatorDefault,
    kCFNumberSInt64Type,
    &inFrameDisplayTime);

  CFDictionaryRef dict = CFDictionaryCreate(
    kCFAllocatorDefault,
    (const void **)&key,
    (const void **)&value,
    1,
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);
    
    CFRelease(value);

  return dict;
}

// example helper function to extract a time from our dictionary
static int64_t GetFrameDisplayTimeFromDictionary(CFDictionaryRef inFrameInfoDictionary)
{
  CFNumberRef timeNumber = NULL;
  int64_t outValue = 0;

  if (NULL == inFrameInfoDictionary) return 0;

  timeNumber = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("MyFrameDisplayTimeKey"));
  if (timeNumber)
    CFNumberGetValue(timeNumber, kCFNumberSInt64Type, &outValue);

  return outValue;
}

CMFormatDescriptionRef
vtdec_create_format_description(AppContext *ctx)
{
  CMFormatDescriptionRef fmt_desc;
  OSStatus status;

  status = CMVideoFormatDescriptionCreate(
    NULL,             // CFAllocatorRef allocator
    ctx->format_id,
    ctx->sourceWidth,
    ctx->sourceHeight,
    NULL,             // CFDictionaryRef extensions
    &fmt_desc);

  if (!status)
    return fmt_desc;
  else
    return NULL;
}

CMFormatDescriptionRef
vtdec_create_format_description_from_codec_data(AppContext *ctx)
{
  CMFormatDescriptionRef fmt_desc;
  OSStatus status;

  status = FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
    NULL,             // CFAllocatorRef allocator ???
    ctx->format_id,
    ctx->sourceWidth,
    ctx->sourceHeight,
    'avcC',
    ctx->codec_context->extradata,
    ctx->codec_context->extradata_size,
    &fmt_desc);

  if (!status)
    return fmt_desc;
  else
    return NULL;
}

void
vtdec_output_frame(void *data, CFDictionaryRef frameInfo, OSStatus result, uint32_t infoFlags, CVBufferRef cvbuf)
{
  if (!result) {
    OSType format_type = CVPixelBufferGetPixelFormatType(cvbuf);
    int width, height;
    if (CVPixelBufferIsPlanar(cvbuf) ) {
      width  = CVPixelBufferGetWidthOfPlane(cvbuf, 0);
      height = CVPixelBufferGetHeightOfPlane(cvbuf, 0);
    } else {
      width  = CVPixelBufferGetWidth(cvbuf);
      height = CVPixelBufferGetHeight(cvbuf);
    }
    uint64_t pts = GetFrameDisplayTimeFromDictionary(frameInfo);
    printf("format(%c%c%c%c), width(%d), height(%d), cvbuf(%p), frameInfo(%p), infoFlags(%d), pts(%llu)\n",
      (int)((format_type >>24) & 0xff), (int)((format_type >> 16) & 0xff), (int)((format_type >>8) & 0xff), (int)(format_type & 0xff),
      width, height, cvbuf, frameInfo, infoFlags, pts);
  }
}

VTDecompressionSessionRef
vtdec_create_session(AppContext *ctx)
{
  VTDecompressionSessionRef session = NULL;
  CFMutableDictionaryRef destinationPixelBufferAttributes;
  VTDecompressionOutputCallback outputCallback;
  OSStatus status;

  destinationPixelBufferAttributes = CFDictionaryCreateMutable(
    NULL, // CFAllocatorRef allocator
    0,    // CFIndex capacity
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);

  // The recommended pixel format choices are 
  //  kCVPixelFormatType_32BGRA or kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferPixelFormatTypeKey, 
    kCVPixelFormatType_32BGRA);
    //kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferWidthKey,
    ctx->sourceWidth);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferHeightKey,
    ctx->sourceHeight);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferBytesPerRowAlignmentKey,
    2 * ctx->sourceWidth);

  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kVTVideoDecoderSpecification_EnableSandboxedVideoDecoder, 
    TRUE);
    

  outputCallback.func = vtdec_output_frame;
  outputCallback.data = ctx;

  status = VTDecompressionSessionCreate(
    NULL, // CFAllocatorRef allocator
    ctx->fmt_desc,
    NULL, // CFTypeRef sessionOptions
    destinationPixelBufferAttributes,
    &outputCallback,
    &session);
  if (status)
    printf("VTDecompressionSessionCreate failed %d\n", (int)status);

  CFRelease(destinationPixelBufferAttributes);

  printf("vtdec session created\n");
  return session;
}

void
vtdec_destroy_session(VTDecompressionSessionRef *session)
{
  VTDecompressionSessionInvalidate(*session);
  VTDecompressionSessionRelease(*session);
  *session = NULL;
  printf("vtdec session destroyed\n");
}

CMSampleBufferRef
vtdec_sample_buffer_from(AppContext *ctx, void *demux_buff, size_t demux_size)
{
  OSStatus status;
  CMBlockBufferRef newBBufOut = NULL;
  CMSampleBufferRef sBufOut = NULL;

  status = CMBlockBufferCreateWithMemoryBlock(
    NULL,             // CFAllocatorRef structureAllocator
    demux_buff,       // void *memoryBlock
    demux_size,       // size_t blockLengt
    kCFAllocatorNull, // CFAllocatorRef blockAllocator
    NULL,             // const CMBlockBufferCustomBlockSource *customBlockSource
    0,                // size_t offsetToData
    demux_size,       // size_t dataLength
    FALSE,            // CMBlockBufferFlags flags
    &newBBufOut);     // CMBlockBufferRef *newBBufOut

  if (!status) {
    status = CMSampleBufferCreate(
      NULL,           // CFAllocatorRef allocator
      newBBufOut,     // CMBlockBufferRef dataBuffer
      TRUE,           // Boolean dataReady
      0,              // CMSampleBufferMakeDataReadyCallback makeDataReadyCallback
      0,              // void *makeDataReadyRefcon
      ctx->fmt_desc,  // CMFormatDescriptionRef formatDescription
      1,              // CMItemCount numSamples
      0,              // CMItemCount numSampleTimingEntries
      NULL,           // const CMSampleTimingInfo *sampleTimingArray
      0,              // CMItemCount numSampleSizeEntries
      NULL,           // const size_t *sampleSizeArray
      &sBufOut);      // CMSampleBufferRef *sBufOut
  }

  FigBlockBufferRelease(newBBufOut);

  return sBufOut;
}

OSStatus
vtdec_decode_buffer(AppContext *ctx, void *demux_buff, size_t demux_size, uint64_t dts, uint64_t pts)
{
  CMSampleBufferRef sample_buff;
  CFDictionaryRef frameinfo;
  uint32_t decoderFlags;
  OSStatus status;

  sample_buff = vtdec_sample_buffer_from(ctx, demux_buff, demux_size);

  frameinfo = MakeDictionaryWithDisplayTime(pts);
  decoderFlags = 0;
  status = VTDecompressionSessionDecodeFrame(ctx->session, sample_buff, decoderFlags, frameinfo, 0);
  if (status != 0) {
    printf("VTDecompressionSessionDecodeFrame returned %d\n", (int)status);
  }

  status = VTDecompressionSessionWaitForAsynchronousFrames(ctx->session);
  if (status != 0) {
    printf("VTDecompressionSessionWaitForAsynchronousFrames returned %d\n", (int)status);
  }

  CFRelease(frameinfo);
  FigSampleBufferRelease(sample_buff);

  return status;
}

int main (int argc, char * const argv[])
{
  std::string input_filename;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if (strncasecmp(argv[i], "--input", 7) == 0) {
        // check the next arg with the proper value.
        int next = i + 1;
        if (next < argc) {
          input_filename = argv[next];
          i++;
        }
      } else if (strncasecmp(argv[i], "-h", 2) == 0 || strncasecmp(argv[i], "--help", 6) == 0) {
        printf("Usage: %s [OPTIONS]...\n", argv[0]);
        printf("Arguments:\n");
        printf("  --input <filename> \tInput video filename\n");
        exit(0);
      }
    }
  }
  if (input_filename.empty()) {
    printf("no input file specified\n");
    exit(0);
  }

  // install signal handlers
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // initialize App contex.
  AppContext ctx;
  memset(&ctx, 0, sizeof(ctx));

  // create the ffmepg file reader/demuxer
  ctx.demuxer = new FFmpegFileReader(input_filename.c_str());
  if (!ctx.demuxer->Initialize()) {
    fprintf(stderr, "ERROR: Can't initialize FFmpegFileReader\n");
    goto fail;
  }
  
  ctx.codec_context = ctx.demuxer->GetCodecContext();
  if (!ctx.codec_context) {
    fprintf(stderr, "ERROR: Invalid FFmpegFileReader Codec Context\n");
    goto fail;
  }
  
  if ( ctx.codec_context->codec_id != CODEC_ID_H264) {
    fprintf(stderr, "ERROR: Invalid FFmpegFileReader Codec Format (not h.264)\n");
    goto fail;
  }
  
  ctx.sourceWidth = ctx.codec_context->width;
  ctx.sourceHeight = ctx.codec_context->height;
  printf("video width(%d), height(%d), extradata_size(%d)\n",
    (int)ctx.sourceWidth, (int)ctx.sourceHeight, ctx.codec_context->extradata_size);
  
  // initialize video decoder.
  ctx.format_id = kVTFormatH264;
  if (ctx.codec_context->extradata_size) {
    ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx);
  } else {
    ctx.fmt_desc = vtdec_create_format_description(&ctx);
  }
  
  ctx.session = vtdec_create_session(&ctx);
  if (!ctx.session) {
    fprintf(stderr, "ERROR: vtdec_create_session\n");
    goto fail;
  }
  
  
  {
    OSStatus status;
    int frame_count, byte_count, total = 0;
    uint64_t bgn, end;
    uint8_t* data;
    uint64_t dts, pts;

    ctx.demuxer->Read(&data, &byte_count, &dts, &pts);
    printf("byte_count(%d), dts(%llu), pts(%llu)\n", byte_count, dts, pts);
    if (!byte_count) {
      fprintf(stderr, "ERROR: Zero bytes read from input\n");
      goto fail;
    }

    usleep(10000);
    frame_count = 0;
    while (!g_signal_abort && byte_count && (frame_count < 300)) {
      bgn = CurrentHostCounter() * 1000 / CurrentHostFrequency();
      status = vtdec_decode_buffer(&ctx, data, byte_count, dts, pts);
      free(data);
      end = CurrentHostCounter() * 1000 / CurrentHostFrequency();
      fprintf(stdout, "decode time(%llu)\n", end-bgn);
      frame_count++;
      usleep(10000);
      ctx.demuxer->Read(&data, &byte_count, &dts, &pts);
      total += byte_count;
      /*
      if (byte_count) {
        fprintf(stdout, "frame_count(%d), total(%d) byte_count(%d), dts(%llu), pts(%llu)\n",
          frame_count, total, byte_count, dts, pts);
      }
      */
    }
  }

fail:
    vtdec_destroy_session(&ctx.session);
    FigFormatDescriptionRelease(ctx.fmt_desc);

  return 0;
}
