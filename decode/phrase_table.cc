#include "decode/phrase_table.hh"

#include "decode/phrase.hh"
#include "decode/scorer.hh"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/mutable_vocab.hh"
#include "util/tokenize_piece.hh"

using namespace util;

namespace decode {

void TargetPhrases::MakePassthrough(util::Pool &pool, Scorer &scorer, ID word) {
  Phrase target(pool, word);
  search::HypoState hypo;
  hypo.history.cvp = target.Base();
  hypo.score = 
		 scorer.Passthrough() 
		 + scorer.LM(&word, &word + 1, hypo.state)
		 + scorer.TargetWordCount(1);
  vertex.Root().InitRoot();
  vertex.Root().AppendHypothesis(hypo);
  vertex.Root().FinishRoot(search::kPolicyLeft);
}

PhraseTable::PhraseTable(const char *file, util::MutableVocab &vocab, Scorer &scorer) {
  max_source_phrase_length_ = 0;
  FilePiece in(file, &std::cerr);

  uint64_t previous_text_hash = 0;
  Entry *entry = NULL;
  std::vector<ID> source;
  StringPiece line;
  try { while (true) {
    TokenIter<MultiCharacter> pipes(in.ReadLine(), "|||");
    // Setup the source phrase correctly.
    uint64_t source_text_hash = util::MurmurHashNative(pipes->data(), pipes->size());
    if (source_text_hash != previous_text_hash) {
      // New source text.
      if (entry) entry->vertex.Root().FinishRoot(search::kPolicyLeft);
      source.clear();
      for (TokenIter<SingleCharacter, true> word(*pipes, ' '); word; ++word) {
        source.push_back(vocab.FindOrInsert(*word));
      }
      max_source_phrase_length_ = std::max(max_source_phrase_length_, source.size());
      entry = &map_[util::MurmurHashNative(&*source.begin(), source.size() * sizeof(ID))];
      UTIL_THROW_IF(!entry->vertex.Empty(), Exception, "Source phrase " << *pipes << " appears non-consecutively in the phrase table.");
      entry->vertex.Root().InitRoot();
      previous_text_hash = source_text_hash;
    }
    Phrase target(phrase_pool_, vocab, *++pipes);
    search::HypoState hypo;
    hypo.history.cvp = target.Base();
    hypo.score = 
			 scorer.Parse(*++pipes)
			 + scorer.LM(target.begin(), target.end(), hypo.state)
			 + scorer.TargetWordCount(target.size());

    entry->vertex.Root().AppendHypothesis(hypo);
    UTIL_THROW_IF(++pipes, Exception, "Extra fields in phrase table: " << *pipes);
  } } catch (const util::EndOfFileException &e) {}
  if (entry) entry->vertex.Root().FinishRoot(search::kPolicyLeft);
}


const PhraseTable::Entry* PhraseTable::Phrases(const ID *begin, const ID *end) const {
  uint64_t hash_code = MurmurHashNative(begin, (end-begin) * sizeof(ID));
  //std::cerr << "Querying (length " << (end-begin) << ") phrase at " << hash_code << std::endl;
  Map::const_iterator hash_iterator = map_.find(hash_code);
  return hash_iterator == map_.end() ? NULL : &(hash_iterator->second);
}
  
}

