#include "decode/objective.hh"

#include "util/exception.hh"

namespace decode {

Objective::Objective(FeatureInit feature_init)
  : feature_init_(feature_init), feature_offsets_() {
  feature_offsets_.push_back(0);
}

void Objective::AddFeature(Feature &feature) {
  features_.push_back(&feature);
  feature.Init(feature_init_);
  unsigned dense_feature_count = feature.DenseFeatureCount();
  feature_offsets_.push_back(dense_feature_count);
  for (unsigned i=0; i<dense_feature_count; i++) {
    weights.push_back(1);
  }
}

void Objective::ScorePhrase(PhrasePair phrase_pair) const {
  auto collector = getCollector();
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i].ScorePhrase(phrase_pair, collector);
  }
  // TODO read collector
}

void Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const Phrase &phrase, const Span source_span) const {
  auto collector = getCollector();
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    // TODO call feature (see ScorePhrase)
  }
  // TODO read collector
}

void Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair, const Span source_span) const {
  auto collector = getCollector();
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    // TODO call feature (see ScorePhrase)
  }
  // TODO read collector
}

unsigned Objective::DenseFeatureCount() const {
  unsigned feature_count = 0;
  for (unsigned i = 0; i < features_.size(); i++) {
    feature_count += features_[i].DenseFeatureCount();
  }
  return feature_count;
}

std::string Objective::FeatureDescription(unsigned index) const {
  assert(index < feature_offsets_.back());
  for (unsigned i=0; ; i++) {
    if (index < feature_offsets_[i+1]) {
      unsigned local_index = index - feature_offsets_[i];
      return features_[local_index].FeatureDescription(local_index);
    }
  }
}

ScoreCollector Objective::getCollector() const {
  return ScoreCollector(); // TODO condition: which ScoreCollector to use (accumulating?)
}

} // namespace decode
