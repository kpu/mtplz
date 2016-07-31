#pragma once

#include "decode/id.hh"
#include "util/mutable_vocab.hh"

namespace decode {

struct VocabWord;
class Objective;
struct BaseVocab;

class VocabMap {
  public:
    VocabMap(Objective &objective, const BaseVocab &base);

    // modifies id if unknown
    VocabWord *FindOrInsert(const StringPiece word, ID &id);

    VocabWord *Find(const ID id) const;

    StringPiece String(const ID id) const;

  private:
    const BaseVocab &base_;
    std::size_t base_size_;

    util::MutableVocab oov_vocab_;
    util::Pool oov_pool_;
    std::vector<VocabWord*> oov_map_;

    Objective &objective_;
};

} // namespace decode
