#ifndef PHRASE_TABLE_PHRASE_TABLE__
#define PHRASE_TABLE_PHRASE_TABLE__

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"
#include "phrase_table/filter.hh"

namespace phrase_table {

  // This is a temporary typedef until we have a better VocabEntry class
  typedef util::MutableVocab::ID VocabEntry;
  typedef std::vector<VocabEntry> Phrase;

  struct SparseScore {
    uint32_t index;
    float val;
  };

  struct ScoredPhrase {
    Phrase phrase;
    std::vector<SparseScore> scores;    
  };

  class PhraseTable {
  public:
    typedef std::vector<ScoredPhrase> Entry;

    PhraseTable(const std::string & file, util::MutableVocab &vocab, const Filter *const filter=NULL);
    ~PhraseTable() { }

    const Entry *getPhrases(Phrase::const_iterator &begin, Phrase::const_iterator &end) const;
  private:
    struct Hash : public std::unary_function<uint64_t, std::size_t> {
      std::size_t operator()(const uint64_t p) const {
	return p;
      }
    };

    typedef boost::unordered_map<uint64_t, Entry, Hash> Map;
    Map map_;
    
  };

} // namespace phrase_table

#endif


