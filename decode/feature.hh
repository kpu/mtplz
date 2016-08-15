#pragma once

#include "decode/source_phrase.hh"
#include "decode/feature_init.hh"
#include "decode/score_collector.hh"
#include "decode/types.hh"
// TODO cleanup imports

#include <cstddef>
#include <string>

namespace decode {

class VocabMap;

// Layouts:
struct Hypothesis;
struct VocabWord;

struct TargetPhraseInfo {
  TargetPhrase *&phrase;
  const VocabMap &vocab_map;
  util::Pool &phrase_pool; // for dynamic-length access to phrase
  TargetPhraseType type;
};
 
struct PhrasePair {
  PhrasePair(const SourcePhrase &source_phrase, TargetPhrase *target_phrase)
    : source(source_phrase), target(target_phrase) {}

  const SourcePhrase source;
  TargetPhrase *target;
  const VocabMap *vocab_map;
};

class Feature {
  public:
    // recommended constructor: Feature(const std::string &config);

    Feature(const StringPiece feature_name) : name(feature_name) {}

    virtual ~Feature(){};
    
    StringPiece name;

    /** Add state fields to the layouts in init. */
    virtual void Init(FeatureInit &feature_init) = 0;

    /** allows to save constant-length data in the word's representation */
    virtual void NewWord(const StringPiece string_rep, VocabWord *word) const = 0;

    /** Allows to add constant-length data to a passthrough or eos pt-phrase.
     * See documentation in pt/access.hh */
    virtual void InitPassthroughPhrase(pt::Row *passthrough, TargetPhraseType type) const = 0;

    /** Score isolated target phrase.
     * Allows to store data in target phrase. */
    virtual void ScoreTargetPhrase(TargetPhraseInfo target, ScoreCollector &collector) const = 0;

    /** collects score and allows to save constant-length data in
     * the hypothesis layout for the next hypothesis (collector.NewHypothesis()) */
    virtual void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const = 0;

    /** collects score and allows to save arbitrary data in the hypothesis
     * layout for the next hypothesis (collector.NewHypothesis()) or
     * constant-length data in the target phrase. */
    virtual void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const = 0;

    /** collects score for a completed hypothesis */
    virtual void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const = 0;

    virtual std::size_t DenseFeatureCount() const = 0;

    virtual std::string FeatureDescription(std::size_t index) const = 0;
};

class ObjectiveBypass {
  public:
    virtual void InitTargetPhrase(TargetPhraseInfo target, lm::ngram::ChartState &state) const = 0;
    virtual void SetSearchScore(Hypothesis *new_hypothesis, float score) const = 0;
};


} // namespace decode
