#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

namespace decode {

typedef std::vector<float> FeatureStore;

class ScoreCollector {
  public:

    explicit ScoreCollector(const std::vector<float> &weights)
      : weights_(weights), dense_features_(NULL) {}

    ScoreCollector(const std::vector<float> &weights, FeatureStore *dense_features)
      : weights_(weights), dense_features_(dense_features) {}

    void SetDenseOffset(std::size_t offset) {
      dense_feature_offset_ = offset;
    }

    float Score() const {
      return score_;
    }

    void AddDense(std::size_t index, float value);

    // TODO (later)
    /* SparseNameBuilder getSparseNameBuilder(float); */

  private:
    float score_ = 0;
    const std::vector<float> &weights_;
    std::size_t dense_feature_offset_;
    FeatureStore *dense_features_;
};

} // namespace decode
