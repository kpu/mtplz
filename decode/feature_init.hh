#pragma once

#include "decode/hypothesis.hh"
#include "lm/state.hh"
#include "pt/access.hh"
#include "util/layout.hh"

namespace decode {

class FeatureInit {
  public:
    typedef lm::ngram::State LMState;

    explicit FeatureInit(const pt::Access phrase_access) :
      phrase_access_(phrase_access),
      hypothesis_field_(hypothesis_layout_),
      lm_state_field_(hypothesis_layout_) {}

    util::Layout &HypothesisLayout() {
      return hypothesis_layout_;
    }

    util::Layout &WordLayout() {
      return word_layout_;
    }

    const pt::Access &PhraseAccess() {
      return phrase_access_;
    }

    const util::PODField<Hypothesis> HypothesisField() const {
      return hypothesis_field_;
    }

    const util::PODField<LMState> LMStateField() const {
      return lm_state_field_;
    }

  private:
    util::Layout hypothesis_layout_;
    util::Layout word_layout_;
    const pt::Access phrase_access_;
    const util::PODField<Hypothesis> hypothesis_field_;
    const util::PODField<LMState> lm_state_field_;
};

} // namespace decode
