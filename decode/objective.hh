#pragma once

#include "decode/feature.hh"

#include <assert.h>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

namespace decode {

class Objective {
  public:
    std::vector<float> weights = std::vector<float>();

    explicit Objective(FeatureInit feature_init);

    void AddFeature(Feature &feature);

    // storage can be null
    float ScorePhrase(PhrasePair phrase_pair, FeatureStore *storage) const;

    // storage can be null
    float ScoreHypothesisWithSourcePhrase(
        HypothesisAndSourcePhrase combination, FeatureStore *storage) const;

    // storage can be null
    float ScoreHypothesisWithPhrasePair(
        HypothesisAndPhrasePair combination, FeatureStore *storage) const;

    std::size_t DenseFeatureCount() const;

    std::string FeatureDescription(std::size_t index) const;

  private:
    boost::ptr_vector<Feature> features_;
    std::vector<std::size_t> feature_offsets_;
    FeatureInit feature_init_;

    ScoreCollector getCollector(FeatureStore *storage) const;
};

} // namespace decode
