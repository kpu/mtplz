#pragma once

#include "pt/types.hh"
#include "util/murmur_hash.hh"

namespace pt {

inline uint64_t HashSource(const WordIndex *begin, const WordIndex *end) {
  return util::MurmurHash64A(begin, sizeof(WordIndex) * (end - begin), 1323231561ULL /* mashed on keyboard */);
}

} // namespace pt
