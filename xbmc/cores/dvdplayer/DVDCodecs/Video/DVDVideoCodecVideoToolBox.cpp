/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif

#if defined(__APPLE__) && defined(__arm__)
//#if defined(HAVE_VIDEOTOOLBOXDECODER)
#include "GUISettings.h"
#include "DVDClock.h"
#include "DVDStreamInfo.h"
#include "DVDCodecUtils.h"
#include "DVDVideoCodecVideoToolBox.h"
#include "Codecs/DllSwScale.h"
#include "Codecs/DllAvFormat.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "osx/atv2/iOS_Utils.h"

#if defined(__cplusplus)
extern "C"
{
#endif
    
#pragma pack(push, 4)

//-----------------------------------------------------------------------------------
// /System/Library/PrivateFrameworks/VideoToolbox.framework
enum VTFormat
{
  kVTFormatJPEG         = 'jpeg', // kCMVideoCodecType_JPEG
  kVTFormatH264         = 'avc1', // kCMVideoCodecType_H264 (MPEG-4 Part 10))
  kVTFormatMPEG4Video   = 'mp4v', // kCMVideoCodecType_MPEG4Video (MPEG-4 Part 2)
  kVTFormatMPEG2Video   = 'mp2v'  // kCMVideoCodecType_MPEG2Video
};
enum {
  kVTDecoderNoErr = 0,
  kVTDecoderHardwareNotSupportedErr = -12470,
  kVTDecoderFormatNotSupportedErr = -12471,
  kVTDecoderConfigurationError = -12472,
  kVTDecoderDecoderFailedErr = -12473,
};
enum {
  kVTDecodeInfo_Asynchronous = 1UL << 0,
  kVTDecodeInfo_FrameDropped = 1UL << 1
};
enum {
  // tells the decoder not to bother returning a CVPixelBuffer
  // in the outputCallback. The output callback will still be called.
  kVTDecoderDecodeFlags_DontEmitFrame = 1 << 0
};
enum {
  // decode and return buffers for all frames currently in flight.
  kVTDecoderFlush_EmitFrames = 1 << 0		
};

typedef UInt32 VTFormatId;
typedef CFTypeRef VTDecompressionSessionRef;

//typedef void (*VTDecompressionOutputCallbackFunc) (void *data, CFDictionaryRef unk1,
//  OSStatus result, uint32_t unk2, CVBufferRef cvbuf);
typedef void (*VTDecompressionOutputCallbackFunc)(
  void            *refCon,
  CFDictionaryRef frameInfo,
  OSStatus        status,
  UInt32          infoFlags,
  CVBufferRef     imageBuffer);

typedef struct _VTDecompressionOutputCallback VTDecompressionOutputCallback;
struct _VTDecompressionOutputCallback
{
  VTDecompressionOutputCallbackFunc callback;
  void *refcon;
};

extern OSStatus VTDecompressionSessionCreate(
  CFAllocatorRef allocator,
  CMFormatDescriptionRef videoFormatDescription,
  CFTypeRef sessionOptions,
  CFDictionaryRef destinationPixelBufferAttributes,
  VTDecompressionOutputCallback *outputCallback,
  VTDecompressionSessionRef *session);

extern OSStatus VTDecompressionSessionDecodeFrame(
  VTDecompressionSessionRef session, CMSampleBufferRef sbuf,
  uint32_t decoderFlags, CFDictionaryRef frameInfo, uint32_t unk1);

extern void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session);
extern void VTDecompressionSessionRelease(VTDecompressionSessionRef session);
extern VTDecompressionSessionRef VTDecompressionSessionRetain(VTDecompressionSessionRef session);
extern OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session);
//-----------------------------------------------------------------------------------
// /System/Library/Frameworks/CoreMedia.framework
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

