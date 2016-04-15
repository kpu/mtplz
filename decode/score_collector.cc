#include "decode/score_collector.hh"

namespace decode {

void ScoreCollector::AddDense(unsigned index, float value) {
  if (dense_features_) {
    (*dense_features_)[dense_feature_offset_ + index] = value;
  }
  score_ += weights_[dense_feature_offset_ + index] * value;
}


} // namespace decode
