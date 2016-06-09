#include "decode/chart.hh"

#include "util/file_piece.hh"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(const pt::Table &table, StringPiece input, util::MutableVocab &vocab) 
  : max_source_phrase_length_(/*TODO table.MaxSourcePhraseLength()*/ 16) {
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    // TODO thread-local ids?
    words_.push_back(vocab.FindOrInsert(*word));
  }
  // There's some unreachable ranges off the edge.  Meh.
  entries_.resize(words_.size() * max_source_phrase_length_);
  for (std::size_t begin = 0; begin != words_.size(); ++begin) {
    for (std::size_t end = begin + 1; (end != words_.size() + 1) && (end <= begin + max_source_phrase_length_); ++end) {
      /* TODO: Lookup? SetRange(begin, end, table.Phrases(&words[begin], &*words.begin() + end)); */
    }
    if (!Range(begin, begin + 1)) {
      // Add passthrough for words not known to the phrase table.
      TargetPhrases *pass = passthrough_.construct();
      // TODO: replace following by something in the new phrase table!
      // pass->MakePassthrough(passthrough_phrases_, scorer, words[begin]);
      SetRange(begin, begin + 1, pass);
    }
  }
}

} // namespace
