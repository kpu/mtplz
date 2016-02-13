#include "hash.hh"

namespace ProbingPT {

std::vector<uint64_t> getVocabIDs(const StringPiece &textin) {
  //Tokenize
  std::vector<uint64_t> output;
  for (util::TokenIter<util::SingleCharacter> it(textin, util::SingleCharacter(' ')); it; ++it) {
    output.push_back(getHash(*it));
  }
  return output;
}

} // namespace ProbingPT

