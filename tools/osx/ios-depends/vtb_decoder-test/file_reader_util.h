// Copyright (c) 2010 The Chromium Authors. All rights reserved.  Use of this
// source code is governed by a BSD-style license that can be found in the
// LICENSE file.

#ifndef MEDIA_TOOLS_OMX_TEST_FILE_READER_UTIL_H_
#define MEDIA_TOOLS_OMX_TEST_FILE_READER_UTIL_H_

#include <string>

struct AVCodecContext;
struct AVFormatContext;

class BitstreamConverter;

class FFmpegFileReader {
 public:
  explicit FFmpegFileReader(const std::string& filename);
  virtual ~FFmpegFileReader();
  virtual bool Initialize();
  virtual bool Read(uint8_t** output, int* size, uint64_t *dts, uint64_t *pts);
  uint64_t ConvertTimestamp(uint64_t pts, int den, int num);

  AVCodecContext* GetCodecContext(void);

 private:
  std::string m_filename;
  AVFormatContext* m_format_context;
  AVCodecContext* m_codec_context;
  int m_target_stream;
#define FF_PROFILE_H264_BASELINE    66
#define FF_PROFILE_H264_MAIN        77
#define FF_PROFILE_H264_EXTENDED    88
#define FF_PROFILE_H264_HIGH        100
#define FF_PROFILE_H264_HIGH_10     110
#define FF_PROFILE_H264_HIGH_422    122
#define FF_PROFILE_H264_HIGH_444    244
#define FF_PROFILE_H264_CAVLC_444   44
  int m_level;
#define FF_PROFILE_H264_BASELINE    66
#define FF_PROFILE_H264_MAIN        77
#define FF_PROFILE_H264_EXTENDED    88
#define FF_PROFILE_H264_HIGH        100
#define FF_PROFILE_H264_HIGH_10     110
#define FF_PROFILE_H264_HIGH_422    122
#define FF_PROFILE_H264_HIGH_444    244
#define FF_PROFILE_H264_CAVLC_444   44
  int m_profile;
  int m_has_b_frames;

  BitstreamConverter *m_converter;
};

#endif