////////////////////////////////////////////////////////////////////////////////////////////
// helper function that inserts an int32_t into a dictionary
static void
CFDictionarySetSInt32(CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32)
{
  CFNumberRef number;

  number = CFNumberCreate(NULL, kCFNumberSInt32Type, &numberSInt32);
  CFDictionarySetValue(dictionary, key, number);
  CFRelease(number);
}
// helper function that inserts an double into a dictionary
static void
CFDictionarySetDouble(CFMutableDictionaryRef dictionary, CFStringRef key, double numberDouble)
{
    CFNumberRef number;
    
    number = CFNumberCreate(NULL, kCFNumberDoubleType, &numberDouble);
    CFDictionaryAddValue(dictionary, key, number);
    CFRelease(number);
}
// helper function that wraps dts/pts into a dictionary
static CFDictionaryRef
CreateDictionaryWithDisplayTime(double time, double dts, double pts)
{
  CFStringRef key[3] = {
    CFSTR("VideoDisplay_TIME"),
    CFSTR("VideoDisplay_DTS"),
    CFSTR("VideoDisplay_PTS")};
  CFNumberRef value[3];
  CFDictionaryRef display_time;

  value[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &time);
  value[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &dts);
  value[2] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &pts);

  display_time = CFDictionaryCreate(
    kCFAllocatorDefault, (const void **)&key, (const void **)&value, 3,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  CFRelease(value[0]);
  CFRelease(value[1]);
  CFRelease(value[2]);

  return display_time;
}
// helper function to extract dts/pts from a dictionary
static void
GetFrameDisplayTimeFromDictionary(
  CFDictionaryRef inFrameInfoDictionary, frame_queue *frame)
{
  // default to DVD_NOPTS_VALUE
  frame->sort_time = -1.0;
  frame->dts = DVD_NOPTS_VALUE;
  frame->pts = DVD_NOPTS_VALUE;
  if (inFrameInfoDictionary == NULL)
    return;

  CFNumberRef value[3];
  //
  value[0] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_TIME"));
  if (value[0])
    CFNumberGetValue(value[0], kCFNumberDoubleType, &frame->sort_time);
  value[1] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_DTS"));
  if (value[1])
    CFNumberGetValue(value[1], kCFNumberDoubleType, &frame->dts);
  value[2] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_PTS"));
  if (value[2])
    CFNumberGetValue(value[2], kCFNumberDoubleType, &frame->pts);

  return;
}
// helper function to create a format descriptor
static CMFormatDescriptionRef
CreateFormatDescription(VTFormatId format_id, int width, int height)
{
  CMFormatDescriptionRef fmt_desc;
  OSStatus status;

  status = CMVideoFormatDescriptionCreate(
    NULL,             // CFAllocatorRef allocator
    format_id,
    width,
    height,
    NULL,             // CFDictionaryRef extensions
    &fmt_desc);

  if (status == kVTDecoderNoErr)
    return fmt_desc;
  else
    return NULL;
}
// helper function to create a avcC atom format descriptor
static CMFormatDescriptionRef
CreateFormatDescriptionFromCodecData(VTFormatId format_id, int width, int height, const uint8_t *extradata, int extradata_size)
{
  CMFormatDescriptionRef fmt_desc;
  OSStatus status;

  status = FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
    NULL,             // CFAllocatorRef allocator ???
    format_id,
    width,
    height,
    'avcC',
    extradata,
    extradata_size,
    &fmt_desc);

  if (status == kVTDecoderNoErr)
    return fmt_desc;
  else
    return NULL;
}
// helper function to create a CMSampleBufferRef from demuxer data
static CMSampleBufferRef
CreateSampleBufferFrom(CMFormatDescriptionRef fmt_desc, void *demux_buff, size_t demux_size)
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

  if (!status)
  {
    status = CMSampleBufferCreate(
      NULL,           // CFAllocatorRef allocator
      newBBufOut,     // CMBlockBufferRef dataBuffer
      TRUE,           // Boolean dataReady
      0,              // CMSampleBufferMakeDataReadyCallback makeDataReadyCallback
      0,              // void *makeDataReadyRefcon
      fmt_desc,       // CMFormatDescriptionRef formatDescription
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

static const uint8_t *avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
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

const uint8_t *avc_find_startcode(const uint8_t *p, const uint8_t *end)
{
  const uint8_t *out= avc_find_startcode_internal(p, end);
  if (p<out && out<end && !out[-1])
    out--;
  return out;
}

const int avc_parse_nal_units(DllAvFormat *av_format_ctx,
  ByteIOContext *pb, const uint8_t *buf_in, int size)
{
  const uint8_t *p = buf_in;
  const uint8_t *end = p + size;
  const uint8_t *nal_start, *nal_end;

  size = 0;
  nal_start = avc_find_startcode(p, end);
  while (nal_start < end)
  {
    while (!*(nal_start++));
    nal_end = avc_find_startcode(nal_start, end);
    av_format_ctx->put_be32(pb, nal_end - nal_start);
    av_format_ctx->put_buffer(pb, nal_start, nal_end - nal_start);
    size += 4 + nal_end - nal_start;
    nal_start = nal_end;
  }
  return size;
}

const int avc_parse_nal_units_buf(DllAvUtil *av_util_ctx, DllAvFormat *av_format_ctx,
  const uint8_t *buf_in, uint8_t **buf, int *size)
{
  ByteIOContext *pb;
  int ret = av_format_ctx->url_open_dyn_buf(&pb);
  if (ret < 0)
    return ret;

  avc_parse_nal_units(av_format_ctx, pb, buf_in, *size);

  av_util_ctx->av_freep(buf);
  *size = av_format_ctx->url_close_dyn_buf(pb, buf);
  return 0;
}

/*
 * if extradata size is greater than 7, then have a valid quicktime 
 * avcC atom header.
 *
 *      -: avcC atom header :-
 *  -----------------------------------
 *  1 byte  - version
 *  1 byte  - h.264 stream profile
 *  1 byte  - h.264 compatible profiles
 *  1 byte  - h.264 stream level
 *  6 bits  - reserved set to 63
 *  2 bits  - NAL length 
 *            ( 0 - 1 byte; 1 - 2 bytes; 3 - 4 bytes)
 *  3 bit   - reserved
 *  5 bits  - number of SPS 
 *  for (i=0; i < number of SPS; i++) {
 *      2 bytes - SPS length
 *      SPS length bytes - SPS NAL unit
 *  }
 *  1 byte  - number of PPS
 *  for (i=0; i < number of PPS; i++) {
 *      2 bytes - PPS length 
 *      PPS length bytes - PPS NAL unit 
 *  }
 
 how to detect the interlacing used on an existing stream:
- progressive is signalled by setting frame_mbs_only_flag: 1 in the SPS
- interlaced is signalled by setting frame_mbs_only_flag: 0 in the SPS and field_pic_flag: 1 on all frames
- paff is signalled by setting frame_mbs_only_flag: 0 in the SPS and field_pic_flag: 1 on all frames that get interlaced and field_pic_flag: 0 on all frames that get progressive
- mbaff is signalled by setting frame_mbs_only_flag: 0 and mb_adaptive_frame_field_flag: 1 in the SPS and field_pic_flag: 0 on the frames (field_pic_flag: 1 would indicate a normal interlaced frame)
*/
const int isom_write_avcc(DllAvUtil *av_util_ctx, DllAvFormat *av_format_ctx,
  ByteIOContext *pb, const uint8_t *data, int len)
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

      int ret = avc_parse_nal_units_buf(av_util_ctx, av_format_ctx, data, &buf, &len);
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

      av_format_ctx->put_byte(pb, 1); /* version */
      av_format_ctx->put_byte(pb, sps[1]); /* profile */
      av_format_ctx->put_byte(pb, sps[2]); /* profile compat */
      av_format_ctx->put_byte(pb, sps[3]); /* level */
      av_format_ctx->put_byte(pb, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
      av_format_ctx->put_byte(pb, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

      av_format_ctx->put_be16(pb, sps_size);
      av_format_ctx->put_buffer(pb, sps, sps_size);
      if (pps)
      {
        av_format_ctx->put_byte(pb, 1); /* number of pps */
        av_format_ctx->put_be16(pb, pps_size);
        av_format_ctx->put_buffer(pb, pps, pps_size);
      }
      av_util_ctx->av_free(start);
    }
    else
    {
      av_format_ctx->put_buffer(pb, data, len);
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
CDVDVideoCodecVideoToolBox::CDVDVideoCodecVideoToolBox() : CDVDVideoCodec()
{
  m_vt_session = NULL;
  m_fmt_desc = NULL;
  m_pFormatName = "vtb";

  m_queue_depth = 0;
  m_display_queue = NULL;
  pthread_mutex_init(&m_queue_mutex, NULL);

  m_convert_bytestream = false;
  m_dllAvUtil = NULL;
  m_dllAvFormat = NULL;
  memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));
}

CDVDVideoCodecVideoToolBox::~CDVDVideoCodecVideoToolBox()
{
  Dispose();
  pthread_mutex_destroy(&m_queue_mutex);
}

bool CDVDVideoCodecVideoToolBox::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  //if (g_guiSettings.GetBool("videoplayer.usevideotoolbox") && !hints.software)
  {
    CCocoaAutoPool pool;
    int32_t width, height, profile, level;
    uint8_t *extradata; // extra data for codec to use
    unsigned int extrasize; // size of extra data

    //
    width  = hints.width;
    height = hints.height;
    level  = hints.level;
    profile = hints.profile;
    extrasize = hints.extrasize;
    extradata = (uint8_t*)hints.extradata;
 
    switch (hints.codec)
    {
      case CODEC_ID_MPEG4:
        m_fmt_desc = CreateFormatDescription(kVTFormatMPEG4Video, width, height);
        m_pFormatName = "vtb-mpeg4";
      break;

      case CODEC_ID_MPEG2VIDEO:
        m_fmt_desc = CreateFormatDescription(kVTFormatMPEG2Video, width, height);
        m_pFormatName = "vtb-mpeg2";
      break;

      case CODEC_ID_H264:
        // TODO: need to quality h264 encoding (profile, level and number of reference frame)
        // source must be H.264 with valid avcC atom data in extradata
        if (extrasize < 7 || extradata == NULL)
        {
          //m_fmt_desc = CreateFormatDescription(kVTFormatH264, width, height);
          CLog::Log(LOGNOTICE, "%s - avcC atom too data small or missing", __FUNCTION__);
          return false;
        }

        if (extradata[0] == 1)
        {
          // valid avcC atom data always starts with the value 1 (version)
          m_fmt_desc = CreateFormatDescriptionFromCodecData(kVTFormatH264, width, height, extradata, extrasize);
          CLog::Log(LOGNOTICE, "%s - using avcC atom of size(%d)", __FUNCTION__, extrasize);
        }
        else
        {
          if (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 0 && extradata[3] == 1)
          {
            // video content is from x264 or from bytestream h264 (AnnexB format)
            // NAL reformating to bitstream format needed
            m_dllAvUtil = new DllAvUtil;
            m_dllAvFormat = new DllAvFormat;
            if (!m_dllAvUtil->Load() || !m_dllAvFormat->Load())
              return false;

            ByteIOContext *pb;
            if (m_dllAvFormat->url_open_dyn_buf(&pb) < 0)
              return false;

            m_convert_bytestream = true;
            // create a valid avcC atom data from ffmpeg's extradata
            isom_write_avcc(m_dllAvUtil, m_dllAvFormat, pb, extradata, extrasize);
            // unhook from ffmpeg's extradata
            extradata = NULL;
            // extract the avcC atom data into extradata getting size into extrasize
            extrasize = m_dllAvFormat->url_close_dyn_buf(pb, &extradata);
            // CFDataCreate makes a copy of extradata contents
            m_fmt_desc = CreateFormatDescriptionFromCodecData(kVTFormatH264, width, height, extradata, extrasize);
            // done with the converted  new extradata, we MUST free using av_free
            m_dllAvUtil->av_free(extradata);
            CLog::Log(LOGNOTICE, "%s - created avcC atom of size(%d)", __FUNCTION__, extrasize);
          }
          else
          {
            CLog::Log(LOGNOTICE, "%s - invalid avcC atom data", __FUNCTION__);
            return false;
          }
        }
        m_pFormatName = "vtb-h264";
      break;

      default:
        return false;
      break;
    }
    
    if (m_fmt_desc == NULL)
      return false;

    CreateVTSession(width, height, m_fmt_desc);
    if (m_vt_session == NULL)
    {
      if (m_fmt_desc)
      {
        FigFormatDescriptionRelease(m_fmt_desc);
        m_fmt_desc = NULL;
      }
      return false;
    }

    // allocate a YV12 DVDVideoPicture buffer.
    // first make sure all properties are reset.
    memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));

    m_videobuffer.dts = DVD_NOPTS_VALUE;
    m_videobuffer.pts = DVD_NOPTS_VALUE;
    m_videobuffer.format = DVDVideoPicture::FMT_CVBREF;
    m_videobuffer.color_range  = 0;
    m_videobuffer.color_matrix = 4;
    m_videobuffer.iFlags  = DVP_FLAG_ALLOCATED;
    m_videobuffer.iWidth  = hints.width;
    m_videobuffer.iHeight = hints.height;
    m_videobuffer.iDisplayWidth  = hints.width;
    m_videobuffer.iDisplayHeight = hints.height;

    m_DropPictures = false;
    m_sort_time_offset = (CurrentHostCounter() * 1000.0) / CurrentHostFrequency();

    return true;
  }

  return false;
}

