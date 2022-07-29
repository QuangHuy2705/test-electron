#pragma once

#include "error.h"
#include "ffmpeg.h"
#include "logger.h"

class Packet
{
public:
  Packet(AVPacket* p) : packet_(p) {}

  ~Packet() {
    // LOG(INFO) << "free packet";
    if (packet_) av_packet_free(&packet_);
  }

  AVPacket* data() {
    return packet_;
  }

private:
  AVPacket* packet_;
};
