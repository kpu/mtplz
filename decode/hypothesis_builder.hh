#pragma once

#include "decode/hypothesis.hh"
#include "decode/feature_init.hh"

namespace decode {

class HypothesisBuilder {
  public:
    HypothesisBuilder(util::Pool &pool, FeatureInit &feature_init)
      : pool_(pool), feature_init_(feature_init) {}

    // root hypothesis
    Hypothesis *BuildHypothesis(const lm::ngram::Right &state, float score);

    Hypothesis *BuildHypothesis(
        Hypothesis *base,
        const lm::ngram::Right &state,
        float score,
        const Hypothesis *previous,
        std::size_t source_begin,
        std::size_t source_end,
        const TargetPhrase *target);

    inline Hypothesis *NextHypothesis();

  private:
    FeatureInit &feature_init_;

    util::Pool &pool_;
};

} // namespace decode
