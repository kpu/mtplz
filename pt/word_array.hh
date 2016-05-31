#pragma once

#include "pt/format.hh"

#include <cassert>
#include <cstring>

namespace pt {

class WordArray {
  public:
    explicit WordArray(FileFormat &format)
      : array_(format.Attach()),
        current_(array_.end()) {
      assert(current_ >= array_.begin());
      if (format.Writing())
        Resize(1);
      assert(current_ >= array_.begin());
    }

    void operator()(StringPiece str) {
      while (current_ + str.size() + 1 > array_.end()) {
        Resize(array_.size() << 1);
      }
      std::memcpy(current_, str.data(), str.size());
      current_ += str.size();
      *current_++ = 0;
      assert(current_ >= array_.begin());
    }

    void Finish() {
      assert(current_ >= array_.begin());
      if (current_ != array_.end()) {
        HugeRealloc(current_ - array_.begin(), false, array_);
      }
    }

  private:
    void Resize(std::size_t to) {
      assert(current_ >= array_.begin());
      std::size_t off = current_ - array_.begin();
      HugeRealloc(to, false, array_);
      current_ = array_.begin() + off;
      assert(current_ >= array_.begin());
    }

    WordArray(const WordArray &) = delete;
    WordArray &operator=(const WordArray &) = delete;

    util::scoped_memory &array_;

    char *current_;
};

} // namespace pt
