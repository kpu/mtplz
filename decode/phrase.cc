#include "decode/phrase.hh"

#include "util/exception.hh"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

using namespace util;

namespace decode {

Phrase::Phrase(util::Pool &pool, util::MutableVocab &vocab, const StringPiece &tokens) {
  void *base = pool.Allocate(sizeof(ID));
  ID size = 0;
  for (TokenIter<SingleCharacter, true> target(tokens, ' '); target; ++target, ++size) {
    pool.Continue(base, sizeof(ID));
    // Intentionally start at index 1 because size is stored at index 0.
    reinterpret_cast<ID*>(base)[size + 1] = vocab.FindOrInsert(*target);
  }
  *reinterpret_cast<ID*>(base) = size;
  base_ = reinterpret_cast<ID*>(base);
}

Phrase::Phrase(util::Pool &pool, ID word) {
  ID *base = reinterpret_cast<ID*>(pool.Allocate(2 * sizeof(ID)));
  base[0] = 1;
  base[1] = word;
  base_ = base;
}
  
} // namespace decode
