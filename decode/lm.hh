#pragma once

#include "decode/feature.hh"
#include "lm/model.hh"
#include "lm/state.hh"
#include "util/layout.hh"

namespace util { class MutableVocab; }

namespace decode {

class LM : public Feature, public TargetPhraseInitializer {
  public:
    LM(const char *model);

    uint8_t Init(FeatureInit &feature_init) override;

    void NewWord(const StringPiece string_rep, VocabWord *word) const override;

    void InitPassthroughPhrase(pt::Row *passthrough) const override {}

    void InitTargetPhrase(TargetPhraseInfo target, lm::ngram::ChartState &state) const override;

    void ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const override;

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override {}

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}

    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}

    std::size_t DenseFeatureCount() const override;

    std::string FeatureDescription(std::size_t index) const override;

    const lm::ngram::Model &Model() const { return model_; }

  private:
    lm::ngram::Model model_;
    const pt::Access *phrase_access_;
    util::PODField<const pt::Row*> pt_row_field_;
    util::PODField<lm::WordIndex> lm_word_index_;
    util::PODField<float> phrase_score_field_;
};

} // namespace decode
