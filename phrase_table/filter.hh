#ifndef PHRASE_TABLE_FILTER__
#define PHRASE_TABLE_FILTER__

#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"

namespace phrase_table {

  typedef util::MutableVocab::ID VocabEntry;
  typedef std::vector<VocabEntry> Phrase;

  // Provides a function which returns a bool for a token sequence.
  // The return val is true iff the token sequence passes the filter.
  // This can be used, e.g., for phrase table filtering based on
  // a known source file to translate.
  class Filter {
  public:
    Filter() : ngram_length_(0) { }
    Filter(const std::string& file, util::MutableVocab& vocab, const std::size_t ngram_length = 3);
    bool PassesFilter(Phrase const& phrase) const;

  private:
    typedef boost::unordered_set<Phrase> Map;
    Map ngram_map_;
    std::size_t ngram_length_;

  };
}

#endif
