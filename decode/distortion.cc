#include "decode/distortion.hh"

namespace decode {

Distortion::Distortion() : Feature("distortion") {}

uint8_t Distortion::Init(FeatureInit &feature_init) {
  return ScoreMethod::Source;
}

void Distortion::ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const {
  std::size_t jump_size = abs(
      (int) hypothesis.SourceEndIndex() - (int) source_phrase.Span().first);
  collector.AddDense(0, jump_size);
}

std::size_t Distortion::DenseFeatureCount() const { return 1; }

std::string Distortion::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "distortion";
}

} // namespace decode
