#include "decode/output.hh"

#include "decode/vocab_map.hh"
#include "decode/feature_init.hh"
#include "util/file_stream.hh"

#include <string.h>

namespace decode {

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

void Output(const Hypothesis &hypo, const VocabMap &vocab,
    ScoreHistoryMap &map, util::FileStream &out, const FeatureInit &feature_init,
    bool verbose) {
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (h->Target() == nullptr) continue;
    hypos.push_back(h);
  }
  out << hypo.GetScore();
  float previous_score = 0.0;
  assert(feature_init.phrase_access.target);
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend()-1/*ignore EOS*/; ++i) {
    auto ids = feature_init.phrase_access.target(feature_init.pt_row_field((*i)->Target()));
    for (const ID id : ids) {
      out << ' ' << vocab.String(id);
    }
    if (verbose) {
      float this_score = hypo.GetScore();
      float score_delta = this_score - previous_score;
      previous_score = this_score;
      // TODO make this work again
      /* PrintOptionalInfo(map, score_delta, out); */
    }
  }
}

} // namespace decode
