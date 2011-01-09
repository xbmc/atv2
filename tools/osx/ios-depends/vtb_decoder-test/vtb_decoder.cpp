// Copyright (c) 2010 TeamXBMC. All rights reserved.
// undocumented linker flag -no_compact_linkedit

#include <unistd.h>
#include <queue>
#include <vector>
#include <semaphore.h>
#include <mach/mach_time.h>

#include <CoreVideo/CoreVideo.h>
#include <CoreVideo/CVHostTime.h>

#include "ffmpeg_common.h"
#include "ffmpeg_file_protocol.h"
#include "file_reader_util.h"

#if TARGET_OS_IPHONE
#include <CoreMedia/CoreMedia.h>

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

enum _VTStatus
{
  kVTSuccess = 0
};

/*
enum _VTFormat
{
  kCMVideoCodecType_JPEG             = 'jpeg',
  kCMVideoCodecType_H264             = 'avc1', // MPEG-4 Part 10, Advanced Video 
  kCMVideoCodecType_MPEG4Video       = 'mp4v', // MPEG-4 Part 2 video format.
  kCMVideoCodecType_MPEG2Video       = 'mp2v',
};
*/
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

extern OSStatus VTDecompressionSessionCopyProperty(VTDecompressionSessionRef session, CFTypeRef key, void* unk, CFTypeRef * value);
extern OSStatus VTDecompressionSessionCopySupportedPropertyDictionary(VTDecompressionSessionRef session, CFDictionaryRef * dict);
extern OSStatus VTDecompressionSessionSetProperty(VTDecompressionSessionRef session, CFStringRef propName, CFTypeRef propValue);
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
#else
#if defined(__cplusplus)
extern "C"
{
#endif
#pragma pack(push, 4)
//-----------------------------------------------------------------------------------
// CoreMedia.framework (private)
// 10.6+
typedef struct _GstCMApi GstCMApi;
typedef struct _GstCMApiClass GstCMApiClass;

typedef CFTypeRef FigBaseObjectRef;
typedef struct _FigBaseVTable FigBaseVTable;
typedef struct _FigBaseIface FigBaseIface;

typedef CFTypeRef CMFormatDescriptionRef;
typedef struct _CMVideoDimensions CMVideoDimensions;
typedef struct _CMTime CMTime;

typedef CFTypeRef CMBufferQueueRef;
typedef SInt32 CMBufferQueueTriggerCondition;
typedef struct _CMBufferQueueTriggerToken *CMBufferQueueTriggerToken;
typedef CFTypeRef CMSampleBufferRef;
typedef CFTypeRef CMBlockBufferRef;

typedef void (* CMBufferQueueTriggerCallback) (void *triggerRefcon,
    CMBufferQueueTriggerToken triggerToken);
typedef Boolean (* CMBufferQueueValidationCallback) (CMBufferQueueRef queue,
    CMSampleBufferRef buf, void *refCon);

enum _FigMediaType
{
  kFigMediaTypeVideo = 'vide'
};

enum _FigCodecType
{
  kComponentVideoUnsigned           = 'yuvs',
  kFigVideoCodecType_JPEG_OpenDML   = 'dmb1',
  kYUV420vCodecType                 = '420v'
};

enum _CMBufferQueueTriggerCondition
{
  kCMBufferQueueTrigger_WhenDurationBecomesLessThan             = 1,
  kCMBufferQueueTrigger_WhenDurationBecomesLessThanOrEqualTo    = 2,
  kCMBufferQueueTrigger_WhenDurationBecomesGreaterThan          = 3,
  kCMBufferQueueTrigger_WhenDurationBecomesGreaterThanOrEqualTo = 4,
  kCMBufferQueueTrigger_WhenMinPresentationTimeStampChanges     = 5,
  kCMBufferQueueTrigger_WhenMaxPresentationTimeStampChanges     = 6,
  kCMBufferQueueTrigger_WhenDataBecomesReady                    = 7,
  kCMBufferQueueTrigger_WhenEndOfDataReached                    = 8,
  kCMBufferQueueTrigger_WhenReset                               = 9,
  kCMBufferQueueTrigger_WhenBufferCountBecomesLessThan          = 10,
  kCMBufferQueueTrigger_WhenBufferCountBecomesGreaterThan       = 11
};

struct _FigBaseVTable
{
  uint32_t unk;
  FigBaseIface * base;
  void * derived;
};

struct _FigBaseIface
{
  uint32_t unk1;
  uint32_t unk2;
  uint32_t unk3;
  OSStatus (* Invalidate) (FigBaseObjectRef obj);
  OSStatus (* Finalize) (FigBaseObjectRef obj);
  void* unk4;
  OSStatus (* CopyProperty) (FigBaseObjectRef obj, CFTypeRef key, void *unk,
      CFTypeRef * value);
  OSStatus (* SetProperty) (FigBaseObjectRef obj, CFTypeRef key,
      CFTypeRef value);
};

struct _CMVideoDimensions
{
  UInt32 width;
  UInt32 height;
};

struct _CMTime
{
  UInt8 data[24];
};

extern void *CMGetAttachment(CFTypeRef obj, CFStringRef attachmentKey,
      UInt32 * foundWherePtr);

extern void FigFormatDescriptionRelease(CMFormatDescriptionRef desc);
  CMFormatDescriptionRef (* FigFormatDescriptionRetain) (
      CMFormatDescriptionRef desc);
  Boolean (* CMFormatDescriptionEqual) (CMFormatDescriptionRef desc1,
      CMFormatDescriptionRef desc2);
  CFTypeRef (* CMFormatDescriptionGetExtension) (
      const CMFormatDescriptionRef desc, CFStringRef extensionKey);
  UInt32 (* CMFormatDescriptionGetMediaType) (
      const CMFormatDescriptionRef desc);
  UInt32 (* CMFormatDescriptionGetMediaSubType) (
      const CMFormatDescriptionRef desc);

extern OSStatus FigVideoFormatDescriptionCreate(
      CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
      CFDictionaryRef extensions, CMFormatDescriptionRef * desc);
extern OSStatus FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom
      (CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
      UInt32 atomId, const UInt8 * data, CFIndex len,
      CMFormatDescriptionRef * formatDesc);
extern CMVideoDimensions CMVideoFormatDescriptionGetDimensions (
      const CMFormatDescriptionRef desc);

extern CMTime CMTimeMake(int64_t value, int32_t timescale);

extern OSStatus FigSampleBufferCreate(CFAllocatorRef allocator,
      CMBlockBufferRef blockBuf, Boolean unkBool, UInt32 unkDW1, UInt32 unkDW2,
      CMFormatDescriptionRef fmtDesc, UInt32 unkCountA, UInt32 unkCountB,
      const void * unkTimeData, UInt32 unkCountC, const void * unkDWordData,
      CMSampleBufferRef * sampleBuffer);
extern Boolean CMSampleBufferDataIsReady(
      const CMSampleBufferRef buf);
extern CMBlockBufferRef CMSampleBufferGetDataBuffer(
      const CMSampleBufferRef buf);
extern CMFormatDescriptionRef CMSampleBufferGetFormatDescription(
      const CMSampleBufferRef buf);
extern CVImageBufferRef CMSampleBufferGetImageBuffer(
      const CMSampleBufferRef buf);
extern SInt32 CMSampleBufferGetNumSamples(
      const CMSampleBufferRef buf);
extern CFArrayRef CMSampleBufferGetSampleAttachmentsArray(
      const CMSampleBufferRef buf, SInt32 sampleIndex);
extern SInt32 CMSampleBufferGetSampleSize(
      const CMSampleBufferRef buf, SInt32 sampleIndex);
extern void FigSampleBufferRelease(CMSampleBufferRef buf);
extern CMSampleBufferRef FigSampleBufferRetain(CMSampleBufferRef buf);

extern OSStatus FigBlockBufferCreateWithMemoryBlock
      (CFAllocatorRef allocator, void *data, UInt32 size,
      CFAllocatorRef dataAllocator, void *unk1, UInt32 sizeA, UInt32 sizeB,
      Boolean unkBool, CMBlockBufferRef * blockBuffer);
extern SInt32 CMBlockBufferGetDataLength(const CMBlockBufferRef buf);
extern OSStatus CMBlockBufferGetDataPointer(
      const CMBlockBufferRef buf, UInt32 unk1, UInt32 unk2, UInt32 unk3,
      Byte ** dataPtr);
extern void FigBlockBufferRelease(CMBlockBufferRef buf);
extern CMBlockBufferRef FigBlockBufferRetain(CMBlockBufferRef buf);

extern CMSampleBufferRef CMBufferQueueDequeueAndRetain
      (CMBufferQueueRef queue);
extern CFIndex CMBufferQueueGetBufferCount(CMBufferQueueRef queue);
extern OSStatus CMBufferQueueInstallTrigger(CMBufferQueueRef queue,
      CMBufferQueueTriggerCallback triggerCallback, void * triggerRefCon,
      CMBufferQueueTriggerCondition triggerCondition, CMTime triggerTime,
      CMBufferQueueTriggerToken * triggerTokenOut);
extern Boolean CMBufferQueueIsEmpty(CMBufferQueueRef queue);
extern void FigBufferQueueRelease(CMBufferQueueRef queue);
extern OSStatus CMBufferQueueRemoveTrigger(CMBufferQueueRef queue,
      CMBufferQueueTriggerToken triggerToken);
extern OSStatus CMBufferQueueSetValidationCallback(CMBufferQueueRef queue,
      CMBufferQueueValidationCallback func, void *refCon);

extern CFStringRef * kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms;
extern CFStringRef * kCMSampleAttachmentKey_DependsOnOthers;
extern CMTime * kCMTimeInvalid;

//
/*
extern OSStatus FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
  CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
  UInt32 atomId, const UInt8 *data, CFIndex len, CMFormatDescriptionRef *formatDesc);
extern void FigFormatDescriptionRelease(CMFormatDescriptionRef desc);
extern CMSampleBufferRef FigSampleBufferRetain(CMSampleBufferRef buf);
extern void FigSampleBufferRelease(CMSampleBufferRef buf);
extern void FigBlockBufferRelease(CMBlockBufferRef buf);
*/
//-----------------------------------------------------------------------------------
// -F/System/Library/PrivateFrameworks -framework VideoToolbox
typedef uint32_t VTFormatId;

typedef CFTypeRef VTDecompressionSessionRef;
typedef struct _VTDecompressionOutputCallback VTDecompressionOutputCallback;

typedef void (*VTDecompressionOutputCallbackFunc) (void *data, CFDictionaryRef unk1,
  OSStatus result, uint32_t unk2, CVBufferRef cvbuf);

enum _VTFormat
{
  kCMVideoCodecType_JPEG             = 'jpeg',
  kCMVideoCodecType_H264             = 'avc1', // MPEG-4 Part 10, Advanced Video 
  kCMVideoCodecType_MPEG4Video       = 'mp4v', // MPEG-4 Part 2 video format.
  kCMVideoCodecType_MPEG2Video       = 'mp2v',
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

extern OSStatus VTDecompressionSessionCopyProperty(VTDecompressionSessionRef session, CFTypeRef key, void* unk, CFTypeRef * value);
extern OSStatus VTDecompressionSessionCopySupportedPropertyDictionary(VTDecompressionSessionRef session, CFDictionaryRef * dict);
extern OSStatus VTDecompressionSessionSetProperty(VTCompressionSessionRef session, CFStringRef propName, CFTypeRef propValue);
extern void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session);
extern void VTDecompressionSessionRelease(VTDecompressionSessionRef session);
extern VTDecompressionSessionRef VTDecompressionSessionRetain(VTDecompressionSessionRef session);
extern OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session);
//-----------------------------------------------------------------------------------
#pragma pack(pop)
#if defined(__cplusplus)
}
#endif
#endif

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
/* MPEG-4 esds (elementary stream descriptor) */
typedef struct {
  int version;
  long flags;

  uint16_t esid;
  uint8_t stream_priority;

  uint8_t  objectTypeId;
  uint8_t  streamType;
  uint32_t bufferSizeDB;
  uint32_t maxBitrate;
  uint32_t avgBitrate;

  int      decoderConfigLen;
  uint8_t* decoderConfig;
} quicktime_esds_t;

