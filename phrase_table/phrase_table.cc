#include "phrase_table/phrase_table.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include <math.h>

using namespace util;

namespace phrase_table {

namespace {
static const double_conversion::StringToDoubleConverter kConverter(
    double_conversion::StringToDoubleConverter::NO_FLAGS,
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::quiet_NaN(),
    "inf",
    "NaN");
} // namespace

  PhraseTable::PhraseTable(const std::string & file, alone::Vocab &vocab, const Filter *const filter /* =NULL */) {
    FilePiece in(file.c_str(), &std::cout);

    try {
      std::size_t expected_num_scores = 0;
      uint64_t most_recent_hash_code = 0;
      Map::iterator most_recent_hash_iterator = map_.end();

      while (true) {
	//frage ||| issue ||| 0.25 0.285714 0.25 0.166667 2.718

	StringPiece line = in.ReadLine();
	std::vector<std::vector<StringPiece> > regions;

	for(	
	    TokenIter<MultiCharacter> it(line, MultiCharacter("|||"));
	    it; 
	    it++) 
	  {
	    StringPiece s = *it;
	    
	    regions.push_back(std::vector<StringPiece>());
	    // Now tokenize the region to get, e.g., space-separated tokens
	    for(
		TokenIter<SingleCharacter,true> it2(s, SingleCharacter(' ')); 
		it2; 
		it2++) 
	      {
		regions.back().push_back(*it2);
	      }
	    
	  }

	// Error checking of fields read
	UTIL_THROW_IF(regions.size() != 3, Exception, "Invalidly formatted phrase table line: " << line);
	UTIL_THROW_IF(regions.at(0).size()==0, Exception, "No source words in phrase table line: " << line);
	UTIL_THROW_IF(regions.at(1).size()==0, Exception, "No target words in phrase table line: " << line);
	if(expected_num_scores==0) { // This condition means this is the first line of the phrase table
	  UTIL_THROW_IF(regions.at(2).size()==0, Exception, "No scores in phrase table line: " << line);
	  expected_num_scores = regions.at(2).size();
	}
	else {
	  UTIL_THROW_IF(regions.at(2).size()!=expected_num_scores, Exception, "Unexpected #scores in phrase table line: " << line);
	}

	// Create vocabulary items for source and target words
	Phrase sourcePhrase;
	ScoredPhrase scoredTargetPhrase;

	for(std::vector<StringPiece>::iterator i=regions.at(0).begin(); i!=regions.at(0).end(); i++) {
	  sourcePhrase.push_back(VocabItem(vocab.FindOrAdd(*i)));
	}

	for(std::vector<StringPiece>::iterator i=regions.at(1).begin(); i!=regions.at(1).end(); i++) {
	  scoredTargetPhrase.phrase.push_back(VocabItem(vocab.FindOrAdd(*i)));
	}

	std::size_t score_index=0;
	for(std::vector<StringPiece>::iterator i=regions.at(2).begin(); i!=regions.at(2).end(); i++) {
	  SparseScore score;
	  score.index = score_index++;
          int length;
          score.val = kConverter.StringToFloat(i->data(), i->size(), &length);
          UTIL_THROW_IF(isnan(score.val), "Bad score in the phrase table: " << *i);
	  scoredTargetPhrase.scores.push_back(score);
	}

	// Add this phrase pair to the map. 
	// Get hash for source phrase
	uint64_t hash_code = MurmurHashNative(sourcePhrase.data(), sizeof(VocabItem)*sourcePhrase.size());
	if(hash_code==most_recent_hash_code) {
	  (*most_recent_hash_iterator).second.push_back(scoredTargetPhrase);
	}
	else {
          
	  Map::iterator hash_iterator = map_.find(hash_code);
	  if(hash_iterator==map_.end()) {
	    Entry entry;
	    entry.push_back(scoredTargetPhrase);
	    std::pair<Map::iterator,bool> pair = map_.insert(std::make_pair(hash_code,entry));
	    UTIL_THROW_IF(!pair.second, Exception, "Item not successfully inserted into phrase table.");
	    hash_iterator = pair.first;
	  }
	  else {
	    (*hash_iterator).second.push_back(scoredTargetPhrase);
	  }
	  most_recent_hash_iterator = hash_iterator;
	  most_recent_hash_code = hash_code;
	}
      }
    } catch (const util::EndOfFileException &e) { }  
  }
  
  const PhraseTable::Entry* PhraseTable::getPhrases(const Phrase & source_phrase) const {
    uint64_t hash_code = MurmurHashNative(source_phrase.data(), sizeof(VocabItem)*source_phrase.size());
    Map::const_iterator hash_iterator = map_.find(hash_code);
    if(hash_iterator==map_.end()) {
      return NULL;
    }
    else {
      return &(hash_iterator->second);
    }
  }

}

