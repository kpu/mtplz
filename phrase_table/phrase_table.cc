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
  
  PhraseTable::PhraseTable(const std::string & file, util::MutableVocab &vocab, const Filter *const filter /* =NULL */) {
    max_source_phrase_length_ = 0;
    FilePiece in(file.c_str(), &std::cout);
    try {
      std::size_t expected_num_scores = 0;
      uint64_t most_recent_hash_code = 0;
      Entry* most_recent_entry_ptr = NULL;
      
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
	Phrase source_phrase;
	ScoredPhrase scored_target_phrase;
	
	for(std::vector<StringPiece>::iterator i=regions.at(0).begin(); i!=regions.at(0).end(); i++) {
	  source_phrase.push_back(vocab.FindOrInsert(*i));
	}
	
	for(std::vector<StringPiece>::iterator i=regions.at(1).begin(); i!=regions.at(1).end(); i++) {
	  scored_target_phrase.phrase.push_back(vocab.FindOrInsert(*i));
	}
	
	std::size_t score_index=0;
	for(std::vector<StringPiece>::iterator i=regions.at(2).begin(); i!=regions.at(2).end(); i++) {
	  SparseScore score;
	  score.index = score_index++;
          int length;
          score.val = kConverter.StringToFloat(i->data(), i->size(), &length);
          UTIL_THROW_IF(isnan(score.val), Exception, "Bad score in the phrase table: " << *i);
	  scored_target_phrase.scores.push_back(score);
	}

	// Update max_source_phrase_length_ if necessary
	if(source_phrase.size() > max_source_phrase_length_)
	  max_source_phrase_length_ = source_phrase.size();

	// Add this phrase pair to the map. 
	// Get hash for source phrase
	uint64_t hash_code = MurmurHashNative(source_phrase.data(), sizeof(VocabEntry)*source_phrase.size());
	if(most_recent_entry_ptr==NULL || hash_code!=most_recent_hash_code) {
	  most_recent_entry_ptr = &map_[hash_code];
	  most_recent_hash_code = hash_code;
	}
	//std::cerr << "Adding phrase to " << most_recent_hash_code << std::endl;
	most_recent_entry_ptr->push_back(scored_target_phrase);
      }
    } catch (const util::EndOfFileException &e) { }  
  }
  
  
  const PhraseTable::Entry* PhraseTable::getPhrases(Phrase::iterator begin, Phrase::iterator end) const {
    uint64_t hash_code = MurmurHashNative(&(*begin), (end-begin) * sizeof(VocabEntry));
    //std::cerr << "Querying (length " << (end-begin) << ") phrase at " << hash_code << std::endl;
    Map::const_iterator hash_iterator = map_.find(hash_code);
    if(hash_iterator==map_.end()) {
      return NULL;
    }
    else {
      return &(hash_iterator->second);
    }
  }
  
}

