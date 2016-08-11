#pragma once

#include "util/layout.hh"

namespace util { class Pool; }

namespace decode {

class Hypothesis;

class FeatureStore {
  public:
    FeatureStore(const util::ArrayField<float> access, void *data)
      : access_(access), data_(data) {}

    void Init() {
      if (data_) {
        // TODO memset instead?
        for (std::size_t i = 0; i < access_.size(); ++i) {
          access_(data_)[i] = 0;
        }
      }
    }

    operator bool() const { return data_; }

    boost::iterator_range<float*> operator()() {
      return access_(data_);
    }
  private:
    const util::ArrayField<float> access_;
    void *data_;
};

class ScoreCollector {
  public:
    ScoreCollector(
        const std::vector<float> &weights,
        Hypothesis *&new_hypothesis,
        util::Pool *hypothesis_pool,
        FeatureStore dense_features) :
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
    FeatureStore dense_features_;
};

} // namespace decode
