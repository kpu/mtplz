#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "decode/source_phrase.hh"
#include "decode/vocab_map.hh"
#include "pt/format.hh"
#include "search/vertex.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/pool/object_pool.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace pt { class Row; }
namespace util { class MutableVocab; }

namespace decode {

class Objective;
struct FeatureInit;

typedef search::Vertex TargetPhrases;

// TODO currently not thread-safe because of state_buffer_!
// Target phrases that correspond to each source span
class Chart {
  public:
    typedef boost::unordered_map<const pt::Row*,search::HypoState> StateMap;

    static constexpr ID EOS_WORD = 2;

    Chart(std::size_t max_source_phrase_length, VocabMap &vocab_map, Objective &objective, StateMap &state_map);

    void ReadSentence(StringPiece input);

    template <class PhraseTable> void LoadPhrases(const PhraseTable &table) {
      // There's some unreachable ranges off the edge. Meh.
      entries_.resize(sentence_.size() * max_source_phrase_length_);
      for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
        for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
          auto phrases = table.Lookup(&sentence_ids_[begin], &*sentence_ids_.begin() + end);
          if (phrases) {
            SourcePhrase source_phrase(sentence_, begin, end);
            search::Vertex &vertex = *vertex_pool_.construct();
            vertex.Root().InitRoot();
            for (auto phrase = phrases.begin(); phrase != phrases.end(); ++phrase) {
              AddTargetPhraseToVertex(&*phrase, source_phrase, vertex, false);
            }
            vertex.Root().FinishRoot(search::kPolicyLeft);
            SetRange(begin, end, &vertex);
          }
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

    const VocabMap &VocabMapping() const { return vocab_map_; }

  private:
    void SetRange(std::size_t begin, std::size_t end, TargetPhrases *to) {
      assert(end - begin <= max_source_phrase_length_);
      assert(begin * max_source_phrase_length_ + end - begin - 1 < entries_.size());
      entries_[begin * max_source_phrase_length_ + end - begin - 1] = to;
    }

    void AddTargetPhraseToVertex(
        const pt::Row *phrase,
        const SourcePhrase &source_phrase,
        search::Vertex &vertex,
        bool passthrough);

    void AddPassthrough(std::size_t position);

    VocabMap &vocab_map_;

    util::Pool target_phrase_pool_;
    boost::object_pool<search::Vertex> vertex_pool_;

    Objective &objective_;
    FeatureInit &feature_init_;

    std::vector<VocabWord*> sentence_;
    std::vector<ID> sentence_ids_;

    // Backs any oov words that are passed through.  
    util::Pool passthrough_pool_;

    pt::Row *eos_phrase_;

    // Banded array: different source lengths are next to each other.
    std::vector<TargetPhrases*> entries_;

    const std::size_t max_source_phrase_length_;

    // TODO replace with something better, possibly remove totally.
    StateMap &state_buffer_;
};

} // namespace decode

#endif // DECODE_CHART__
