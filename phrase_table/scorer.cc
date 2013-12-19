#include "phrase_table/scorer.hh"

#include "lm/left.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

#include <math.h>

namespace phrase_table {
namespace {
static const double_conversion::StringToDoubleConverter kConverter(
    double_conversion::StringToDoubleConverter::NO_FLAGS,
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::quiet_NaN(),
    "inf",
    "NaN");

} // namespace

Scorer::Scorer(const char *model, util::MutableVocab &vocab, const StringPiece &weights) : model_(model), vocab_(vocab) {
  for (util::TokenIter<util::SingleCharacter, true> token(weights, ' '); token; ++token) {
    int length;
    phrase_weights_.push_back(kConverter.StringToFloat(token->data(), token->size(), &length));
    UTIL_THROW_IF(isnan(phrase_weights_.back()), util::Exception, "Bad feature weight " << *token);
  }
  UTIL_THROW_IF(phrase_weights_.empty(), util::Exception, "Expected LM weight");
  // TODO hack
  lm_weight_ = phrase_weights_.back();
  phrase_weights_.pop_back();
}

float Scorer::Parse(const StringPiece &features) const {
  std::vector<float>::const_iterator w = phrase_weights_.begin();
  float ret = 0.0;
  for (util::TokenIter<util::SingleCharacter, true> token(features, ' '); token; ++token, ++w) {
    int length;
    float val = kConverter.StringToFloat(token->data(), token->size(), &length);
    UTIL_THROW_IF(isnan(val), util::Exception, "Bad score " << *token);
    UTIL_THROW_IF(w == phrase_weights_.end(), util::Exception, "More features than weights in " << features);
    ret += *w * val;
  }
  UTIL_THROW_IF(w != phrase_weights_.end(), util::Exception, "Expected " << phrase_weights_.size() << " features, but got " << features);
  return ret;
}

float Scorer::LM(const ID *words_begin, const ID *words_end, lm::ngram::ChartState &state) {
  lm::ngram::RuleScore<lm::ngram::Model> scorer(model_, state);
  for (const ID *i = words_begin; i != words_end; ++i) {
    scorer.Terminal(Convert(*i));
  }
  return lm_weight_ * scorer.Finish();
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

} // namespace phrase_table
