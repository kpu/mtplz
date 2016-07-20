#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "decode/id.hh"
#include "decode/source_phrase.hh"
#include "pt/query.hh"
#include "search/vertex.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace util { class MutableVocab; }

namespace decode {

class System;
struct VocabWord; // conforms to FeatureInit WordLayout

typedef search::Vertex TargetPhrases;

// Target phrases that correspond to each source span
class Chart {
  public:
    // TODO pass system for local vocab mapping and objective
    Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab, System &system);

    std::size_t SentenceLength() const { return sentence_.size(); }

    const std::vector<VocabWord*> &Sentence() const { return sentence_; }

    // TODO: make this reflect the longent source phrase for this sentence.
    std::size_t MaxSourcePhraseLength() const { return max_source_phrase_length_; }

    TargetPhrases *Range(std::size_t begin, std::size_t end) const {
      assert(end > begin);
      assert(end - begin <= max_source_phrase_length_);
      assert(end <= SentenceLength());
      assert(begin * max_source_phrase_length_ + end - begin - 1 < entries_.size());
      return entries_[begin * max_source_phrase_length_ + end - begin - 1];
    }

    // TODO get end of sentence marker phrase

  private:
    void SetRange(std::size_t begin, std::size_t end, TargetPhrases *to) {
      assert(end - begin <= max_source_phrase_length_);
      entries_[begin * max_source_phrase_length_ + end - begin - 1] = to;
    }

    VocabWord *MapToLocalWord(const ID global_word);

    void AddTargetPhraseToVertex(
        const pt::Row *phrase,
        const SourcePhrase &source_phrase,
        search::Vertex &vertex,
        bool passthrough);

    util::Pool target_phrase_wrappers_;
    boost::object_pool<TargetPhrases> phrases_; // TODO name confuseable with entries_

    System &system_;

    std::vector<VocabWord*> sentence_;

    // Backs any oov words that are passed through.  
    util::Pool oov_words_;

    // Banded array: different source lengths are next to each other.
    std::vector<TargetPhrases*> entries_;

    const std::size_t max_source_phrase_length_;
};

} // namespace decode

#endif // DECODE_CHART__
