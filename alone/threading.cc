#include "alone/threading.hh"

#include "alone/output.hh"
#include "alone/graph.hh"
#include "alone/read.hh"
#include "alone/vocab.hh"
#include "lm/model.hh"
#include "search/applied.hh"
#include "search/context.hh"
#include "search/edge_generator.hh"
#include "search/nbest.hh"
#include "search/vertex_generator.hh"

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <sstream>

namespace alone {
namespace {
// Return root's history.  
template <class Model, class Best> search::History InnerDecode(feature::Computer &features, search::Context<Model> &context, Graph &graph, util::FilePiece &in, Best &best) {
  assert(graph.VertexCapacity());
  for (std::size_t v = 0; v < graph.VertexCapacity() - 1; ++v) {
    search::EdgeGenerator edges;
    ReadEdges(features, context.LanguageModel(), in, graph, edges);
    search::VertexGenerator<Best> vertex_gen(context, *graph.NewVertex(), best);
    edges.Search(context, vertex_gen);
  }
  search::EdgeGenerator edges;
  ReadEdges(features, context.LanguageModel(), in, graph, edges);
  search::Vertex &root = *graph.NewVertex();
  search::RootVertexGenerator<Best> vertex_gen(root, best);
  edges.Search(context, vertex_gen);
  return root.BestChild();
}

template <class Model> void Decode(search::Context<Model> &context, feature::Computer &features, unsigned int sentence_id, util::FilePiece *in_ptr, std::ostream &out) {
  boost::scoped_ptr<util::FilePiece> in(in_ptr);
  Graph graph(context.LanguageModel().GetVocabulary());
  ReadGraphCounts(*in, graph);
  if (context.GetConfig().GetNBest().size == 1) {
    if (!graph.VertexCapacity()) {
      out << sentence_id << '\n';
      return;
    }
    search::SingleBest single_best;
    search::Applied best(InnerDecode(features, context, graph, *in, single_best));
    if (!best.Valid()) {
      out << sentence_id << '\n';
    } else {
      std::vector<search::Applied> as_vec(1, best);
      WriteNBest(out, sentence_id, as_vec, features, context.LanguageModel());
    }
  } else {
    if (!graph.VertexCapacity()) return;
    search::NBest n_best(context.GetConfig().GetNBest());
    WriteNBest(out, sentence_id, n_best.Extract(InnerDecode(features, context, graph, *in, n_best)), features, context.LanguageModel());
  }
}
} // namespace

#ifdef WITH_THREADS
template <class Model> void DecodeHandler<Model>::operator()(Input message) {
  try {
    std::stringstream assemble;
    search::Context<Model> context(config_, model_);
    Decode(context, features_, message.sentence_id, message.file, assemble);
    Produce(message.sentence_id, assemble.str());
  } catch (util::Exception &e) {
    e << " in sentence " << message.sentence_id;
    throw;
  }
}

template <class Model> void DecodeHandler<Model>::Produce(unsigned int sentence_id, const std::string &str) {
  Output out;
  out.sentence_id = sentence_id;
  out.str = new std::string(str);
  out_.Produce(out);
}

void PrintHandler::operator()(Output message) {
  unsigned int relative = message.sentence_id - done_;
  if (waiting_.size() <= relative) waiting_.resize(relative + 1);
  waiting_[relative] = message.str;
  for (std::string *lead; !waiting_.empty() && (lead = waiting_[0]); waiting_.pop_front(), ++done_) {
    out_ << *lead;
    delete lead;
  }
}

template <class Model> Controller<Model>::Controller(const search::Config &config, const Model &model, const feature::Weights &weights, size_t decode_workers, std::ostream &to) : 
  sentence_id_(0),
  printer_(decode_workers, 1, boost::ref(to), Output::Poison()),
  decoder_(
      3, // Buffer of three FilePieces for threads to read.  
      decode_workers,
      boost::in_place(boost::ref(config), boost::ref(model), boost::ref(weights), boost::ref(printer_.In())),
      Input::Poison()) {}

template class Controller<lm::ngram::RestProbingModel>;
template class Controller<lm::ngram::ProbingModel>;
template class Controller<lm::ngram::TrieModel>;
template class Controller<lm::ngram::QuantTrieModel>;
template class Controller<lm::ngram::ArrayTrieModel>;
template class Controller<lm::ngram::QuantArrayTrieModel>;

#endif

template <class Model> void InThread<Model>::Add(util::FilePiece *in) {
  search::Context<Model> context(config_, model_);
  Decode(context, features_, sentence_id_++, in, to_);
}

template class InThread<lm::ngram::RestProbingModel>;
template class InThread<lm::ngram::ProbingModel>;
template class InThread<lm::ngram::TrieModel>;
template class InThread<lm::ngram::QuantTrieModel>;
template class InThread<lm::ngram::ArrayTrieModel>;
template class InThread<lm::ngram::QuantArrayTrieModel>;


} // namespace alone
