#include "decode/lm.hh"

#include "util/mutable_vocab.hh"

namespace decode {

LM::LM(const char *model, util::MutableVocab &vocab) :
  Feature("lm"), model_(model), vocab_(vocab) {}

void LM::Init(FeatureInit &feature_init) {
  lm_state_field_ = &feature_init.LMStateField();
}

/* float Scorer::LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state) { */
/*   lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state); */
/*   for (const ID *i = words_begin; i != words_end; ++i) { */
/*     scorer.Terminal(Convert(*i)); */
/*   } */
/*   return weights_.LMWeight() * scorer.Finish(); */
/* } */
void LM::ScoreHypothesisWithPhrasePair(
    HypothesisAndPhrasePair combination,
    ScoreCollector &collector) const {
  // TODO
}

std::size_t LM::DenseFeatureCount() const { return 1; }

std::string LM::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "lm";
}

} // namespace decode
