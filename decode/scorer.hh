#ifndef DECODE_SCORER__
#define DECODE_SCORER__

#include "lm/model.hh"
#include "decode/id.hh"
#include "util/string_piece.hh"

#include <vector>

namespace lm { namespace ngram { class ChartState; } }
namespace util { class MutableVocab; }

namespace decode {

class Hypothesis;
class TargetPhrases;

// Poorly designed object that does scoring.
class Scorer {
  public:
    Scorer(const char *model, const StringPiece &weights, util::MutableVocab &vocab);

    // Parse feature values on a phrase into a score.
    float Parse(const StringPiece &features) const;

    float LMWeight() const {
      return lm_weight_;
    }

    // Includes lm weight.
    float LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state);

    // TODO: configurable.
    float Passthrough() const { return -100.0; }

    // TODO: distortion and apply hypothesis's share of the lexro.
    float Transition(const Hypothesis &hypothesis, const TargetPhrases &phrases, std::size_t source_begin, std::size_t source_end) {
      return 0.0;
    }

  private:
    lm::WordIndex Convert(ID from);

    std::vector<float> phrase_weights_;

    float lm_weight_;

    lm::ngram::Model model_;
    std::vector<lm::WordIndex> vocab_mapping_;
    util::MutableVocab &vocab_;
};

} // namespace decode

#endif // DECODE_SCORER__
