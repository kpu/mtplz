#pragma once

#include "pt/format.hh"

#include <cstring>

namespace pt {

class WordArray {
  public:
    explicit WordArray(FileFormat &format)
      : array_(format.Attach()),
        current_(array_.end()) {
      if (format.Writing())
        Resize(1);
    }

    void operator()(StringPiece str) {
      while (current_ + str.size() + 1 > array_.end()) {
        Resize(array_.size() << 1);
      }
      std::memcpy(current_, str.data(), str.size());
      current_ += str.size();
      *current_++ = 0;
    }

    void Finish() {
      if (current_ != array_.end()) {
        HugeRealloc(current_ - array_.begin(), false, array_);
      }
    }

  private:
    void Resize(std::size_t to) {
      std::size_t off = current_ - array_.begin();
      HugeRealloc(to, false, array_);
      current_ = array_.begin() + off;
    }

    util::scoped_memory &array_;

    char *current_;
};

} // namespace pt
