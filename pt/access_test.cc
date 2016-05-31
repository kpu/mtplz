#include "pt/access.hh"
#include "util/mmap.hh"

#define BOOST_TEST_MODULE AccessTest
#include <boost/test/unit_test.hpp>

namespace pt { namespace {

void SaveRestore(const FieldConfig config) {
  util::scoped_memory mem;
  config.Save(mem);
  FieldConfig restored;
  restored.Restore(mem);
  BOOST_CHECK_EQUAL(config.target, restored.target);
  BOOST_CHECK_EQUAL(config.dense_features, restored.dense_features);
  BOOST_CHECK_EQUAL(config.sparse_features, restored.sparse_features);
  BOOST_CHECK_EQUAL(config.lexical_reordering, restored.lexical_reordering);
}

BOOST_AUTO_TEST_CASE(SaveFieldConfig) {
  FieldConfig config;
  config.target = false;
  SaveRestore(config);
  config.dense_features = 3;
  SaveRestore(config);
  config.lexical_reordering = 10;
  SaveRestore(config);
  config.sparse_features = true;
  SaveRestore(config);
  config.target = true;
  SaveRestore(config);
}

}} // namespaces
