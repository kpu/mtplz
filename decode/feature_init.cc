#include "decode/feature_init.hh"

namespace decode {

util::Layout &FeatureInit::getHypothesisLayout() {
  return hypothesis_;
}

util::Layout &FeatureInit::getTargetPhraseLayout() {
  return target_phrase_;
}

util::Layout &FeatureInit::getWordLayout() {
  return word_;
}

} // namespace decode
