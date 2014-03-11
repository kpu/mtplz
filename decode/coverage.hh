#ifndef DECODE_COVERAGE
#define DECODE_COVERAGE

#include "util/murmur_hash.hh"

#include <algorithm>
#include <iosfwd>

#include <assert.h>
#include <stdint.h>

namespace util { class Pool; }

namespace decode {

// TODO: coverage longer than 64 bits, which is needed if max_phrase + distortion > 64
class Coverage {
  public:
    Coverage() : first_zero_(0), bits_(0) {}

    bool operator==(const Coverage &other) const {
      return (first_zero_ == other.first_zero_) && (bits_ == other.bits_);
    }

    void Set(std::size_t begin, std::size_t end) {
      assert(Compatible(begin, end));
      if (begin == first_zero_) {
        first_zero_ = end;
        bits_ >>= (end - begin);
        while (bits_ & 1) {
          ++first_zero_;
          bits_ >>= 1;
        }
      } else {
        bits_ |= Pattern(begin, end);
      }
    }

    bool Compatible(std::size_t begin, std::size_t end) const {
      return (begin >= first_zero_) && !(Pattern(begin, end) & bits_);
    }

    std::size_t FirstZero() const { return first_zero_; }

    // The following two functions find gaps.
    // When a phrase [begin, end) is to be covered,
    //   [LeftOpen(begin), RightOpen(end, sentence_length)) 
    // indicates the larger gap in which the phrase sits.
    // Find the left bound of the gap in which the phrase [begin, ...) sits.
    // TODO: integer log2 optimization?
    std::size_t LeftOpen(std::size_t begin) const {
      for (std::size_t i = begin - first_zero_; i; --i) {
        if (bits_ & (1ULL << i)) {
          assert(Compatible(i + first_zero_ + 1, begin));
          assert(!Compatible(i + first_zero_, begin));
          return i + first_zero_ + 1;
        }
      }
      assert(Compatible(first_zero_, begin));
      return first_zero_;
    }

    // Find the right bound of the gap in which the phrase [..., end) sits.  This bit is a 1 or end of sentence.
    std::size_t RightOpen(std::size_t end, std::size_t sentence_length) const {
      for (std::size_t i = end - first_zero_; i < std::min((size_t)64, sentence_length - first_zero_); ++i) {
        if (bits_ & (1ULL << i)) {
          return i + first_zero_;
        }
      }
      return sentence_length;
    }

  private:
    friend inline uint64_t hash_value(const Coverage &coverage) {
      return util::MurmurHashNative(&coverage.first_zero_, sizeof(coverage.first_zero_), coverage.bits_);
    }

    friend std::ostream &operator<<(std::ostream &stream, const Coverage &coverage);

    inline uint64_t Pattern(std::size_t begin, std::size_t end) const {
      assert(begin >= first_zero_);
      assert(end - first_zero_ < 64);
      // 1 in the bits.
      return (1ULL << (end - first_zero_)) - (1ULL << (begin - first_zero_));
    }

    std::size_t first_zero_;
    // Bits with the first zero removed.  
    // We also assume anything beyond this is zero due to the reordering window.
    // Lowest bits correspond to next word.
    uint64_t bits_;
};

} // namespace decode

#endif // DECODE_COVERAGE
