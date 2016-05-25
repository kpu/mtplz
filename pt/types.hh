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

// Not defined as this is backed by Layout.  This is just a pointer to pass
// around.  See Access on how to interpret a const Phrase *.
class Phrase;

} // namespace pt
