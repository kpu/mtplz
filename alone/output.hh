#ifndef ALONE_OUTPUT__
#define ALONE_OUTPUT__

#include "alone/feature_computer.hh"

#include <iosfwd>
#include <vector>

namespace search { class Applied; }

namespace alone {

class Edge;

std::ostream &TextOutput(std::ostream &to, const search::Applied final);

std::ostream &DerivationOutput(std::ostream &o, const search::Applied final, const Edge *base_edge, const char *indent_str = "  ");

template <class Model> void WriteNBest(std::ostream &o, unsigned sentence_id, const std::vector<search::Applied> &applied, const feature::Computer &features, const Model &model) {
  for (std::vector<search::Applied>::const_iterator i = applied.begin(); i != applied.end(); ++i) {
    o << sentence_id << " ||| ";
    TextOutput(o, *i);
    o << " ||| ";
    features.Write(o, *i, model);
    o << " ||| " << i->GetScore() << '\n';
  }
}

} // namespace alone

#endif // ALONE_OUTPUT__
