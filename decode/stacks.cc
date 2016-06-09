#include "decode/stacks.hh"

#include "decode/chart.hh"
#include "decode/future.hh"
#include "decode/hypothesis.hh"
#include "search/edge_generator.hh"
#include "util/murmur_hash.hh"
#include "util/mutable_vocab.hh"

#include <iostream>
#include <boost/unordered_map.hpp>

namespace decode {

namespace {

// Add a hypothesis from this decoder to the search algorithm's vertex.
// The input is an antecedent hypothesis, and a score delta for any non-LM, non-phrase-internal features.
// The final parameter is an output param which will get populated.
// This function is highly specific to this particular phrasal search strategy -- e.g., it assumes
// that we are always appending to a hypothesis on the right side.
void AddHypothesisToVertex(
    const Hypothesis *hypothesis, float score_delta,
    search::Vertex &vertex, FeatureInit &feature_init) {
  search::HypoState add;
  add.history.cvp = hypothesis;
  add.state.right = feature_init.LMStateField()(hypothesis);
  add.state.left.length = 0;
  add.state.left.full = true;
  add.score = hypothesis->GetScore() + score_delta;
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
    explicit Vertices(FeatureInit &feature_init) : feature_init_(feature_init) {}

    void Add(const Hypothesis *hypothesis, uint32_t source_begin, uint32_t source_end, float score_delta) {
      search::IntPair key;
      key.first = source_begin;
      key.second = source_end;
      AddHypothesisToVertex(hypothesis, score_delta, map_[key], feature_init_);
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
    FeatureInit &feature_init_;
    // TODO: dense as 2D array?
    // Key is start and end
    typedef boost::unordered_map<search::IntPair, search::Vertex, IntPairHash> Map;
    Map map_;
};

Hypothesis *HypothesisFromEdge(search::PartialEdge complete, HypothesisBuilder &hypo_builder) {
  assert(complete.Valid());
  const search::IntPair &source_range = complete.GetNote().ints;
  // The note for the first NT is the hypothesis.  The note for the second
  // NT is the target phrase.
  return hypo_builder.BuildHypothesis(
      complete.CompletedState().right,
      complete.GetScore(),
      reinterpret_cast<const Hypothesis*>(complete.NT()[0].End().cvp),
      (std::size_t)source_range.first,
      (std::size_t)source_range.second,
      Phrase(complete.NT()[1].End().cvp));
}

// TODO n-best lists.
class EdgeOutput {
  public:
    EdgeOutput(Stack &stack, HypothesisBuilder &hypo_builder, const std::vector<ID> &sentence)
      : stack_(stack), hypothesis_builder_(hypo_builder), sentence_(sentence) {}

    void NewHypothesis(search::PartialEdge complete) {
      // TODO score hypo + PhrasePair{SourcePhrase(sentence_, from, to), targetphrase}
      stack_.push_back(HypothesisFromEdge(complete, hypothesis_builder_));
      // Note: stack_ has reserved for pop limit so pointers should survive.
      std::pair<Dedupe::iterator, bool> res(deduper_.insert(stack_.back()));
      if (!res.second) {
        // Already present.  Keep the top-scoring one.
        Hypothesis *already = *res.first;
        if (already->GetScore() < stack_.back()->GetScore()) {
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

    HypothesisBuilder &hypothesis_builder_;

    const std::vector<ID> &sentence_;
};

// Pick only the best hypothesis for end of sentence.
class PickBest {
  public:
    PickBest(Stack &stack, Objective &objective, HypothesisBuilder &hypo_builder) :
      stack_(stack), objective_(objective), hypothesis_builder_(hypo_builder) {
      stack_.clear();
      stack_.reserve(1);
    }

    void NewHypothesis(search::PartialEdge complete) {
      Hypothesis *new_hypo = HypothesisFromEdge(complete, hypothesis_builder_);
      new_hypo->SetScore(objective_.RescoreHypothesis(*new_hypo, NULL));
      if (best_ == NULL || new_hypo->GetScore() > best_->GetScore()) {
        best_ = new_hypo;
      }
    }

    void FinishedSearch() {
      if (best_ != NULL)
        stack_.push_back(best_);
    }

  private:
    Stack &stack_;
    const Objective &objective_;
    HypothesisBuilder &hypothesis_builder_;
    Hypothesis *best_;
};

} // namespace

Stacks::Stacks(System &system, Chart &chart) :
  hypothesis_builder_(hypothesis_pool_, system.GetObjective().GetFeatureInit()) {
  Future future(chart);
  // Reservation is critical because pointers to Hypothesis objects are retained as history.
  stacks_.reserve(chart.SentenceLength() + 2 /* begin/end of sentence */);
  stacks_.resize(1);
  // Initialize root hypothesis with <s> context and future cost for everything.
  stacks_[0].push_back(hypothesis_builder_.BuildHypothesis(
        system.GetObjective().BeginSentenceState(),
        future.Full()));
  // Decode with increasing numbers of source words.
  for (std::size_t source_words = 1; source_words <= chart.SentenceLength(); ++source_words) {
    Vertices vertices(system.GetObjective().GetFeatureInit());
    // Iterate over stacks to continue from.
    for (std::size_t from = source_words - std::min(source_words, chart.MaxSourcePhraseLength());
         from < source_words;
         ++from) {
      const std::size_t phrase_length = source_words - from;
      // Iterate over antecedents in this stack.
      for (Stack::const_iterator ant = stacks_[from].begin(); ant != stacks_[from].end(); ++ant) {
        const Coverage &coverage = (*ant)->GetCoverage();
        std::size_t begin = coverage.FirstZero();
        const std::size_t last_end = std::min(coverage.FirstZero() + system.GetConfig().reordering_limit, chart.SentenceLength());
        const std::size_t last_begin = (last_end > phrase_length) ? (last_end - phrase_length) : 0;
        // We can always go from first_zero because it doesn't create a reordering gap.
        do {
          const TargetPhrases *phrases = chart.Range(begin, begin + phrase_length);
          if (!phrases || !coverage.Compatible(begin, begin + phrase_length)) continue;
          // distortion etc.
          const Hypothesis *ant_hypo = *ant;
          float score_delta = system.GetObjective().ScoreHypothesisWithSourcePhrase(
              *ant_hypo, SourcePhrase(chart.Sentence(), begin, begin + phrase_length), NULL);
          // Future costs: remove span to be filled.
          score_delta += future.Change(coverage, begin, begin + phrase_length);
          vertices.Add(*ant, begin, begin + phrase_length, score_delta);
        // Enforce the reordering limit on later iterations.
        } while (++begin <= last_begin);
      }
    }
    search::EdgeGenerator gen;
    vertices.Apply(chart, gen);
    stacks_.resize(stacks_.size() + 1);
    stacks_.back().reserve(system.SearchContext().PopLimit());
    EdgeOutput output(stacks_.back(), hypothesis_builder_, chart.Sentence());
    gen.Search(system.SearchContext(), output);
  }
  PopulateLastStack(system, chart);
}

void Stacks::PopulateLastStack(System &system, Chart &chart) {
  // First, make Vertex of all hypotheses
  search::Vertex all_hyps;
  for (Stack::const_iterator ant = stacks_[chart.SentenceLength()].begin(); ant != stacks_[chart.SentenceLength()].end(); ++ant) {
    assert(chart.SentenceLength() == (*ant)->GetCoverage().FirstZero());
    // TODO: the zero in the following line assumes that EOS is not scored for distortion. 
    // This assumption might need to be revisited.
    AddHypothesisToVertex(*ant, 0, all_hyps, system.GetObjective().GetFeatureInit());
  }
  
  // Next, make Vertex which consists of a single EOS phrase.
  // The seach algorithm will attempt to find the best hypotheses in the "cross product" of these two sets.
  // TODO: Maybe this should belong to the phrase table.  It's constant.
  // TODO: rewrite with new table
  /* search::Vertex eos_vertex; */
  /* search::HypoState eos_hypo; */
  /* Phrase eos_phrase(eos_phrase_pool_, context.GetVocab(), "</s>"); */

  /* eos_hypo.history.cvp = eos_phrase.Base(); */
  // TODO !!!!!!!!!
  /* eos_hypo.score = context.GetScorer().LM(eos_phrase.begin(), eos_phrase.end(), eos_hypo.state); */
  /* eos_hypo.score = context.GetObjective().ScoreHypothesisWithPhrasePair( */
  /*     PhrasePair{eos_phrase.begin(), eos_phrase.end(), NULL}, S */
  /*     ); */
  /* eos_vertex.Root().AppendHypothesis(eos_hypo); */
  /* eos_vertex.Root().FinishRoot(search::kPolicyLeft); */

  /* // Add edge that tacks </s> on */
  /* search::EdgeGenerator gen; */
  /* search::Note note; */
  /* note.ints.first = chart.SentenceLength(); */
  /* note.ints.second = chart.SentenceLength(); */
  /* AddEdge(all_hyps, eos_vertex, note, gen); */

  /* stacks_.resize(stacks_.size() + 1); */
  /* PickBest output(stacks_.back(), system.GetObjective(), hypothesis_builder_); */
  /* gen.Search(system.SearchContext(), output); */

  end_ = stacks_.back().empty() ? NULL : stacks_.back()[0];
}

} // namespace decode
