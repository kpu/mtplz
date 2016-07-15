#pragma once

#include "decode/hypothesis.hh"
#include "lm/state.hh"
#include "pt/access.hh"
#include "util/layout.hh"

namespace decode {

/** A FeatureInit instance is owned by the Objective and is passed to each
 * Feature on Init. Features can add new fields to the provided layouts or
 * memorize already provided accessors.
 */
class FeatureInit {
  public:
    typedef lm::ngram::State LMState;

    explicit FeatureInit(const pt::Access &phrase_access) :
      phrase_access_(phrase_access),
      hypothesis_field_(hypothesis_layout_),
      lm_state_field_(hypothesis_layout_),
      pt_id_field_(word_layout_) {}

    /** The first field of a hypothesis layout is always the Hypothesis
     * object, which stores the most important attributes of a hypothesis.
     * This way, we can pass around a Hypothesis*, have easy access to
     * attributes which are always present, and can use the layout accessors
     * to access additional information.
     */
    util::Layout &HypothesisLayout() {
      return hypothesis_layout_;
    }

    util::Layout &TargetPhraseLayout() {
      return target_phrase_layout_;
    }

    util::Layout &WordLayout() {
      return word_layout_;
    }

    /** Acess to target phrase layout */
    const pt::Access &PhraseAccess() {
      return phrase_access_;
    }

    const util::PODField<Hypothesis> HypothesisField() const {
      return hypothesis_field_;
    }

    const util::PODField<LMState> LMStateField() const {
      return lm_state_field_;
    }

    const util::PODField<ID> PTIDField() const {
      return pt_id_field_;
    }

  private:
    util::Layout hypothesis_layout_;
    util::Layout target_phrase_layout_;
    util::Layout word_layout_;
    const pt::Access &phrase_access_;
    const util::PODField<Hypothesis> hypothesis_field_;
    const util::PODField<LMState> lm_state_field_;
    const util::PODField<ID> pt_id_field_;
};

} // namespace decode
