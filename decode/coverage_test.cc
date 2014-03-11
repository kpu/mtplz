#include "decode/coverage.hh"

#define BOOST_TEST_MODULE CoverageTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

BOOST_AUTO_TEST_CASE(LeftOpen) {
  Coverage coverage;
  BOOST_CHECK_EQUAL(0, coverage.LeftOpen(0));
  BOOST_CHECK_EQUAL(0, coverage.LeftOpen(1));
  BOOST_CHECK_EQUAL(0, coverage.LeftOpen(10));
  BOOST_CHECK_EQUAL(70, coverage.RightOpen(0, 70));
  BOOST_CHECK_EQUAL(63, coverage.RightOpen(10, 63));

  coverage.Set(2, 3);
  BOOST_CHECK(coverage.Compatible(0,2));
  BOOST_CHECK(!coverage.Compatible(2,3));
  BOOST_CHECK_EQUAL(0, coverage.FirstZero());
  BOOST_CHECK_EQUAL(0, coverage.LeftOpen(1));
  BOOST_CHECK_EQUAL(3, coverage.LeftOpen(4));
  BOOST_CHECK_EQUAL(2, coverage.RightOpen(0, 40));
  BOOST_CHECK_EQUAL(40, coverage.RightOpen(3, 40));
}

} // namespace
} // namespace decode
