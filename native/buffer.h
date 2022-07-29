#pragma once

#include "error.h"
#include "ffmpeg.h"
#include "logger.h"
#include <memory>

struct Buffer
{
public:
  Buffer(uintptr_t buf_p, int buf_size) : bytes(reinterpret_cast<uint8_t*>(buf_p)), size(buf_size), pos(0) {}

  inline void skip(size_t size) {
    pos += size;
  }

  template <typename T>
  inline T read() {
    size_t sz = sizeof(T) / sizeof(uint8_t);
    T value = 0;
    // big endian
    for (size_t i = pos; i < pos + sz; ++i) {
      value |= bytes[i] << (8 * (sz - i - 1));
    }
    // LOG(INFO) << "size: " << sz << " pos: " << pos << " value: " << value;
    pos += sz;

    return value;
  }

  template <typename C, typename T>
  C read_cast() {
    return static_cast<C>(read<T>());
  }

  inline void read(uint8_t* data, size_t size) {
    memcpy(data, bytes + pos, size);
    pos += size;
  }

  inline AVRational read_rational() {
    AVRational t;
    t.num = read<int32_t>();
    t.den = read<int32_t>();
    return t;
  }

  void read_codec(AVCodecParameters*);
  void read_packet(AVPacket*);

private:
  size_t pos;
  size_t size;
  const uint8_t* bytes;
};
