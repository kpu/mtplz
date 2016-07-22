#include "decode/chart.hh"

#include "decode/system.hh"
#include "pt/statistics.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab, System &system)
  : max_source_phrase_length_(table.Stats().max_source_phrase_length),
    system_(system),
    feature_init_(system.GetObjective().GetFeatureInit()) {
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    const ID id = vocab.FindOrInsert(*word);
    sentence_ids_.push_back(id);
    sentence_.push_back(MapToVocabWord(*word, id));
  }
  LoadPhrases(table);
}

void Chart::LoadPhrases(const pt::Table &table) {
  // There's some unreachable ranges off the edge. Meh.
  entries_.resize(sentence_.size() * max_source_phrase_length_);
  for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
    for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
      search::Vertex &vertex = *vertex_pool_.construct();
      vertex.Root().InitRoot();
      auto phrases = table.Lookup(&sentence_ids_[begin], &*sentence_ids_.begin() + end);
      SourcePhrase source_phrase(sentence_, begin, end);
      for (pt::RowIterator phrase = phrases.begin(); phrase != phrases.end(); ++phrase) {
        AddTargetPhraseToVertex(&*phrase, source_phrase, vertex, false);
      }
      vertex.Root().FinishRoot(search::kPolicyLeft);
      SetRange(begin, end, &vertex);
    }
    if (!Range(begin, begin + 1)) {
      AddPassthrough(begin);
    }
  }
}

VocabWord *Chart::MapToVocabWord(const StringPiece word, const ID id) {
  std::size_t local_id = id - system_.VocabSize();
  if (id < system_.VocabSize()) {
    return system_.GetVocabMapping(id);
  } else {
    if (local_id >= oov_words_.size()) {
      oov_words_.resize(local_id + 1);
    }
    if (oov_words_[local_id] == nullptr) {
      VocabWord *new_word = reinterpret_cast<VocabWord*>(feature_init_.word_layout.Allocate<util::Pool>(oov_pool_));
      feature_init_.pt_id_field(new_word) = id;
      system_.GetObjective().NewWord(word, new_word);
      oov_words_[local_id] = new_word;
    }
    return oov_words_[local_id];
  }
}

void Chart::AddTargetPhraseToVertex(
    const pt::Row *phrase,
    const SourcePhrase &source_phrase,
    search::Vertex &vertex,
    bool passthrough) {
  TargetPhrase *phrase_wrapper = reinterpret_cast<TargetPhrase*>(
      feature_init_.target_phrase_layout.Allocate(target_phrase_pool_));
  feature_init_.pt_row_field(phrase_wrapper) = phrase;
  feature_init_.passthrough_field(phrase_wrapper) = passthrough;
  float score = system_.GetObjective().ScorePhrase(PhrasePair{source_phrase, phrase_wrapper}, nullptr);
  feature_init_.phrase_score_field(phrase_wrapper) = score;
  search::HypoState hypo;
  hypo.history.cvp = phrase_wrapper;
  hypo.score = score;
  vertex.Root().AppendHypothesis(hypo);
}

void Chart::AddPassthrough(std::size_t position) {
  TargetPhrases &pass = *vertex_pool_.construct();
  pass.Root().InitRoot();
  pt::Access access = feature_init_.phrase_access;
  pt::Row* pt_phrase = access.Allocate(oov_pool_);
  system_.GetObjective().InitPassthroughPhrase(pt_phrase);
  if (access.target) {
    access.target(pt_phrase, oov_pool_).push_back(sentence_ids_[position]);
  }
  AddTargetPhraseToVertex(pt_phrase, SourcePhrase(sentence_, position, position+1), pass, true);
  pass.Root().FinishRoot(search::kPolicyLeft);
  SetRange(position, position+1, &pass);
}

TargetPhrases &Chart::EndOfSentence() {
  search::Vertex &eos = *vertex_pool_.construct();
  search::HypoState eos_hypo;
  TargetPhrase *eos_phrase = reinterpret_cast<TargetPhrase*>(
      feature_init_.target_phrase_layout.Allocate(
        target_phrase_pool_));
  feature_init_.pt_id_field(eos_phrase) = 2; // </s> , see mutable vocab
  feature_init_.pt_row_field(eos_phrase) = nullptr;

  eos_hypo.history.cvp = eos_phrase;
  SourcePhrase source_phrase(sentence_, SentenceLength(), SentenceLength());
  eos_hypo.score = system_.GetObjective().ScorePhrase(PhrasePair{source_phrase,eos_phrase},nullptr);
  eos.Root().AppendHypothesis(eos_hypo);
  eos.Root().FinishRoot(search::kPolicyLeft);
  return eos;
}

} // namespace
