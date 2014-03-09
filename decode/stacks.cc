#include "decode/stacks.hh"

#include "decode/context.hh"
#include "decode/chart.hh"
#include "decode/hypothesis.hh"
#include "decode/phrase_table.hh"
#include "search/edge_generator.hh"
#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"

#include <iostream>

namespace decode {

namespace {

// Add a hypothesis from this decoder to the search algorithm's vertex.
// The input is an antecedent hypothesis, and a score delta for any non-LM, non-phrase-internal features.
// The final parameter is an output param which will get populated.
// This function is highly specific to this particular phrasal search strategy -- e.g., it assumes
// that we are always appending to a hypothesis on the right side.
void AddHypothesisToVertex(const Hypothesis &hypothesis, float score_delta, search::Vertex &vertex) {
  search::HypoState add;
  add.history.cvp = &hypothesis;
  add.state.right = hypothesis.State();
  add.state.left.length = 0;
  add.state.left.full = true;
  add.score = hypothesis.Score() + score_delta;
  vertex.Root().AppendHypothesis(add);
}

void AddEdge(search::Vertex &hypos, search::Vertex &extensions, search::Note note, search::EdgeGenerator &out) {
  hypos.Root().FinishRoot(search::kPolicyRight);
  if (hypos.Empty()) return;
  search::PartialEdge edge(out.AllocateEdge(2));
  // Empty LM state before/between/after
  for (unsigned int j = 0; j < 3; ++j) {
    edge.Between()[j].left.length = 0;
    edge.Between()[j].left.full = false;
    edge.Between()[j].right.length = 0;
  }
  edge.SetNote(note);
  // Include top scores.
  edge.SetScore(hypos.Bound() + extensions.Bound());
  edge.NT()[0] = hypos.RootAlternate();
  edge.NT()[1] = extensions.RootAlternate();
  out.AddEdge(edge);
}

struct IntPairHash : public std::unary_function<const search::IntPair &, std::size_t> {
  std::size_t operator()(const search::IntPair &p) const {
    return util::MurmurHashNative(&p, sizeof(search::IntPair));
  }
};

class Vertices {
  public:
    void Add(const Hypothesis &hypothesis, uint32_t source_begin, uint32_t source_end, float score_delta) {
      search::IntPair key;
      key.first = source_begin;
      key.second = source_end;
      AddHypothesisToVertex(hypothesis, score_delta, map_[key]);
    }

    void Apply(Chart &chart, search::EdgeGenerator &out) {
      for (Map::iterator i = map_.begin(); i != map_.end(); ++i) {
        // Record source range in the note for the edge.
        search::Note note;
        note.ints = i->first;
        AddEdge(i->second, chart.Range(i->first.first, i->first.second)->vertex, note, out);
      }
    }

  private:
    // TODO: dense as 2D array?
    // Key is start and end
    typedef boost::unordered_map<search::IntPair, search::Vertex, IntPairHash> Map;
    Map map_;
};

void AppendToStack(search::PartialEdge complete, Stack &out) {
  const search::IntPair &source_range = complete.GetNote().ints;
  // The note for the first NT is the hypothesis.  The note for the second
  // NT is the target phrase.
  out.push_back(Hypothesis(complete.CompletedState().right,
        complete.GetScore(), // TODO: call scorer to adjust for last of lexro?
        *static_cast<const Hypothesis*>(complete.NT()[0].End().cvp),
        (std::size_t)source_range.first,
        (std::size_t)source_range.second,
        Phrase(complete.NT()[1].End().cvp)));
}

// TODO n-best lists.
class EdgeOutput {
  public:
    explicit EdgeOutput(Stack &stack)
      : stack_(stack) {}

    void NewHypothesis(search::PartialEdge complete) {
      AppendToStack(complete, stack_);
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

// Pick only the best hypothesis for end of sentence.
class PickBest {
  public:
    explicit PickBest(Stack &stack) : stack_(stack) {
      stack_.clear();
      stack_.reserve(1);
    }

    void NewHypothesis(search::PartialEdge complete) {
      if (!best_.Valid() || complete > best_) {
        best_ = complete;
      }
    }

    void FinishedSearch() {
      AppendToStack(best_, stack_);
    }

  private:
    Stack &stack_;
    search::PartialEdge best_;
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
        std::size_t begin = coverage.FirstZero();
        const std::size_t last_begin = std::min(coverage.FirstZero() + context.GetConfig().reordering_limit, chart.SentenceLength()) - phrase_length;
        // We can always go from first_zero because it doesn't create a reordering gap.
        do {
          const TargetPhrases *phrases = chart.Range(begin, begin + phrase_length);
          if (!phrases || !coverage.Compatible(begin, begin + phrase_length)) continue;
          vertices.Add(*ant, begin, begin + phrase_length, context.GetScorer().Transition(*ant, *phrases, begin, begin + phrase_length));
        // Enforce the reordering limit on later iterations.
        } while (++begin <= last_begin);
      }
    }
    search::EdgeGenerator gen;
    vertices.Apply(chart, gen);
    stacks_.resize(stacks_.size() + 1);
    stacks_.back().reserve(context.SearchContext().PopLimit());
    EdgeOutput output(stacks_.back());
    gen.Search(context.SearchContext(), output);
  }

  PopulateLastStack(context, chart);
}

void Stacks::PopulateLastStack(Context &context, Chart &chart) {
  // First, make Vertex of all hypotheses
  search::Vertex all_hyps;
  for (Stack::const_iterator ant = stacks_[chart.SentenceLength()].begin(); ant != stacks_[chart.SentenceLength()].end(); ++ant) {
    // TODO: the zero in the following line assumes that EOS is not scored for distortion. 
    // This assumption might need to be revisited.
    AddHypothesisToVertex(*ant, 0, all_hyps);
  }
  
  // Next, make Vertex which consists of a single EOS phrase.
  // The seach algorithm will attempt to find the best hypotheses in the "cross product" of these two sets.
  // TODO: Maybe this should belong to the phrase table.  It's constant.
  search::Vertex eos_vertex;
  search::HypoState eos_hypo;
  Phrase eos_phrase(eos_phrase_pool_, context.GetVocab(), "</s>");

  eos_hypo.history.cvp = eos_phrase.Base();
  eos_hypo.score = context.GetScorer().LM(eos_phrase.begin(), eos_phrase.end(), eos_hypo.state);
  eos_vertex.Root().AppendHypothesis(eos_hypo);

  // Add edge that tacks </s> on
  search::EdgeGenerator gen;
  search::Note note;
  note.ints.first = chart.SentenceLength();
  note.ints.second = chart.SentenceLength();
  AddEdge(all_hyps, eos_vertex, note, gen);

  stacks_.resize(stacks_.size() + 1);
  PickBest output(stacks_.back());
  gen.Search(context.SearchContext(), output);

  end_ = stacks_.back().empty() ? NULL : &stacks_.back()[0];
}

} // namespace decode
