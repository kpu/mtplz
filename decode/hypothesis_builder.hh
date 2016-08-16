#pragma once

#include "decode/hypothesis.hh"
#include "decode/feature_init.hh"

namespace pt { struct Row; }

namespace decode {

/**HypothesisBuilder:
 * Allocates and initializes new hypotheses in a pool.
 */
class HypothesisBuilder {
  public:
    HypothesisBuilder(util::Pool &pool, FeatureInit &feature_init)
      : pool_(pool), feature_init_(feature_init) {}

    /** Build root hypothesis */
    Hypothesis *BuildHypothesis(
        const lm::ngram::Right &state,
        float score,
        const pt::Row *target);

    /** Initializes an instance of Hypothesis on the layout at *base */
    Hypothesis *BuildHypothesis(
        Hypothesis *base,
        const lm::ngram::Right &state,
        float score,
        const Hypothesis *previous,
        std::size_t source_begin,
        std::size_t source_end,
        const TargetPhrase *target);

    /** Allocates an incomplete hypothesis, consisting only of a
     * back-reference */
    Hypothesis *NextHypothesis(const Hypothesis *previous_hypothesis);

    /** Allocates a copy of the fixed-size part of hypothesis */
    Hypothesis *CopyHypothesis(Hypothesis *hypothesis) const;

    util::Pool &HypothesisPool() {
      return pool_;
    }
  private:
    FeatureInit &feature_init_;

    util::Pool &pool_;
};

} // namespace decode
