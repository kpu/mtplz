#include "phrase_table/vocab.hh"

#include "util/murmur_hash.hh"

namespace phrase_table {

bool Vocab::Find(const StringPiece &str, uint32_t &out) {
  Map::ConstIterator it;
  bool ret = map_.Find(util::MurmurHashNative(str.data(), str.size()), it);
  if (ret) out = it->id;
  return ret;
}

uint32_t Vocab::FindOrAdd(const StringPiece &str) {
  VocabInternal entry;
  entry.key = util::MurmurHashNative(str.data(), str.size());
  Map::MutableIterator it;
  if (map_.FindOrInsert(entry, it)) {
    return it->id;
  }
  it->id = strings_.size();
  
  char *copied = static_cast<char*>(piece_backing_.Allocate(str.size()));
  memcpy(copied, str.data(), str.size());
  strings_.push_back(StringPiece(copied, str.size()));
  return it->id;
}

} // namespace phrase_table
