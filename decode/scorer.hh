#ifndef DECODE_SCORER__
#define DECODE_SCORER__

#include "lm/model.hh"
#include "decode/id.hh"
#include "decode/hypothesis.hh"
#include "decode/weights.hh"
#include "util/string_piece.hh"

#include <vector>

namespace lm { namespace ngram { class ChartState; } }
namespace util { class MutableVocab; }

namespace decode {

class TargetPhrases;

// Poorly designed object that does scoring.
class Scorer {
	public:
	typedef lm::ngram::Model Model;
	
	Scorer(const char *model, const StringPiece &weights_file, util::MutableVocab &vocab);
	
	// Parse feature values on a phrase into a score.
	float Parse(const StringPiece &features) const;
	
	const Weights& GetWeights() const {
		return weights_;
	}
	
	const Model &LanguageModel() const { return model_; }

    // Includes lm weight.
	 float LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state);

	 // Weighted score of inserting 'num_words' target words
	 float TargetWordCount(std::size_t num_words) { return weights_.TargetWordInsertionWeight() * num_words; }

    // TODO: configurable.
    float Passthrough() const { return -100.0; }

    // TODO: distortion and apply hypothesis's share of the lexro.
    float Transition(const Hypothesis &hypothesis, const TargetPhrases &phrases, std::size_t source_begin, std::size_t source_end);

  private:
    lm::WordIndex Convert(ID from);

	 Weights weights_;

    lm::ngram::Model model_;
    std::vector<lm::WordIndex> vocab_mapping_;
    util::MutableVocab &vocab_;
};

} // namespace decode

#endif // DECODE_SCORER__
