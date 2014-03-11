#ifndef DECODE_COVERAGE
#define DECODE_COVERAGE

#include "util/murmur_hash.hh"

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
