#include "decode/chart.hh"

#include "decode/system.hh"
#include "decode/lm.hh"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(std::size_t max_source_phrase_length, Objective &objective)
  : max_source_phrase_length_(max_source_phrase_length),
    objective_(objective),
    feature_init_(objective.GetFeatureInit()),
    eos_phrase_(reinterpret_cast<pt::Row*>(objective.GetFeatureInit().target_phrase_layout.Allocate(oov_pool_))) {
  UTIL_THROW_IF(objective.GetLanguageModelFeature() == nullptr, util::Exception,
      "Missing language model for objective!");
  if (feature_init_.phrase_access.target) {
    pt::Access access = feature_init_.phrase_access;
    access.target(eos_phrase_, oov_pool_).resize(1);
    access.target(eos_phrase_)[0] = EOS_WORD;
  }
  objective_.InitPassthroughPhrase(eos_phrase_);
}

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
  search::HypoState hypo;
  // Bypass objective to allow the language model access to a hypo.state reference.
  objective_.GetLanguageModelFeature()->ScoreTargetPhrase(phrase_wrapper, hypo.state);
  PhrasePair phrase_pair(source_phrase, phrase_wrapper);
  phrase_pair.target_phrase_pool = &target_phrase_pool_;
  float score = objective_.ScorePhrase(phrase_pair, nullptr);
  feature_init_.phrase_score_field(phrase_wrapper) = score;
  hypo.history.cvp = phrase_wrapper;
  hypo.score = score;
  vertex.Root().AppendHypothesis(hypo);
}

void Chart::AddPassthrough(std::size_t position) {
  SourcePhrase source_phrase(sentence_, position, position+1);
  TargetPhrases *pass = vertex_pool_.construct();
  pass->Root().InitRoot();
  pt::Access access = feature_init_.phrase_access;
  pt::Row* pt_phrase = access.Allocate(oov_pool_);
  if (access.target) {
    access.target(pt_phrase, oov_pool_).resize(1);
    access.target(pt_phrase)[0] = sentence_ids_[position];
  }
  objective_.InitPassthroughPhrase(pt_phrase);
  AddTargetPhraseToVertex(pt_phrase, source_phrase, *pass, true);
  pass->Root().FinishRoot(search::kPolicyLeft);
  SetRange(position, position+1, pass);
}

TargetPhrases &Chart::EndOfSentence() {
  search::Vertex &eos = *vertex_pool_.construct();
  eos.Root().InitRoot();
  SourcePhrase source_phrase(sentence_, SentenceLength(), SentenceLength());
  AddTargetPhraseToVertex(eos_phrase_, source_phrase, eos, false);
  eos.Root().FinishRoot(search::kPolicyLeft);
  return eos;
}

} // namespace