unsigned int descrLength(unsigned int len)
{
  int i;
  for(i=1; len>>(7*i); i++);
  return len + 1 + i;
}

void putDescr(ByteIOContext *pb, int tag, unsigned int size)
{
  int i= descrLength(size) - size - 2;
  put_byte(pb, tag);
  for(; i>0; i--)
    put_byte(pb, (size>>(7*i)) | 0x80);
  put_byte(pb, size & 0x7F);
}

void quicktime_write_esds(ByteIOContext *pb, quicktime_esds_t *esds)
{
  //quicktime_atom_t atom;
  int decoderSpecificInfoLen = esds->decoderConfigLen ? descrLength(esds->decoderConfigLen):0;
  //quicktime_atom_write_header(file, &atom, "esds");

/*
  put_byte(pb, 0);  // Version
  put_be24(pb, 0);  // Flags

  // ES descriptor
  putDescr(pb, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) + descrLength(1));

  put_be16(pb, esds->esid);
  put_byte(pb, esds->stream_priority);
  // DecoderConfig descriptor
  putDescr(pb, 0x04, 13 + esds->decoderConfigLen);
  // Object type indication
  put_byte(pb, esds->objectTypeId); // objectTypeIndication
  put_byte(pb, esds->streamType);   // streamType

  put_be24(pb, esds->bufferSizeDB); // buffer size
  put_be32(pb, esds->maxBitrate);   // max bitrate
  put_be32(pb, esds->avgBitrate);   // average bitrate

  // DecoderSpecific info descriptor
  if (decoderSpecificInfoLen) {
    putDescr(pb, 0x05, esds->decoderConfigLen);
    put_buffer(pb, esds->decoderConfig, esds->decoderConfigLen);
  }
  // SL descriptor
  putDescr(pb, 0x06, 1);
  put_byte(pb, 0x02);
*/

  put_byte(pb, 0);  // Version

  // ES descriptor
  putDescr(pb, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) + descrLength(1));
  put_be16(pb, esds->esid);
  put_byte(pb, 0x00); // flags (= no flags)

  // DecoderConfig descriptor
  putDescr(pb, 0x04, 13 + decoderSpecificInfoLen);

  // Object type indication
  put_byte(pb, esds->objectTypeId);

  // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
  // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
  put_byte(pb, esds->streamType); // flags (0x11 = Visualstream)

  put_be24(pb, esds->bufferSizeDB); // buffer size
  //put_byte(pb,  track->enc->rc_buffer_size>>(3+16));    // Buffersize DB (24 bits)
  //put_be16(pb, (track->enc->rc_buffer_size>>3)&0xFFFF); // Buffersize DB

  put_be32(pb, esds->maxBitrate);   // max bitrate
  put_be32(pb, esds->avgBitrate);   // average bitrate
  //put_be32(pb, FFMAX(track->enc->bit_rate, track->enc->rc_max_rate)); // maxbitrate (FIXME should be max rate in any 1 sec window)
  //if(track->enc->rc_max_rate != track->enc->rc_min_rate || track->enc->rc_min_rate==0)
  //    put_be32(pb, 0); // vbr
  //else
  //    put_be32(pb, track->enc->rc_max_rate); // avg bitrate

  // DecoderSpecific info descriptor
  if (decoderSpecificInfoLen) {
    putDescr(pb, 0x05, esds->decoderConfigLen);
    put_buffer(pb, esds->decoderConfig, esds->decoderConfigLen);
  }

  // SL descriptor
  putDescr(pb, 0x06, 1);
  put_byte(pb, 0x02);

  //quicktime_atom_write_footer(file, &atom);
}

