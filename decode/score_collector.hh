#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

namespace util { class Pool; }

namespace decode {

class Hypothesis;

typedef std::vector<float> FeatureStore;

class ScoreCollector {
  public:
    ScoreCollector(
        const std::vector<float> &weights,
        Hypothesis *&new_hypothesis,
        util::Pool *hypothesis_pool,
        FeatureStore *dense_features) :
      weights_(weights),
      new_hypothesis_(new_hypothesis),
      hypothesis_pool_(hypothesis_pool),
      dense_features_(dense_features) {}

    void SetDenseOffset(std::size_t offset) {
      dense_feature_offset_ = offset;
    }

    float Score() const {
      return score_;
    }

    Hypothesis *&NewHypothesis() {
      return new_hypothesis_;
    }

    util::Pool *HypothesisPool() {
      return hypothesis_pool_;
    }

    void AddDense(std::size_t index, float value);

    // TODO (later)
    /* SparseNameBuilder getSparseNameBuilder(float); */

  private:
    float score_ = 0;
    const std::vector<float> &weights_;
    Hypothesis *&new_hypothesis_;
    util::Pool *hypothesis_pool_;
    std::size_t dense_feature_offset_;
    FeatureStore *dense_features_;
};

} // namespace decode
