// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_FFMPEG_FFMPEG_COMMON_H_
#define MEDIA_FFMPEG_FFMPEG_COMMON_H_

#include <stdint.h>
// Used for FFmpeg error codes.
#include <cerrno>

// Used with URLProtocol.
typedef int64_t offset_t;

// Include FFmpeg header files.
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}  // extern "C"

#endif  // MEDIA_FFMPEG_FFMPEG_COMMON_H_
