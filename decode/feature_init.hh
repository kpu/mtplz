#pragma once

#include "decode/hypothesis.hh"
#include "lm/state.hh"
#include "pt/access.hh"
#include "util/layout.hh"

namespace decode {

struct VocabWord; // conforms to FeatureInit word_layout

typedef lm::ngram::Right LMState;

/** A FeatureInit instance is owned by the Objective and is passed to each
 * Feature on Init. Features can add new fields to the provided layouts or
 * memorize already provided accessors.
 */
struct FeatureInit {

  explicit FeatureInit(const pt::Access &phrase_accessor) :
    phrase_access(phrase_accessor),
    hypothesis_field(hypothesis_layout),
    lm_state_field(hypothesis_layout),
    pt_id_field(word_layout),
    pt_row_field(target_phrase_layout),
    phrase_score_field(target_phrase_layout),
    passthrough_field(target_phrase_layout) {}

  /** The first field of a hypothesis layout is always the Hypothesis
    * object, which stores the most important attributes of a hypothesis.
    * This way, we can pass around a Hypothesis*, have easy access to
    * attributes which are always present, and can use the layout accessors
    * to access additional information.
    *
    * A hypothesis layout can use all types of data which the layout class
    * offers. However, VectorFields can only be updated when scoring with a
    * target phrase.
    */
  util::Layout hypothesis_layout;
  const util::PODField<Hypothesis> hypothesis_field; // has to be first field
  const util::PODField<LMState> lm_state_field;

  /** Use to store information about the target phrase when scoring in
    * isolation (ScorePhrase). */
  util::Layout target_phrase_layout;
  const util::PODField<const pt::Row*> pt_row_field; // has to be first field
  const util::PODField<float> phrase_score_field;
  const util::PODField<bool> passthrough_field;

  /** Store info about a word when NewWord is called. This call happens
    * once in bulk for all known source words from training and later every
    * time a new word is encountered.
    */
  util::Layout word_layout;
  const util::PODField<ID> pt_id_field;

  const pt::Access &phrase_access;
};

} // namespace decode
