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
  BOOST_CHECK(it);
  BOOST_CHECK_EQUAL("a", *it);
}

} } // namespaces