void CDVDVideoCodecVideoToolBox::Dispose()
{
  CCocoaAutoPool pool;

  DestroyVTSession();
  if (m_fmt_desc)
  {
    FigFormatDescriptionRelease(m_fmt_desc);
    m_fmt_desc = NULL;
  }
  
  if (m_videobuffer.iFlags & DVP_FLAG_ALLOCATED)
  {
    // release any previous retained cvbuffer reference
    if (m_videobuffer.cvBufferRef)
      CVPixelBufferRelease(m_videobuffer.cvBufferRef);
    m_videobuffer.cvBufferRef = NULL;
    m_videobuffer.iFlags = 0;
  }
  if (m_dllAvUtil)
  {
    delete m_dllAvUtil;
    m_dllAvUtil = NULL;
  }
  if (m_dllAvFormat)
  {
    delete m_dllAvFormat;
    m_dllAvFormat = NULL;
  }
}

void CDVDVideoCodecVideoToolBox::SetDropState(bool bDrop)
{
  m_DropPictures = bDrop;
}

int CDVDVideoCodecVideoToolBox::Decode(BYTE* pData, int iSize, double dts, double pts)
{
  CCocoaAutoPool pool;
  //
  if (pData)
  {
    OSStatus status;
    double sort_time;
    uint32_t decoderFlags = 0;
    CFDictionaryRef frameInfo;
    CMSampleBufferRef sampleBuff;

    if (m_convert_bytestream)
    {
      // convert demuxer packet from bytestream (AnnexB) to bitstream
      ByteIOContext *pb;
      int demux_size;
      uint8_t *demux_buff;

      if(m_dllAvFormat->url_open_dyn_buf(&pb) < 0)
      {
        return VC_ERROR;
      }
      demux_size = avc_parse_nal_units(m_dllAvFormat, pb, pData, iSize);
      demux_size = m_dllAvFormat->url_close_dyn_buf(pb, &demux_buff);
      sampleBuff = CreateSampleBufferFrom(m_fmt_desc, demux_buff, demux_size);
      m_dllAvUtil->av_free(demux_buff);
    }
    else
    {
      sampleBuff = CreateSampleBufferFrom(m_fmt_desc, pData, iSize);
    }
    sort_time = (CurrentHostCounter() * 1000.0) / CurrentHostFrequency();
    frameInfo = CreateDictionaryWithDisplayTime(sort_time - m_sort_time_offset, dts, pts);

    if (m_DropPictures)
      decoderFlags = kVTDecoderDecodeFlags_DontEmitFrame;

    // submit for decoding
    status = VTDecompressionSessionDecodeFrame(m_vt_session, sampleBuff, decoderFlags, frameInfo, 0);
    if (status != kVTDecoderNoErr) {
      CLog::Log(LOGNOTICE, "%s - VTDecompressionSessionDecodeFrame returned(%d)",
        __FUNCTION__, (int)status);
      CFRelease(frameInfo);
      FigSampleBufferRelease(sampleBuff);
      return VC_ERROR;
    }

    // wait for decoding to finish
    status = VTDecompressionSessionWaitForAsynchronousFrames(m_vt_session);
    if (status != kVTDecoderNoErr) {
      CLog::Log(LOGNOTICE, "%s - VTDecompressionSessionWaitForAsynchronousFrames returned(%d)",
        __FUNCTION__, (int)status);
      CFRelease(frameInfo);
      FigSampleBufferRelease(sampleBuff);
      return VC_ERROR;
    }

    CFRelease(frameInfo);
    FigSampleBufferRelease(sampleBuff);
  }

  // TODO: queue depth is related to the number of reference frames in encoded h.264.
  // so we need to buffer until we get N ref frames + 1.
  if (m_queue_depth < 4)
  {
    return VC_BUFFER;
  }

  return VC_PICTURE | VC_BUFFER;
}

