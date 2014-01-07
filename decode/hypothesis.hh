#ifdef DECODE_HYPOTHESIS__
#define DECODE_HYPOTHESIS__

#include "decode/id.hh"
#include "lm/state.hh"
#include "util/murmur_hash.hh"

#include <boost/utility.hpp>

#include <assert.h>
#include <stdint.h>

namespace util { class Pool; }

namespace decode {

// TODO: coverage longer than 64 bits, which is needed if max_phrase + distortion > 64
class Coverage {
  public:
    Coverage() : first_zero_(0), bits_(0) {}

    bool operator==(const Coverage &other) const {
      return (first_zero_ == other_.first_zero_) && (bits_ == other_.bits_);
    }

    void Set(std::size_t begin, std::size_t end) {
      assert(Compatible(begin, end));
      if (begin == first_zero_) {
        first_zero_ = end;
        bits_ >>= (end - begin);
      } else {
        bits_ |= Pattern(begin, end);
      }
    }

    bool Compatible(std::size_t begin, std::size_t end) const {
      return (begin >= first_zero_) && !(Pattern(begin, end) & bits_);
    }

    std::size_t FirstZero() const { return first_zero_: }

  private:
    friend std::size_t hash_value(const Coverage &coverage);

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

uint64_t hash_value(const Coverage &coverage) {
  return util::MurmurHashNative(&coverage.first_zero_, sizeof(coverage.first_zero_), coverage.bits_);
}

// TODO properly factored feature state.
class Hypothesis : boost::noncopyable {
  public:
    // Extend a previous hypothesis.
    Hypothesis(
        const lm::ngram::Right &state,
        float score,
        const Hypothesis &previous,
        std::size_t source_begin,
        std::size_t source_end,
        const Phrase *target) :
      score_(score),
      state_(state),
      pre_(&previous),
      last_source_index_(source_end),
      target_(&target),
      coverage_(previous.coverage_) {
      coverage_.Set(source_begin, source_end);
    }

    // Initialize root hypothesis.  Provide the LM's BeginSentence.
    Hypothesis(const lm::ngram::Right &begin_sentence) :
      score_(0.0),
      state_(begin_sentence),
      pre_(NULL),
      last_source_index_(0),
      target_(NULL),
      coverage_() {}

    const Coverage &GetCoverage() const { return coverage_; }

    float Score() const { return score_; }

    const lm::ngram::Right &State() const { return state_; }

  private:
    float score_;

    lm::ngram::Right state_;
    // Null for base hypothesis.
    const Hypothesis *pre_;
    std::size_t last_source_index_;
    // Null for base hypothesis.
    const Phrase *target_;

    Coverage coverage_;
};



} // namespace decode

#endif // DECODE_HYPOTHESIS__
