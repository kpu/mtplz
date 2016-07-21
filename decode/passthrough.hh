#pragma once

#include "decode/feature.hh"

namespace util { class MutableVocab; }

namespace decode {

class Passthrough : public Feature {
  public:
    Passthrough() : Feature("Passthrough") {}

    void Init(FeatureInit &feature_init) override {
      pt_row_field_ = feature_init.pt_row_field;
    }

    static const StringPiece Name();

    void NewWord(const StringPiece string_rep, VocabWord *word) const override {}

    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {
      // TODO make sparse
      collector.AddDense(0, (float)(pt_row_field_(phrase_pair.target_phrase) == nullptr));
    }

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override { return 1; }

    std::string FeatureDescription(std::size_t index) const override {
      assert(index == 0);
      return "passthrough";
    }

  private:
    util::PODField<const pt::Row*> pt_row_field_;
};

} // namespace decode
