#include "decode/objective.hh"

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
  weights.resize(dense_feature_count, 1);
}

float Objective::ScorePhrase(PhrasePair phrase_pair, FeatureStore *storage) const {
  auto collector = getCollector(storage);
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i].ScorePhrase(phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    HypothesisAndSourcePhrase combination, FeatureStore *storage) const {
  auto collector = getCollector(storage);
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i].ScoreHypothesisWithSourcePhrase(combination, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    HypothesisAndPhrasePair combination, FeatureStore *storage) const {
  auto collector = getCollector(storage);
  for (unsigned i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i].ScoreHypothesisWithPhrasePair(combination, collector);
  }
  return collector.Score();
}

unsigned Objective::DenseFeatureCount() const {
  return feature_offsets_.back();
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

ScoreCollector Objective::getCollector(FeatureStore *storage) const {
  return ScoreCollector(weights, storage);
}

} // namespace decode
