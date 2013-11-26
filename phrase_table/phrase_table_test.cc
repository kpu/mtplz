#include "phrase_table/vocab.hh"

#define BOOST_TEST_MODULE VocabTest
#include <boost/test/unit_test.hpp>

namespace phrase_table {
namespace {

BOOST_AUTO_TEST_CASE(small) {
  Vocab vocab;
  BOOST_CHECK_EQUAL(1, vocab.FindOrInsert("Foo"));
  BOOST_CHECK_EQUAL(2, vocab.Size());
  BOOST_CHECK_EQUAL(1, vocab.Find("Foo"));
  BOOST_CHECK_EQUAL("Foo", vocab.String(1));
}

} // namespace
} // namespace phrase_table