quicktime_esds_t* quicktime_set_esds(const uint8_t * decoderConfig, int decoderConfigLen)
{
  // ffmpeg's codec->avctx->extradata, codec->avctx->extradata_size
  // are decoderConfig/decoderConfigLen
  quicktime_esds_t *esds;

  esds = (quicktime_esds_t*)malloc(sizeof(quicktime_esds_t));
  memset(esds, 0, sizeof(quicktime_esds_t));

  esds->version         = 0;
  esds->flags           = 0;
  
  esds->esid            = 0;
  esds->stream_priority = 0;      // 16 ?
  
  esds->objectTypeId    = 32;     // 32 = CODEC_ID_MPEG4, 33 = CODEC_ID_H264
  // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
  // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
  esds->streamType      = 0x11;
  esds->bufferSizeDB    = 64000;  // Hopefully not important :)
  
  // Maybe correct these later?
  esds->maxBitrate      = 200000; // 0 for vbr
  esds->avgBitrate      = 200000;
  
  esds->decoderConfigLen = decoderConfigLen;
  esds->decoderConfig = (uint8_t*)malloc(esds->decoderConfigLen);
  memcpy(esds->decoderConfig, decoderConfig, esds->decoderConfigLen);
  return esds;
}

void quicktime_esds_dump(quicktime_esds_t * esds)
{
  int i;
  printf("esds: \n");
  printf(" Version:          %d\n",       esds->version);
  printf(" Flags:            0x%06lx\n",  esds->flags);
  printf(" ES ID:            0x%04x\n",   esds->esid);
  printf(" Priority:         0x%02x\n",   esds->stream_priority);
  printf(" objectTypeId:     %d\n",       esds->objectTypeId);
  printf(" streamType:       0x%02x\n",   esds->streamType);
  printf(" bufferSizeDB:     %d\n",       esds->bufferSizeDB);

  printf(" maxBitrate:       %d\n",       esds->maxBitrate);
  printf(" avgBitrate:       %d\n",       esds->avgBitrate);
  printf(" decoderConfigLen: %d\n",       esds->decoderConfigLen);
  printf(" decoderConfig:");
  for(i = 0; i < esds->decoderConfigLen; i++) {
    if(!(i % 16))
      printf("\n ");
    printf("%02x ", esds->decoderConfig[i]);
  }
  printf("\n");
}

