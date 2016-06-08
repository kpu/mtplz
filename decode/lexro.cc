#include "decode/lexro.hh"
#include "util/exception.hh"

namespace decode {

void LexicalizedReordering::Init(FeatureInit &feature_init) {
  UTIL_THROW_IF(feature_init.PhraseAccess().lexical_reordering, util::Exception,
      "requested lexicalized reordering but feature values are missing in phrase access");
  phrase_access_ = &feature_init.PhraseAccess();
}

void LexicalizedReordering::ScoreHypothesisWithSourcePhrase(
    HypothesisAndSourcePhrase combination, ScoreCollector &collector) const {
  // TODO calculate
}

void LexicalizedReordering::ScoreHypothesisWithPhrasePair(
    HypothesisAndPhrasePair combination, ScoreCollector &collector) const {
  // TODO store for next
}

void LexicalizedReordering::RescoreHypothesis(
    const Hypothesis &hypothesis, ScoreCollector &collector) const {
  // TODO execute backward model
}

std::string LexicalizedReordering::FeatureDescription(std::size_t index) const {
  assert(index < DenseFeatureCount());
  switch(index) {
    case 0: return "forward lexicalized reordering";
    case 1: return "backward lexicalized reordering";
  }
}
} // namespace decode
