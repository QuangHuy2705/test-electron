#pragma once

#include "error.h"
#include "ffmpeg.h"
#include "options.h"
#include <iostream>
#include <memory>
#include <utility>

class Stream
{
public:
  Stream(const StreamVideoOptions& options)
    : options_(options), format_ctx_(nullptr), codec_ctx_(nullptr), frame_(nullptr) {
    codecpar_ = avcodec_parameters_alloc();
    if (!codecpar_)
      throw StreamError("cannot alloc codecpar_");

    frame_ = av_frame_alloc();
    if (!frame_)
      throw StreamError("cannot alloc frame_");
  }

  ~Stream() noexcept {
    if (codecpar_) {
      avcodec_parameters_free(&codecpar_);
    }
    if (frame_) {
      av_frame_free(&frame_);
    }
    if (codec_ctx_) {
      avcodec_close(codec_ctx_);
      avcodec_free_context(&codec_ctx_);
    }
    if (format_ctx_) {
      avformat_close_input(&format_ctx_);
    }
  }

  AVFrame* GetFrame(AVPacket* packet) {
    if (codec_ctx_ == nullptr) {
      auto codec = avcodec_find_decoder(codecpar_->codec_id);
      if (codec == nullptr) {
        throw StreamError("Decoder not found");
      }

      codec_ctx_ = avcodec_alloc_context3(codec);
      if (codec_ctx_ == nullptr)
        throw StreamError("Codec alloc context fail");

      int ret = avcodec_parameters_to_context(codec_ctx_, codecpar_);
      if (ret < 0)
        throw StreamError(ret);

      if (options_.dec_thread_count > 0) {
        codec_ctx_->thread_count = options_.dec_thread_count;
      }

      if (options_.dec_thread_type > 0) {
        codec_ctx_->thread_type = options_.dec_thread_type;
      }

      ret = avcodec_open2(codec_ctx_, codec, nullptr);
      if (ret != 0)
        throw StreamError(ret);
    }

    int ret = avcodec_send_packet(codec_ctx_, packet);
    if (ret != 0) {
      if (ret == AVERROR(EAGAIN)) {
        return nullptr;
      }
      else {
        throw StreamError(ret);
      }
    }

    ret = avcodec_receive_frame(codec_ctx_, frame_);
    if (ret != 0) {
      if (ret == AVERROR(EAGAIN)) {
        return nullptr;
      }
      else {
        throw StreamError(ret);
      }
    }
    AVFrame* result = frame_;

    frame_ = av_frame_alloc();
    if (!frame_)
      throw StreamError("cannot alloc frame_");

    return result;
  }

  AVPacket* GetPacket() const {
    auto packet_ = av_packet_alloc();
    if (!packet_)
      throw StreamError("cannot alloc packet_");
    return packet_;
  }

  AVCodecParameters* GetParams() const {
    return codecpar_;
  }

private:
  StreamVideoOptions options_;
  AVCodecParameters* codecpar_;
  AVFormatContext* format_ctx_;
  AVCodecContext* codec_ctx_;
  AVFrame* frame_;
};
