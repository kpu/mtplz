#include "decode/lexro.hh"

#define BOOST_TEST_MODULE LexRoTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

pt::Row *Row(pt::Access &access, util::Pool &pool) {
  pt::Row *row = access.Allocate(pool);
  int value = 3;
  // 3 4 5  6 7 8
  for (unsigned i = 0; i < 6; ++i) {
    access.lexical_reordering(row)[i] = (float)value++;
  }
  return row;
}

BOOST_AUTO_TEST_CASE(LexRo) {
  util::Pool pool;
  pt::FieldConfig config;
  config.lexical_reordering = 6;
  pt::Access access(config);
  FeatureInit init(access);
  
  pt::Row *row = Row(access, pool);
  LexicalizedReordering lexro_obj = LexicalizedReordering();
  Feature &lexro = lexro_obj;
  lexro.Init(init);

  // init hypothesis
  Hypothesis *zero_hypo = reinterpret_cast<Hypothesis*>(init.HypothesisLayout().Allocate(pool));
  init.HypothesisField()(zero_hypo) = Hypothesis(0);
  Hypothesis *hypo = reinterpret_cast<Hypothesis*>(init.HypothesisLayout().Allocate(pool));
  init.HypothesisField()(hypo) = Hypothesis(0,zero_hypo,3,5,NULL);
  Hypothesis *next = reinterpret_cast<Hypothesis*>(init.HypothesisLayout().Allocate(pool));

  std::vector<float> weights({1,1});
  FeatureStore store({0,0});
  std::vector<ID> sentence;
  SourcePhrase source_phrase = SourcePhrase(sentence, 5,6);
  ScoreCollector collector(weights, next, &store);
  collector.SetDenseOffset(0);
  lexro.ScoreHypothesisWithSourcePhrase(*hypo, source_phrase, collector);
  BOOST_CHECK_EQUAL(0, collector.Score()); // no lexro info based on source only
  // pairing with phrase pair allows for forward scoring
  // TODO we could score backwards for the previous pairing
  lexro.ScoreHypothesisWithPhrasePair(*hypo, PhrasePair{source_phrase, *row}, collector);
  BOOST_CHECK_EQUAL(3, collector.Score());
  BOOST_CHECK_EQUAL(3, store[0]);
  BOOST_CHECK_EQUAL(0, store[1]);

  // on a final hypothesis, we can get a final score
  init.HypothesisField()(next) = Hypothesis(3,hypo,5,6,row);
  lexro.RescoreHypothesis(*next, collector);
  BOOST_CHECK_EQUAL(9, collector.Score());
  BOOST_CHECK_EQUAL(6, store[1]);
}

} // namespace
} // namespace decode
