#pragma once

#include "decode/id.hh"
#include "util/string_piece.hh"

namespace decode {

struct VocabWord;
class Chart;

class VocabMap {
  public:
    explicit VocabMap(const std::vector<VocabWord*> &vocab_mapping, Chart &chart)
      : vocab_mapping_(vocab_mapping), chart_(chart) {}

    VocabWord *FindOrInsert(const StringPiece word, const ID global_word);
    VocabWord *Find(const ID global_word) const;
  private:
    const std::vector<VocabWord*> &vocab_mapping_;
    Chart &chart_;
    std::vector<VocabWord*> oov_words_;
};

} // namespace decode
