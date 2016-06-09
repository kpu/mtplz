#pragma once

#include "decode/feature.hh"

namespace decode {
  
class LexicalizedReordering : public Feature {
  public:
    LexicalizedReordering() : Feature("lexro") {}

    void Init(FeatureInit &feature_init) override;

    void NewWord(StringPiece string_rep, VocabWord *word) const override {}

    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override;

    void RescoreHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override;

    std::size_t DenseFeatureCount() const override { return 2; }

    std::string FeatureDescription(std::size_t index) const override;
  private:
    const pt::Access *phrase_access_;
};

} // namespace decode
