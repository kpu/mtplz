#include "alone/output.hh"

#include "alone/edge.hh"
#include "alone/features.hh"
#include "search/applied.hh"

#include <iostream>

namespace alone {

std::ostream &TextOutput(std::ostream &o, const search::Applied final) {
  const Edge::WordVec &words = static_cast<const Edge*>(final.GetNote().vp)->Words();
  if (words.empty()) return o;
  const search::Applied *child = final.Children();
  Edge::WordVec::const_iterator i(words.begin());
  // Initial case: check for <s>.
  if (*i && (*i)->first == StringPiece("<s>")) {
    if (++i == words.end()) return o;
  }
  // Middle case.  
  for (; i != words.end() - 1; ++i) {
    if (*i) {
      o << (*i)->first << ' ';
    } else {
      TextOutput(o, *child) << ' ';
      ++child;
    }
  }

  // Final case: check for </s>.
  if (*i) {
    if ((*i)->first != StringPiece("</s>"))
      o << (*i)->first;
  } else {
    TextOutput(o, *child);
  }

  return o;
}

namespace {

void MakeIndent(std::ostream &o, const char *indent_str, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i)
    o << indent_str;
}

void DerivationOutputInternal(std::ostream &o, const search::Applied final, const Edge *base_edge, const char *indent_str, unsigned int indent) {
  const Edge *edge = static_cast<const Edge*>(final.GetNote().vp);
  o << "(" << (edge - base_edge) << "\n";
  MakeIndent(o, indent_str, indent);
  const Edge::WordVec &words = edge->Words();
  const search::Applied *child = final.Children();
  for (Edge::WordVec::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (*i) {
      o << (*i)->first;
      if (i == words.end() - 1) {
        o << '\n';
        MakeIndent(o, indent_str, indent);
      } else {
        o << ' ';
      }
    } else {
      // One extra indent from the line we're currently on.  
      o << indent_str;
      DerivationOutputInternal(o, *child, base_edge, indent_str, indent + 1);
      for (unsigned int i = 0; i < indent; ++i) o << indent_str;
      ++child;
    }
  }
  o << ")=" << final.GetScore() << '\n';
}
} // namespace

std::ostream &DerivationOutput(std::ostream &o, const search::Applied final, const Edge *base_edge, const char *indent_str) {
  DerivationOutputInternal(o, final, base_edge, indent_str, 0);
  return o;
}

} // namespace alone
