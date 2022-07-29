#pragma once 

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class SafeQueue {
public:
  using clear_callback_t = std::function<bool(T)>;

  SafeQueue() : q(), m(), c() {}

  ~SafeQueue(void) {}

  // Add an element to the queue.
  bool enqueue(T t) {
    std::lock_guard<std::mutex> lock(m);
    if (cap > 0 && q.size() >= cap) {
      return false;
    }
    q.push(std::move(t));
    c.notify_one();
    return true;
  }

  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T dequeue(void) {
    std::unique_lock<std::mutex> lock(m);
    c.wait(lock, [&] { return !q.empty(); });
    T val = std::move(q.front());
    q.pop();
    return std::move(val);
  }

  T dequeue_n(void) {
    std::unique_lock<std::mutex> lock(m);
    if (q.empty()) return nullptr;

    T val = std::move(q.front());
    q.pop();
    return std::move(val);
  }

  size_t size() const {
    return q.size();
  }

  void clear(clear_callback_t cb) {
    std::lock_guard<std::mutex> lock(m);
    while (q.size()) {
      if (!cb || cb(q.front())) q.pop();
    }
  }

  size_t capacity() const {
    return cap;
  }

  void set_capacity(size_t c) {
    cap = c;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
  size_t cap;
};