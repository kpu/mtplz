#include "alone/read.hh"

#include "alone/features.hh"
#include "alone/feature_computer.hh"
#include "alone/graph.hh"
#include "alone/vocab.hh"
#include "lm/model.hh"
#include "search/context.hh"
#include "search/edge.hh"
#include "search/edge_generator.hh"
#include "util/file_piece.hh"

#include <boost/unordered_map.hpp>

#include <algorithm>
#include <cstdlib>

namespace alone {

namespace {

template <class Model> void ReadEdge(feature::Computer &features, Model &model, util::FilePiece &from, Graph &graph, search::EdgeGenerator &generator) {
  Edge &alone_edge = *graph.NewEdge();

  StringPiece got;
  std::vector<lm::WordIndex> words;
  std::vector<search::Vertex*> children;
  unsigned long int terminals = 0;
  float below_score = 0.0;
  while ("|||" != (got = from.ReadDelimited())) {
    if ('[' == *got.data() && ']' == got.data()[got.size() - 1]) {
      // non-terminal
      char *end_ptr;
      unsigned long int child = std::strtoul(got.data() + 1, &end_ptr, 10);
      UTIL_THROW_IF(end_ptr != got.data() + got.size() - 1, FormatException, "Bad non-terminal" << got);
      UTIL_THROW_IF(child >= graph.VertexSize(), FormatException, "Reference to vertex " << child << " but we only have " << graph.VertexSize() << " vertices.  Is the file in bottom-up format?");
      alone_edge.AppendWord(NULL);
      words.push_back(lm::kMaxWordIndex);
      children.push_back(&graph.GetVertex(child));
      if (children.back()->Empty()) {
        from.ReadLine();
        return;
      }
      below_score += children.back()->Bound();
    } else {
      const Vocab::Entry &found = graph.MutableVocab().FindOrAdd(got);
      alone_edge.AppendWord(&found);
      words.push_back(found.second);
      ++terminals;
    }
  }

  if (words.empty()) return;
  search::PartialEdge edge(generator.AllocateEdge(children.size()));
  std::vector<search::Vertex*>::const_iterator i = children.begin();
  search::PartialVertex *nt = edge.NT();
/*  if (words.front() == lm::kMaxWordIndex) {
    *(nt++) = (*i)->RootFirst();
    ++i;
  }*/
  for (; i != children.end(); ++i, ++nt) {
    *nt = (*i)->RootAlternate();
  }
/*  if (children.size() > 1 && words.back() == lm::kMaxWordIndex) {
    edge.NT()[children.size() - 1] = children.back()->RootLast();
  }*/

  edge.SetScore(below_score + features.Read(model, words, edge.Between(), from.ReadLine(), alone_edge.InitFeatures()));

  search::Note note;
  note.vp = &alone_edge;
  edge.SetNote(note);
  generator.AddEdge(edge);
}

} // namespace

void ReadGraphCounts(util::FilePiece &from, Graph &graph) {
  unsigned long int vertices = from.ReadULong();
  unsigned long int edges = from.ReadULong();
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after counts");
  graph.SetCounts(vertices, edges);
}

template <class Model> void ReadEdges(feature::Computer &features, Model &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &edges) {
  unsigned long int edge_count = from.ReadULong();
  UTIL_THROW_IF('\n' != from.get(), FormatException, "Expected newline after edge count");
  for (unsigned long int e = 0; e < edge_count; ++e) {
    ReadEdge(features, context, from, graph, edges);
  }
}

template void ReadEdges(feature::Computer &features, const lm::ngram::ProbingModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(feature::Computer &features, const lm::ngram::RestProbingModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(feature::Computer &features, const lm::ngram::TrieModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(feature::Computer &features, const lm::ngram::QuantTrieModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(feature::Computer &features, const lm::ngram::ArrayTrieModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);
template void ReadEdges(feature::Computer &features, const lm::ngram::QuantArrayTrieModel &context, util::FilePiece &from, Graph &graph, search::EdgeGenerator &gen);

} // namespace alone
