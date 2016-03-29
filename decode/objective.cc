#include "decode/objective.hh"

#include "util/exception.hh"

namespace decode {
  // TODO make exception classes more descriptive

void Objective::AddFeature(Feature &feature) {
  features_.push_back(&feature);
  feature.Init(*feature_init_);
}

void Objective::ScorePhrase(PhrasePair phrase_pair) const {
  auto collector = getCollector();
  ScorePhrase(phrase_pair, collector);
  // TODO read collector
}
void Objective::ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const {
  // TODO
}

void Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const Phrase &phrase, const Span source_span) const {
  auto collector = getCollector();
  ScoreHypothesisWithSourcePhrase(hypothesis, phrase, source_span, collector);
  // TODO read collector
}
void Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const Phrase &phrase,
    const Span source_span, ScoreCollector &collector) const {
  // TODO
}

void Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair, const Span source_span) const {
  auto collector = getCollector();
  ScoreHypothesisWithPhrasePair(hypothesis, phrase_pair, source_span);
  // TODO read collector
}
void Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair,
    const Span source_span, ScoreCollector &collector) const {
  // TODO
}

unsigned Objective::DenseFeatureCount() const {
  unsigned feature_count = 0;
  for (const Feature *feature : features_) {
    feature_count += feature->DenseFeatureCount();
  }
  return feature_count;
}

std::string Objective::FeatureDescription(unsigned index) const {
  unsigned local_index = index;
  for (const Feature *feature: features_) {
    unsigned current_feature_count = feature->DenseFeatureCount();
    if (current_feature_count <= local_index) {
      local_index -= current_feature_count;
    } else {
      return feature->FeatureDescription(local_index);
    }
  }
  // TODO std::out_of_range instead?
  UTIL_THROW(util::Exception, "index out of range!" << index);
}

void Objective::Init(FeatureInit &feature_init) {
  UTIL_THROW(util::Exception, "Objective does not need to be initialized!");
}

ScoreCollector Objective::getCollector() const {
  return ScoreCollector(features_);
}

} // namespace decode
