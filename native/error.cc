#include "error.h"

StreamError::StreamError(int av_err) noexcept
    : code_(av_err)
{
  char str[AV_ERROR_MAX_STRING_SIZE];
  av_make_error_string(&str[0], AV_ERROR_MAX_STRING_SIZE, av_err);
  what_message_ = str;
}

StreamError::StreamError(const std::string &what_arg) noexcept
    : StreamError(STREAM_ERROR_ANY, std::move(what_arg)) {}

StreamError::StreamError(int code, const std::string &what_arg) noexcept
    : code_(code), what_message_(std::move(what_arg)) {}

int StreamError::code() const noexcept
{
  return code_;
}

const char *StreamError::what() const noexcept
{
  return what_message_.c_str();
}