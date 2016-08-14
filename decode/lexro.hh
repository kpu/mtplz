#pragma once

#include "decode/feature.hh"

namespace decode {
  
class LexicalizedReordering : public Feature {
  public:
    typedef uint8_t Relation;

    static constexpr uint8_t VALUE_COUNT = 6;
    static constexpr float DEFAULT_VALUE = 0;

    static constexpr uint8_t FORWARD = 0;
    static constexpr uint8_t BACKWARD = 3;
    static constexpr Relation MONOTONE = 0;
    static constexpr Relation SWAP = 1;
    static constexpr Relation DISCONTINUOUS = 2;

    LexicalizedReordering() : Feature("lexical_reordering") {}

    void Init(FeatureInit &feature_init) override;

    void NewWord(const StringPiece string_rep, VocabWord *word) const override {}

    void InitPassthroughPhrase(pt::Row *passthrough) const override;

    void ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override;

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override { return 6; }

    std::string FeatureDescription(std::size_t index) const override;
  private:
    Relation PhraseRelation(SourceSpan phrase1, SourceSpan phrase2) const;

    util::PODField<const pt::Row*> pt_row_;
    util::PODField<std::size_t> phrase_start_;
    const pt::Access *phrase_access_;
};

} // namespace decode
