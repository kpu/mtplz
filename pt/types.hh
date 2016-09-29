#pragma once
#include <stdint.h>

namespace pt {

typedef uint32_t WordIndex;
// Type for sizing VectorField in the binary format.
typedef uint8_t VectorSize;

struct SparseFeature {
  uint32_t index;
  float value;
};

// Row in a phrase table, except for the source phrase.  This is just a pointer
// to pass around.  See Access on how to interpret a const Row *.
class Row;

// Number of target phrases per source phrase.
typedef uint32_t RowCount;

} // namespace pt
