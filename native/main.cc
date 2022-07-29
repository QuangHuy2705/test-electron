#include "decoder.h"
#include "frame.h"
#include "options.h"
#include <emscripten/bind.h>

using namespace emscripten; // NOLINT

EMSCRIPTEN_BINDINGS(decoder) {
  value_object<StreamVideoOptions>("StreamOptions")
    .field("threadCount", &StreamVideoOptions::dec_thread_count)
    .field("threadType", &StreamVideoOptions::dec_thread_type)
    .field("speed", &StreamVideoOptions::speed)
    .field("maxFrameCount", &StreamVideoOptions::dec_max_frame_count)
    .field("maxDataCount", &StreamVideoOptions::dec_max_data_count);

  class_<Frame>("Frame")
    .smart_ptr<std::shared_ptr<Frame>>("shared_ptr<Frame>")
    .property("width", &Frame::width)
    .property("height", &Frame::height)
    .property("format", &Frame::format)
    .property("pts", &Frame::pts)
    .function("linesize", &Frame::linesize)
    .function("getBytes", &Frame::GetBytes);

  class_<Decoder>("Decoder")
    .constructor<>()
    .property("queueSize", &Decoder::GetQueueSize)
    .property("frameQueueSize", &Decoder::GetFrameQueueSize)
    .function("setOptions", &Decoder::SetOptions, allow_raw_pointers())
    .function("getFrame", &Decoder::GetFrame, allow_raw_pointers())
    .function("configure", &Decoder::Configure, allow_raw_pointers())
    .function("decode", &Decoder::Decode, allow_raw_pointers())
    .function("stopThread", &Decoder::stop_codec_thread, allow_raw_pointers())
    .function("clearQueue", &Decoder::clear_queue);
}