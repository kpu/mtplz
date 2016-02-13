#pragma once

#include "util/murmur_hash.hh"
#include "util/string_piece.hh"
#include <string>
#include <vector>

namespace ProbingPT {

//Gets the MurmurmurHash for give string
uint64_t getHash(StringPiece text) {
  return util::MurmurHashNative(text.data(), text.size());
}

// Vocab ids are hashes.
uint64_t getVocabID(const StringPiece &candidate) {
  return getHash(candidate);
}

// TODO: this vector usage is inefficient.
std::vector<uint64_t> getVocabIDs(const StringPiece &textin);

} // namespace ProbingPT
