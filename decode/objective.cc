#include "decode/objective.hh"

#include "decode/weights.hh"

namespace decode {

Objective::Objective(
    const pt::Access &phrase_access,
    const lm::ngram::State &lm_begin_sentence_state)
  : feature_init_(phrase_access),
    lm_begin_sentence_state_(lm_begin_sentence_state) {}

void Objective::AddFeature(Feature &feature) {
  uint8_t score_methods = feature.Init(feature_init_);
  assert(score_methods <= ScoreMethod::Max);
  features_.push_back(FeatureInfo{&feature,dense_feature_count_,score_methods});
  dense_feature_count_ += feature.DenseFeatureCount();
  weights.resize(dense_feature_count_, 1);
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
}

void Objective::NewWord(const StringPiece string_rep, VocabWord *word) const {
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::NewWord) {
      feature.feature->NewWord(string_rep, word);
    }
  }
}

void Objective::InitPassthroughPhrase(pt::Row *passthrough) const {
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::InitPassthrough) {
      feature.feature->InitPassthroughPhrase(passthrough);
    }
  }
}

float Objective::ScorePhrase(PhrasePair phrase_pair) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr);
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::Phrase) {
      collector.SetDenseOffset(feature.offset);
      feature.feature->ScorePhrase(phrase_pair, collector);
    }
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase,
    Hypothesis *&new_hypothesis) const {
  auto collector = GetCollector(new_hypothesis, nullptr);
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::Source) {
      collector.SetDenseOffset(feature.offset);
      feature.feature->ScoreHypothesisWithSourcePhrase(hypothesis, source_phrase, collector);
    }
  }
  return collector.Score();
}

float Objective::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair,
    Hypothesis *&new_hypothesis, util::Pool &hypothesis_pool) const {
  auto collector = GetCollector(new_hypothesis, &hypothesis_pool);
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::Pair) {
      collector.SetDenseOffset(feature.offset);
      feature.feature->ScoreHypothesisWithPhrasePair(hypothesis, phrase_pair, collector);
    }
  }
  return collector.Score();
}

float Objective::ScoreFinalHypothesis(const Hypothesis &hypothesis) const {
  Hypothesis *null_hypo = nullptr;
  auto collector = GetCollector(null_hypo, nullptr);
  for (auto feature : features_) {
    if (feature.score_methods & ScoreMethod::Final) {
      collector.SetDenseOffset(feature.offset);
      feature.feature->ScoreFinalHypothesis(hypothesis, collector);
    }
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
    util::Pool *hypothesis_pool) const {
  return ScoreCollector(weights, new_hypothesis, hypothesis_pool, feature_storage);
}

} // namespace decode
