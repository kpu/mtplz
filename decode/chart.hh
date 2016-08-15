#ifndef DECODE_CHART__
#define DECODE_CHART__

#include "decode/source_phrase.hh"
#include "decode/vocab_map.hh"
#include "decode/types.hh"
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
struct BaseVocab;
struct FeatureInit;

typedef search::Vertex TargetPhrases;

// TODO currently not thread-safe because of cache_!
// Target phrases that correspond to each source span
class Chart {
  public:
    typedef boost::unordered_map<SourcePhrase,search::Vertex,SourcePhraseHasher,SourcePhraseEqual> VertexMap;
    struct VertexCache {
      VertexCache() {}
      explicit VertexCache(std::size_t size) : map(size) {}
      VertexMap map;
      util::Pool target_phrase_pool;
    };

    static constexpr ID EOS_WORD = 2;

    Chart(std::size_t max_source_phrase_length, const BaseVocab &vocab, Objective &objective, VertexCache &cache);

    void ReadSentence(StringPiece input);

    template <class PhraseTable> void LoadPhrases(const PhraseTable &table) {
      // There's some unreachable ranges off the edge. Meh.
      entries_.resize(sentence_.size() * max_source_phrase_length_);
      for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
        for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
          SourcePhrase source_phrase(sentence_, begin, end);
          search::Vertex *vertex;
          bool use_cache = end - begin <= cached_phrase_max_length_;
          if (use_cache) {
            vertex = &cache_.map[source_phrase];
            if (!vertex->Empty()) {
              SetRange(begin, end, vertex);
              continue;
            }
          } else {
            vertex = vertex_pool_.construct();
          }
          auto phrases = table.Lookup(&sentence_ids_[begin], &*sentence_ids_.begin() + end);
          if (phrases) {
            vertex->Root().InitRoot();
            for (auto phrase = phrases.begin(); phrase != phrases.end(); ++phrase) {
              AddTargetPhraseToVertex(&*phrase, *vertex, TargetPhraseType::Table, use_cache);
            }
            vertex->Root().FinishRoot(search::kPolicyLeft);
            SetRange(begin, end, vertex);
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
        search::Vertex &vertex,
        TargetPhraseType type,
        bool use_cache);

    void AddPassthrough(std::size_t position);

    VocabMap vocab_map_;

    boost::object_pool<search::Vertex> vertex_pool_;
    util::Pool target_phrase_pool_;

    Objective &objective_;
    FeatureInit &feature_init_;

    std::vector<VocabWord*> sentence_;
    std::vector<ID> sentence_ids_;

    // Backs any oov phrases that are passed through.  
    util::Pool passthrough_pool_;

    pt::Row *eos_phrase_;

    // Banded array: different source lengths are next to each other.
    std::vector<TargetPhrases*> entries_;

    const std::size_t max_source_phrase_length_;
    const std::size_t cached_phrase_max_length_ = 2;

    // TODO replace with something better, possibly remove totally.
    // or at least pre-fill and make constant
    VertexCache &cache_;
};

} // namespace decode

#endif // DECODE_CHART__
