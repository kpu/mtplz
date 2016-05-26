#include "decode/feature_init.hh"

namespace decode {

util::Layout &FeatureInit::GetHypothesisLayout() {
  return hypothesis_;
}

util::Layout &FeatureInit::GetTargetPhraseLayout() {
  return target_phrase_;
}

util::Layout &FeatureInit::GetWordLayout() {
  return word_;
}

} // namespace decode
