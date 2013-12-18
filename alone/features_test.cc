#include "alone/features.hh"

#define BOOST_TEST_MODULE WeightTest
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace alone {
namespace feature {

// Friend to get access to the underlying vector.  
class Test {
  public:
    typedef Vector::Entry Entry;
    static const std::vector<Entry> &Get(Vector &vec) {
      return vec.values_;
    }
    static std::vector<Entry> &Mutable(Vector &vec) {
      return vec.values_;
    }
};

namespace {

typedef Test::Entry Entry;

void Append(std::vector<Test::Entry> *to, ID id, search::Score score) {
  Entry val;
  val.id = id;
  val.score = score;
  to->push_back(val);
}

BOOST_AUTO_TEST_CASE(parse) {
  Weights w;
  w.AppendFromString("rarity=0 phrase-SGT=0 phrase-TGS=9.45117 lhsGrhs=0 lexical-SGT=2.33833 lexical-TGS=-28.3317 abstract?=0 LanguageModel=3 lexical?=1 glue?=5 OOV 1.7 foo 6.3");
  BOOST_CHECK_CLOSE(3.0, w.Lookup("LanguageModel"), 0.001);
  BOOST_CHECK_CLOSE(1.7, w.Lookup("OOV"), 0.001);
  Vector out;
  const std::vector<Test::Entry> &behind = Test::Get(out);

  BOOST_CHECK_CLOSE(9.45117 * 3.0, w.Parse("phrase-TGS=3.0", out), 0.001);
  BOOST_REQUIRE_EQUAL(1U, behind.size());
  BOOST_CHECK_EQUAL(2U, behind[0].id);
  BOOST_CHECK_CLOSE(3.0, behind[0].score, 0.001);

  BOOST_CHECK_CLOSE(6.3, w.Parse("foo 1 bar \t 2\n OOV=0", out), 0.001);

  BOOST_CHECK_CLOSE(9.45117 * 7.0 + 10.0 * 3.0, w.Parse("phrase-TGS=7.0 LanguageModel=10", out), 0.001);
  BOOST_CHECK_CLOSE(9.45117 * 3.0 + 3.0 * 10.0 + 28.3317 * 17.4, w.Parse("rarity=5 phrase-TGS=3.0 LanguageModel=10 lexical-TGS=-17.4", out), 0.001);
}

BOOST_AUTO_TEST_CASE(adder) {
  Vector vec[3], out;
  std::vector<Test::Entry> *behind[3];
  for (unsigned int i = 0; i < 3; ++i) {
    behind[i] = &Test::Mutable(vec[i]);
  }

  // Empty vector
  {
    Adder adder;
    adder.Add(vec[0]);
    adder.Finish(out);
    BOOST_CHECK(Test::Get(out).empty());
  }

  // Simple test case
  Append(behind[0], 1, 3.1);
  Append(behind[0], 3, 4.7);
  Append(behind[1], 0, 6.0);
  Append(behind[1], 3, 2.7);
  Append(behind[1], 4, 2.7);
  Append(behind[2], 1, 0.5);
  Append(behind[2], 3, 0.07);

  Adder adder;
  adder.Add(vec[0]);
  adder.Add(vec[1]);
  adder.Add(vec[2]);
  adder.Finish(out);
  const std::vector<Entry> &f = Test::Get(out);
  BOOST_REQUIRE_EQUAL(4U, f.size()); // 0, 1, 3, 4
  BOOST_CHECK_EQUAL(0U, f[0].id);
  BOOST_CHECK_CLOSE(6.0, f[0].score, 0.001);
  BOOST_CHECK_EQUAL(1U, f[1].id);
  BOOST_CHECK_CLOSE(3.1 + 0.5, f[1].score, 0.001);
  BOOST_CHECK_EQUAL(3U, f[2].id);
  BOOST_CHECK_CLOSE(4.7 + 2.7 + 0.07, f[2].score, 0.001);
  BOOST_CHECK_EQUAL(4U, f[3].id);
  BOOST_CHECK_CLOSE(2.7, f[3].score, 0.001);
}

} // namespace
} // namespace feature
} // namespace alone
