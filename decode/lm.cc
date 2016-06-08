#include "decode/lm.hh"

#include "util/mutable_vocab.hh"

namespace decode {

LM::LM(const char *model, util::MutableVocab &vocab) : model_(model), vocab_(vocab) {}

void LM::Init(FeatureInit &feature_init) {
  lm_state_field_ = &feature_init.LMStateField();
}

const StringPiece LM::Name() const { return "lm"; }


std::size_t LM::DenseFeatureCount() const { return 1; }

std::string LM::FeatureDescription(std::size_t index) const {
  assert(index == 0);
  return "lm";
}

} // namespace decode
