#include "util/mutable_vocab.hh"
#include "decode/filter.hh"

#include <iostream>
#include <fstream>

#define BOOST_TEST_MODULE FilterTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

  // Unfortunate code to determine the file locations for test inputs.
  // This is necessary because bjam is nondeterministic in how it
  // orders the command line arguments for this test. 
  const char *FilterTextLocation() {
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    return argv[strstr(argv[1], "filter_text") ? 1 : 2];
  }

  const char *FilterQueriesLocation() {
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    return argv[strstr(argv[1], "filter_queries") ? 1 : 2];
  }
  
  BOOST_AUTO_TEST_CASE(small) {
    std::string in_file = FilterTextLocation();
    std::string q_file  = FilterQueriesLocation();
    std::size_t ngram_length = 3;
    util::MutableVocab vocab;
    
    Filter filter(in_file, vocab, ngram_length);
    
    std::ifstream fs(q_file.c_str());
    if(fs.is_open()) {
      std::string line;
      while(getline(fs, line)) {
	std::stringstream ls(line);
	std::string token;
	bool should_pass;
	Phrase sentence;

	ls >> should_pass;
	while(ls >> token) sentence.push_back(vocab.FindOrInsert(token));
	bool does_pass = filter.PassesFilter(sentence);
	std::cerr << "Should pass: " << should_pass << " " << "does pass: " << does_pass << std::endl; 
	BOOST_CHECK_EQUAL(does_pass, should_pass);
      }
    }
    else {
      throw std::runtime_error("Could not open file: "+q_file);
    }

  }


} // namespace
} // namespace decode
