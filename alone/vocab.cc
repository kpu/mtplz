#include "alone/vocab.hh"

#include "lm/virtual_interface.hh"
#include "util/string_piece.hh"

namespace alone {

Vocab::Vocab(const lm::base::Vocabulary &backing) : backing_(backing) {}

const Vocab::Entry &Vocab::FindOrAdd(const StringPiece &str) {
#if BOOST_VERSION >= 104200
  Map::const_iterator i= map_.find(str, Hash(), Equals());
#else
  std::string copied_str(str.data(), str.size());
  Map::const_iterator i = map_.find(copied_str.c_str());
#endif
  if (i != map_.end()) return *i;
  char *copied = static_cast<char*>(piece_backing_.Allocate(str.size() + 1));
  memcpy(copied, str.data(), str.size());
  copied[str.size()] = 0;
  return *map_.insert(Entry(copied, backing_.Index(str))).first;
}

} // namespace alone
