#include "decode/scorer.hh"

#include "lm/left.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

#include <math.h>

namespace decode {
namespace {
static const double_conversion::StringToDoubleConverter kConverter(
    double_conversion::StringToDoubleConverter::NO_FLAGS,
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::quiet_NaN(),
    "inf",
    "NaN");

} // namespace

Scorer::Scorer(const char *model, const StringPiece &weights_file, util::MutableVocab &vocab) : model_(model), vocab_(vocab) {
	weights_.ReadFromFile(weights_file);
}

float Scorer::Parse(const StringPiece &features) const {
	const std::vector<float> &phrase_weights = weights_.PhraseTableWeights();
  std::vector<float>::const_iterator w = phrase_weights.begin();
  float ret = 0.0;
  for (util::TokenIter<util::SingleCharacter, true> token(features, ' '); token; ++token, ++w) {
    int length;
    float val = kConverter.StringToFloat(token->data(), token->size(), &length);
    UTIL_THROW_IF(isnan(val), util::Exception, "Bad score " << *token);
    UTIL_THROW_IF(w == phrase_weights.end(), util::Exception, "Have " << phrase_weights.size() << " weights but was given feature vector " << features);
    ret += *w * val;
  }
  UTIL_THROW_IF(w != phrase_weights.end(), util::Exception, "Expected " << phrase_weights.size() << " features, but got " << features);
  return ret;
}

float Scorer::LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state) {
  lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state);
  for (const ID *i = words_begin; i != words_end; ++i) {
    scorer.Terminal(Convert(*i));
  }
  return weights_.LMWeight() * scorer.Finish();
}

float Scorer::Transition(const Hypothesis &hypothesis, const TargetPhrases &phrases, std::size_t source_begin, std::size_t source_end) {
	std::size_t jump_size = abs((int) hypothesis.LastSourceIndex() - source_begin);
	return (jump_size * weights_.DistortionWeight());
}

lm::WordIndex Scorer::Convert(ID from) {
  if (from >= vocab_mapping_.size()) {
    vocab_mapping_.reserve(vocab_.Size());
    while (vocab_mapping_.size() < vocab_.Size()) {
      vocab_mapping_.push_back(model_.GetVocabulary().Index(vocab_.String(vocab_mapping_.size())));
    }
  }
  return vocab_mapping_[from];
}

} // namespace decode
