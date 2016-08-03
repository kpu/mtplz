#define BOOST_TEST_MODULE PhraseTableTest
#include <boost/test/unit_test.hpp>

#include "pt/access.hh"
#include "pt/create.hh"
#include "pt/query.hh"
#include "util/file.hh"

namespace pt { namespace {

util::scoped_fd MakeFile() {
  util::scoped_fd file(util::MakeTemp(util::DefaultTempDirectory()));
  const char text[] = 
    "a b c ||| B A C ||| 0.25 0.285714 0.25 0.166667 2.718\n"
    "a b c ||| B A ||| 0.1 0.3 0.25 0.166667 2.718\n"
    "d e ||| D E F ||| 0.1 0.2 0.4 0.8 1.6\n";
  util::WriteOrThrow(file.get(), text, sizeof(text) - 1 /* no null at end */);
  util::SeekOrThrow(file.get(), 0);
  return file;
}

BOOST_AUTO_TEST_CASE(CreateAndQuery) {
  util::scoped_fd binary(util::MakeTemp(util::DefaultTempDirectory()));
  TextColumns columns;
  FieldConfig fields;
  fields.dense_features = 1;
  CreateTable(MakeFile().release(), util::DupOrThrow(binary.get()), columns, fields);
  BOOST_CHECK_EQUAL(5, fields.dense_features);
  util::SeekOrThrow(binary.get(), 0);
  Table table(binary.release(), util::READ);

  VocabRange range(table.Vocab());
  VocabRange::Iterator it(range.begin());
  BOOST_REQUIRE(it);
  BOOST_CHECK_EQUAL("<unk>", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("<s>", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("</s>", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("a", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("b", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("c", *it);
  BOOST_REQUIRE(++it);
  BOOST_CHECK_EQUAL("B", *it);


  WordIndex abc[3] = {3, 4, 5};
  // No a b phrase
  BOOST_CHECK(table.Lookup(abc, abc + 2).empty());
  // Lookup a b c
  boost::iterator_range<RowIterator> abc_targets(table.Lookup(abc, abc + 3));
  RowIterator row = abc_targets.begin();
  BOOST_REQUIRE(row != abc_targets.end());
  BOOST_REQUIRE(row.Accessor().target);
  BOOST_CHECK_EQUAL(3, row.Accessor().target(row).size());
  BOOST_REQUIRE(row.Accessor().dense_features);
  BOOST_CHECK_EQUAL(5, row.Accessor().dense_features(row).size());
  BOOST_CHECK_CLOSE(log(0.25), row.Accessor().dense_features(row)[0], 0.001);
}

} } // namespaces