// tracks a frame in and output queue in display order
typedef struct frame_queue {
  int64_t             dts;
  int64_t             pts;
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

////////////////////////////////////////////////////////////////////////////////////////////
void fetch_nal(uint8_t *buffer, int buffer_size, int type, uint8_t **obuff, int *osize)
{
    int i;
    uint8_t *data = buffer;
    uint8_t *nal_buffer;
    int nal_idx = 0;
    int nal_len = 0;
    int nal_type = 0;
    int found = 0;
    int done = 0;
    
    printf("Fetching NAL, type %d\n", type);
    for (i = 0; i < buffer_size - 5; i++) {
        if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0 
            && data[i + 3] == 1) {
            if (found == 1) {
                nal_len = i - nal_idx;
                done = 1;
                break;
            }

            nal_type = (data[i + 4]) & 0x1f;
            if (nal_type == type)
            {
                found = 1;
                nal_idx = i + 4;
                i += 4;
            }
        }
    }
    
    /* Check if the NAL stops at the end */
    if (found == 1 && done != 0 && i >= buffer_size - 4) {
        nal_len = buffer_size - nal_idx;
        done = 1;
    }
    
    if (done == 1) {
        printf("Found NAL, bytes [%d-%d] len [%d]\n", nal_idx, nal_idx + nal_len - 1, nal_len);
        nal_buffer = (uint8_t*)malloc(nal_len);
        memcpy(nal_buffer, &data[nal_idx], nal_len);
        *obuff = nal_buffer;
        *osize = nal_len;
        //return nal_buffer;
    } else {
        printf("Did not find NAL type %d\n", type);
        *obuff = NULL;
        *osize = 0;
        //return NULL;
    }
}

#define NAL_LENGTH 4
void h264_generate_avcc_atom_data(uint8_t *buffer, int buffer_size, uint8_t **obuff, int *osize)
{
  uint8_t *avcc = NULL;
  uint8_t *avcc_data = NULL;
  int avcc_len = 7;  // Default 7 bytes w/o SPS, PPS data
  int i;

  uint8_t *sps = NULL;
  int sps_size;
  uint8_t *sps_data = NULL;
  int num_sps=0;

  uint8_t *pps = NULL;
  int pps_size;
  int num_pps=0;

  uint8_t profile;
  uint8_t compatibly;
  uint8_t level;

  // 7 = SPS
  fetch_nal(buffer, buffer_size, 7, &sps, &sps_size);
  if (sps) {
      num_sps = 1;
      avcc_len += sps_size + 2;
      sps_data = sps;

      profile     = sps_data[1];
      compatibly  = sps_data[2];
      level       = sps_data[3]; 
      
      printf("SPS: profile=%d, compatibly=%d, level=%d\n", profile, compatibly, level);
  } else {
      printf("No SPS found\n");

      profile     = 66;   // Default Profile: Baseline
      compatibly  = 0;
      level       = 30;   // Default Level: 3.0
  }
  // 8 = PPS
  fetch_nal(buffer, buffer_size, 8, &pps, &pps_size); 
  if (pps) {
      num_pps = 1;
      avcc_len += pps_size + 2;
  } else {
      printf("No PPS found\n");
  }

  avcc = (uint8_t*)malloc(avcc_len);
  avcc_data = avcc;
  avcc_data[0] = 1;             // [0] 1 byte - version
  avcc_data[1] = profile;       // [1] 1 byte - h.264 stream profile
  avcc_data[2] = compatibly;    // [2] 1 byte - h.264 compatible profiles
  avcc_data[3] = level;         // [3] 1 byte - h.264 stream level
  avcc_data[4] = 0xfc | (NAL_LENGTH-1);  // [4] 6 bits - reserved all ONES = 0xfc
                                // [4] 2 bits - NAL length ( 0 - 1 byte; 1 - 2 bytes; 3 - 4 bytes)
  avcc_data[5] = 0xe0 | num_sps;// [5] 3 bits - reserved all ONES = 0xe0
                                // [5] 5 bits - number of SPS    
  i = 6;
  if (num_sps > 0) {
    avcc_data[i++] = sps_size >> 8;
    avcc_data[i++] = sps_size & 0xff;
    memcpy(&avcc_data[i], sps, sps_size);
    i += sps_size;
    free(sps);
  }
  avcc_data[i++] = num_pps;     // [6] 1 byte  - number of PPS
  if (num_pps > 0) {
    avcc_data[i++] = pps_size >> 8;
    avcc_data[i++] = pps_size & 0xff;
    memcpy(&avcc_data[i], pps, pps_size);
    i += pps_size;
    free(pps);
  }
  *obuff = avcc;
  *osize = avcc_len;
}

/* Transforms from bytestream into packetized stream.
 * It removes any SPS and PPS NALU
 */
