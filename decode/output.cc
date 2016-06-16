#include "decode/output.hh"

#include "decode/hypothesis.hh"
#include "util/file_stream.hh"
#include "util/mutable_vocab.hh"

#include <string.h>

namespace decode {

bool Valid(const pt::Row *row) {
  // TODO replace with proper validity check, if there is such a thing for row
  // previous: row.Valid()
  return true;
}

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab, util::FileStream &to) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!Valid(h->Target())) continue;
    hypos.push_back(h);
  }
  to << hypo.GetScore();
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend() - 1 /* skip EOS */; ++i) {
    for (const ID *id = /*(*i)->Target().begin() TODO replace with access.source_phrase..*/NULL; id != /*TODO (*i)->Target().end()*/NULL; ++id) {
      to << ' ' << vocab.String(*id);
    }
  }
}


// TODO: fix bad code duplication. 

void OutputVerbose(const Hypothesis &hypo, const util::MutableVocab &vocab, ScoreHistoryMap &map, util::FileStream &out) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!Valid(h->Target())) continue;
    hypos.push_back(h);
  }
	map.clear();
	float previous_score = 0.0;
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend()-1 /* skip EOS */; ++i) {
    for (const ID *id = /*TODO replace with access.source_phrase.. (*i)->Target().begin()*/NULL; id != /*TODO (*i)->Target().end()*/NULL; ++id) {
      StringPiece str(vocab.String(*id));
      out << str << ' ';
    }
		float this_score = (*i)->GetScore();
		float score_delta = this_score - previous_score;
		previous_score = this_score;

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
}

} // namespace decode
