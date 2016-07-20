#include "decode/chart.hh"

#include "decode/system.hh"
#include "pt/statistics.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab, System &system)
  : max_source_phrase_length_(table.Stats().max_source_phrase_length), system_(system) {
  std::vector<ID> ids;
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    ID id = vocab.FindOrInsert(*word);
    ids.push_back(id);
    sentence_.push_back(MapToLocalWord(id));
  }
  // There's some unreachable ranges off the edge.  Meh.
  entries_.resize(sentence_.size() * max_source_phrase_length_);
  for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
    for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
      search::Vertex &vertex = *phrases_.construct();
      vertex.Root().InitRoot();
      auto phrases = table.Lookup(&ids[begin], &*ids.begin() + end);
      SourcePhrase source_phrase(sentence_, begin, end);
      for (pt::RowIterator phrase = phrases.begin(); phrase != phrases.end(); ++phrase) {
        AddTargetPhraseToVertex(&*phrase, source_phrase, vertex, false);
      }
      vertex.Root().FinishRoot(search::kPolicyLeft);
      SetRange(begin, end, &vertex);
    }
    if (!Range(begin, begin + 1)) {
      // Add passthrough for words not known to the phrase table.
      TargetPhrases &pass = *phrases_.construct();
      pass.Root().InitRoot();
      AddTargetPhraseToVertex(nullptr, SourcePhrase(sentence_, begin, begin+1), pass, true);
      pass.Root().FinishRoot(search::kPolicyLeft);
      SetRange(begin, begin + 1, &pass);
    }
  }
}

VocabWord *Chart::MapToLocalWord(const ID global_word) {
  if (global_word < system_.VocabSize()) {
    return system_.GetVocabMapping(global_word);
  } else { // word not in dict, pass source word to target, oov words
    // TODO
    return NULL;
  }
}

void Chart::AddTargetPhraseToVertex(
    const pt::Row *phrase,
    const SourcePhrase &source_phrase,
    search::Vertex &vertex,
    bool passthrough) {
  FeatureInit feature_init = system_.GetObjective().GetFeatureInit();
  TargetPhrase *phrase_wrapper = reinterpret_cast<TargetPhrase*>(
      feature_init.target_phrase_layout.Allocate(target_phrase_wrappers_));
  feature_init.pt_row_field(phrase_wrapper) = &*phrase;
  feature_init.passthrough_field(phrase_wrapper) = passthrough;
  float score = system_.GetObjective().ScorePhrase(PhrasePair{source_phrase, *phrase_wrapper}, NULL);
  feature_init.phrase_score_field(phrase_wrapper) = score;
  search::HypoState hypo;
  hypo.history.cvp = phrase_wrapper;
  hypo.score = score;
  vertex.Root().AppendHypothesis(hypo);
}

} // namespace
