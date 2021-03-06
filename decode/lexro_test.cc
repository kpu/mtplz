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
  pt::Row *pt_row0 = Row(access,pool,0);  //  M  S  D   M  S  D
  pt::Row *pt_row1 = Row(access,pool,11); // 11 12 13  14 15 16
  pt::Row *pt_row2 = Row(access, pool,3); //  3  4  5   6  7  8
  TargetPhrase *row0 = reinterpret_cast<TargetPhrase*>(init.target_phrase_layout.Allocate(pool));
  TargetPhrase *row1 = reinterpret_cast<TargetPhrase*>(init.target_phrase_layout.Allocate(pool));
  TargetPhrase *row2 = reinterpret_cast<TargetPhrase*>(init.target_phrase_layout.Allocate(pool));
  init.pt_row_field(row0) = pt_row0;
  init.pt_row_field(row1) = pt_row1;
  init.pt_row_field(row2) = pt_row2;
  LexicalizedReordering lexro_obj = LexicalizedReordering();
  Feature &lexro = lexro_obj;
  lexro.Init(init);

  // init hypothesis
  Hypothesis *zero_hypo = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(zero_hypo) = Hypothesis(0, row0);
  Hypothesis *hypo = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(hypo) = Hypothesis(0,zero_hypo,3,5,row1);
  Hypothesis *next = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));
  init.hypothesis_field(next) = Hypothesis(500,hypo,5,6,row1);
  Hypothesis *snd_next = reinterpret_cast<Hypothesis*>(init.hypothesis_layout.Allocate(pool));

  // setup scoring
  std::vector<float> weights({1,1,1,1,1,1});
  util::Layout fstore_layout;
  util::ArrayField<float> fstore(fstore_layout, 6);
  FeatureStore store(fstore, fstore_layout.Allocate(pool));
  std::vector<VocabWord*> sentence;
  for (int i=0; i<6; ++i) sentence.push_back(nullptr);
  SourcePhrase source_phrase(sentence, 5,6);
  ScoreCollector collector(weights, next, nullptr, store);
  collector.SetDenseOffset(0);

  // monotone source, forward scoring
  lexro.ScoreHypothesisWithSourcePhrase(*hypo, source_phrase, collector);
  BOOST_CHECK_EQUAL(14, collector.Score());
  BOOST_CHECK_SMALL(store()[0], 0.00001f);
  BOOST_CHECK_EQUAL(14, store()[3]);
  lexro.ScoreHypothesisWithPhrasePair(*hypo, PhrasePair(source_phrase, row2), collector);
  BOOST_CHECK_EQUAL(14+3, collector.Score());
  BOOST_CHECK_EQUAL(3, store()[0]);
  BOOST_CHECK_EQUAL(14, store()[3]);

  // for next source phrase we can use backwards reordering score
  SourcePhrase swap_source(sentence,1,5);
  FeatureStore store2(fstore, fstore_layout.Allocate(pool));
  ScoreCollector collector2(weights, snd_next, nullptr, store2);
  collector2.SetDenseOffset(0);
  lexro.ScoreHypothesisWithSourcePhrase(*next, swap_source, collector2);
  BOOST_CHECK_EQUAL(15, collector2.Score());
  BOOST_CHECK_SMALL(store2()[1], 0.000001f);
  BOOST_CHECK_EQUAL(15, store2()[4]);

  // TODO test no score on addition of zero-length source phrase (eos)
}

} // namespace
} // namespace decode
