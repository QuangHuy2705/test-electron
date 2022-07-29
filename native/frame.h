#pragma once

#include "ffmpeg.h"
#include "logger.h"
#include <emscripten/val.h>
#include <memory>

class Frame
{
public:
  Frame(AVFrame* f) : frame_(f) {}

  ~Frame() {
    if (frame_) {
      av_frame_free(&frame_);
    }
  }

  int linesize(size_t i) { return frame_->linesize[i]; }
  int width() const { return frame_->width; }
  int height() const { return frame_->height; }
  int format() const { return frame_->format; }
  // int64_t in C++ results in UnboundTypeError
  //  https://github.com/emscripten-core/emscripten/issues/11140
  double pts() const { return frame_->pts; }

  emscripten::val GetBytes(size_t i, size_t n) {
    return emscripten::val(emscripten::typed_memory_view(frame_->linesize[i] * n, frame_->data[i]));
  }

private:
  AVFrame* frame_;
};
