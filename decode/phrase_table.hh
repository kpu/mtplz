#ifndef DECODE_PHRASE_TABLE__
#define DECODE_PHRASE_TABLE__

#include "decode/id.hh"
#include "search/vertex.hh"
#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"
#include "util/string_piece.hh"

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <string>

namespace util { class Pool; }

namespace decode {

class Scorer;

// First ID* is the length of the array.
class Phrase {
  public:
    explicit Phrase(util::Pool &pool, util::MutableVocab &vocab, const StringPiece &tokens);

    // For passthroughs.
    explicit Phrase(util::Pool &pool, ID word);

    // Use to reconstruct from a void* pointer.
    explicit Phrase(const void *already_initialized)
      : base_(reinterpret_cast<const ID*>(already_initialized)) {}
    
    ID size() const { return *base_; }

    typedef const ID *const_iterator;
    const ID *begin() const { return base_ + 1; }
    const ID *end() const { return begin() + size(); }

    const void *Base() const { return base_; }

  private:
    // Points to length.
    const ID *base_;
};

struct TargetPhrases : boost::noncopyable {
  // Mutable for lazy evaluation.
  // Each entry in this vertex has a history pointer to the phrase.
  mutable search::Vertex vertex;

  void MakePassthrough(util::Pool &pool, Scorer &scorer, ID word);
};

class PhraseTable : boost::noncopyable {
  public:
    typedef TargetPhrases Entry;

    PhraseTable(const std::string &file, util::MutableVocab &vocab, Scorer &scorer);

    // Get all target phrases matching the source phrase specified by [begin, end)
    // Returns NULL if the source phrase does not exist in the table.
    const Entry *Phrases(Phrase::const_iterator begin, Phrase::const_iterator end) const;
    std::size_t MaxSourcePhraseLength() const { return max_source_phrase_length_; }

  private:
    struct Hash : public std::unary_function<uint64_t, std::size_t> {
      std::size_t operator()(const uint64_t p) const {
        return p;
      }
    };

    typedef boost::unordered_map<uint64_t, Entry, Hash> Map;
    Map map_;
    std::size_t max_source_phrase_length_;

    util::Pool phrase_pool_;
};

} // namespace decode

#endif // DECODE_PHRASE_TABLE__
