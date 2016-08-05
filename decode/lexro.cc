#include "decode/lexro.hh"
#include "util/exception.hh"

namespace decode {

uint8_t LexicalizedReordering::Init(FeatureInit &feature_init) {
  pt_row_ = feature_init.pt_row_field;
  UTIL_THROW_IF(!feature_init.phrase_access.lexical_reordering, util::Exception,
      "requested lexicalized reordering but feature values are missing in phrase access");
  phrase_start_ = util::PODField<std::size_t>(feature_init.hypothesis_layout);
  phrase_access_ = &feature_init.phrase_access;
  return ScoreMethod::InitPassthrough | ScoreMethod::Source | ScoreMethod::Pair | ScoreMethod::Final;
}

void LexicalizedReordering::InitPassthroughPhrase(pt::Row *passthrough) const {
  for(uint8_t i = 0; i < VALUE_COUNT; ++i) {
    phrase_access_->lexical_reordering(passthrough)[i] = DEFAULT_VALUE;
  }
}

void LexicalizedReordering::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const {
  // store phrase start index in layout; phrase end is already stored in the hypothesis
  phrase_start_(collector.NewHypothesis()) = source_phrase.Span().first;
  // score backward lexro
  SourceSpan hypo_span;
  if (hypothesis.Previous()) {
    hypo_span = SourceSpan(phrase_start_(&hypothesis), hypothesis.SourceEndIndex());
  } else { // start of sentence
    hypo_span = SourceSpan(0,0);
  }
  uint8_t index = BACKWARD + PhraseRelation(hypo_span, source_phrase.Span());
  const pt::Row *target = pt_row_(hypothesis.Target());
  collector.AddDense(index, phrase_access_->lexical_reordering(target)[index]);
}

void LexicalizedReordering::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const {
  SourceSpan hypo_span(phrase_start_(&hypothesis), hypothesis.SourceEndIndex());
  uint8_t index = FORWARD + PhraseRelation(hypo_span, phrase_pair.source_phrase.Span());
  const pt::Row *target = pt_row_(phrase_pair.target_phrase);
  float score = phrase_access_->lexical_reordering(target)[index];
  collector.AddDense(index, score);
}

void LexicalizedReordering::ScoreFinalHypothesis(
    const Hypothesis &hypothesis, ScoreCollector &collector) const {
  // remove eos scoring
  SourceSpan hypo_span = SourceSpan(phrase_start_(&hypothesis), hypothesis.SourceEndIndex());
  SourceSpan prev_span;
  if (hypothesis.Previous()->Previous()) {
    prev_span = SourceSpan(phrase_start_(hypothesis.Previous()), hypothesis.Previous()->SourceEndIndex());
  } else { // start of sentence
    prev_span = SourceSpan(0,0);
  }
  uint8_t index = BACKWARD + PhraseRelation(prev_span, hypo_span);
  const pt::Row *target = pt_row_(hypothesis.Previous()->Target());
  collector.AddDense(index, -phrase_access_->lexical_reordering(target)[index]);
}

LexicalizedReordering::Relation LexicalizedReordering::PhraseRelation(
    SourceSpan phrase1, SourceSpan phrase2) const {
  if (phrase1.second == phrase2.first) {
    return MONOTONE;
  } else if (phrase1.first == phrase2.second) {
    return SWAP;
  } else {
    return DISCONTINUOUS;
  }
}

std::string LexicalizedReordering::FeatureDescription(std::size_t index) const {
  assert(index < DenseFeatureCount());
  switch (index) {
    case 0: return "monotone forward lexicalized reordering";
    case 1: return "swap forward lexicalized reordering";
    case 2: return "discontinuous forward lexicalized reordering";
    case 3: return "monotone backward lexicalized reordering";
    case 4: return "swap backward lexicalized reordering";
    case 5: return "discontinuous backward lexicalized reordering";
    default: return "no such feature in lexro!";
  }
}

} // namespace decode
