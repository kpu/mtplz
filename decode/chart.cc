#include "decode/chart.hh"

#include "decode/system.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(std::size_t max_source_phrase_length, Objective &objective)
  : max_source_phrase_length_(max_source_phrase_length),
    objective_(objective),
    feature_init_(objective.GetFeatureInit()) {}

void Chart::ReadSentence(StringPiece input, util::MutableVocab &vocab, const std::vector<VocabWord*> &vocab_mapping) {
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    const ID id = vocab.FindOrInsert(*word);
    sentence_ids_.push_back(id);
    sentence_.push_back(MapToVocabWord(*word, id, vocab_mapping));
  }
}

VocabWord *Chart::MapToVocabWord(const StringPiece word, const ID id, const std::vector<VocabWord*> &vocab_mapping) {
  std::size_t local_id = id - vocab_mapping.size();
  if (id < vocab_mapping.size()) {
    return vocab_mapping[id];
  } else {
    if (local_id >= oov_words_.size()) {
      oov_words_.resize(local_id + 1);
    }
    if (oov_words_[local_id] == nullptr) {
      VocabWord *new_word = reinterpret_cast<VocabWord*>(feature_init_.word_layout.Allocate<util::Pool>(oov_pool_));
      feature_init_.pt_id_field(new_word) = id;
      objective_.NewWord(word, new_word);
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
  float score = objective_.ScorePhrase(PhrasePair{source_phrase, phrase_wrapper}, nullptr);
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
  objective_.InitPassthroughPhrase(pt_phrase);
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
  feature_init_.pt_id_field(eos_phrase) = EOS_WORD;
  feature_init_.pt_row_field(eos_phrase) = nullptr;

  eos_hypo.history.cvp = eos_phrase;
  SourcePhrase source_phrase(sentence_, SentenceLength(), SentenceLength());
  eos_hypo.score = objective_.ScorePhrase(PhrasePair{source_phrase,eos_phrase},nullptr);
  eos.Root().AppendHypothesis(eos_hypo);
  eos.Root().FinishRoot(search::kPolicyLeft);
  return eos;
}

} // namespace
