#include "decode/chart.hh"

#include "decode/system.hh"
#include "pt/statistics.hh"
#include "util/file_piece.hh"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab, System &system)
  : max_source_phrase_length_(table.Stats().max_source_phrase_length), system_(system) {
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    sentence_.push_back(MapToLocalWord(vocab.FindOrInsert(*word)));
  }
  // There's some unreachable ranges off the edge.  Meh.
  entries_.resize(sentence_.size() * max_source_phrase_length_);
  for (std::size_t begin = 0; begin != sentence_.size(); ++begin) {
    for (std::size_t end = begin + 1; (end != sentence_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
      /* SetRange(begin, end, table.Lookup(&sentence_[begin], &*sentence_.begin() + end)); */
      // TODO iterate through the table results and score in isolation
      // PhrasePair: add featurefunction state layout
    }
    // TODO check for passthrough, if there is one add a passthrough flag
    // (would this be a dense feature?)
    if (!Range(begin, begin + 1)) {
      // Add passthrough for words not known to the phrase table.
      TargetPhrases *pass = passthrough_.construct();
      // TODO: replace following by something in the new phrase table!
      // pass->MakePassthrough(passthrough_phrases_, scorer, words[begin]);
      SetRange(begin, begin + 1, pass);
    }
  }
}

VocabWord *Chart::MapToLocalWord(const ID global_word) {
  if (global_word < system_.GetVocabMapping().size()) {
    return system_.GetVocabMapping()[global_word];
  } else { // word not in dict, pass source word to target
    // TODO
    return NULL;
  }
}

} // namespace
