#pragma once

#include "decode/feature.hh"

namespace util { class MutableVocab; }

namespace decode {

class WordInsertion : public Feature {
  public:
    WordInsertion() : Feature("target_word_insertion") {}

    void Init(FeatureInit &feature_init) override {
      UTIL_THROW_IF(!feature_init.phrase_access.target, util::Exception,
          "requested word insertion penalty but target words missing in phrase access");
      phrase_access_ = &feature_init.phrase_access;
      pt_row_field_ = feature_init.pt_row_field;
    }

    static const StringPiece Name();

    void NewWord(const StringPiece string_rep, VocabWord *word) const override {}

    void InitPassthroughPhrase(pt::Row *passthrough) const override {}

    void ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const override {
      collector.AddDense(0, phrase_access_->target(pt_row_field_(target.phrase)).size());
    }

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {
      collector.AddDense(0, -1); // compensate for counting eos as insert
    }

    std::size_t DenseFeatureCount() const override { return 1; }

    std::string FeatureDescription(std::size_t index) const override {
      assert(index == 0);
      return "word insertion penalty";
    }

  private:
    const pt::Access *phrase_access_;
    util::PODField<const pt::Row*> pt_row_field_;
};

} // namespace decode
