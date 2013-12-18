// Compute all internal features for a rule.   
#ifndef ALONE_FEATURE_COMPUTER__
#define ALONE_FEATURE_COMPUTER__

#include "alone/features.hh"
#include "lm/word_index.hh"
#include "search/applied.hh"
#include "search/context.hh"
#include "search/rule.hh"

#include <algorithm>
#include <vector>

namespace alone {
namespace feature {

void AssembleFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, Vector &stored);

class Computer {
  public:
    static const char *kLanguageModelName;
    static const char *kOOVName;
    static const char *kWordPenaltyName;

    static void CheckForWeights(const Weights &in);

    explicit Computer(const Weights &process_weights);

    template <class Model> search::Score Read(const Model &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *state_out, StringPiece others, Vector &store) {
      Breakdown down(model, words, state_out);
      return down.prob * lm_ + down.oov * oov_ + down.word_penalty * word_penalty_ + weights_.Parse(others, store);
    }

    template <class Model> std::ostream &Write(std::ostream &o, search::Applied final, const Model &model) const {
      std::vector<lm::WordIndex> words;
      Vector stored;
      AssembleFeatures(final, words, stored);
      weights_.Write(o, stored);
      lm::ngram::ChartState ignored;
      Breakdown down(model, words, &ignored);
      return down.Print(o);
    }

  private:
    struct Breakdown : search::ScoreRuleRet {
      template <class Model> Breakdown(const Model &model, const std::vector<lm::WordIndex> &words, lm::ngram::ChartState *state_out) 
        : search::ScoreRuleRet(search::ScoreRule(model, words, state_out)),
          word_penalty(-static_cast<float>(words.size() - std::count(words.begin(), words.end(), search::kNonTerminal)) / M_LN10) {}

      std::ostream &Print(std::ostream &o) const;

      search::Score word_penalty;
    };

    Weights weights_;

    search::Score lm_, oov_, word_penalty_;
};

} // namespace feature
} // namespace alone

#endif // ALONE_FEATURE_COMPUTER__