void CDVDVideoCodecVideoToolBox::Reset(void)
{
  CCocoaAutoPool pool;

  while (m_queue_depth)
    DisplayQueuePop();

  m_sort_time_offset = (CurrentHostCounter() * 1000.0) / CurrentHostFrequency();
}

bool CDVDVideoCodecVideoToolBox::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
  CCocoaAutoPool pool;
  FourCharCode pixel_buffer_format;
  CVPixelBufferRef picture_buffer_ref;

  // release any previous retained cvbuffer reference
  if (pDvdVideoPicture->cvBufferRef)
    CVPixelBufferRelease(pDvdVideoPicture->cvBufferRef);

  // clone the video picture buffer settings.
  *pDvdVideoPicture = m_videobuffer;

  // get the top yuv frame, we risk getting the wrong frame if the frame queue
  // depth is less than the number of encoded reference frames. If queue depth
  // is greater than the number of encoded reference frames, then the top frame
  // will never change and we can just grab a ref to the top frame. This way
  // we don't lockout the vdadecoder while doing color format convert.
  pthread_mutex_lock(&m_queue_mutex);
  picture_buffer_ref = m_display_queue->pixel_buffer_ref;
  pixel_buffer_format = m_display_queue->pixel_buffer_format;
  pDvdVideoPicture->dts = m_display_queue->dts;
  pDvdVideoPicture->pts = m_display_queue->pts;
  pDvdVideoPicture->cvBufferRef = m_display_queue->pixel_buffer_ref;
  CVPixelBufferRetain(pDvdVideoPicture->cvBufferRef);
  pthread_mutex_unlock(&m_queue_mutex);

  // now we can pop the top frame.
  DisplayQueuePop();

  //CLog::Log(LOGNOTICE, "%s - VDADecoderDecode dts(%f), pts(%f)", __FUNCTION__,
  //  pDvdVideoPicture->dts, pDvdVideoPicture->pts);

  return VC_PICTURE | VC_BUFFER;
}

