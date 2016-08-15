#include "decode/score_collector.hh"
#include <iostream>

namespace decode {

void ScoreCollector::AddDense(std::size_t index, float value) {
  std::size_t global_index = dense_feature_offset_ + index;
  if (dense_features_) {
    dense_features_()[global_index] += value;
  }
  score_ += weights_[global_index] * value;
}


} // namespace decode
