#include "decode/stacks.hh"

#include "decode/context.hh"
#include "decode/chart.hh"
#include "decode/hypothesis.hh"
#include "decode/phrase_table.hh"
#include "search/edge_generator.hh"
#include "util/murmur_hash.hh"

namespace decode {

namespace {

struct IntPairHash : public std::unary_function<const search::IntPair &, std::size_t> {
  std::size_t operator()(const search::IntPair &p) const {
    return util::MurmurHashNative(&p, sizeof(search::IntPair));
  }
};

class Vertices {
  public:
    void Add(const Hypothesis &hypothesis, uint32_t source_begin, uint32_t source_end, float score_delta) {
      search::HypoState add;
      add.history.cvp = &hypothesis;
      add.state.right = hypothesis.State();
      add.state.left.length = 0;
      add.state.left.full = true;
      add.score = hypothesis.Score() + score_delta;
      search::IntPair key;
      key.first = source_begin;
      key.second = source_end;
      map_[key].Root().AppendHypothesis(add);
    }

    void Apply(Chart &chart, search::EdgeGenerator &out) {
      for (Map::iterator i = map_.begin(); i != map_.end(); ++i) {
        search::PartialEdge edge(out.AllocateEdge(2));
        // Empty LM state before/between/after
        for (unsigned int j = 0; j < 3; ++j) {
          edge.Between()[j].left.length = 0;
          edge.Between()[j].left.full = false;
          edge.Between()[j].right.length = 0;
        }
        edge.SetScore(0.0);
        search::Note note;

        note.ints = i->first;
        edge.SetNote(note); // Record source range in the note for the edge.
        i->second.Root().FinishRoot(search::kPolicyRight);
        edge.NT()[0] = i->second.RootAlternate();
        edge.NT()[1] = chart.Range(i->first.first, i->first.second)->vertex.RootAlternate();
        out.AddEdge(edge);
      }
    }

  private:
    // TODO: dense as 2D array?
    // Key is start and end
    typedef boost::unordered_map<search::IntPair, search::Vertex, IntPairHash> Map;
    Map map_;
};

// TODO n-best lists.
class EdgeOutput {
  public:
    explicit EdgeOutput(Stack &stack)
      : stack_(stack) {}

    void NewHypothesis(search::PartialEdge complete) {
      const search::IntPair &source_range = complete.GetNote().ints;
      // The note for the first NT is the hypothesis.  The note for the second
      // NT is the target phrase.
      stack_.push_back(Hypothesis(complete.CompletedState().right,
            complete.GetScore(), // TODO: call scorer to adjust for last of lexro?
            *static_cast<const Hypothesis*>(complete.NT()[0].End().cvp),
            (std::size_t)source_range.first,
            (std::size_t)source_range.second,
            Phrase(complete.NT()[1].End().cvp)));
      // Note: stack_ has reserved for pop limit so pointers should survive.
      std::pair<Dedupe::iterator, bool> res(deduper_.insert(&stack_.back()));
      if (!res.second) {
        // Already present.  Keep the top-scoring one.
        Hypothesis &already = **res.first;
        if (already.Score() < stack_.back().Score()) {
          already = stack_.back();
        }
        stack_.resize(stack_.size() - 1);
      }
    }

    void FinishedSearch() {}

  private:
    struct RecombineHashPtr : public std::unary_function<const Hypothesis *, uint64_t> {
      uint64_t operator()(const Hypothesis *hyp) const {
        return RecombineHash()(*hyp);
      }
    };
    struct RecombineEqualPtr : public std::binary_function<const Hypothesis *, const Hypothesis *, bool> {
      bool operator()(const Hypothesis *first, const Hypothesis *second) const {
        return RecombineEqual()(*first, *second);
      }
    };

    typedef boost::unordered_set<Hypothesis *, RecombineHashPtr, RecombineEqualPtr> Dedupe;
    Dedupe deduper_;

    Stack &stack_;
};

} // namespace

Stacks::Stacks(Context &context, Chart &chart) {
  // Reservation is critical because pointers to Hypothesis objects are retained as history.
  stacks_.reserve(chart.SentenceLength() + 2 /* begin/end of sentence */);
  stacks_.resize(1);
  stacks_[0].push_back(Hypothesis(context.GetScorer().LanguageModel().BeginSentenceState()));
  // Decode with increasing numbers of source words.
  for (std::size_t source_words = 1; source_words <= chart.SentenceLength(); ++source_words) {
    Vertices vertices;
    // Iterate over stacks to continue from.
    for (std::size_t from = source_words - std::min(source_words, chart.MaxSourcePhraseLength());
         from < source_words;
         ++from) {
      const std::size_t phrase_length = source_words - from;
      // Iterate over antecedents in this stack.
      for (Stack::const_iterator ant = stacks_[from].begin(); ant != stacks_[from].end(); ++ant) {
        const Coverage &coverage = ant->GetCoverage();
        const std::size_t first_zero = coverage.FirstZero();
        std::size_t begin = first_zero;
        // We can always go from first_zero because it doesn't create a reordering gap.
        do {
          const TargetPhrases *phrases = chart.Range(begin, begin + phrase_length);
          if (!phrases || !coverage.Compatible(begin, begin + phrase_length)) continue;
          vertices.Add(*ant, begin, begin + phrase_length, context.GetScorer().Transition(*ant, *phrases, begin, begin + phrase_length));
        // Enforce the reordering limit on later iterations.
        } while (++begin + phrase_length <= first_zero + context.GetConfig().reordering_limit);
      }
    }
    search::EdgeGenerator gen;
    vertices.Apply(chart, gen);
    stacks_.resize(stacks_.size() + 1);
    stacks_.back().reserve(context.SearchContext().PopLimit());
    EdgeOutput output(stacks_.back());
    gen.Search(context.SearchContext(), output);
  }
  // TODO EOS
}

} // namespace decode
