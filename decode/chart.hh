#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "decode/id.hh"
#include "pt/query.hh"
#include "search/vertex.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace util { class MutableVocab; }

namespace decode {

struct TargetPhrases : boost::noncopyable {
  // Mutable for lazy evaluation.
  // Each entry in this vertex has a history pointer to the phrase.
  mutable search::Vertex vertex;
};

// Target phrases that correspond to each source span
class Chart {
  public:
    Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab);

    std::size_t SentenceLength() const { return words_.size(); }

    const std::vector<ID> &Sentence() const { return words_; }

    // TODO: make this reflect the longent source phrase for this sentence.
    std::size_t MaxSourcePhraseLength() const { return max_source_phrase_length_; }

    const TargetPhrases *Range(std::size_t begin, std::size_t end) const {
      assert(end > begin);
      assert(end - begin <= max_source_phrase_length_);
      assert(end <= SentenceLength());
      assert(begin * max_source_phrase_length_ + end - begin - 1 < entries_.size());
      return entries_[begin * max_source_phrase_length_ + end - begin - 1];
    }

  private:
    void SetRange(std::size_t begin, std::size_t end, const TargetPhrases *to) {
      assert(end - begin <= max_source_phrase_length_);
      entries_[begin * max_source_phrase_length_ + end - begin - 1] = to;
    }

    std::vector<ID> words_;

    // These back any oov words that are passed through.  
    util::Pool passthrough_phrases_;
    boost::object_pool<TargetPhrases> passthrough_;

    // Banded array: different source lengths are next to each other.
    std::vector<const TargetPhrases*> entries_;

    const std::size_t max_source_phrase_length_;
};

} // namespace decode

#endif // DECODE_CHART__
