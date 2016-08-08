#include "decode/chart.hh"

#include "decode/system.hh"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(std::size_t max_source_phrase_length,
    VocabMap &vocab_map,
    Objective &objective,
    VertexCache &cache)
    : max_source_phrase_length_(max_source_phrase_length),
      objective_(objective),
      feature_init_(objective.GetFeatureInit()),
      vocab_map_(vocab_map),
      cache_(cache) {
  UTIL_THROW_IF(objective.GetLanguageModelFeature() == nullptr, util::Exception,
      "Missing language model for objective!");
  pt::Access access = feature_init_.phrase_access;
  eos_phrase_ = access.Allocate(passthrough_pool_);
  if (feature_init_.phrase_access.target) {
    access.target(eos_phrase_, passthrough_pool_).resize(1);
    access.target(eos_phrase_)[0] = EOS_WORD;
  }
  objective_.InitPassthroughPhrase(eos_phrase_);
}

void Chart::ReadSentence(StringPiece input) {
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    ID id; // set by VocabMap
    sentence_.push_back(vocab_map_.FindOrInsert(*word, id));
    sentence_ids_.push_back(id);
  }
}

void Chart::AddTargetPhraseToVertex(
    const pt::Row *phrase,
    search::Vertex &vertex,
    bool passthrough) {
  TargetPhrase *phrase_wrapper = reinterpret_cast<TargetPhrase*>(
      feature_init_.target_phrase_layout.Allocate(cache_.target_phrase_pool));
  feature_init_.pt_row_field(phrase_wrapper) = phrase;
  feature_init_.passthrough_field(phrase_wrapper) = passthrough;
  search::HypoState hypo;
  TargetPhraseInfo target{phrase_wrapper, vocab_map_, cache_.target_phrase_pool};
  // Bypass objective to allow the language model access to a hypo.state reference.
  objective_.GetLanguageModelFeature()->InitTargetPhrase(target, hypo.state);
  float score = objective_.ScoreTargetPhrase(target);
  feature_init_.phrase_score_field(phrase_wrapper) = score;
  hypo.score = score;
  hypo.history.cvp = phrase_wrapper;
  vertex.Root().AppendHypothesis(hypo);
}

void Chart::AddPassthrough(std::size_t position) {
  TargetPhrases *pass = vertex_pool_.construct();
  pass->Root().InitRoot();
  pt::Access access = feature_init_.phrase_access;
  pt::Row* pt_phrase = access.Allocate(passthrough_pool_);
  if (access.target) {
    access.target(pt_phrase, passthrough_pool_).resize(1);
    access.target(pt_phrase)[0] = sentence_ids_[position];
  }
  objective_.InitPassthroughPhrase(pt_phrase);
  AddTargetPhraseToVertex(pt_phrase, *pass, true);
  pass->Root().FinishRoot(search::kPolicyLeft);
  SetRange(position, position+1, pass);
}

TargetPhrases &Chart::EndOfSentence() {
  search::Vertex &eos = *vertex_pool_.construct();
  eos.Root().InitRoot();
  AddTargetPhraseToVertex(eos_phrase_, eos, false);
  eos.Root().FinishRoot(search::kPolicyLeft);
  return eos;
}

} // namespace
