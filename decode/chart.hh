#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "decode/id.hh"
#include "decode/source_phrase.hh"
#include "search/vertex.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace util { class MutableVocab; }
namespace pt { class Row; }

namespace decode {

class Objective;
struct FeatureInit;
struct VocabWord; // conforms to FeatureInit WordLayout

typedef search::Vertex TargetPhrases;

// Target phrases that correspond to each source span
class Chart {
  public:
    // TODO should be 2 to indicate </s> rather than <unk>,
    // but </s> is currently only in GrowableVocab, not in MutableVocab
    static constexpr ID EOS_WORD = 0;

    Chart(std::size_t max_source_phrase_length, Objective &objective);

    void ReadSentence(StringPiece input, util::MutableVocab &vocab,
        const std::vector<VocabWord*> &vocab_mapping);

    template <class PhraseTable> void LoadPhrases(const PhraseTable &table) {
      // There's some unreachable ranges off the edge. Meh.
      entries_.resize(sentence_.size() * max_source_phrase_length_);
      for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
        for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
          search::Vertex &vertex = *vertex_pool_.construct();
          vertex.Root().InitRoot();
          auto phrases = table.Lookup(&sentence_ids_[begin], &*sentence_ids_.begin() + end);
          SourcePhrase source_phrase(sentence_, begin, end);
          for (auto phrase = phrases.begin(); phrase != phrases.end(); ++phrase) {
            AddTargetPhraseToVertex(&*phrase, source_phrase, vertex, false);
          }
          vertex.Root().FinishRoot(search::kPolicyLeft);
          SetRange(begin, end, &vertex);
        }
        if (!Range(begin, begin + 1)) {
          AddPassthrough(begin);
        }
      }
    }

    std::size_t SentenceLength() const { return sentence_.size(); }

    const std::vector<VocabWord*> &Sentence() const { return sentence_; }

    std::size_t MaxSourcePhraseLength() const { return max_source_phrase_length_; }

    TargetPhrases *Range(std::size_t begin, std::size_t end) const {
      assert(end > begin);
      assert(end - begin <= max_source_phrase_length_);
      assert(end <= SentenceLength());
      assert(begin * max_source_phrase_length_ + end - begin - 1 < entries_.size());
      return entries_[begin * max_source_phrase_length_ + end - begin - 1];
    }

    TargetPhrases &EndOfSentence();

  private:
    void SetRange(std::size_t begin, std::size_t end, TargetPhrases *to) {
      assert(end - begin <= max_source_phrase_length_);
      entries_[begin * max_source_phrase_length_ + end - begin - 1] = to;
    }

    VocabWord *MapToVocabWord(const StringPiece word, const ID global_word,
        const std::vector<VocabWord*> &vocab_mapping);

    void AddTargetPhraseToVertex(
        const pt::Row *phrase,
        const SourcePhrase &source_phrase,
        search::Vertex &vertex,
        bool passthrough);

    void AddPassthrough(std::size_t position);

    util::Pool target_phrase_pool_;
    boost::object_pool<search::Vertex> vertex_pool_;

    Objective &objective_;
    FeatureInit &feature_init_;

    std::vector<VocabWord*> sentence_;
    std::vector<ID> sentence_ids_;

    // Backs any oov words that are passed through.  
    util::Pool oov_pool_;
    std::vector<VocabWord*> oov_words_;

    // Banded array: different source lengths are next to each other.
    std::vector<TargetPhrases*> entries_;

    const std::size_t max_source_phrase_length_;
};

} // namespace decode

#endif // DECODE_CHART__
