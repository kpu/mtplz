#include "decode/objective.hh"

#include "decode/weights.hh"

namespace decode {

Objective::Objective()
  : feature_init_(), feature_offsets_() {
  feature_offsets_.push_back(0);
  util::PODField<Hypothesis> hypothesis(feature_init_.HypothesisLayout());
}

void Objective::AddFeature(Feature &feature) {
  features_.push_back(&feature);
  feature.Init(feature_init_);
  std::size_t dense_feature_count = feature.DenseFeatureCount();
  feature_offsets_.push_back(dense_feature_count);
  weights.resize(dense_feature_count, 1);
}

void Objective::LoadWeights(Weights &loaded_weights) {
  weights.resize(DenseFeatureCount());
  for (std::size_t i=0; i < features_.size(); i++) {
    std::vector<float> feature_weights = loaded_weights.GetWeights(features_[i]->name);
    assert(feature_weights.size() == features_[i]->DenseFeatureCount());
    for (std::size_t j=0; j < feature_weights.size(); j++) {
      weights[feature_offsets_[i] + j] = feature_weights[j];
    }
  }
}

float Objective::ScorePhrase(PhrasePair phrase_pair, FeatureStore *storage) const {
  auto collector = GetCollector(storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScorePhrase(phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    HypothesisAndSourcePhrase combination, FeatureStore *storage) const {
  auto collector = GetCollector(storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScoreHypothesisWithSourcePhrase(combination, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    HypothesisAndPhrasePair combination, FeatureStore *storage) const {
  auto collector = GetCollector(storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScoreHypothesisWithPhrasePair(combination, collector);
  }
  return collector.Score();
}

float Objective::RescoreHypothesis(
    const Hypothesis &hypothesis, FeatureStore *storage) const {
  auto collector = GetCollector(storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->RescoreHypothesis(hypothesis, collector);
  }
  return collector.Score();
}

std::size_t Objective::DenseFeatureCount() const {
  return feature_offsets_.back();
}

std::string Objective::FeatureDescription(std::size_t index) const {
  assert(index < feature_offsets_.back());
  for (std::size_t i=0; ; i++) {
    if (index < feature_offsets_[i+1]) {
      std::size_t local_index = index - feature_offsets_[i];
      return features_[local_index]->FeatureDescription(local_index);
    }
  }
}

ScoreCollector Objective::GetCollector(FeatureStore *storage) const {
  return ScoreCollector(weights, storage);
}

} // namespace decode
