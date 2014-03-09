#include "decode/output.hh"

#include "decode/hypothesis.hh"
#include "util/fake_ofstream.hh"
#include "util/mutable_vocab.hh"

#include <string.h>

namespace decode {

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab, util::FakeOFStream &to) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!h->Target().Valid()) continue;
    hypos.push_back(h);
  }
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend() - 1 /* skip EOS */; ++i) {
    for (const ID *id = (*i)->Target().begin(); id != (*i)->Target().end(); ++id) {
      to << vocab.String(*id) << ' ';
    }
  }
  to << "||| " << hypo.Score();
}


// TODO: fix bad code duplication. 

void OutputVerbose(const Hypothesis &hypo, const util::MutableVocab &vocab, ScoreHistoryMap &map, util::FakeOFStream &out) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!h->Target().Valid()) continue;
    hypos.push_back(h);
  }
	map.clear();
	float previous_score = 0.0;
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend()-1 /* skip EOS */; ++i) {
    for (const ID *id = (*i)->Target().begin(); id != (*i)->Target().end(); ++id) {
      StringPiece str(vocab.String(*id));
      out << str << ' ';
    }
		float this_score = (*i)->Score();
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
