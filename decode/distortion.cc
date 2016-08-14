#include "decode/distortion.hh"

namespace decode {

Distortion::Distortion() : Feature("distortion") {}

void Distortion::Init(FeatureInit &feature_init) {}

void Distortion::ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const {
  if (source_phrase.Length() > 0){
    std::size_t jump_size = abs(
        static_cast<int>(hypothesis.SourceEndIndex())
        - static_cast<int>(source_phrase.Span().first));
    collector.AddDense(0, jump_size);
  }
}

std::size_t Distortion::DenseFeatureCount() const { return 1; }

std::string Distortion::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "distortion";
}

} // namespace decode
