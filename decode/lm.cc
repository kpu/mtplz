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
}

void LM::NewWord(const StringPiece string_rep, VocabWord *word) const {
  lm_word_index_(word) = model_.GetVocabulary().Index(string_rep);
}

void LM::ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const {
  collector.AddDense(0, phrase_score_field_(phrase_pair.target_phrase));
}

void LM::ScoreTargetPhrase(PhrasePair pair, lm::ngram::ChartState &state) const {
  lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state);
  const pt::Row *pt_target_phrase = pt_row_field_(pair.target_phrase);
  for (const ID i : phrase_access_->target(pt_target_phrase)) {
    scorer.Terminal(lm_word_index_(pair.vocab_map->Find(i)));
  }
  phrase_score_field_(pair.target_phrase) = scorer.Finish();
}

std::size_t LM::DenseFeatureCount() const { return 1; }

std::string LM::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "lm";
}

} // namespace decode
