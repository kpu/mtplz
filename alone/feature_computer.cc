#include "alone/feature_computer.hh"
#include "alone/edge.hh"

#include <iostream>

namespace alone {
namespace feature {

namespace {
void RecurseFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, feature::Adder &adder) {
  const Edge &edge = *static_cast<const Edge*>(final.GetNote().vp);
  adder.Add(edge.Features());
  const Edge::WordVec &vec = edge.Words();
  const search::Applied *child = final.Children();
  for (Edge::WordVec::const_iterator i(vec.begin()); i != vec.end(); ++i) {
    if (*i) {
      words.push_back((*i)->second);
    } else {
      RecurseFeatures(*child++, words, adder);
    }
  }
}
} // namespace

void AssembleFeatures(const search::Applied final, std::vector<lm::WordIndex> &words, feature::Vector &vec) {
  assert(final.Valid());
  feature::Adder adder;
  RecurseFeatures(final, words, adder);
  adder.Finish(vec);
}

const char *Computer::kLanguageModelName = "LanguageModel";
const char *Computer::kOOVName = "LanguageModel_OOV";
const char *Computer::kWordPenaltyName = "WordPenalty";

void Computer::CheckForWeights(const Weights &in) {
  in.Lookup(kLanguageModelName, &std::cerr);
  in.Lookup(kOOVName, &std::cerr);
  in.Lookup(kWordPenaltyName, &std::cerr);
}

Computer::Computer(const Weights &process_weights) :
  weights_(Weights::ForThread(), process_weights),
  lm_(weights_.Lookup(kLanguageModelName)),
  oov_(weights_.Lookup(kOOVName)),
  word_penalty_(weights_.Lookup(kWordPenaltyName)) {
}

std::ostream &Computer::Breakdown::Print(std::ostream &o) const {
  return o << ' ' << kLanguageModelName << '=' << prob << ' ' << kOOVName << '=' << oov << ' ' << kWordPenaltyName << '=' << word_penalty;
}

} // namespace feature
} // namespace alone
