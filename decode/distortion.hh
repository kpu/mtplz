#pragma once

#include "decode/feature.hh"

namespace decode {

class Distortion : public Feature {
  public:
    Distortion();

    void Init(FeatureInit &feature_init) override {}

    void NewWord(const StringPiece string_rep, VocabWord *word) override {}

    void InitPassthroughPhrase(pt::Row *passthrough) const override {}

    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override;

    std::string FeatureDescription(std::size_t index) const override;
};

} // namespace decode
