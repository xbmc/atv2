// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "file_reader_util.h"

#include <stdio.h>
#include <string>
//why do I need this ??
#ifndef INT64_C
# if defined (SIZEOF_LONG) && SIZEOF_LONG == 8
#  define INT64_C(c) c ## L
# elif defined (SIZEOF_INT) && SIZEOF_INT == 8
#  define INT64_C(c) c 
# else
#  define INT64_C(c) c ## LL
# endif
#endif

#define DVD_TIME_BASE 1000000

#include "ffmpeg_common.h"
#include "ffmpeg_file_protocol.h"
#include "bitstream_converter.h"

//////////////////////////////////////////////////////////////////////////////
// FFmpegFileReader
FFmpegFileReader::FFmpegFileReader(const std::string& filename)
  : m_filename(filename),
    m_format_context(NULL),
    m_codec_context(NULL),
    m_target_stream(-1),
    m_converter(NULL)
{
}

FFmpegFileReader::~FFmpegFileReader() {
  if (m_format_context)
    av_close_input_file(m_format_context);
}

bool FFmpegFileReader::Initialize() {
  avcodec_init();
  av_register_all();
  av_register_protocol(&kFFmpegFileProtocol);

  int result = av_open_input_file(&m_format_context, m_filename.c_str(),
                                  NULL, 0, NULL);
  if (result < 0) {
    switch (result) {
      case AVERROR_NOFMT:
        printf("Error: File format not supported %s\n", m_filename.c_str());
        break;
      default:
        printf("Error: Could not open input for %s\n", m_filename.c_str());
        break;
    }
    return false;
  }
  if (av_find_stream_info(m_format_context) < 0) {
    printf("can't use FFmpeg to parse stream info\n");
    return false;
  }

  for (size_t i = 0; i < m_format_context->nb_streams; ++i) {
    m_codec_context = m_format_context->streams[i]->codec;

    // Find the video stream.
    if (m_codec_context->codec_type == CODEC_TYPE_VIDEO) {
      m_target_stream = i;
      break;
    }
  }
  if (m_target_stream == -1) {
    printf("no video in the stream\n");
    return false;
  }

  // Initialize the bitstream filter if needed.
  // TODO(hclam): find a better way to identify mp4 container.
  if (m_codec_context->codec_id == CODEC_ID_H264) {
    m_level = m_codec_context->level;
    m_profile = m_codec_context->profile;
    m_has_b_frames = m_codec_context->has_b_frames;
    //m_converter = new FFmpegBitstreamConverter("h264_mp4toannexb", m_codec_context);
  } else if (m_codec_context->codec_id == CODEC_ID_MPEG4) {
    m_converter = new FFmpegBitstreamConverter("mpeg4video_es", m_codec_context);
  } else if (m_codec_context->codec_id == CODEC_ID_WMV3) {
    m_converter = new FFmpegBitstreamConverter("vc1_asftorcv", m_codec_context);
  } else if (m_codec_context->codec_id == CODEC_ID_VC1) {
    m_converter = new FFmpegBitstreamConverter("vc1_asftoannexg", m_codec_context);
  }

  if (m_converter) {
    if (!m_converter->Initialize()) {
        delete m_converter;
        m_converter = NULL;
        printf("failed to initialize bitstream converter filter\n");
        return false;
    }
  }

  return true;
}

bool FFmpegFileReader::Read(uint8_t** output, int* size, uint64_t *dts, uint64_t *pts) {
  if (!m_format_context || !m_codec_context || m_target_stream == -1) {
    *size = 0;
    *output = NULL;
    return false;
  }

  AVPacket packet;
  bool eof, found = false;
  while (!found) {
    int result = av_read_frame(m_format_context, &packet);
    if (result < 0) {
      *output = NULL;
      *size = 0;
      return false;
    }
    if (packet.stream_index == m_target_stream) {
      if (m_converter) {
        if (!m_converter->ConvertPacket(&packet)) {
          printf("failed to convert AVPacket\n");
        }
      }

      *output = new uint8_t[packet.size];
      *size = packet.size;
      memcpy(*output, packet.data, packet.size);
      *dts = ConvertTimestamp(packet.dts,
        m_format_context->streams[packet.stream_index]->time_base.den,
        m_format_context->streams[packet.stream_index]->time_base.num);
      *pts = ConvertTimestamp(packet.pts,
        m_format_context->streams[packet.stream_index]->time_base.den,
        m_format_context->streams[packet.stream_index]->time_base.num);
      if(m_format_context->pb)
        eof = m_format_context->pb->eof_reached;
      if(packet.size == 0)
        eof = true;

      found = true;
    }
    av_free_packet(&packet);
  }
  
  return eof;
}

uint64_t FFmpegFileReader::ConvertTimestamp(uint64_t pts, int den, int num)
{
  if (pts == (uint64_t)AV_NOPTS_VALUE)
    return 0LL;

  // do calculations in floats as they can easily overflow otherwise
  // we don't care for having a completly exact timestamp anyway
  double timestamp = (double)pts * num  / den;
  double starttime = 0.0f;

  if ((uint64_t)m_format_context->start_time != (uint64_t)AV_NOPTS_VALUE)
    starttime = (double)m_format_context->start_time / AV_TIME_BASE;

  if(timestamp > starttime)
    timestamp -= starttime;
  else if( timestamp + 0.1f > starttime )
    timestamp = 0;

  return timestamp*DVD_TIME_BASE*1000.0;
}

AVCodecContext* FFmpegFileReader::GetCodecContext(void)
{
  return(m_codec_context);
}
