#include "decode/lm.hh"

#include "lm/left.hh"
#include "util/mutable_vocab.hh"
#include "util/exception.hh"

namespace decode {

LM::LM(const char *model, util::MutableVocab &vocab) :
  Feature("lm"), model_(model), vocab_(vocab) {}

void LM::Init(FeatureInit &feature_init) {
  lm_state_field_ = feature_init.LMStateField();
  chart_state_field_ = util::PODField<lm::ngram::ChartState>(feature_init.HypothesisLayout());
  UTIL_THROW_IF(feature_init.PhraseAccess().target, util::Exception,
      "requested language model but target phrase text is missing in phrase access");
  phrase_access_ = &feature_init.PhraseAccess();
}

void LM::ScoreHypothesisWithPhrasePair(
    HypothesisAndPhrasePair combination,
    ScoreCollector &collector) const {
  auto state = chart_state_field_(&combination.hypothesis);
  lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state);
  for (const ID i : phrase_access_->target(combination.phrase_pair.target_phrase)) {
    scorer.Terminal(Convert(i));
  }
  collector.AddDense(0, scorer.Finish());
}

lm::WordIndex LM::Convert(ID from) const {
  /* TODO find replacement which allows for const
  if (from >= vocab_mapping_.size()) {
    vocab_mapping_.reserve(vocab_.Size());
    while (vocab_mapping_.size() < vocab_.Size()) {
      vocab_mapping_.push_back(model_.GetVocabulary().Index(vocab_.String(vocab_mapping_.size())));
    }
  } */
  return vocab_mapping_[from];
}

std::size_t LM::DenseFeatureCount() const { return 1; }

std::string LM::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "lm";
}

} // namespace decode
