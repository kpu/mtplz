#include "decode/lexro.hh"

#define BOOST_TEST_MODULE LexRoTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

pt::Row *Row(pt::Access &access, util::Pool &pool, int value) {
  pt::Row *row = access.Allocate(pool);
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
                                          //  forward  backward
                                          //  M  S  D   M  S  D
  pt::Row *pt_row1 = Row(access,pool,11); // 11 12 13  14 15 16
  pt::Row *pt_row2 = Row(access, pool,3); //  3  4  5   6  7  8
  TargetPhrase *row1 = reinterpret_cast<TargetPhrase*>(init.target_phrase_layout.Allocate(pool));
  TargetPhrase *row2 = reinterpret_cast<TargetPhrase*>(init.target_phrase_layout.Allocate(pool));
  init.pt_row_field(row1) = pt_row1;
  init.pt_row_field(row2) = pt_row2;
  LexicalizedReordering lexro_obj = LexicalizedReordering();
  Feature &lexro = lexro_obj;
  lexro.Init(init);

  // init hypothesis
  Hypothesis *zero_hypo = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(zero_hypo) = Hypothesis((float)0);
  Hypothesis *hypo = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(hypo) = Hypothesis(0,zero_hypo,3,5,row1);
  Hypothesis *next = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(next) = Hypothesis(500,hypo,5,6,row1);
  Hypothesis *snd_next = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));

  // setup scoring
  std::vector<float> weights({1,1});
  FeatureStore store({0,0});
  std::vector<VocabWord*> sentence;
  SourcePhrase source_phrase(sentence, 5,6);
  ScoreCollector collector(weights, next, &store);
  collector.SetDenseOffset(0);

  // monotone source, forward scoring
  lexro.ScoreHypothesisWithSourcePhrase(*hypo, source_phrase, collector);
  BOOST_CHECK_EQUAL(14, collector.Score());
  BOOST_CHECK_EQUAL(0, store[0]);
  BOOST_CHECK_EQUAL(14, store[1]);
  lexro.ScoreHypothesisWithPhrasePair(*hypo, PhrasePair{source_phrase, row2}, collector);
  BOOST_CHECK_EQUAL(14+3, collector.Score());
  BOOST_CHECK_EQUAL(3, store[0]);
  BOOST_CHECK_EQUAL(14, store[1]);

  // for next source phrase we can use backwards reordering score
  SourcePhrase swap_source(sentence,1,5);
  FeatureStore store2({0,0});
  ScoreCollector collector2(weights, snd_next, &store2);
  collector2.SetDenseOffset(0);
  lexro.ScoreHypothesisWithSourcePhrase(*next, swap_source, collector2);
  BOOST_CHECK_EQUAL(15, collector2.Score());
  BOOST_CHECK_EQUAL(0, store2[0]);
  BOOST_CHECK_EQUAL(15, store2[1]);

  // on a final hypothesis, we can get a final score
  init.hypothesis_field(next) = Hypothesis(3,hypo,5,6,row2);
  lexro.ScoreFinalHypothesis(*next, collector);
  BOOST_CHECK_EQUAL(14+3+6, collector.Score());
  BOOST_CHECK_EQUAL(3, store[0]);
  BOOST_CHECK_EQUAL(6, store[1]);
}

} // namespace
} // namespace decode
