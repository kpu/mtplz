#include "phrase_table/filter.hh"

using namespace std;

namespace phrase_table {
  Filter::Filter(std::string const& file, const std::size_t ngram_length) :
    ngram_length_(ngram_length)
  {
    ifstream fs(file.c_str());
    if(fs.is_open()) {
      string line;
      while(getline(fs, line)) {
	std::stringstream ls(line);
	std::string token;
	std::vector<std::string> tokens;
	while(ls >> token) tokens.push_back(token);
	for(std::vector<std::string>::iterator i=tokens.begin(); i!=tokens.end(); i++) {
	  for(std::size_t len=1; len<=ngram_length && (i+len-1)!=tokens.end(); len++) {
	    std::vector<std::string> ngram(i,i+len);
	    ngram_map_.insert(ngram);

	    cerr << "FILTER NGRAM: ";
	    for(std::vector<std::string>::iterator j=ngram.begin(); j!=ngram.end(); j++) {
	      cerr << *j << " ";
	    }
	    cerr << std::endl;
	  }
	}	
      }
    }
    else {
      throw std::runtime_error("Could not open file: "+file);
    }
  }
  
  bool Filter::PassesFilter(std::vector<VocabItem> const & words) const {
    std::vector<std::string> word_strings;
    for(std::vector<VocabItem>::const_iterator i=words.begin(); i!=words.end(); i++) {
      //word_strings.push_back(string(i->first));
      word_strings.push_back(string(i->getWord()));
    }

    if(word_strings.size() <= ngram_length_) {
      return (ngram_map_.find(word_strings) != ngram_map_.end());
    }
    else {
      for(std::vector<std::string>::iterator first=word_strings.begin(); (first+ngram_length_) != word_strings.end(); first++) {
	// Hmm, looks like we need to create/copy subvectors, which is unfortunate.
	// Not sure if there is a better way
	std::vector<std::string> sub(first, first+ngram_length_);
	if (ngram_map_.find(sub) == ngram_map_.end()) return false;
      }
      return true;
    }
    return true;
  }    
}	    


// int main(int argc, char *argv[]) {
//   phrase_table::Filter f(argv[1], 3);
//   return 0;
// }

