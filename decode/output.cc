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

} // namespace decode
