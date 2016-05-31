#pragma once

#include "decode/hypothesis.hh"
#include "lm/state.hh"
#include "util/layout.hh"

namespace decode {

class FeatureInit {
  public:
    typedef lm::ngram::State LMState;

    FeatureInit() :
      hypothesis_field_(hypothesis_layout_),
      lm_state_field_(hypothesis_layout_) {}

    util::Layout &HypothesisLayout() {
      return hypothesis_layout_;
    }

    util::Layout &TargetPhraseLayout() {
      return target_phrase_layout_;
    }

    util::Layout &WordLayout() {
      return word_layout_;
    }

    util::PODField<Hypothesis> HypothesisField() const {
      return hypothesis_field_;
    }

    util::PODField<LMState> LMStateField() const {
      return lm_state_field_;
    }

  private:
    util::Layout hypothesis_layout_ = util::Layout();
    util::Layout target_phrase_layout_ = util::Layout();
    util::Layout word_layout_ = util::Layout();
    const util::PODField<Hypothesis> hypothesis_field_;
    const util::PODField<LMState> lm_state_field_;
};

} // namespace decode
