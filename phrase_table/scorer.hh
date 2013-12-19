#ifndef PHRASE_TABLE_SCORER__
#define PHRASE_TABLE_SCORER__

#include "lm/model.hh"
#include "phrase_table/id.hh"
#include "util/string_piece.hh"

#include <vector>

namespace lm { namespace ngram { class ChartState; } }
namespace util { class MutableVocab; }

namespace phrase_table {

// Poorly designed object that does scoring.
class Scorer {
  public:
    Scorer(const char *model, util::MutableVocab &vocab, const StringPiece &weights);

    // Parse feature values on a phrase into a score.
    float Parse(const StringPiece &features) const;

    // Includes lm weight.
    float LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state);

  private:
    lm::WordIndex Convert(ID from);

    std::vector<float> phrase_weights_;

    float lm_weight_;

    lm::ngram::Model model_;
    std::vector<lm::WordIndex> vocab_mapping_;
    util::MutableVocab &vocab_;
};

} // namespace phrase_table

#endif // PHRASE_TABLE_SCORER__
