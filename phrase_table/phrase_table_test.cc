#include "phrase_table/phrase_table.hh"
#include <iostream>

#define BOOST_TEST_MODULE VocabTest
#include <boost/test/unit_test.hpp>

namespace phrase_table {
namespace {

  // Unfortunate code to determine the file locations for test inputs.
  // This is necessary because bjam is nondeterministic in how it
  // orders the command line arguments for this test. 
  const char *PhraseTableLocation() {
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    return argv[strstr(argv[1], "phrase_table") ? 1 : 2];
  }

  const char *SourceTextLocation() {
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    return argv[strstr(argv[1], "source_text") ? 1 : 2];
  }
  
  // TODO: We need a more thorough test
  BOOST_AUTO_TEST_CASE(phrase_table) {
    std::string source_text_file = SourceTextLocation();
    std::string phrase_table_file  = PhraseTableLocation();
    util::MutableVocab vocab;

    PhraseTable phrase_table(phrase_table_file, vocab, NULL);
    
    std::ifstream fs(source_text_file.c_str());
    if(fs.is_open()) {
      std::string line;
      while(getline(fs, line)) {
	std::stringstream ls(line);
	std::string token;
	bool should_find_phrases;
	Phrase phrase;

	ls >> should_find_phrases;
	while(ls >> token) {
	  std::cerr << "   " << token;
	  phrase.push_back(vocab.FindOrInsert(token));
	}
	std::cerr << std::endl;
	bool does_find_phrases = (phrase_table.getPhrases(phrase.begin(), phrase.end()) != NULL);
	std::cerr << "does_find_phrases: " << does_find_phrases << "  should_find_phrases: " << should_find_phrases << std::endl;
	BOOST_CHECK_EQUAL(does_find_phrases, should_find_phrases);
      }
    }
    else {
      throw std::runtime_error("Could not open file: "+source_text_file);
    }

  }

} // namespace
} // namespace phrase_table
