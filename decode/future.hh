#ifndef DECODE_FUTURE
#define DECODE_FUTURE

#include "decode/coverage.hh"

#include <cstddef>
#include <vector>

#include <assert.h>

/* Future cost exstimates */

namespace decode {

class Chart;

class Future {
  public:
    explicit Future(const Chart &chart);

    float Full() const {
      return Entry(0, sentence_length_plus_1_ - 1);
    }

    // Calculate change in rest cost when the given coverage is to be covered.
    float Change(const Coverage &coverage, std::size_t begin, std::size_t end) {
      std::size_t left = coverage.LeftOpen(begin);
      std::size_t right = coverage.RightOpen(end, sentence_length_plus_1_ - 1);
      return Entry(left, begin) + Entry(end, right) - Entry(left, right);
    }

  private:
    float Entry(std::size_t begin, std::size_t end) const {
      assert(end >= begin);
      assert(end < sentence_length_plus_1_);
      return entries_[begin * sentence_length_plus_1_ + end];
    }

    float &Entry(std::size_t begin, std::size_t end) {
      assert(end >= begin);
      assert(end < sentence_length_plus_1_);
      return entries_[begin * sentence_length_plus_1_ + end];
    }

    // sentence_length is a valid value of end.
    std::size_t sentence_length_plus_1_;

    // Square matrix with half the values ignored.  TODO: waste less memory.
    std::vector<float> entries_;
};

} // namespace decode

#endif // DECODE_FUTURE
