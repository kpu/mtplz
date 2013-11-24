#ifndef PHRASE_TABLE_PHRASE_TABLE__
#define PHRASE_TABLE_PHRASE_TABLE__

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "util/murmur_hash.hh"
#include "alone/vocab.hh"
#include "phrase_table/filter.hh"

namespace phrase_table {

  typedef alone::VocabEntry VocabItem;
  typedef std::vector<VocabItem> Phrase;


  struct SparseScore {
    uint32_t index;
    double val;
  };

  struct ScoredPhrase {
    Phrase phrase;
    std::vector<SparseScore> scores;    
  };

  class PhraseTable {
  public:
    typedef std::vector<ScoredPhrase> Entry;

    PhraseTable(const std::string & file, alone::Vocab &vocab, const Filter *const filter=NULL);
    ~PhraseTable() { }

    // TODO: Right now this is returned by value because I don't think there's a simple way around that using 
    // the unordered_map container.
    const Entry *getPhrases(const Phrase & source_phrase) const;
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


//     struct Equals : public std::binary_function<Phrase, Phrase, bool> {
//       bool operator()(const Phrase& first, const Phrase& second) const {
//         return first == second;
//       }
//     };

