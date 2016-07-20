#include "decode/hypothesis_builder.hh"

namespace decode {
  
Hypothesis *HypothesisBuilder::BuildHypothesis(const lm::ngram::Right &state, float score) {
  void *hypo = feature_init_.hypothesis_layout.Allocate(pool_);
  feature_init_.hypothesis_field(hypo) = Hypothesis(score);
  feature_init_.lm_state_field(hypo) = state;
  return reinterpret_cast<Hypothesis*>(hypo);
}

Hypothesis *HypothesisBuilder::BuildHypothesis(
    Hypothesis *base,
    const lm::ngram::Right &state,
    float score,
    const Hypothesis *previous,
    std::size_t source_begin,
    std::size_t source_end,
    const TargetPhrase *target) {
  feature_init_.hypothesis_field(base) = Hypothesis(score, previous, source_begin, source_end, target);
  feature_init_.lm_state_field(base) = state;
  return base;
}

Hypothesis *HypothesisBuilder::NextHypothesis(const Hypothesis *previous_hypothesis) {
  void *hypo = feature_init_.hypothesis_layout.Allocate(pool_);
  feature_init_.hypothesis_field(hypo) = Hypothesis(previous_hypothesis);
  return reinterpret_cast<Hypothesis*>(hypo);
}

Hypothesis *HypothesisBuilder::CopyHypothesis(Hypothesis *hypothesis) const {
  std::size_t hypothesis_size = feature_init_.hypothesis_layout.OffsetsBegin();
  void *copy = feature_init_.hypothesis_layout.Allocate(pool_);
  std::memcpy(copy, hypothesis, hypothesis_size);
  return reinterpret_cast<Hypothesis*>(copy);
}

} // namespace decode
