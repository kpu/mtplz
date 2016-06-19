#pragma once

#include "decode/feature.hh"

#include <assert.h>
#include <vector>

namespace decode {

class Weights;

class Objective {
  public:
    std::vector<float> weights = std::vector<float>();

    explicit Objective(const pt::Access &phrase_access,
        const lm::ngram::State &lm_begin_sentence_state);

    void AddFeature(Feature &feature);

    // cannot be const because it contains layouts,
    // which are modified on alloc
    FeatureInit &GetFeatureInit() { return feature_init_; }

    void LoadWeights(const Weights &weights);

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

    const lm::ngram::State &lm_begin_sentence_state_;
    ScoreCollector GetCollector(Hypothesis *new_hypothesis, FeatureStore *storage) const;
};

} // namespace decode
