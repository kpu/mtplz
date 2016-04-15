#include "decode/score_collector.hh"

namespace decode {

void ScoreCollector::SetDenseOffset(unsigned offset) {
  dense_feature_offset_ = offset;
}

float ScoreCollector::Score() {
  return score_;
}

void ScoreCollector::AddDense(unsigned index, float value) {
  if (dense_features_) {
    (*dense_features_)[dense_feature_offset_ + index] = value;
  }
  score_ += value;
}


} // namespace decode
