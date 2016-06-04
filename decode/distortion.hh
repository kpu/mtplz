#pragma once

#include "decode/feature.hh"

namespace decode {

class Distortion : public Feature {
  public:
    const StringPiece Name() const override;

    void Init(FeatureInit &feature_init) override {}

    void NewWord(StringPiece string_rep, VocabWord *word) const override {}

    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithSourcePhrase(
        HypothesisAndSourcePhrase combination, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        HypothesisAndPhrasePair combination, ScoreCollector &collector) const override {}

    void RescoreHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override;

    std::string FeatureDescription(std::size_t index) const override;
};

} // namespace decode
