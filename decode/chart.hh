#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "util/string_piece.hh"

#include <boost/pool/object_pool.hpp>

#include <vector>

namespace util { class MutableVocab; }

namespace decode {

struct TargetPhrases;
class PhraseTable;
class Scorer;

// Target phrases that correspond to each source span
class Chart {
  public:
    Chart(const PhraseTable &table, StringPiece input, util::MutableVocab &vocab, Scorer &scorer);

    std::size_t SentenceLength() const { return sentence_length_; }

    // TODO: make this reflect the longent source phrase for this sentence.
    std::size_t MaxSourcePhraseLength() const { return max_source_phrase_length_; }

    const TargetPhrases *Range(std::size_t begin, std::size_t end) const {
      return entries_[begin * max_source_phrase_length_ + end - begin - 1];
    }

    // TODO test this.
    std::pair<std::size_t, std::size_t> RangeFromPointer(const TargetPhrases *pointer) const {
      std::pair<std::size_t, std::size_t> ret;
      ret.first = (pointer - &*entries_.begin()) / max_source_phrase_length_;
      ret.second = ret.first + 1 + (pointer - &*entries_.begin()) % max_source_phrase_length_;
      return ret;
    }
    

  private:
    void SetRange(unsigned int begin, unsigned int length, const TargetPhrases *to) {
      entries_[begin * max_source_phrase_length_ + length - 1] = to;
    }

    // These back any oov words that are passed through.  
    boost::object_pool<TargetPhrases> passthrough_;

    // Banded array: different source lengths are next to each other.
    std::vector<const TargetPhrases*> entries_;

    std::size_t sentence_length_;

    const std::size_t max_source_phrase_length_;
};

} // namespace decode

#endif // DECODE_CHART__
