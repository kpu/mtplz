#pragma once

#include "decode/id.hh"
#include "decode/phrase.hh"
#include "decode/feature_init.hh"
#include "decode/score_collector.hh"

#include <utility> // for std::pair
#include <string>

namespace decode {

typedef std::pair<ID,ID> Span;

// Layouts:
struct TargetPhrase;
struct Hypothesis;
struct VocabWord;
 
struct PhrasePair {
  const Phrase *phrase;
  TargetPhrase *targetPhrase;
};

class Feature {
  public:
    // recommended constructor: Feature(const std::string &config);

    virtual ~Feature(){};

    virtual void Init(FeatureInit &feature_init) = 0;

    virtual void NewWord(StringPiece string_rep, VocabWord *word) const = 0;

    virtual void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const = 0;

    virtual void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const Phrase &phrase,
        const Span source_span, ScoreCollector &collector) const = 0;

    virtual void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair,
        const Span source_span, ScoreCollector &collector) const = 0;

    virtual unsigned DenseFeatureCount() const = 0;

    virtual std::string FeatureDescription(unsigned index) const = 0;
};

} // namespace decode
