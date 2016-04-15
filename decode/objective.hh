#pragma once

#include "decode/feature.hh"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

namespace decode {

class Objective {
  public:
    std::vector<float> weights = std::vector<float>();
    //
    /*
     * about hypotheses:
     * has coverage, n-best combinations of previous hypothesis
     * combinations)
     * */

    // TODO what constructor?
    Objective(FeatureInit feature_init);

    void AddFeature(Feature &feature);

    void ScorePhrase(PhrasePair phrase_pair) const;

    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const Phrase &phrase, const Span source_span) const;

    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, const Span source_span) const;

    unsigned DenseFeatureCount() const;

    std::string FeatureDescription(unsigned index) const;

  private:
    boost::ptr_vector<Feature> features_;
    std::vector<unsigned> feature_offsets_;
    FeatureInit feature_init_;

    ScoreCollector getCollector() const;
};

} // namespace decode
