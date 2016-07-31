#include "decode/vocab_map.hh"

#include "decode/system.hh"

namespace decode {

VocabMap::VocabMap(Objective &objective, const BaseVocab &base)
  : objective_(objective), base_(base), base_size_(base.Size()) {}

VocabWord *VocabMap::FindOrInsert(const StringPiece word, ID &id) {
  id = base_.vocab.Find(word);
  if (id > 0) { // word in base vocab
    return base_.map[id];
  }
  std::size_t local_id = oov_vocab_.FindOrInsert(word);
  id = base_size_ + local_id - 1;
  if (local_id > oov_map_.size()) { // insert new word
    assert(local_id == oov_map_.size() + 1);
    VocabWord *new_word = reinterpret_cast<VocabWord*>(
        objective_.GetFeatureInit().word_layout.Allocate(oov_pool_));
    objective_.GetFeatureInit().pt_id_field(new_word) = id;
    objective_.NewWord(word, new_word);
    oov_map_.push_back(new_word);
  }
  return oov_map_[local_id - 1];
}

VocabWord *VocabMap::Find(const ID id) const {
  return id < base_size_ ? base_.map[id] : oov_map_[id - base_.Size()];
}

StringPiece VocabMap::String(const ID id) const {
  return id < base_size_ ? base_.vocab.String(id) : oov_vocab_.String(id - base_.Size() + 1);
}

} // namespace decode
