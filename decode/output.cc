#include "decode/output.hh"

#include "decode/hypothesis.hh"
#include "util/mutable_vocab.hh"

#include <string.h>

namespace decode {

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab, std::string &to) {
  // TODO more efficient algorithm?  Also, I wish rope was part of the standard.
  std::vector<const Hypothesis*> hypos;
  for (const Hypothesis *h = &hypo; h; h = h->Previous()) {
    if (!h->Target().Valid()) continue;
    hypos.push_back(h);
  }
  to.clear();
  for (std::vector<const Hypothesis*>::const_reverse_iterator i = hypos.rbegin(); i != hypos.rend(); ++i) {
    for (const ID *id = (*i)->Target().begin(); id != (*i)->Target().end(); ++id) {
      StringPiece str(vocab.String(*id));
      to.append(str.data(), str.size());
      to += ' ';
    }
  }
  to.resize(to.size() - 1); // trailing space.
}

} // namespace decode
