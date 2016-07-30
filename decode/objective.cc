#include "decode/objective.hh"

#include "decode/weights.hh"

namespace decode {

Objective::Objective(
    const pt::Access &phrase_access,
    const lm::ngram::State &lm_begin_sentence_state)
  : feature_init_(phrase_access), feature_offsets_(),
    lm_begin_sentence_state_(lm_begin_sentence_state) {
  feature_offsets_.push_back(0);
}

void Objective::AddFeature(Feature &feature) {
  features_.push_back(&feature);
  feature.Init(feature_init_);
  std::size_t feature_end = feature_offsets_.back() + feature.DenseFeatureCount();
  feature_offsets_.push_back(feature_end);
  weights.resize(feature_end, 1);
}

void Objective::LoadWeights(const Weights &loaded_weights) {
  assert(weights.size() == DenseFeatureCount());
  for (std::size_t i=0; i < features_.size(); i++) {
    std::vector<float> feature_weights = loaded_weights.GetWeights(features_[i]->name);
    assert(feature_weights.size() == features_[i]->DenseFeatureCount());
    for (std::size_t j=0; j < feature_weights.size(); j++) {
      weights[feature_offsets_[i] + j] = feature_weights[j];
    }
  }
}

void Objective::NewWord(const StringPiece string_rep, VocabWord *word) const {
  for (auto feature : features_) {
    feature->NewWord(string_rep, word);
  }
}

void Objective::InitPassthroughPhrase(pt::Row *passthrough) const {
  for (auto feature : features_) {
    feature->InitPassthroughPhrase(passthrough);
  }
}

float Objective::ScorePhrase(PhrasePair phrase_pair, FeatureStore *storage) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr, storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScorePhrase(phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase,
    Hypothesis *&new_hypothesis, FeatureStore *storage) const {
  auto collector = GetCollector(new_hypothesis, nullptr, storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScoreHypothesisWithSourcePhrase(hypothesis, source_phrase, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair,
    Hypothesis *&new_hypothesis, util::Pool &hypothesis_pool,
    FeatureStore *storage) const {
  auto collector = GetCollector(new_hypothesis, &hypothesis_pool, storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScoreHypothesisWithPhrasePair(hypothesis, phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreFinalHypothesis(
    const Hypothesis &hypothesis, FeatureStore *storage) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr, storage);
  for (std::size_t i=0; i<features_.size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[i]->ScoreFinalHypothesis(hypothesis, collector);
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

ScoreCollector Objective::GetCollector(
    Hypothesis *&new_hypothesis,
    util::Pool *hypothesis_pool,
    FeatureStore *storage) const {
  return ScoreCollector(weights, new_hypothesis, hypothesis_pool, storage);
}

} // namespace decode
