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
    const Hypothesis *hypothesis, float score_delta, Hypothesis *next_hypothesis,
    search::Vertex &vertex, FeatureInit &feature_init) {
  search::HypoState add;
  add.history.cvp = next_hypothesis;
  add.state.right = feature_init.lm_state_field(hypothesis);
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

    void Add(const Hypothesis *hypothesis, uint32_t source_begin, uint32_t source_end,
        Hypothesis *next_hypothesis, float score_delta) {
      search::IntPair key;
      key.first = source_begin;
      key.second = source_end;
      AddHypothesisToVertex(hypothesis, score_delta, next_hypothesis, map_[key], feature_init_);
    }

    void Apply(Chart &chart, search::EdgeGenerator &out) {
      for (Map::iterator i = map_.begin(); i != map_.end(); ++i) {
        // Record source range in the note for the edge.
        search::Note note;
        note.ints = i->first;
        AddEdge(i->second, *chart.Range(note.ints.first, note.ints.second), note, out);
      }
    }

  private:
    FeatureInit &feature_init_;
    // TODO: dense as 2D array?
    // Key is start and end
    typedef boost::unordered_map<search::IntPair, search::Vertex, IntPairHash> Map;
    Map map_;
};

struct MergeInfo {
  Objective &objective;
  HypothesisBuilder &hypo_builder;
  const Chart& chart;
  float lm_weight;
};

template <class LMState>
class Recombinator : public std::hash<const Hypothesis*>, public std::equal_to<const Hypothesis*> {
  public:
    Recombinator(
        const util::PODField<LMState> lm_state_field,
        const Objective &objective)
      : lm_state_field_(lm_state_field), objective_(objective) {}

    size_t operator()(const Hypothesis *hypothesis) const {
      std::size_t source_index = hypothesis->SourceEndIndex();
      return util::MurmurHashNative(&source_index, sizeof(std::size_t),
          hash_value(lm_state_field_(hypothesis), hash_value(hypothesis->GetCoverage())));
    }

    bool operator()(const Hypothesis *first, const Hypothesis *second) const {
      if (! (lm_state_field_(first) == lm_state_field_(second))) return false;
      if (! (first->GetCoverage() == second->GetCoverage())) return false;
      if (! (first->SourceEndIndex() == second->SourceEndIndex())) return false;
      return objective_.HypothesisEqual(*first, *second);
    }

  private:
    const util::PODField<LMState> lm_state_field_;
    const Objective &objective_;
};

Hypothesis *GetHypothesis(search::PartialEdge complete) {
  return reinterpret_cast<Hypothesis*>(complete.GetData());
}

bool IsCompleteHypothesis(search::PartialEdge complete) {
  assert(complete.Valid());
  return complete.GetData() != NULL;
}

void UpdateHypothesisInEdge(search::PartialEdge complete, MergeInfo &merge_info) {
  assert(complete.Valid());
  const search::IntPair &source_range = complete.GetNote().ints;
  // The note for the first NT is the hypothesis.  The note for the second
  // NT is the target phrase.
  Hypothesis *sourcephrase_hypo = reinterpret_cast<Hypothesis*>(complete.NT()[0].End().cvp);
  const Hypothesis *prev_hypo = sourcephrase_hypo->Previous();
  TargetPhrase *target_phrase = reinterpret_cast<TargetPhrase*>(complete.NT()[1].End().cvp);
  SourcePhrase source_phrase(merge_info.chart.Sentence(), source_range.first, source_range.second);
  Hypothesis *next_hypo = merge_info.hypo_builder.CopyHypothesis(sourcephrase_hypo);
  PhrasePair phrase_pair(source_phrase, target_phrase);
  phrase_pair.vocab_map = &merge_info.chart.VocabMapping();

  float score = next_hypo->GetScore() + merge_info.objective.GetFeatureInit().phrase_score_field(target_phrase);
  // get language model score only
  float search_score = (complete.GetScore() - score) / merge_info.lm_weight;
  merge_info.objective.GetLanguageModelFeature()->SetSearchScore(next_hypo, search_score);
  score += merge_info.objective.ScoreHypothesisWithPhrasePair(
      *prev_hypo, phrase_pair, next_hypo, merge_info.hypo_builder.HypothesisPool());
  
  merge_info.hypo_builder.BuildHypothesis(
      next_hypo,
      complete.CompletedState().right,
      score,
      prev_hypo,
      (std::size_t)source_range.first,
      (std::size_t)source_range.second,
      target_phrase);
  complete.SetData(next_hypo);
  complete.SetScore(score);
}

