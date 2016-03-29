#pragma once

#include <iterator>

namespace decode {

class SparseNameBuilder {
};

class ScoreCollector {
  // TODO cumulative feature indices
  public:
    void AddDense(unsigned index, float value);

    // TODO (later)
    /* SparseNameBuilder getSparseNameBuilder(float); */
};

} // namespace decode
