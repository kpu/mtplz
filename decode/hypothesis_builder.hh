#pragma once

#include "decode/hypothesis.hh"
#include "decode/feature_init.hh"

namespace decode {

/**HypothesisBuilder:
 * Allocates and initializes new hypotheses in a pool.
 */
class HypothesisBuilder {
  public:
    HypothesisBuilder(util::Pool &pool, FeatureInit &feature_init)
      : pool_(pool), feature_init_(feature_init) {}

    /** Build root hypothesis */
    Hypothesis *BuildHypothesis(const lm::ngram::Right &state, float score);

    /** Initializes an instance of Hypothesis on the layout at *base */
    Hypothesis *BuildHypothesis(
        Hypothesis *base,
        const lm::ngram::Right &state,
        float score,
        const Hypothesis *previous,
        std::size_t source_begin,
        std::size_t source_end,
        const TargetPhrase *target);

    /** Allocates a new hypothesis */
    Hypothesis *NextHypothesis();

    /** Allocates a copy of hypothesis */
    Hypothesis *CopyHypothesis(Hypothesis *hypothesis) const;
  private:
    FeatureInit &feature_init_;

    util::Pool &pool_;
};

} // namespace decode
