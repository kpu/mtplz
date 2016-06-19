#pragma once

#include "decode/feature.hh"

namespace decode {
  
class LexicalizedReordering : public Feature {
  public:
    typedef std::size_t Relation;
    static constexpr std::size_t FORWARD = 0;
    static constexpr std::size_t BACKWARD = 3;
    static constexpr Relation MONOTONE = 0;
    static constexpr Relation SWAP = 1;
    static constexpr Relation DISCONTINUOUS = 2;

    LexicalizedReordering() : Feature("lexro") {}

    void Init(FeatureInit &feature_init) override;

    void NewWord(StringPiece string_rep, VocabWord *word) const override {}

    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override;

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override;

    std::size_t DenseFeatureCount() const override { return 2; }

    std::string FeatureDescription(std::size_t index) const override;
  private:
    Relation PhraseRelation(const Hypothesis &hypothesis, SourceSpan span) const;

    util::PODField<std::size_t> phrase_start_;
    const pt::Access *phrase_access_;
};

} // namespace decode
