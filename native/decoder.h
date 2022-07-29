#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>
#include <thread>
#include <exception>
#include <typeinfo>
#include <stdexcept>

#include "buffer.h"
#include "error.h"
#include "frame.h"
#include "packet.h"
#include "logger.h"
#include "stream.h"
#include "queue.h"

using namespace std;
using namespace std::chrono;
using decode_callback_t = std::function<void(std::shared_ptr<Frame>)>;
using void_callback_t = std::function<void(void)>;

class Decoder
{
public:
  Decoder() {}

  ~Decoder() {}

  void SetOptions(StreamVideoOptions options) {
    options_ = options;
    results_.set_capacity(options.dec_max_frame_count);
    datas_.set_capacity(options.dec_max_data_count);
  }

  std::shared_ptr<Frame> GetFrame() {
    // callback results in main thread
    return results_.dequeue_n();
  }

  size_t GetQueueSize() const {
    return datas_.size();
  }

  size_t GetFrameQueueSize() const {
    return results_.size();
  }

  // embind doesn't support pointers to primitive types
  //  https://stackoverflow.com/a/27364643
  void Configure(uintptr_t buf_p, int buf_size) {
    Buffer b(buf_p, buf_size);
    // b.skip(1);
    stream_ = std::make_shared<Stream>(options_);
    time_base_ = b.read_rational();
    b.read_codec(stream_->GetParams());

    LOG(INFO) << "codec: " << stream_->GetParams()->codec_id << ", pix fmt: " << stream_->GetParams()->format << ", color range: " << stream_->GetParams()->color_range;
  }

  void Decode(uintptr_t buf_p, int buf_size, bool reset) {
    Buffer b(buf_p, buf_size);
    // b.skip(1);
    auto p = stream_->GetPacket();
    b.read_packet(p);

    if (!key_frame_found_) {
      if ((p->flags & AV_PKT_FLAG_KEY) == 0) {
        av_packet_free(&p);
        // LOG(INFO) << "waiting for keyframe";
        return;
      }
      key_frame_found_ = true;
      LOG(INFO) << "keyframe found";
    }

    start_codec_thread();
    // LOG(INFO) << "enqueue";
    if (reset) {
      LOG(INFO) << "RESET";
      datas_.clear(nullptr);
      results_.clear(nullptr);
    }

    if (!datas_.enqueue(std::make_shared<Packet>(p))) {
      LOG(INFO) << "enqueue failed";
      datas_.clear(check_keyframe);
    }
  }

  void clear_queue() {
    datas_.clear(nullptr);
    results_.clear(nullptr);
  }

  void stop_codec_thread() {
    if (stop_) return;
    clear_queue();
    stop_ = true;
    LOG(INFO) << "stop codec thread";
  }

private:
  void start_codec_thread() {
    if (!stop_) return;
    stop_ = false;
    LOG(INFO) << "start codec thread";
    std::thread(&Decoder::run_codec_thread, this).detach();
		// 
  }

  void run_codec_thread() {
    // int64_t lastPts = 0;
    int64_t skipped = 0;
    while (!stop_) {
      try {
        // LOG(INFO) << "dequeue";
        auto&& p = datas_.dequeue();
        if (!p) {
          LOG(INFO) << "nil packet";
          continue;
        }

        if (stop_) break;

        auto frame = this->stream_->GetFrame(p->data());
        if (frame == nullptr) {
          LOG(WARNING) << "decode frame is null, need new packets";
          continue;
        }

        frame->pts = 1000 * p->data()->pts * time_base_.num / time_base_.den;

        if (stop_) break;

        // lastPts = p->data()->pts;

        // if (options_.speed >= 2 && skipped <= options_.speed - 1) {
        //   ++skipped;
        //   av_frame_free(&frame);
        //   continue;
        // }

        // skipped = 0;

        results_.enqueue(std::make_shared<Frame>(frame));
      }
      catch (...) {
        LOG(WARNING) << "unknown decode error";
        // std::exception_ptr p = std::current_exception();
        // LOG(ERROR) << (p ? p.__cxa_exception_type()->name() : "null");
      }
    }
  }

  static inline bool check_keyframe(std::shared_ptr<Packet> t) {
    return (t->data()->flags & AV_PKT_FLAG_KEY) == 0;
  }

  std::shared_ptr<Stream> stream_;
  std::atomic_bool stop_{ true };
  SafeQueue<std::shared_ptr<Packet>> datas_;
  SafeQueue<std::shared_ptr<Frame>> results_;
  bool key_frame_found_{ false };
  bool background_{ false };
  StreamVideoOptions options_;
  AVRational time_base_;
};