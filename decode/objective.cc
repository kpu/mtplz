#include "decode/objective.hh"

#include "decode/weights.hh"

namespace decode {

Objective::Objective(
    const pt::Access &phrase_access,
    const lm::ngram::State &lm_begin_sentence_state)
  : feature_init_(phrase_access),
    lm_begin_sentence_state_(lm_begin_sentence_state) {}

void Objective::AddFeature(Feature &feature) {
  feature.Init(feature_init_);
  features_.push_back(FeatureInfo{&feature,dense_feature_count_});
  dense_feature_count_ += feature.DenseFeatureCount();
  weights.resize(dense_feature_count_, 1);
}

std::vector<float> Objective::GetFeatureValues(const Hypothesis &hypothesis) {
  assert(store_feature_values_);
  std::vector<float> values;
  for (std::size_t i = 0; i < hypothesis_feature_values_.size(); ++i) {
    values.push_back(hypothesis_feature_values_(&hypothesis)[i]);
  }
  return values;
}

void Objective::LoadWeights(const Weights &loaded_weights) {
  assert(weights.size() == DenseFeatureCount());
  for (FeatureInfo feature : features_) {
    std::vector<float> feature_weights = loaded_weights.GetWeights(feature.feature->name);
    assert(feature_weights.size() == feature.feature->DenseFeatureCount());
    for (std::size_t j=0; j < feature_weights.size(); j++) {
      weights[feature.offset + j] = feature_weights[j];
    }
  }
  if (store_feature_values_) {
    phrase_feature_values_ = util::ArrayField<float>(
        feature_init_.target_phrase_layout, DenseFeatureCount());
    hypothesis_feature_values_ = util::ArrayField<float>(
        feature_init_.hypothesis_layout, DenseFeatureCount());
  }
}

void Objective::NewWord(const StringPiece string_rep, VocabWord *word) const {
  for (auto feature : features_) {
    feature.feature->NewWord(string_rep, word);
  }
}

void Objective::InitPassthroughPhrase(pt::Row *passthrough, TargetPhraseType type) const {
  for (auto feature : features_) {
    feature.feature->InitPassthroughPhrase(passthrough, type);
  }
}

float Objective::ScoreTargetPhrase(TargetPhraseInfo target) const {
  Hypothesis *null_hypo = nullptr;
  FeatureStore store(phrase_feature_values_, store_feature_values_ ? target.phrase : nullptr);
  store.Init();
  auto collector = GetCollector(null_hypo, nullptr, store);
  for (auto feature : features_) {
    collector.SetDenseOffset(feature.offset);
    feature.feature->ScoreTargetPhrase(target, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase,
    Hypothesis *&new_hypothesis) const {
  FeatureStore store(hypothesis_feature_values_, store_feature_values_ ? new_hypothesis : nullptr);
  store.Init();
  auto collector = GetCollector(new_hypothesis, nullptr, store);
  for (auto feature : features_) {
    collector.SetDenseOffset(feature.offset);
    feature.feature->ScoreHypothesisWithSourcePhrase(hypothesis, source_phrase, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair,
    Hypothesis *&new_hypothesis, util::Pool &hypothesis_pool) const {
  FeatureStore store(hypothesis_feature_values_, store_feature_values_ ? new_hypothesis : nullptr);
  if (store_feature_values_) { // copy phrase values to hypothesis
    for (std::size_t i = 0; i < phrase_feature_values_.size(); ++i) {
      store()[i] += phrase_feature_values_(phrase_pair.target)[i];
    }
  }
  auto collector = GetCollector(new_hypothesis, &hypothesis_pool, store);
  for (auto feature : features_) {
    collector.SetDenseOffset(feature.offset);
    feature.feature->ScoreHypothesisWithPhrasePair(hypothesis, phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreFinalHypothesis(Hypothesis &hypothesis) const {
  Hypothesis *null_hypo = nullptr;
  FeatureStore store(hypothesis_feature_values_, store_feature_values_ ? &hypothesis : nullptr);
  auto collector = GetCollector(null_hypo, nullptr, store);
  for (auto feature : features_) {
    collector.SetDenseOffset(feature.offset);
    feature.feature->ScoreFinalHypothesis(hypothesis, collector);
  }
  return collector.Score();
}

std::size_t Objective::DenseFeatureCount() const {
  return dense_feature_count_;
}

std::string Objective::FeatureDescription(std::size_t index) const {
  assert(index < dense_feature_count_);
  for (auto feature : features_) {
    std::size_t local_index = index - feature.offset;
    if (local_index < feature.feature->DenseFeatureCount()) {
      return feature.feature->FeatureDescription(local_index);
    }
  }
}

ScoreCollector Objective::GetCollector(
    Hypothesis *&new_hypothesis,
    util::Pool *hypothesis_pool,
    FeatureStore feature_store) const {
  return ScoreCollector(weights, new_hypothesis, hypothesis_pool, feature_store);
}

} // namespace decode
