#ifndef DECODE_CONTEXT__
#define DECODE_CONTEXT__

#include "decode/scorer.hh"

#include "util/mutable_vocab.hh"
#include "util/string_piece.hh"

namespace decode {

struct Config {
  std::size_t reordering_limit;
};

class Context {
  public:
    Context(const char *lm, const StringPiece &weights, const Config &config)
      : vocab_(), scorer_(lm, weights, vocab_), config_(config) {}

    util::MutableVocab &GetVocab() { return vocab_; }

    Scorer &GetScorer() { return scorer_; }

    const Config &GetConfig() const { return config_; }

  private:
    util::MutableVocab vocab_;
    Scorer scorer_;
    Config config_;
};

} // namespace decode

#endif // DECODE_CONTEXT__
