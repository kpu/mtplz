#include "decode/output.hh"

#include "decode/feature_init.hh"
#include "util/file_stream.hh"
#include "util/mutable_vocab.hh"

#include <string.h>

namespace decode {

bool Valid(const TargetPhrase *row) {
  // TODO replace with proper validity check, if there is such a thing for row
  // previous: row.Valid()
  return true;
}

void PrintOptionalInfo(ScoreHistoryMap &map, float score_delta, util::FileStream &out) {
  map["_total"].scores.push_back(score_delta);
  map["_total"].total += score_delta;

  out << '\n';

  for(decode::ScoreHistoryMap::iterator i=map.begin(); i!=map.end(); i++) {
    out << i->first << ": " << i->second.total << "\n";
    out << "    [";
    for(std::vector<float>::iterator i2 = i->second.scores.begin(); i2!=i->second.scores.end(); i2++) {
      out << *i2 << ",";
    }
    out << "]" << "\n";
  }
}

// verbose print
void Output(const Hypothesis &hypo, const util::MutableVocab &vocab,
    ScoreHistoryMap &map, util::FileStream &to, const FeatureInit &feature_init,
    bool verbose) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!Valid(h->Target())) continue;
    hypos.push_back(h);
  }
  to << hypo.GetScore();
  float previous_score = 0.0;
  assert(feature_init.phrase_access.target); // check that there is a target
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend() - 1 /* skip EOS */; ++i) {
    if ((*i)->Target() != nullptr) {
      auto ids = feature_init.phrase_access.target(feature_init.pt_row_field((*i)->Target()));
      for (const ID id : ids) {
        to << ' ' << vocab.String(id);
      }
    }
    if (verbose) {
      float this_score = hypo.GetScore();
      float score_delta = this_score - previous_score;
      previous_score = this_score;
      PrintOptionalInfo(map, score_delta, to);
    }
  }
}

} // namespace decode
