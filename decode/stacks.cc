#include "decode/stacks.hh"

#include "search/edge_generator.hh"

namespace decode {

namespace {
class Vertices {
  public:
    void Add(const Hypothesis &hypothesis, const TargetPhrases &phrases, float score_delta) {
      search::HypoState add;
      add.history = &hypothesis;
      add.state.right = hypothesis.State();
      add.state.left.length = 0;
      add.state.left.full = true;
      add.score = hypothesis.Score() + score_delta;
      map_[&phrases].Root().AppendHypothesis(add);
    }

    void Apply(EdgeGenerator &out) {
      for (Map::iterator i = map_.begin(); i != map_.end(); ++i) {
        search::PartialEdge edge(to.AllocateEdge(2));
        // Empty LM state before/between/after
        for (unsigned int i = 0; i < 3; ++i) {
          edge.Between()[i].left.length = 0;
          edge.Between()[i].left.full = false;
          edge.Between()[i].right.length = 0;
        }
        edge.SetScore(0.0);
        edge.SetNote(*(i->first)); // Record TargetPhrases in note.
        i->second.Root().FinishRoot(kPolicyRight);
        edge.NT()[0] = i->second.Root();
        edge.NT()[1] = i->first->vertex.RootAlternate();
        out.AddEdge(edge);
      }
    }

  private:
    // TODO: dense as 2D array?
    typedef boost::unordered_map<const TargetPhrases *, search::Vertex> Map;
    Map map_;
};

class EdgeOutput {
  public:
    EdgeOutput(boost::object_pool<Hypothesis> &hypo_pool, Stack &stack, const Chart &chart)
      : hypo_pool_(hypo_pool), stack_(stack), chart_(chart) {}

    void NewHypothesis(search::PartialEdge complete) {
      const TargetPhrases *phrase = static_cast<const TargetPhrases*>(complete.GetNote());
      std::pair<std::size_t, std::size_t> source_range(chart_.RangeFromPointer(phrase));
      stack_.push_back(hypo_pool_.construct(
            complete.CompletedState().right,
            complete.Score(), // TODO: call scorer to adjust for last of lexro?
            *static_cast<const Hypothesis*>(complete.NT()[0].End()),
            source_range.first,
            source_range.second,
            phrase));
    }

  private:
    boost::object_pool<Hypothesis> &hypo_pool_;
    Stack &stack_;

    const Chart &chart_;
};

} // namespace

Stacks::Stacks(const Context &context, const Chart &chart) {
  stacks_.reserve(chart.SentenceLength() + 2 /* begin/end of sentence */);
  stacks_.push_back(pool_.construct(context.GetScorer().BeginSentence()));
  // Decode with increasing numbers of source words.
  for (std::size_t source_words = 1; source_words <= chart.SentenceLength(); ++source_words) {
    Vertices vertices;
    // Iterate over stacks to continue from.
    for (std::size_t from = source_words - std::min(source_words, chart.MaxSourcePhraseLength());
         from < source_words;
         ++from) {
      const std::size_t phrase_length = source_words - from;
      // Iterate over antecedents in this stack.
      for (Stack::const_iterator ant = from.begin(); ant != from.end(); ++ant) {
        const Coverage &coverage = (*ant)->GetCoverage();
        const std::size_t first_zero = coverage.FirstZero();
        std::size_t begin = first_zero;
        // We can always go from first_zero because it doesn't create a reordering gap.
        do {
          const TargetPhrases *phrases = chart.Range(begin, begin + phrase_length);
          if (!phrases || !coverage.Compatible(begin, begin + phrase_length)) continue;
          vertices.Add(**ant, *phrases, context.GetScorer().Transition(**ant, *phrases, begin, begin + phrase_length));
        // Enforce the reordering limit on later iterations.
        } while (++begin + phrase_length <= first_zero + context.GetConfig().reordering_limit);
      }
    }
    search::EdgeGenerator gen;
    vertices.Apply(gen);
    stacks_.ressize(stacks_.size() + 1);
    EdgeOutput out(pool_, stacks_.back(), chart);
    gen.Search(context.SearchContext(), output);
  }
}

} // namespace decode
