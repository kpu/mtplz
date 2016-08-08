#pragma once

#include "decode/source_phrase.hh"
#include "decode/feature_init.hh"
#include "decode/score_collector.hh"
// TODO cleanup imports

#include <cstddef>
#include <string>

namespace decode {

enum ScoreMethod : uint8_t {
  Any = 0,
  NewWord = 1, InitPassthrough = 2, Phrase = 4, Source = 8, Pair = 16, Final = 32,
  Max = 63
};

class VocabMap;

// Layouts:
struct Hypothesis;
struct VocabWord;

struct TargetPhraseInfo {
  TargetPhrase *&phrase;
  const VocabMap &vocab_map;
  util::Pool &target_phrase_pool; // for dynamic-length access to phrase
};
 
// TODO store TargetPhraseInfo instead of single values
struct PhrasePair {
  const SourcePhrase source_phrase;
  TargetPhrase *&target_phrase;
  const VocabMap *vocab_map;
  // pool is null when target phrase only allows constant-length data changes
  util::Pool *target_phrase_pool = nullptr;
};

class Feature {
  public:
    // recommended constructor: Feature(const std::string &config);

    Feature(const StringPiece feature_name) : name(feature_name) {}

    virtual ~Feature(){};
    
    StringPiece name;

    /** Add state fields to the layouts in init. Return which scoring
     * methods should be used (see ScoreMethod above) */
    virtual uint8_t Init(FeatureInit &feature_init) = 0;

    /** allows to save constant-length data in the word's representation */
    virtual void NewWord(const StringPiece string_rep, VocabWord *word) const = 0;

    /** Allows to add constant-length data to a passthrough or eos pt-phrase.
     * See documentation in pt/access.hh */
    virtual void InitPassthroughPhrase(pt::Row *passthrough) const = 0;

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

// for LM bypass
class TargetPhraseInitializer {
  public:
    virtual void InitTargetPhrase(TargetPhraseInfo target, lm::ngram::ChartState &state) const = 0;
};


} // namespace decode
