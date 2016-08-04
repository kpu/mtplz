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
  uint8_t score_methods = feature.Init(feature_init_);
  assert(score_methods <= ScoreMethod::Max);
  for (uint8_t i = 32; i > 0; i >>= 1) {
    if (score_methods & i) {
      features_[i].push_back(&feature);
    }
  }
  features_[0].push_back(&feature);
  std::size_t feature_end = feature_offsets_.back() + feature.DenseFeatureCount();
  feature_offsets_.push_back(feature_end);
  weights.resize(feature_end, 1);
}

void Objective::LoadWeights(const Weights &loaded_weights) {
  assert(weights.size() == DenseFeatureCount());
  std::vector<Feature*> features = features_[ScoreMethod::Any];
  for (std::size_t i=0; i < features.size(); i++) {
    std::vector<float> feature_weights = loaded_weights.GetWeights(features[i]->name);
    assert(feature_weights.size() == features[i]->DenseFeatureCount());
    for (std::size_t j=0; j < feature_weights.size(); j++) {
      weights[feature_offsets_[i] + j] = feature_weights[j];
    }
  }
}

void Objective::NewWord(const StringPiece string_rep, VocabWord *word) const {
  for (auto feature : features_[ScoreMethod::NewWord]) {
    feature->NewWord(string_rep, word);
  }
}

void Objective::InitPassthroughPhrase(pt::Row *passthrough) const {
  for (auto feature : features_[ScoreMethod::InitPassthrough]) {
    feature->InitPassthroughPhrase(passthrough);
  }
}

float Objective::ScorePhrase(PhrasePair phrase_pair) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr);
  for (std::size_t i=0; i<features_[ScoreMethod::Phrase].size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[ScoreMethod::Phrase][i]->ScorePhrase(phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase,
    Hypothesis *&new_hypothesis) const {
  auto collector = GetCollector(new_hypothesis, nullptr);
  for (std::size_t i=0; i<features_[ScoreMethod::Source].size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[ScoreMethod::Source][i]->ScoreHypothesisWithSourcePhrase(hypothesis, source_phrase, collector);
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair,
    Hypothesis *&new_hypothesis, util::Pool &hypothesis_pool) const {
  auto collector = GetCollector(new_hypothesis, &hypothesis_pool);
  for (std::size_t i=0; i<features_[ScoreMethod::Pair].size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[ScoreMethod::Pair][i]->ScoreHypothesisWithPhrasePair(hypothesis, phrase_pair, collector);
  }
  return collector.Score();
}

float Objective::ScoreFinalHypothesis(const Hypothesis &hypothesis) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr);
  for (std::size_t i=0; i<features_[ScoreMethod::Final].size(); i++) {
    collector.SetDenseOffset(feature_offsets_[i]);
    features_[ScoreMethod::Final][i]->ScoreFinalHypothesis(hypothesis, collector);
  }
  return collector.Score();
}

std::size_t Objective::DenseFeatureCount() const {
  return feature_offsets_.back();
}

std::string Objective::FeatureDescription(std::size_t index) const {
  assert(index < feature_offsets_.back());
  for (std::size_t i=0; i < feature_offsets_.back(); i++) {
    if (index < feature_offsets_[i+1]) {
      std::size_t local_index = index - feature_offsets_[i];
      return features_[ScoreMethod::Any][i]->FeatureDescription(local_index);
    }
  }
}

ScoreCollector Objective::GetCollector(
    Hypothesis *&new_hypothesis,
    util::Pool *hypothesis_pool) const {
  return ScoreCollector(weights, new_hypothesis, hypothesis_pool, feature_storage);
}

} // namespace decode