void h264_byte_to_bit_transform(uint8_t *buffer, int buffer_size, uint8_t **obuff, int *osize)
{
  uint8_t *data = buffer;
  int i, mark = 0, nal_type = -1, j = 0;
  int size = buffer_size;
  uint8_t *outBuf = (uint8_t*)malloc(size);
  uint8_t *dest = outBuf;

  for (i = 0; i < size - 4; i++) {
    if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
      /* Do not copy if current NAL is nothing, PPS or SPS */
      if (nal_type == -1 || nal_type == 7 || nal_type == 8) {
        /* Mark where next NALU starts */
        mark = i + 3;
      } else {
        // Insert the lenght and the NAL data
        int length = i - mark ;
        int k;
        for (k = (NAL_LENGTH - 1); k >= 0 ; k--) {
            dest[j + k] = length & 0xff;
            length >>= 8;
        }
        memcpy(&dest[j+NAL_LENGTH], &data[mark], i-mark);
        j += NAL_LENGTH + (i-mark);
        mark = i + 3;
      }
      nal_type = (data[i + 3]) & 0x1f;
    }
  }
  if (i == (size - 4)) {
    /* We reach the end of the buffer */
    if (nal_type != -1 && nal_type != 7 && nal_type != 8) {
      // Insert the lenght and the NAL data
      int length = size - mark ;
      int k;
      for (k = (NAL_LENGTH - 1); k >= 0 ; k--) {
          dest[j + k] = length & 0xff;
          length >>= 8;
      }
      memcpy(&dest[j+NAL_LENGTH], &data[mark], size-mark);
      j += NAL_LENGTH + (size-mark);
    }
  }

  *obuff = outBuf;
  *osize = j;
}


////////////////////////////////////////////////////////////////////////////////////////////
// TODO: refactor this so as not to need these ffmpeg routines.
// These are not exposed in ffmpeg's API so we dupe them here.
// AVC helper functions for muxers,
//  * Copyright (c) 2006 Baptiste Coudurier <baptiste.coudurier@smartjog.com>
// This is part of FFmpeg
//  * License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
#define VDA_RB24(x)                          \
  ((((const uint8_t*)(x))[0] << 16) |        \
   (((const uint8_t*)(x))[1] <<  8) |        \
   ((const uint8_t*)(x))[2])

#define VDA_RB32(x)                          \
  ((((const uint8_t*)(x))[0] << 24) |        \
   (((const uint8_t*)(x))[1] << 16) |        \
   (((const uint8_t*)(x))[2] <<  8) |        \
   ((const uint8_t*)(x))[3])

static const uint8_t *my_avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
  const uint8_t *a = p + 4 - ((intptr_t)p & 3);

  for (end -= 3; p < a && p < end; p++)
  {
    if (p[0] == 0 && p[1] == 0 && p[2] == 1)
      return p;
  }

  for (end -= 3; p < end; p += 4)
  {
    uint32_t x = *(const uint32_t*)p;
    if ((x - 0x01010101) & (~x) & 0x80808080) // generic
    {
      if (p[1] == 0)
      {
        if (p[0] == 0 && p[2] == 1)
          return p;
        if (p[2] == 0 && p[3] == 1)
          return p+1;
      }
      if (p[3] == 0)
      {
        if (p[2] == 0 && p[4] == 1)
          return p+2;
        if (p[4] == 0 && p[5] == 1)
          return p+3;
      }
    }
  }

  for (end += 3; p < end; p++)
  {
    if (p[0] == 0 && p[1] == 0 && p[2] == 1)
      return p;
  }

  return end + 3;
}

const uint8_t *my_avc_find_startcode(const uint8_t *p, const uint8_t *end)
{
  const uint8_t *out= my_avc_find_startcode_internal(p, end);
  if (p<out && out<end && !out[-1])
    out--;
  return out;
}

const int my_avc_parse_nal_units(ByteIOContext *pb, const uint8_t *buf_in, int size)
{
  const uint8_t *p = buf_in;
  const uint8_t *end = p + size;
  const uint8_t *nal_start, *nal_end;

  size = 0;
  nal_start = my_avc_find_startcode(p, end);
  while (nal_start < end)
  {
    while (!*(nal_start++));
    nal_end = my_avc_find_startcode(nal_start, end);
    put_be32(pb, nal_end - nal_start);
    put_buffer(pb, nal_start, nal_end - nal_start);
    size += 4 + nal_end - nal_start;
    nal_start = nal_end;
  }
  return size;
}

const int my_avc_parse_nal_units_buf(const uint8_t *buf_in, uint8_t **buf, int *size)
{
  ByteIOContext *pb;
  int ret = url_open_dyn_buf(&pb);
  if (ret < 0)
    return ret;

  my_avc_parse_nal_units(pb, buf_in, *size);

  av_freep(buf);
  *size = url_close_dyn_buf(pb, buf);
  return 0;
}