// TODO n-best lists.
class EdgeOutput {
  public:
    typedef boost::unordered_set<Hypothesis *, Recombinator<LMState>, Recombinator<LMState>> Dedupe;

    EdgeOutput(Stack &stack, MergeInfo merge_info, Dedupe deduper, search::EdgeGenerator &gen)
      : stack_(stack), merge_info_(merge_info), deduper_(deduper), queue_(gen) {}

    bool NewHypothesis(search::PartialEdge complete) {
      if (!IsCompleteHypothesis(complete)) {
        UpdateHypothesisInEdge(complete, merge_info_);
        queue_.AddEdge(complete);
        return false;
      }
      stack_.push_back(GetHypothesis(complete));
      // Note: stack_ has reserved for pop limit so pointers should survive.
      std::pair<Dedupe::iterator, bool> res(deduper_.insert(stack_.back()));
      if (!res.second) {
        // Already present.  Keep the top-scoring one.
        Hypothesis *already = *res.first;
        if (already->GetScore() < stack_.back()->GetScore()) {
          already->SetScore(stack_.back()->GetScore());
          for (Hypothesis *h : stack_) {
            if (h == already) {
              h = stack_.back();
            }
          }
        }
        stack_.resize(stack_.size() - 1);
      }
      return true;
    }

    void FinishedSearch() {}

  private:
    Dedupe deduper_;

    Stack &stack_;

    search::EdgeGenerator &queue_;

    MergeInfo merge_info_;
};

// Pick only the best hypothesis for end of sentence.
class PickBest {
  public:
    PickBest(Stack &stack, MergeInfo merge_info, search::EdgeGenerator &gen) :
      stack_(stack), merge_info_(merge_info), queue_(gen) {
      stack_.clear();
      stack_.reserve(1);
    }

    bool NewHypothesis(search::PartialEdge complete) {
      if (!IsCompleteHypothesis(complete)) {
        UpdateHypothesisInEdge(complete, merge_info_);
        queue_.AddEdge(complete);
        return false;
      }
      Hypothesis *new_hypo = GetHypothesis(complete);
      new_hypo->SetScore(new_hypo->GetScore() + merge_info_.objective.ScoreFinalHypothesis(*new_hypo));
      if (best_ == NULL || new_hypo->GetScore() > best_->GetScore()) {
        best_ = new_hypo;
      }
      return true;
    }

    void FinishedSearch() {
      if (best_ != NULL)
        stack_.push_back(best_);
    }

  private:
    Stack &stack_;
    search::EdgeGenerator &queue_;
    MergeInfo merge_info_;
    Hypothesis *best_ = NULL;
};

} // namespace

