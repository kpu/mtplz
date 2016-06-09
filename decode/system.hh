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
        const Weights &weights, const lm::ngram::Model &lm)
      : config_(config), weights_(weights),
        objective_(phrase_access, lm.BeginSentenceState()),
        search_context_(search::Config(
              weights.LMWeight(),
              config.pop_limit,
              search::NBestConfig(1)), lm) {}

    void LoadWeights() {
      objective_.LoadWeights(weights_);
    }

    const Config &GetConfig() const { return config_; }

    const search::Context<lm::ngram::Model> &SearchContext() const {
      return search_context_;
    }

    Objective &GetObjective() { return objective_; }

  private:
    Objective objective_;
    const Config config_;
    search::Context<lm::ngram::Model> search_context_;
    const Weights &weights_;
};
  
} // namespace decode
