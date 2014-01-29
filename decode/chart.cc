#include "decode/chart.hh"

#include "decode/phrase_table.hh"
#include "util/file_piece.hh"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

namespace decode {

Chart::Chart(const PhraseTable &table, StringPiece input, util::MutableVocab &vocab, Scorer &scorer) 
  : max_source_phrase_length_(table.MaxSourcePhraseLength()) {
  std::vector<ID> words;
  for (util::TokenIter<util::BoolCharacter, true> word(input, util::kSpaces); word; ++word) {
    // TODO thread-local ids?
    words.push_back(vocab.FindOrInsert(*word));
  }
  sentence_length_ = words.size();
  // There's some unreachable ranges off the edge.  Meh.
  entries_.resize(sentence_length_ * max_source_phrase_length_);
  for (std::vector<ID>::const_iterator begin = words.begin(); begin != words.end(); ++begin) {
    for (std::vector<ID>::const_iterator end = begin + 1; (end != words.end()) && (end <= begin + max_source_phrase_length_); ++end) {
      SetRange(begin - words.begin(), end - begin, table.Phrases(&*begin, &*end));
    }
    if (!Range(begin - words.begin(), 1)) {
      // Add passthrough for words not known to the phrase table.
      TargetPhrases *pass = passthrough_.construct();
      pass->MakePassthrough(passthrough_phrases_, scorer, *begin);
      SetRange(begin - words.begin(), 1, pass);
    }
  }
}

} // namespace