const int my_isom_write_avcc(ByteIOContext *pb, const uint8_t *data, int len)
{
  // extradata from bytestream h264, convert to avcC atom data for bitstream
  if (len > 6)
  {
    /* check for h264 start code */
    if (VDA_RB32(data) == 0x00000001 || VDA_RB24(data) == 0x000001)
    {
      uint8_t *buf=NULL, *end, *start;
      uint32_t sps_size=0, pps_size=0;
      uint8_t *sps=0, *pps=0;

      int ret = my_avc_parse_nal_units_buf(data, &buf, &len);
      if (ret < 0)
        return ret;
      start = buf;
      end = buf + len;

      /* look for sps and pps */
      while (buf < end)
      {
        unsigned int size;
        uint8_t nal_type;
        size = VDA_RB32(buf);
        nal_type = buf[4] & 0x1f;
        if (nal_type == 7) /* SPS */
        {
          sps = buf + 4;
          sps_size = size;
        }
        else if (nal_type == 8) /* PPS */
        {
          pps = buf + 4;
          pps_size = size;
        }
        buf += size + 4;
      }
      assert(sps);

      put_byte(pb, 1); /* version */
      // 66 (Base line profile), 77 (main profile), 100 (high profile)
      put_byte(pb, sps[1]); /* h.264 stream profile */
      put_byte(pb, sps[2]); /* h.264 compatible profiles */
      put_byte(pb, sps[3]); /* h.264 stream level */
      put_byte(pb, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
      put_byte(pb, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

      put_be16(pb, sps_size);
      put_buffer(pb, sps, sps_size);
      if (pps)
      {
        put_byte(pb, 1); /* number of pps */
        put_be16(pb, pps_size);
        put_buffer(pb, pps, pps_size);
      }
      av_free(start);
    }
    else
    {
      put_buffer(pb, data, len);
    }
  }
  return 0;
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

// helper function that wraps dts/pts into a dictionary
static CFDictionaryRef
CreateDictionaryWithDisplayTime(uint64_t dts, uint64_t pts)
{
  CFStringRef key[2] = {
    CFSTR("VideoDisplay_DTS"),
    CFSTR("VideoDisplay_PTS")};
  CFNumberRef value[2];
  CFDictionaryRef display_time;

  value[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &dts);
  value[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &pts);

  display_time = CFDictionaryCreate(
    kCFAllocatorDefault, (const void **)&key, (const void **)&value, 2,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  CFRelease(value[0]);
  CFRelease(value[1]);

  return display_time;
}
// helper function to extract dts/pts from a dictionary
static void
GetFrameDisplayTimeFromDictionary(CFDictionaryRef inFrameInfoDictionary, frame_queue *frame)
{
  frame->dts = 0;
  frame->pts = 0;
  if (inFrameInfoDictionary == NULL)
    return;

  CFNumberRef value[2];
  //
  value[0] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_DTS"));
  if (value[0])
    CFNumberGetValue(value[0], kCFNumberDoubleType, &frame->dts);

  value[1] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_PTS"));
  if (value[1])
    CFNumberGetValue(value[1], kCFNumberDoubleType, &frame->pts);

  return;
}

char* vtutil_string_to_utf8(CFStringRef s)
{
  char *result;
  CFIndex size;

  size = CFStringGetMaximumSizeForEncoding(CFStringGetLength (s), kCFStringEncodingUTF8);
  result = (char*)malloc(size + 1);
  CFStringGetCString(s, result, size + 1, kCFStringEncodingUTF8);

  return result;
}

char* vtutil_object_to_string(CFTypeRef obj)
{
  char *result;
  CFStringRef s;

  if (obj == NULL)
    return strdup ("(null)");

  s = CFCopyDescription(obj);
  result = vtutil_string_to_utf8(s);
  CFRelease(s);

  return result;
}

typedef struct
{
  VTDecompressionSessionRef session;
} VTDumpDecompressionPropCtx;

void
vtdec_session_dump_property(CFStringRef prop_name, CFDictionaryRef prop_attrs, VTDumpDecompressionPropCtx *dpc)
{
  char *name_str;
  CFTypeRef prop_value;
  OSStatus status;

  name_str = vtutil_string_to_utf8(prop_name);
  if (true) {
    char *attrs_str;

    attrs_str = vtutil_object_to_string(prop_attrs);
    printf("%s = %s\n", name_str, attrs_str);
    free(attrs_str);
  }

  status = VTDecompressionSessionCopyProperty(dpc->session, prop_name, NULL, &prop_value);
  if (status == kVTSuccess) {
    char *value_str;

    value_str = vtutil_object_to_string(prop_value);
    printf("%s = %s\n", name_str, value_str);
    free(value_str);

    if (prop_value != NULL)
      CFRelease(prop_value);
  } else {
    printf("%s = <failed to query: %d>\n", name_str, (int)status);
  }

  free(name_str);
}

void vtdec_session_dump_properties(VTDecompressionSessionRef session)
{
  VTDumpDecompressionPropCtx dpc = { session };
  CFDictionaryRef dict;
  OSStatus status;

  status = VTDecompressionSessionCopySupportedPropertyDictionary(session, &dict);
  if (status != kVTSuccess)
    goto error;
  CFDictionaryApplyFunction(dict, (CFDictionaryApplierFunction)vtdec_session_dump_property, &dpc);
  CFRelease(dict);

  return;

error:
  printf("failed to dump properties\n");
}

CMFormatDescriptionRef
vtdec_create_format_description(AppContext *ctx)
{
  printf("vtdec_create_format_description\n");

  CMFormatDescriptionRef fmt_desc;
  OSStatus status;
 
#if TARGET_OS_IPHONE
  status = CMVideoFormatDescriptionCreate
#else
  status = FigVideoFormatDescriptionCreate
#endif
    (NULL,             // CFAllocatorRef allocator
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
vtdec_create_format_description_from_codec_data(AppContext *ctx, UInt32 atom)
{
  printf("vtdec_create_format_description_from_codec_data\n");

  CMFormatDescriptionRef fmt_desc;
  OSStatus status;

  status = FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
    NULL,             // CFAllocatorRef allocator ???
    ctx->format_id,
    ctx->sourceWidth,
    ctx->sourceHeight,
    atom,
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
    frame_queue frame;
    GetFrameDisplayTimeFromDictionary(frameInfo, &frame);
    printf("format(%c%c%c%c), width(%d), height(%d), cvbuf(%p), frameInfo(%p), infoFlags(%d), dts(%llu), pts(%llu)\n",
      (int)((format_type >>24) & 0xff), (int)((format_type >> 16) & 0xff), (int)((format_type >>8) & 0xff), (int)(format_type & 0xff),
      width, height, cvbuf, frameInfo, infoFlags, frame.dts, frame.pts);
  }
}

#define kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange '420v'

VTDecompressionSessionRef
vtdec_create_session(AppContext *ctx)
{
  VTDecompressionSessionRef session = NULL;
  CFMutableDictionaryRef destinationPixelBufferAttributes;
  VTDecompressionOutputCallback outputCallback;
  OSStatus status;
  int width, height;
  
  width = ctx->sourceWidth;
  height= ctx->sourceHeight;
  #if 1
    // scale output pictures down to 720p size for display
    if (width > 1280)
    {
      double w_scaler = 1280.0 / width;
      width = 1280;
      height = height * w_scaler;
    }
  #endif

  destinationPixelBufferAttributes = CFDictionaryCreateMutable(
    NULL, // CFAllocatorRef allocator
    0,    // CFIndex capacity
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);

  // The recommended pixel format choices are 
  //  kCVPixelFormatType_32BGRA or kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_32BGRA);
    //kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferWidthKey, width);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferHeightKey, height);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferBytesPerRowAlignmentKey, 16);

  outputCallback.func = vtdec_output_frame;
  outputCallback.data = ctx;

  status = VTDecompressionSessionCreate(
    NULL, // CFAllocatorRef allocator
    ctx->fmt_desc,
    NULL, // CFTypeRef sessionOptions
    destinationPixelBufferAttributes,
    &outputCallback,
    &session);
  if (status) {
    if (status == -8971)
      printf("VTDecompressionSessionCreate failed: codecExtensionNotFoundErr\n");
    else
      printf("VTDecompressionSessionCreate failed %d\n", (int)status);
  }
  //VTDecompressionSessionCreate failed -12909

  CFRelease(destinationPixelBufferAttributes);
  
  vtdec_session_dump_properties(session);

  #if TARGET_OS_IPHONE
  status = VTDecompressionSessionSetProperty(session, 
    kVTVideoDecoderSpecification_EnableSandboxedVideoDecoder, kCFBooleanTrue);
  if (status) {
    if (status == -12900)
      printf("VTDecompressionSessionSetProperty failed: -12900\n");
    else
      printf("VTDecompressionSessionSetProperty failed %d\n", (int)status);
  }
  #endif

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

#if TARGET_OS_IPHONE
  status = CMBlockBufferCreateWithMemoryBlock
#else
  status = FigBlockBufferCreateWithMemoryBlock
#endif
    (NULL,             // CFAllocatorRef structureAllocator
    demux_buff,       // void *memoryBlock
    demux_size,       // size_t blockLengt
    kCFAllocatorNull, // CFAllocatorRef blockAllocator
    NULL,             // const CMBlockBufferCustomBlockSource *customBlockSource
    0,                // size_t offsetToData
    demux_size,       // size_t dataLength
    FALSE,            // CMBlockBufferFlags flags
    &newBBufOut);     // CMBlockBufferRef *newBBufOut

  if (!status) {
#if TARGET_OS_IPHONE
    status = CMSampleBufferCreate
#else
    status = FigSampleBufferCreate
#endif
      (NULL,           // CFAllocatorRef allocator
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

  frameinfo = CreateDictionaryWithDisplayTime(dts, pts);
  decoderFlags = 0;
  status = VTDecompressionSessionDecodeFrame(ctx->session, sample_buff, decoderFlags, frameinfo, 0);
  if (status != 0) {
    if (status == -8971)
      printf("VTDecompressionSessionDecodeFrame failed: codecBadDataErr\n");
    //else if (status == -8969)
    //  printf("VTDecompressionSessionDecodeFrame failed: codecBadDataErr\n");
    else
      printf("VTDecompressionSessionDecodeFrame returned %d\n", (int)status);
  } else {
    status = VTDecompressionSessionWaitForAsynchronousFrames(ctx->session);
    if (status != 0) {
      printf("VTDecompressionSessionWaitForAsynchronousFrames returned %d\n", (int)status);
    }
  }
  // VTDecompressionSessionDecodeFrame returned 8969
  // VTDecompressionSessionDecodeFrame returned -12350
  // VTDecompressionSessionDecodeFrame returned -12902

  CFRelease(frameinfo);
  FigSampleBufferRelease(sample_buff);

  return status;
}

void dump_extradata(AppContext *ctx)
{
  for (int i=0; i < ctx->codec_context->extradata_size-1; i++) {
    printf("0x%02X,", ctx->codec_context->extradata[i]);
    if ((i & 0xF) == 0xF) printf("\n");
  }
  printf("0x%02X\n", ctx->codec_context->extradata[ctx->codec_context->extradata_size-1]);
}

int main (int argc, char * const argv[])
{
  bool convert_bytestream = false;
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

  ctx.sourceWidth = ctx.codec_context->width;
  ctx.sourceHeight = ctx.codec_context->height;
  printf("video width(%d), height(%d), extradata_size(%d)\n",
    (int)ctx.sourceWidth, (int)ctx.sourceHeight, ctx.codec_context->extradata_size);
  dump_extradata(&ctx);

  switch(ctx.codec_context->codec_id)
  {
    case CODEC_ID_H264:
      printf("CODEC_ID_H264\n");
      ctx.format_id = kCMVideoCodecType_H264;
      if (ctx.codec_context->extradata_size) {
        // valid avcC atom data always starts with the value 1 (version)
        if ( *ctx.codec_context->extradata == 1 ) {
          printf("using existing avcC atom data\n");
          ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx, 'avcC');
        } else {
          if (ctx.codec_context->extradata[0] == 0 && 
              ctx.codec_context->extradata[1] == 0 && 
              ctx.codec_context->extradata[2] == 0 && 
              ctx.codec_context->extradata[3] == 1)
          {
            #if 1
            uint8_t *saved_extradata;
            unsigned int saved_extrasize;
            saved_extradata = ctx.codec_context->extradata;
            saved_extrasize = ctx.codec_context->extradata_size;

            // video content is from x264 or from bytestream h264 (AnnexB format)
            // NAL reformating to bitstream format needed
            ByteIOContext *pb;
            if (url_open_dyn_buf(&pb) < 0)
              return false;

            convert_bytestream = true;
            // create a valid avcC atom data from ffmpeg's extradata
            my_isom_write_avcc(pb, ctx.codec_context->extradata, ctx.codec_context->extradata_size);
            // unhook from ffmpeg's extradata
            ctx.codec_context->extradata = NULL;
            // extract the avcC atom data into extradata then write it into avcCData for VDADecoder
            ctx.codec_context->extradata_size = url_close_dyn_buf(pb, &ctx.codec_context->extradata);
            printf("convert to avcC atom data\n");
            dump_extradata(&ctx);
            
            ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx, 'avcC');
            // done with the converted extradata, we MUST free using av_free
            av_free(ctx.codec_context->extradata);
            //restore orignal contents
            ctx.codec_context->extradata = saved_extradata;
            ctx.codec_context->extradata_size = saved_extrasize;
            
            #else

            int extrasize;
            uint8_t *extradata;
            extradata = ctx.codec_context->extradata;
            extrasize = ctx.codec_context->extradata_size;

            h264_generate_avcc_atom_data(extradata, extrasize,
              &ctx.codec_context->extradata, &ctx.codec_context->extradata_size);
            printf("convert to avcC atom data\n");
            dump_extradata(&ctx);
            
            ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx, 'avcC');
            // done with the converted extradata
            free(ctx.codec_context->extradata);
            //restore orignal contents
            ctx.codec_context->extradata = extradata;
            ctx.codec_context->extradata_size = extrasize;
            convert_bytestream = true;
            #endif
          } else {
            printf("%s - invalid avcC atom data", __FUNCTION__);
            return false;
          }
        }
      } else {
        ctx.fmt_desc = vtdec_create_format_description(&ctx);
      }
    break;
    case CODEC_ID_MPEG4:
      printf("CODEC_ID_MPEG4\n");
      ctx.format_id = kCMVideoCodecType_MPEG4Video;
      if (ctx.codec_context->extradata_size) {
        ByteIOContext *pb;
        quicktime_esds_t *esds;
        uint8_t *saved_extradata;
        unsigned int saved_extrasize;
        saved_extradata = ctx.codec_context->extradata;
        saved_extrasize = ctx.codec_context->extradata_size;

        if (url_open_dyn_buf(&pb) < 0)
          return false;

        esds = quicktime_set_esds(ctx.codec_context->extradata, ctx.codec_context->extradata_size);
        quicktime_write_esds(pb, esds);

        // unhook from ffmpeg's extradata
        ctx.codec_context->extradata = NULL;
        // extract the esds atom decoderConfig from extradata
        ctx.codec_context->extradata_size = url_close_dyn_buf(pb, &ctx.codec_context->extradata);
        free(esds->decoderConfig);
        free(esds);

        ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx, 'esds');

        // done with the converted extradata, we MUST free using av_free
        av_free(ctx.codec_context->extradata);
        //restore orignal contents
        ctx.codec_context->extradata = saved_extradata;
        ctx.codec_context->extradata_size = saved_extrasize;
      } else {
        ctx.fmt_desc = vtdec_create_format_description(&ctx);
      }
    break;
    case CODEC_ID_MPEG2VIDEO:
      printf("CODEC_ID_MPEG2VIDEO\n");
      ctx.format_id = kCMVideoCodecType_MPEG2Video;
      if (ctx.codec_context->extradata_size) {
        // mp2p
        // mp2t
        ctx.fmt_desc = vtdec_create_format_description_from_codec_data(&ctx, '1234');
      } else {
        ctx.fmt_desc = vtdec_create_format_description(&ctx);
      }
    break;
    default:
      fprintf(stderr, "ERROR: Invalid FFmpegFileReader Codec Format (not h264/mpeg4) = %d\n",
        ctx.codec_context->codec_id);
      goto fail;
    break;
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
      //goto fail;
    }

    usleep(10000);
    frame_count = 0;
    bool done = false;
    while (!g_signal_abort && !done && (frame_count < 5000)) {
      if (convert_bytestream) {
        #if 1
        // convert demuxer packet from bytestream (AnnexB) to bitstream
        ByteIOContext *pb;
        int demuxer_bytes;
        uint8_t *demuxer_content;

        if(url_open_dyn_buf(&pb) < 0)
          goto fail;

        demuxer_bytes = my_avc_parse_nal_units(pb, data, byte_count);
        demuxer_bytes = url_close_dyn_buf(pb, &demuxer_content);
        bgn = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
        status = vtdec_decode_buffer(&ctx, demuxer_content, demuxer_bytes, dts, pts);
        end = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
        av_free(demuxer_content);
        
        #else

        int demuxer_size;
        uint8_t *demuxer_buff;
        h264_byte_to_bit_transform(data, byte_count, &demuxer_buff, &demuxer_size);
        bgn = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
        status = vtdec_decode_buffer(&ctx, demuxer_buff, demuxer_size, dts, pts);
        end = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
        free(demuxer_buff);
        #endif
      } else {
        bgn = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
        status = vtdec_decode_buffer(&ctx, data, byte_count, dts, pts);
        end = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
      }

      free(data);
      fprintf(stdout, "decode time(%llu)\n", end-bgn);
      frame_count++;
      usleep(10000);
      ctx.demuxer->Read(&data, &byte_count, &dts, &pts);
      if (!byte_count) done = true;
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
