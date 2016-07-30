#include "decode/vocab_map.hh"

#include "decode/chart.hh"
#include "decode/system.hh"

namespace decode {

VocabWord *VocabMap::FindOrInsert(const StringPiece word, const ID id) {
  std::size_t local_id = id - vocab_mapping_.size();
  if (id < vocab_mapping_.size()) {
    return vocab_mapping_[id];
  } else {
    if (local_id >= oov_words_.size()) {
      oov_words_.resize(local_id + 1);
    }
    if (oov_words_[local_id] == nullptr) {
      VocabWord *new_word = reinterpret_cast<VocabWord*>(
          chart_.feature_init_.word_layout.Allocate<util::Pool>(chart_.oov_pool_));
      chart_.feature_init_.pt_id_field(new_word) = id;
      chart_.objective_.NewWord(word, new_word);
      oov_words_[local_id] = new_word;
    }
    return oov_words_[local_id];
  }
}

VocabWord *VocabMap::Find(const ID id) const {
  return id < vocab_mapping_.size() ? vocab_mapping_[id] : oov_words_[id - vocab_mapping_.size()];
}

} // namespace decode
