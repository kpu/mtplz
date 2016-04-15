#ifndef DECODE_HYPOTHESIS
#define DECODE_HYPOTHESIS

#include "decode/coverage.hh"
#include "decode/id.hh"
#include "decode/phrase.hh"
#include "util/murmur_hash.hh"

#include <boost/utility.hpp>

#include <iosfwd>

#include <assert.h>
#include <stdint.h>

namespace util { class Pool; }

namespace decode {

// TODO properly factored feature state.
class Hypothesis {
  public:
    // STL default constructor.
    Hypothesis() : target_(NULL) {}

    // Extend a previous hypothesis.
    Hypothesis(
        float score,
        const Hypothesis &previous,
        std::size_t source_begin,
        std::size_t source_end,
        Phrase target) :
      score_(score),
      pre_(&previous),
      last_source_index_(source_end),
      target_(target),
      coverage_(previous.coverage_) {
      coverage_.Set(source_begin, source_end);
    }

    // Initialize root hypothesis.  Provide the LM's BeginSentence.
    explicit Hypothesis(float score) :
      score_(score),
      pre_(NULL),
      last_source_index_(0),
      target_(NULL),
      coverage_() {}

    const Coverage &GetCoverage() const { return coverage_; }

    float Score() const { return score_; }

    std::size_t LastSourceIndex() const { return last_source_index_; }

    const Hypothesis *Previous() const { return pre_; }

    const Phrase &Target() const { return target_; }

  private:
    float score_;

    // Null for base hypothesis.
    const Hypothesis *pre_;
	// TODO: this is really "one past the last source index." We have to clean this up.
    std::size_t last_source_index_;
    // Null for base hypothesis.
    Phrase target_;

    Coverage coverage_;
};

struct RecombineHash : public std::unary_function<const Hypothesis &, uint64_t> {
  uint64_t operator()(const Hypothesis &hypothesis) const {
    std::size_t source_index = hypothesis.LastSourceIndex();
    return util::MurmurHashNative(&source_index, sizeof(std::size_t), hash_value(hypothesis.GetCoverage()));
  }
};

struct RecombineEqual : public std::binary_function<const Hypothesis &, const Hypothesis &, bool> {
  bool operator()(const Hypothesis &first, const Hypothesis &second) const {
    if (!(first.GetCoverage() == second.GetCoverage())) return false;
    return (first.LastSourceIndex() == second.LastSourceIndex());
  }
};

} // namespace decode

#endif // DECODE_HYPOTHESIS