void CDVDVideoCodecVideoToolBox::DisplayQueuePop(void)
{
  CCocoaAutoPool pool;
  if (!m_display_queue || m_queue_depth == 0)
    return;

  // pop the top frame off the queue
  pthread_mutex_lock(&m_queue_mutex);
  frame_queue *top_frame = m_display_queue;
  m_display_queue = m_display_queue->nextframe;
  m_queue_depth--;
  pthread_mutex_unlock(&m_queue_mutex);

  // and release it
  CVPixelBufferRelease(top_frame->pixel_buffer_ref);
  free(top_frame);
}


void
CDVDVideoCodecVideoToolBox::CreateVTSession(int width, int height, CMFormatDescriptionRef fmt_desc)
{
  VTDecompressionSessionRef vt_session = NULL;
  CFMutableDictionaryRef destinationPixelBufferAttributes;
  VTDecompressionOutputCallback outputCallback;
  OSStatus status;

  destinationPixelBufferAttributes = CFDictionaryCreateMutable(
    NULL, // CFAllocatorRef allocator
    0,    // CFIndex capacity
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);

  // The recommended pixel format choices are 
  //  kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange or kCVPixelFormatType_32BGRA.
  //  TODO: figure out what we need.
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_32BGRA);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferWidthKey, width);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferHeightKey, height);
  CFDictionarySetSInt32(destinationPixelBufferAttributes,
    kCVPixelBufferBytesPerRowAlignmentKey, 2 * width);
  //CFDictionarySetValue(destinationPixelBufferAttributes,
  //  kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);

  // This codec accepts YCbCr input in the form of '2vuy' format pixel buffers.
  // We recommend explicitly defining the gamma level and YCbCr matrix that should be used.
  //CFDictionarySetDouble(destinationPixelBufferAttributes, kCVImageBufferGammaLevelKey, 2.2);
  //CFDictionaryAddValue(destinationPixelBufferAttributes, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_601_4);

  outputCallback.callback = VTDecoderCallback;
  outputCallback.refcon = this;

  status = VTDecompressionSessionCreate(
    NULL, // CFAllocatorRef allocator
    fmt_desc,
    NULL, // CFTypeRef sessionOptions
    destinationPixelBufferAttributes,
    &outputCallback,
    &vt_session);
  if (status != noErr)
  {
    m_vt_session = NULL;
    CLog::Log(LOGERROR, "%s - failed with status = (%d)", __FUNCTION__, (int)status);
  }
  else
    m_vt_session = (void*)vt_session;

  CFRelease(destinationPixelBufferAttributes);
}

