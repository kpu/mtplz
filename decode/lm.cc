#include "decode/lm.hh"

#include "decode/vocab_map.hh"
#include "lm/left.hh"
#include "util/mutable_vocab.hh"
#include "util/exception.hh"

namespace decode {

LM::LM(const char *model) :
  Feature("lm"), model_(model) {}

void LM::Init(FeatureInit &feature_init) {
  pt_row_field_ = feature_init.pt_row_field;
  UTIL_THROW_IF(!feature_init.phrase_access.target, util::Exception,
      "requested language model but target phrase text is missing in phrase access");
  phrase_access_ = &feature_init.phrase_access;
  lm_word_index_ = util::PODField<lm::WordIndex>(feature_init.word_layout);
  phrase_score_field_ = util::PODField<float>(feature_init.target_phrase_layout);
  hypothesis_with_phrase_pair_score_ = util::PODField<float>(feature_init.hypothesis_layout);
}

void LM::NewWord(const StringPiece string_rep, VocabWord *word) const {
  lm_word_index_(word) = model_.GetVocabulary().Index(string_rep);
}

void LM::ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const {
  collector.AddDense(0, phrase_score_field_(target.phrase));
}

void LM::InitTargetPhrase(TargetPhraseInfo target, lm::ngram::ChartState &state) const {
  lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state);
  const pt::Row *pt_target_phrase = pt_row_field_(target.phrase);
  for (const ID i : phrase_access_->target(pt_target_phrase)) {
    scorer.Terminal(lm_word_index_(target.vocab_map.Find(i)));
  }
  phrase_score_field_(target.phrase) = scorer.Finish();
}

void LM::SetSearchScore(Hypothesis *new_hypothesis, float score) const {
  hypothesis_with_phrase_pair_score_(new_hypothesis) = score;
}

void LM::ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const {
  float score = hypothesis_with_phrase_pair_score_(collector.NewHypothesis());
  collector.AddDense(0, score);
}

std::size_t LM::DenseFeatureCount() const { return 1; }

std::string LM::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "lm";
}

} // namespace decode
