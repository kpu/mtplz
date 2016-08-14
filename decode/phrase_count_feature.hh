#pragma once

#include "decode/feature.hh"

namespace decode {

class PhraseCountFeature : public Feature {
  public:
    PhraseCountFeature() : Feature("phrase_insertion") {}

    void Init(FeatureInit &feature_init) override {}

    void NewWord(const StringPiece string_rep, VocabWord *word) const override {}

    void InitPassthroughPhrase(pt::Row *passthrough) const override {}

    void ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const override {
      if (target.type != TargetPhraseType::EOS) {
        collector.AddDense(0, 1);
      }
    }

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override {
      return 1;
    }

    std::string FeatureDescription(std::size_t index) const override {
      assert(index == 0);
      return "phrase count feature";
    }
};

} // namespace decode
