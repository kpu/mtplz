#include "util/layout.hh"

#define BOOST_TEST_MODULE LayoutTest
#include <boost/test/unit_test.hpp>

namespace util { namespace {

BOOST_AUTO_TEST_CASE(Simple) {
  Layout layout;
  PODField<uint32_t> an_int(layout);
  VectorField<uint16_t, uint8_t> variable(layout);
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
  BOOST_CHECK_EQUAL(3, variable(value, pool)[1]);
  BOOST_CHECK_EQUAL(3, variable(value)[1]);
  BOOST_CHECK_EQUAL(3, variable(value).back());
  variable(value, pool).clear();
  BOOST_CHECK_EQUAL(0, variable(value).size());
  BOOST_CHECK(variable(value).empty());
}

BOOST_AUTO_TEST_CASE(TwoVec) {
  Layout layout;
  VectorField<uint16_t, uint8_t> first(layout);
  VectorField<uint16_t, uint8_t> second(layout);
  
  util::Pool pool;
  void *value = layout.Allocate(pool);
  BOOST_CHECK(first(value).empty());
  BOOST_CHECK(second(value).empty());

  first(value, pool).push_back(1);
  first(value, pool).push_back(2);
  BOOST_CHECK_EQUAL(2, first(value).size());
  BOOST_CHECK_EQUAL(1, first(value)[0]);
  BOOST_CHECK_EQUAL(2, first(value)[1]);
  BOOST_CHECK(second(value).empty());
  
  second(value, pool).push_back(3);
  BOOST_CHECK_EQUAL(2, first(value).size());
  BOOST_CHECK_EQUAL(1, first(value)[0]);
  BOOST_CHECK_EQUAL(2, first(value)[1]);
  BOOST_CHECK_EQUAL(3, second(value)[0]);
}

}} // namespaces
