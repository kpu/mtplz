#ifndef DECODE_HYPOTHESIS
#define DECODE_HYPOTHESIS

#include "decode/coverage.hh"
#include "decode/id.hh"
#include "lm/state.hh"
#include "pt/query.hh" // TODO save TargetPhrase differently?
#include "util/murmur_hash.hh"

#include <boost/utility.hpp>

#include <iosfwd>

#include <assert.h>
#include <stdint.h>

namespace util { class Pool; }

namespace decode {

typedef pt::Row TargetPhrase;

class Hypothesis {
  public:
    // STL default constructor.
    Hypothesis() : target_(NULL) {}

    // Extend a previous hypothesis.
    Hypothesis(
        float score,
        const Hypothesis *previous,
        std::size_t source_begin,
        std::size_t source_end,
        const TargetPhrase *target) :
      score_(score),
      pre_(previous),
      end_index_(source_end),
      target_(target),
      coverage_(previous->coverage_) {
      coverage_.Set(source_begin, source_end);
    }

    // Initialize root hypothesis.
    explicit Hypothesis(float score) :
      score_(score),
      pre_(NULL),
      end_index_(0),
      target_(NULL),
      coverage_() {}

    const Coverage &GetCoverage() const { return coverage_; }

    void SetScore(float score) { score_ = score; }
    float GetScore() const { return score_; }

    std::size_t SourceEndIndex() const { return end_index_; }

    const Hypothesis *Previous() const { return pre_; }

    const TargetPhrase *Target() const { return target_; }

  private:
    float score_;

    // Null for base hypothesis.
    const Hypothesis *pre_;
	// one past the last source index of the hypothesis extension
    std::size_t end_index_;
    // Null for base hypothesis.
    const TargetPhrase *target_;

    Coverage coverage_;
};

struct RecombineHash : public std::unary_function<const Hypothesis &, uint64_t> {
  uint64_t operator()(const Hypothesis &hypothesis) const {
    std::size_t source_index = hypothesis.SourceEndIndex();
    return util::MurmurHashNative(&source_index, sizeof(std::size_t), hash_value(hypothesis.GetCoverage()));
    // TODO hash lm state, too? original hypothesis does it
  }
};

struct RecombineEqual : public std::binary_function<const Hypothesis &, const Hypothesis &, bool> {
  bool operator()(const Hypothesis &first, const Hypothesis &second) const {
    // if (!(first.State() == second.State())) return false; // TODO
    // find equivalent with Hypothesis Layout lm_state
    if (!(first.GetCoverage() == second.GetCoverage())) return false;
    return (first.SourceEndIndex() == second.SourceEndIndex());
  }
};

} // namespace decode

#endif // DECODE_HYPOTHESIS
