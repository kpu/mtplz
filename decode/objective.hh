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
    // if non-null, individual feature values are stored here
    std::vector<float> *feature_storage = nullptr;

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

    float ScoreTargetPhrase(TargetPhraseInfo target) const;

    float ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase,
        Hypothesis *&new_hypothesis) const;

    float ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair,
        Hypothesis *&new_hypothesis, util::Pool &hypothesis_pool) const;

    float ScoreFinalHypothesis(const Hypothesis &hypothesis) const;

    std::size_t DenseFeatureCount() const;

    std::string FeatureDescription(std::size_t index) const;

    const lm::ngram::State &BeginSentenceState() const {
      return lm_begin_sentence_state_;
    }

  private:
    struct FeatureInfo {
      const Feature *feature;
      const std::size_t offset;
      const uint8_t score_methods;
    };

    ScoreCollector GetCollector(
        Hypothesis *&new_hypothesis,
        util::Pool *hypothesis_pool) const;

    // TODO use ScoreMethod values to make list of features for each method
    std::vector<FeatureInfo> features_;
    std::size_t dense_feature_count_ = 0;

    FeatureInit feature_init_;
    const TargetPhraseInitializer *lm_feature_ = nullptr;

    const lm::ngram::State &lm_begin_sentence_state_;
};

} // namespace decode
