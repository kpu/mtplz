#include "util/mutable_vocab.hh"

#include "util/murmur_hash.hh"
#include <iostream>

namespace util {

MutableVocab::MutableVocab() {
  strings_.push_back(StringPiece("<unk>"));
}

MutableVocab::ID MutableVocab::Find(const StringPiece &str) const {
  Map::ConstIterator it;
  if (map_.Find(util::MurmurHashNative(str.data(), str.size()), it)) {
    return it->id;
  } else {
    return kUNK;
  }
}

uint32_t MutableVocab::FindOrInsert(const StringPiece &str) {
  MutableVocabInternal entry;
  entry.key = util::MurmurHashNative(str.data(), str.size());
	//std::cout << "foi1: " << str << std::endl;
  Map::MutableIterator it;
	//std::cout << "foi1.1: " << str << std::endl;
  if (map_.FindOrInsert(entry, it)) {
		//std::cout << "foi1.2: " << str << std::endl;
    return it->id;
  }
	//std::cout << "foi2: " << str << std::endl;
  it->id = strings_.size();
  
	//std::cout << "foi3: " << str << std::endl;
  char *copied = static_cast<char*>(piece_backing_.Allocate(str.size()));
  memcpy(copied, str.data(), str.size());
	//std::cout << "foi4: " << str << std::endl;
  strings_.push_back(StringPiece(copied, str.size()));
  return it->id;
}

} // namespace util