Stacks::Stacks(System &system, Chart &chart) :
  hypothesis_builder_(hypothesis_pool_, system.GetObjective().GetFeatureInit()) {
  FeatureInit &feature_init = system.GetObjective().GetFeatureInit();
  Future future(chart);
  // Reservation is critical because pointers to Hypothesis objects are retained as history.
  stacks_.reserve(chart.SentenceLength() + 2 /* begin/end of sentence */);
  stacks_.resize(1);
  // Initialize root hypothesis with <s> context and future cost for everything.
  pt::Access access = feature_init.phrase_access;
  pt::Row *target = access.Allocate(hypothesis_pool_);
  system.GetObjective().InitPassthroughPhrase(target, TargetPhraseType::Begin);
  stacks_[0].push_back(hypothesis_builder_.BuildHypothesis(
        system.GetObjective().BeginSentenceState(),
        future.Full(), target));
  // Decode with increasing numbers of source words.
  for (std::size_t source_words = 1; source_words <= chart.SentenceLength(); ++source_words) {
    Vertices vertices(feature_init);
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
          const Hypothesis *ant_hypo = *ant;
          Hypothesis *next_hypo = hypothesis_builder_.NextHypothesis(ant_hypo);
          float score_delta = system.GetObjective().ScoreHypothesisWithSourcePhrase(
              *ant_hypo, SourcePhrase(chart.Sentence(), begin, begin + phrase_length), next_hypo);
          // Future costs: remove span to be filled.
          score_delta += future.Change(coverage, begin, begin + phrase_length);
          next_hypo->SetScore(ant_hypo->GetScore() + score_delta);
          vertices.Add(*ant, begin, begin + phrase_length, next_hypo, score_delta);
        // Enforce the reordering limit on later iterations.
        } while (++begin <= last_begin);
      }
    }
    search::EdgeGenerator gen;
    vertices.Apply(chart, gen);
    stacks_.resize(stacks_.size() + 1);
    stacks_.back().reserve(system.SearchContext().PopLimit());
    Recombinator<LMState> recombinator(feature_init.lm_state_field, system.GetObjective());
    EdgeOutput::Dedupe deduper(system.SearchContext().PopLimit(), recombinator, recombinator);
    MergeInfo merge_info{system.GetObjective(), hypothesis_builder_, chart, system.SearchContext().LMWeight()};
    EdgeOutput output(stacks_.back(), merge_info, deduper, gen);
    gen.Search(system.SearchContext(), output);
  }
  PopulateLastStack(system, chart);
}

void Stacks::PopulateLastStack(System &system, Chart &chart) {
  // First, make Vertex of all hypotheses
  search::Vertex all_hyps;
  for (Stack::const_iterator ant = stacks_[chart.SentenceLength()].begin(); ant != stacks_[chart.SentenceLength()].end(); ++ant) {
    assert(chart.SentenceLength() == (*ant)->GetCoverage().FirstZero());
    const Hypothesis *ant_hypo = *ant;
    Hypothesis *next_hypo = hypothesis_builder_.NextHypothesis(ant_hypo);
    SourcePhrase source_phrase(chart.Sentence(), chart.SentenceLength(), chart.SentenceLength());
    float score_delta = system.GetObjective().ScoreHypothesisWithSourcePhrase(
        *ant_hypo, source_phrase, next_hypo);
    next_hypo->SetScore(ant_hypo->GetScore() + score_delta);
    AddHypothesisToVertex(ant_hypo, score_delta, next_hypo, all_hyps, system.GetObjective().GetFeatureInit());
  }
  
  // Next, make Vertex which consists of a single EOS phrase.
  // The seach algorithm will attempt to find the best hypotheses in the "cross product" of these two sets.
  search::Vertex &eos_vertex = chart.EndOfSentence();
  // Add edge that tacks </s> on
  search::EdgeGenerator gen;
  search::Note note;
  note.ints.first = chart.SentenceLength();
  note.ints.second = chart.SentenceLength();
  AddEdge(all_hyps, eos_vertex, note, gen);

  stacks_.resize(stacks_.size() + 1);
  MergeInfo merge_info{system.GetObjective(), hypothesis_builder_, chart,system.SearchContext().LMWeight()};
  PickBest output(stacks_.back(), merge_info, gen);
  gen.Search(system.SearchContext(), output);

  end_ = stacks_.back().empty() ? NULL : stacks_.back()[0];
}

} // namespace decode
