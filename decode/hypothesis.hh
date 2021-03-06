#ifndef DECODE_HYPOTHESIS
#define DECODE_HYPOTHESIS

#include "decode/coverage.hh"
#include "decode/id.hh"
#include "lm/state.hh"
#include "util/murmur_hash.hh"

#include <boost/utility.hpp>

#include <iosfwd>

#include <assert.h>
#include <stdint.h>

namespace util { class Pool; }

namespace decode {

// instances of the target phrase layout (see FeatureInit)
struct TargetPhrase;

/** Hypothesis
 * Stores the overall score of the current hypothesis along with its
 * coverage, the most recently added target phrase, and a pointer to the
 * previous hypothesis.
 *
 * The hypothesis object stores only the most basic attributes,
 * all other attributes are stored in the hypothesis layout
 * (see FeatureInit).
 */
class Hypothesis {
  public:
    /** STL default constructor. */
    Hypothesis() : target_(NULL) {}

    /** Extend a previous hypothesis. */
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

    /** Initialize root hypothesis. */
    explicit Hypothesis(float score, const TargetPhrase *target) :
      score_(score),
      pre_(NULL),
      end_index_(0),
      target_(target),
      coverage_() {}

    /** Initialize hypothesis extension for source phrase pairing */
    explicit Hypothesis(const Hypothesis *previous) :
      score_(0),
      pre_(previous),
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

} // namespace decode

#endif // DECODE_HYPOTHESIS
