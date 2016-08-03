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

struct BaseVocab {
  util::MutableVocab vocab;
  std::vector<VocabWord*> map;
  util::Pool pool;

  std::size_t Size() const {
    assert(vocab.Size() == map.size());
    return map.size();
  }
};

class System {
  public:
    System(const Config config, const pt::Access &phrase_access,
        const Weights &weights, const lm::ngram::Model &lm);

    void LoadWeights();

    void LoadVocab(pt::VocabRange vocab, std::size_t vocab_size);

    const Config &GetConfig() const { return config_; }

    const search::Context<lm::ngram::Model> &SearchContext() const {
      return search_context_;
    }

    Objective &GetObjective() { return objective_; }

    BaseVocab &GetBaseVocab() { return base_vocab_; }

  private:
    void InsertNewWord(const ID id);

    Objective objective_;
    const Config config_;

    BaseVocab base_vocab_;

    search::Context<lm::ngram::Model> search_context_;
    const Weights &weights_;
};
  
} // namespace decode
