#include "phrase_table/filter.hh"

using namespace std;

namespace phrase_table {
  Filter::Filter(const std::string & file, util::MutableVocab& vocab, const std::size_t ngram_length) :
    ngram_length_(ngram_length)
  {
    ifstream fs(file.c_str());
    if(fs.is_open()) {
      string line;
      while(getline(fs, line)) {
	std::stringstream ls(line);
	std::string token;
	Phrase sentence;
	while(ls >> token) sentence.push_back(vocab.FindOrInsert(token));

	for(Phrase::iterator i=sentence.begin(); i!=sentence.end(); i++) {
	  for(std::size_t len=1; len<=ngram_length && (i+len-1)!=sentence.end(); len++) {
	    Phrase ngram(i,i+len);
	    ngram_map_.insert(ngram);
	  }
	}	
      }
    }
    else {
      throw std::runtime_error("Could not open file: "+file);
    }
  }
  
  bool Filter::PassesFilter(Phrase const & words) const {
    if(words.size() < ngram_length_) {
      return (ngram_map_.find(words) != ngram_map_.end());      
    }
    else {
      for(Phrase::const_iterator first=words.begin(); (first+ngram_length_-1) != words.end(); first++) {
	Phrase ngram(first, first+ngram_length_);
	if (ngram_map_.find(ngram) == ngram_map_.end()) return false;
      }
      return true;
    }
  }    
}	    
