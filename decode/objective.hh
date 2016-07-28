#pragma once

#include "decode/feature.hh"

#include <assert.h>
#include <vector>

namespace decode {

class Weights;
class LM;
class TargetPhraseInitializer;

class Objective {
  public:
    std::vector<float> weights = std::vector<float>();

    explicit Objective(const pt::Access &phrase_access,
        const lm::ngram::State &lm_begin_sentence_state);

    void AddFeature(Feature &feature);

    void RegisterLanguageModel(TargetPhraseInitializer &lm_feature) {
      lm_feature_ = &lm_feature;
    }

    const TargetPhraseInitializer *GetLanguageModelFeature() const {
      return lm_feature_;
    }

    // cannot be const because it contains layouts,
    // which are modified on alloc
    FeatureInit &GetFeatureInit() { return feature_init_; }

    void LoadWeights(const Weights &weights);

    void NewWord(const StringPiece string_rep, VocabWord *word) const;

    void InitPassthroughPhrase(pt::Row *passthrough) const;

    // storage can be null
    float ScorePhrase(PhrasePair phrase_pair, FeatureStore *storage) const;

    // storage can be null
    float ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase,
        Hypothesis &new_hypothesis, FeatureStore *storage) const;

    // storage can be null
    float ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair,
        Hypothesis &new_hypothesis, FeatureStore *storage) const;

    // storage can be null
    float ScoreFinalHypothesis(
        const Hypothesis &hypothesis, FeatureStore *storage) const;

    std::size_t DenseFeatureCount() const;

    std::string FeatureDescription(std::size_t index) const;

    const lm::ngram::State &BeginSentenceState() const {
      return lm_begin_sentence_state_;
    }

  private:
    std::vector<Feature*> features_;
    std::vector<std::size_t> feature_offsets_;
    FeatureInit feature_init_;
    const TargetPhraseInitializer *lm_feature_ = nullptr;

    const lm::ngram::State &lm_begin_sentence_state_;
    ScoreCollector GetCollector(Hypothesis *new_hypothesis, FeatureStore *storage) const;
};

} // namespace decode
