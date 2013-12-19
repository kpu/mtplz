#ifndef PHRASE_TABLE_PHRASE_TABLE__
#define PHRASE_TABLE_PHRASE_TABLE__

#include "phrase_table/id.hh"
#include "search/vertex.hh"
#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"

#include <boost/unordered_map.hpp>

#include <string>
#include <vector>

namespace phrase_table {

typedef std::vector<ID> Phrase;

class Scorer;

class PhraseTable {
  public:
    struct Entry {
      std::vector<Phrase> content;
      search::Vertex vertex;
    };

    PhraseTable(const std::string &file, util::MutableVocab &vocab, Scorer &scorer);

    // Get all target phrases matching the source phrase specified by [begin, end)
    // Returns NULL if the source phrase does not exist in the table.
    const Entry *getPhrases(Phrase::iterator begin, Phrase::iterator end) const;
    std::size_t getMaxSourcePhraseLength() const { return max_source_phrase_length_; }

  private:
    struct Hash : public std::unary_function<uint64_t, std::size_t> {
      std::size_t operator()(const uint64_t p) const {
        return p;
      }
    };

    typedef boost::unordered_map<uint64_t, Entry, Hash> Map;
    Map map_;
    std::size_t max_source_phrase_length_;
};

} // namespace phrase_table

#endif