void
CDVDVideoCodecVideoToolBox::DestroyVTSession(void)
{
  if (m_vt_session)
  {
    VTDecompressionSessionInvalidate((VTDecompressionSessionRef)m_vt_session);
    VTDecompressionSessionRelease((VTDecompressionSessionRef)m_vt_session);
    m_vt_session = NULL;
  }
}

void 
CDVDVideoCodecVideoToolBox::VTDecoderCallback(
  void               *refcon,
  CFDictionaryRef    frameInfo,
  OSStatus           status,
  UInt32             infoFlags,
  CVBufferRef        imageBuffer)
{
  CCocoaAutoPool pool;
  // Warning, this is an async callback. There can be multiple frames in flight.
  CDVDVideoCodecVideoToolBox *ctx = (CDVDVideoCodecVideoToolBox*)refcon;

  if (imageBuffer == NULL)
  {
    //CLog::Log(LOGDEBUG, "%s - imageBuffer is NULL", __FUNCTION__);
    return;
  }
  OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
  if ((format_type != kCVPixelFormatType_422YpCbCr8) && (format_type != kCVPixelFormatType_32BGRA) )
  {
    CLog::Log(LOGERROR, "%s - imageBuffer format is not '2vuy' or 'BGRA',is reporting 0x%x",
      "VTDecoderCallback", (int)format_type);
    return;
  }
  if (kVTDecodeInfo_FrameDropped & infoFlags)
  {
    CLog::Log(LOGDEBUG, "%s - frame dropped", __FUNCTION__);
    return;
  }

  // allocate a new frame and populate it with some information.
  // this pointer to a frame_queue type keeps track of the newest decompressed frame
  // and is then inserted into a linked list of frame pointers depending on the display time
  // parsed out of the bitstream and stored in the frameInfo dictionary by the client
  frame_queue *newFrame = (frame_queue*)calloc(sizeof(frame_queue), 1);
  newFrame->nextframe = NULL;
  newFrame->pixel_buffer_format = format_type;
  newFrame->pixel_buffer_ref = CVPixelBufferRetain(imageBuffer);
  GetFrameDisplayTimeFromDictionary(frameInfo, newFrame);

  // if both dts or pts are good we use those, else use decoder insert time for frame sort
  if ((newFrame->pts != DVD_NOPTS_VALUE) || (newFrame->dts != DVD_NOPTS_VALUE))
  {
    // if pts is borked (stupid avi's), use dts for frame sort
    if (newFrame->pts == DVD_NOPTS_VALUE)
      newFrame->sort_time = newFrame->dts;
    else
      newFrame->sort_time = newFrame->pts;
  }

  // since the frames we get may be in decode order rather than presentation order
  // our hypothetical callback places them in a queue of frames which will
  // hold them in display order for display on another thread
  pthread_mutex_lock(&ctx->m_queue_mutex);
  //
  frame_queue *queueWalker = ctx->m_display_queue;
  if (!queueWalker || (newFrame->sort_time < queueWalker->sort_time))
  {
    // we have an empty queue, or this frame earlier than the current queue head.
    newFrame->nextframe = queueWalker;
    ctx->m_display_queue = newFrame;
  } else {
    // walk the queue and insert this frame where it belongs in display order.
    bool frameInserted = false;
    frame_queue *nextFrame = NULL;
    //
    while (!frameInserted)
    {
      nextFrame = queueWalker->nextframe;
      if (!nextFrame || (newFrame->sort_time < nextFrame->sort_time))
      {
        // if the next frame is the tail of the queue, or our new frame is earlier.
        newFrame->nextframe = nextFrame;
        queueWalker->nextframe = newFrame;
        frameInserted = true;
      }
      queueWalker = nextFrame;
    }
  }
  ctx->m_queue_depth++;
  //
  pthread_mutex_unlock(&ctx->m_queue_mutex);	
}

#endif
