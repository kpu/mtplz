#ifndef PHRASE_TABLE_FILTER__
#define PHRASE_TABLE_FILTER__

#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "util/murmur_hash.hh"
#include "alone/vocab.hh"

namespace phrase_table {

  //typedef alone::Vocab::Entry VocabItem;
  typedef alone::VocabEntry VocabItem;
  typedef std::vector<VocabItem> Phrase;
  typedef std::vector<std::string> WordVec;

  // Provides a function which returns a bool for a token sequence.
  // If the return val is true, the token sequence passes the filter.
  // This can be used, e.g., for phrase table filtering based on
  // a known source file to translate.
  class Filter {
  public:
    Filter() : ngram_length_(0) { }
    Filter(const std::string& file, const std::size_t ngram_length);
    bool PassesFilter(std::vector<VocabItem> const& words) const;

  private:
//     struct Hash : public std::unary_function<WordVec, std::size_t> {
//       std::size_t operator()(const WordVec& p) const {
// 	return util::MurmurHashNative(p.data(), p.size()*sizeof(VocabItem));
//       }
//     };
    
//     struct Equals : public std::binary_function<WordVec, WordVec, bool> {
//       bool operator()(const WordVec& first, const WordVec& second) const {
//         return first == second;
//       }
//     };

//    typedef boost::unordered_set<Phrase, Hash, Equals> Map;
    typedef boost::unordered_set<WordVec> Map;
    Map ngram_map_;
    std::size_t ngram_length_;

  };
}

#endif
