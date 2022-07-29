#pragma once

#include "ffmpeg.h"
#include <stdexcept>
#include <string>

enum StreamErrorCode
{
  STREAM_ERROR_ANY = -0xA0000000,
};

class StreamError : public std::exception
{
public:
  explicit StreamError(int av_err) noexcept;
  explicit StreamError(const std::string &what_arg) noexcept;
  StreamError(int code, const std::string &what_arg) noexcept;

  int code() const noexcept;
  const char *what() const noexcept;

private:
  int code_;
  std::string what_message_;
};