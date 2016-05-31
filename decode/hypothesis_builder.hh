#pragma once

#include "decode/hypothesis.hh"
#include "decode/feature_init.hh"

namespace decode {

class HypothesisBuilder {
  public:
    HypothesisBuilder(util::Pool &pool, FeatureInit &feature_init)
      : pool_(pool), feature_init_(feature_init) {}

    // root hypothesis
    Hypothesis *BuildHypothesis(const lm::ngram::Right &state, float score) {
      void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
      feature_init_.HypothesisField()(hypo) = Hypothesis(score);
      feature_init_.LMStateField()(hypo) = state;
      return reinterpret_cast<Hypothesis*>(hypo);
    }

    Hypothesis *BuildHypothesis(
        const lm::ngram::Right &state,
        float score,
        const Hypothesis *previous,
        std::size_t source_begin,
        std::size_t source_end,
        Phrase target) {
      void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
      feature_init_.HypothesisField()(hypo) = Hypothesis(score, previous, source_begin, source_end, target);
      feature_init_.LMStateField()(hypo) = state;
      return reinterpret_cast<Hypothesis*>(hypo);
    }

  private:
    FeatureInit &feature_init_;

    util::Pool &pool_;
};

} // namespace decode
