#include "decode/distortion.hh"

namespace decode {

// TODO const expr // static
const StringPiece Distortion::Name() const { return "distortion"; }

void Distortion::ScoreHypothesisWithSourcePhrase(
        HypothesisAndSourcePhrase combination, ScoreCollector &collector) const {
  std::size_t jump_size = abs(
      (int) combination.hypothesis.SourceEndIndex() - (int)combination.source_span.first);
  collector.AddDense(0, jump_size);
}

std::size_t Distortion::DenseFeatureCount() const { return 1; }

std::string Distortion::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "distortion";
}

} // namespace decode
