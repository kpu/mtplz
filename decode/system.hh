#pragma once

#include "lm/model.hh"
#include "decode/objective.hh"
#include "decode/weights.hh"
#include "search/context.hh"

namespace decode {

struct Config {
  std::size_t reordering_limit;
  unsigned int pop_limit;
};

class System {
  public:
    System(const Config config, const pt::Access &phrase_access,
        const Weights &weights, const lm::ngram::Model &lm);

    void LoadWeights();

    void LoadVocab(const util::MutableVocab &vocab);

    std::size_t VocabSize() const {
      return vocab_mapping_.size();
    }

    VocabWord* GetVocabMapping(const ID index) const {
      return vocab_mapping_[index];
    }

    const Config &GetConfig() const { return config_; }

    const search::Context<lm::ngram::Model> &SearchContext() const {
      return search_context_;
    }

    Objective &GetObjective() { return objective_; }

  private:
    Objective objective_;
    const Config config_;

    std::vector<VocabWord*> vocab_mapping_;
    util::Pool word_pool_;

    search::Context<lm::ngram::Model> search_context_;
    const Weights &weights_;
};
  
} // namespace decode
