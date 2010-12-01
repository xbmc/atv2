// Copyright (c) 2010 TeamXBMC. All rights reserved.
// undocumented linker flag -no_compact_linkedit

#include <unistd.h>
#include <queue>
#include <vector>
#include <semaphore.h>

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

struct _VTDecompressionOutputCallback
{
  VTDecompressionOutputCallbackFunc func;
  void *data;
};

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
  uint32_t unk1, uint32_t unk2, uint32_t unk3);

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

  return CFDictionaryCreate(
    kCFAllocatorDefault,
    (const void **)&key,
    (const void **)&value,
    1,
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);
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
  //AppContext *ctx = (AppContext*)data;

  if (!result) {
    OSType format_type = CVPixelBufferGetPixelFormatType(cvbuf);
    int width = CVPixelBufferGetWidthOfPlane(cvbuf, 0);
    int height = CVPixelBufferGetHeightOfPlane(cvbuf, 0);
    printf("buffer format(0x%x), width(%d), height(%d), (%p), (%d)\n",
      (unsigned int)format_type, width, height, frameInfo, infoFlags);
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

  // On iPhone 3G, the recommended pixel format choices are 
  //  kCVPixelFormatType_422YpCbCr8 or kCVPixelFormatType_32BGRA.
  //  TODO: figure out what we need.
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferPixelFormatTypeKey, 
    kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferWidthKey,
    ctx->sourceWidth);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferHeightKey,
    ctx->sourceHeight);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferBytesPerRowAlignmentKey,
    2 * ctx->sourceWidth);

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

  return session;
}

void
vtdec_destroy_session(VTDecompressionSessionRef *session)
{
  VTDecompressionSessionInvalidate(*session);
  VTDecompressionSessionRelease(*session);
  *session = NULL;
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
vtdec_decode_buffer(AppContext *ctx, void *demux_buff, size_t demux_size)
{
  CMSampleBufferRef sample_buff;
  OSStatus status;

  sample_buff = vtdec_sample_buffer_from(ctx, demux_buff, demux_size);

  status = VTDecompressionSessionDecodeFrame(ctx->session, sample_buff, 0, 0, 0);
  if (status != 0) {
    printf("VTDecompressionSessionDecodeFrame returned %d\n", (int)status);
  }

  status = VTDecompressionSessionWaitForAsynchronousFrames(ctx->session);
  if (status != 0) {
    printf("VTDecompressionSessionWaitForAsynchronousFrames returned %d\n", (int)status);
  }

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
  printf("video width(%d), height(%d)\n", (int)ctx.sourceWidth, (int)ctx.sourceHeight);
  
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
  printf("vtdec session created\n");
  
  
  {
    OSStatus status;
    int frame_count, byte_count, total = 0;
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
    while (!g_signal_abort && byte_count && (frame_count < 1000)) {
      status = vtdec_decode_buffer(&ctx, data, byte_count);
      free(data);
      frame_count++;
      usleep(10000);
      ctx.demuxer->Read(&data, &byte_count, &dts, &pts);
      printf("byte_count(%d), dts(%llu), pts(%llu)\n", byte_count, dts, pts);
      total += byte_count;
      if (byte_count) {
        fprintf(stdout, "Read from input: total bytes(%d) byte_count(%d), dts(%llu), pts(%llu)\n",
          total, byte_count, dts, pts);
      }
    }
  }

fail:
    vtdec_destroy_session(&ctx.session);
    FigFormatDescriptionRelease(ctx.fmt_desc);

  return 0;
}
