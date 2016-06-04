#pragma once

#include "decode/id.hh"
#include "decode/phrase.hh"
#include "decode/feature_init.hh"
#include "decode/score_collector.hh"

#include <cstddef>
#include <utility> // for std::pair
#include <string>

namespace decode {

typedef std::pair<std::size_t,std::size_t> Span; // TODO size_t -> ID*, problem with coverage in hypo

// Layouts:
struct TargetPhrase;
struct Hypothesis;
struct VocabWord;
 
struct PhrasePair {
  const Phrase *phrase;
  TargetPhrase *targetPhrase;
};

struct HypothesisAndSourcePhrase {
  const Hypothesis &hypothesis;
  const Span source_span;
  const Phrase &phrase;
};

struct HypothesisAndPhrasePair {
  const Hypothesis &hypothesis;
  const Span source_span;
  PhrasePair phrase_pair;
};

class Feature {
  public:
    // recommended constructor: Feature(const std::string &config);
    Feature(const StringPiece feature_name) : name(feature_name) {}

    virtual ~Feature(){};
    
    const StringPiece name;

    virtual void Init(FeatureInit &feature_init) = 0;

    virtual void NewWord(StringPiece string_rep, VocabWord *word) const = 0;

    virtual void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const = 0;

    virtual void ScoreHypothesisWithSourcePhrase(
        HypothesisAndSourcePhrase combination, ScoreCollector &collector) const = 0;

    virtual void ScoreHypothesisWithPhrasePair(
        HypothesisAndPhrasePair combination, ScoreCollector &collector) const = 0;

    virtual void RescoreHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const = 0;

    virtual std::size_t DenseFeatureCount() const = 0;

    virtual std::string FeatureDescription(std::size_t index) const = 0;
};

} // namespace decode
