#ifndef DECODE_CONTEXT__
#define DECODE_CONTEXT__

#include "decode/scorer.hh"
#include "search/context.hh"
#include "util/mutable_vocab.hh"
#include "util/string_piece.hh"

namespace decode {

struct Config {
  std::size_t reordering_limit;
  unsigned int pop_limit;
};

class Context {
  public:
    Context(const char *lm, const StringPiece &weights_file, const Config &config)
      : vocab_(),
        scorer_(lm, weights_file, vocab_),
        config_(config),
        search_context_(search::Config(scorer_.GetWeights().LMWeight(), config.pop_limit, search::NBestConfig(1)), scorer_.LanguageModel()) {}

    util::MutableVocab &GetVocab() { return vocab_; }

    Scorer &GetScorer() { return scorer_; }

    const Config &GetConfig() const { return config_; }

    const search::Context<Scorer::Model> &SearchContext() { return search_context_; }

  private:
    util::MutableVocab vocab_;
    Scorer scorer_;
    Config config_;

    search::Context<Scorer::Model> search_context_;
};

} // namespace decode

#endif // DECODE_CONTEXT__
