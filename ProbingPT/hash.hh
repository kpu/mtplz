#pragma once

#include "util/murmur_hash.hh"
#include "util/string_piece.hh"
#include <string>
#include <vector>

namespace ProbingPT {

//Gets the MurmurmurHash for give string
uint64_t HashWord(StringPiece text) {
  return util::MurmurHashNative(text.data(), text.size());
}

// Mashing on the keyboard to generate odd numbers.
inline uint64_t ChainHash(uint64_t current, uint64_t next) {
  uint64_t ret = (current * 8978948897894561157ULL) ^ (static_cast<uint64_t>(next) * 17894857484156487943ULL);
  return ret;
}

} // namespace ProbingPT
