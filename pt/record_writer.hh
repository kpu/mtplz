#pragma once

#include "pt/format.hh"

#include <cassert>
#include <memory>

namespace pt {

/* Format written:
 * uin16_t count of target phrases
 * bunch of layouts one after another with target phrases
 */
class TargetWriter {
  public:
    TargetWriter(FileFormat &file, std::size_t initial_buffer = 16384)
      : buffer_(initial_buffer), 
        buffer_end_(static_cast<char*>(buffer_.get()) + initial_buffer),
        file_(file) {}

    uint64_t Offset() { return file_.DirectWriteSize(); }

  private:
    friend class TargetBundleWriter;

    void Write(char *current) {
      file_.DirectWriteTargetPhrases(buffer_.get(), current - static_cast<char*>(buffer_.get()));
    }

    void Reallocate(std::size_t to) {
      buffer_.call_realloc(to);
      buffer_end_ = static_cast<char*>(buffer_.get()) + to;
    }

    util::scoped_malloc buffer_;
    char *buffer_end_;

    FileFormat &file_;

    uint64_t written_ = 0;
};

// Write target phrases for a given source phrase.
class TargetBundleWriter {
  public:
    explicit TargetBundleWriter(TargetWriter &master) :
      master_(master),
      current_(BufferBegin() + sizeof(RowCount)) // save space for count
    {}

    ~TargetBundleWriter() {
      *reinterpret_cast<RowCount*>(BufferBegin()) = count_;
      master_.Write(current_);
    }

    void *Allocate(std::size_t size) {
      ++count_;
      if (UTIL_LIKELY(current_ + size <= BufferEnd()))
        return Increment(size);
      Reallocate(size);
      return Increment(size);
    }

    bool Continue(void *&base, std::ptrdiff_t additional) {
      assert(base >= Buffer().get() && base < BufferEnd());
      if (UTIL_LIKELY(current_ + additional <= BufferEnd())) {
        current_ += additional;
        return false;
      }
      std::size_t difference = static_cast<char*>(base) - BufferBegin();
      Reallocate(additional);
      current_ += additional;
      base = BufferBegin() + difference;
      return true;
    }

  private:
    void *Increment(std::size_t size) {
      void *ret = current_;
      current_ += size;
      return ret;
    }

    void Reallocate(std::size_t additional) {
      std::size_t existing_size = BufferEnd() - BufferBegin();
      std::size_t desired = existing_size + std::max<std::size_t>(additional, existing_size);
      std::size_t current_offset = current_ - BufferBegin();
      master_.Reallocate(desired);
      current_ = BufferBegin() + current_offset;
    }

    util::scoped_malloc &Buffer() { return master_.buffer_; }
    char *BufferBegin() { return reinterpret_cast<char*>(Buffer().get()); }
    char *BufferEnd() { return master_.buffer_end_; }

    char *current_;
    RowCount count_ = 0;
    TargetWriter &master_;
};

} // namespace pt
