#include "decode/hypothesis_builder.hh"

namespace decode {
  
Hypothesis *HypothesisBuilder::BuildHypothesis(const lm::ngram::Right &state, float score) {
  void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
  feature_init_.HypothesisField()(hypo) = Hypothesis(score);
  feature_init_.LMStateField()(hypo) = state;
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
  std::size_t hypothesis_size = feature_init_.HypothesisLayout().Size(base);
  void *hypo = pool_.Allocate(hypothesis_size);
  std::memcpy(hypo, base, hypothesis_size);
  feature_init_.HypothesisField()(hypo) = Hypothesis(score, previous, source_begin, source_end, target);
  feature_init_.LMStateField()(hypo) = state;
  return reinterpret_cast<Hypothesis*>(hypo);
}

Hypothesis *HypothesisBuilder::NextHypothesis() {
  void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
  return reinterpret_cast<Hypothesis*>(hypo);
}

} // namespace decode
