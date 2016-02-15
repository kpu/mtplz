#include "util/layout.hh"

#define BOOST_TEST_MODULE LayoutTest
#include <boost/test/unit_test.hpp>

namespace util { namespace {

BOOST_AUTO_TEST_CASE(Fixed) {
  Layout layout;
  PODField<uint32_t> an_int(layout);
  VectorField<uint8_t, uint16_t> variable(layout);
  ArrayField<uint64_t> fixed(layout, 3);
  BOOST_CHECK_EQUAL(3, fixed.size());

  util::Pool pool;
  void *value(layout.Allocate(pool));
  an_int(value) = 1;
  BOOST_CHECK_EQUAL(1, an_int(value));

  BOOST_CHECK_EQUAL(0, variable(value).size());
  BOOST_CHECK(variable(value).empty());
  variable(value, pool).push_back(10);
  BOOST_CHECK_EQUAL(1, variable(value).size());
  BOOST_CHECK_EQUAL(10, variable(value).front());
  BOOST_CHECK_EQUAL(10, variable(value).back());
  variable(value, pool).push_back(3);
  BOOST_CHECK_EQUAL(2, variable(value).size());
  BOOST_CHECK_EQUAL(10, variable(value).front());
  BOOST_CHECK_EQUAL(3, variable(value).back());
  variable(value, pool).clear();
  BOOST_CHECK_EQUAL(0, variable(value).size());
  BOOST_CHECK(variable(value).empty());
}

}} // namespaces
