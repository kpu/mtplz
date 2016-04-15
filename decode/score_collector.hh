#pragma once

#include <iterator>
#include <vector>

namespace decode {

class ScoreCollector {
  public:
    ScoreCollector() : dense_features_(NULL) {}

    ScoreCollector(std::vector<float> *dense_features) :
      dense_features_(dense_features) {}

    void SetDenseOffset(unsigned offset);

    float Score();

    void AddDense(unsigned index, float value);

    // TODO (later)
    /* SparseNameBuilder getSparseNameBuilder(float); */

  private:
    float score_ = 0;
    unsigned dense_feature_offset_;
    std::vector<float> *dense_features_;
};

} // namespace decode
