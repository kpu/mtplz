#pragma once

#include "decode/feature.hh"

#include <vector>

namespace decode {

class Objective {
  public:
    std::vector<float> weights;
    // TODO boost::ptr_vector features_
    //
    /*
     * about hypotheses:
     * has coverage, n-best combinations of previous hypothesis
     * combinations)
     * */

    // TODO what constructor?

    void AddFeature(Feature &feature);

    void NewWord(StringPiece string_rep, VocabWord *word) const override;

    void ScorePhrase(PhrasePair phrase_pair) const;
    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override;

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const Phrase &phrase, const Span source_span) const;
    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const Phrase &phrase,
        const Span source_span, ScoreCollector &collector) const override;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, const Span source_span) const;
    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair,
        const Span source_span, ScoreCollector &collector) const override;

    unsigned DenseFeatureCount() const override;

    std::string FeatureDescription(unsigned index) const override;

    void Init(FeatureInit &feature_init) override; // not implemented
    
  private:
    std::vector<Feature*> features_;
    FeatureInit *feature_init_;

    ScoreCollector getCollector() const;
};

} // namespace decode
