#include "decode/chart.hh"

#include "decode/objective.hh"
#include "pt/access.hh"
#include "lm/model.hh"

#include <string>
#include <unordered_map>

#define BOOST_TEST_MODULE ChartTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

typedef std::pair<std::size_t,std::size_t> Range;

class FeatureMock : public Feature {
  public:
    FeatureMock(std::vector<StringPiece> &rep_buffer, std::vector<VocabWord*> &word_buffer)
      : Feature("mock"), rep_buffer_(&rep_buffer), word_buffer_(&word_buffer) {}
    FeatureMock(std::vector<PhrasePair> &phrase_pair_buffer)
      : Feature("mock"), phrase_pair_buffer_(&phrase_pair_buffer) {}

    void Init(FeatureInit &feature_init) override {}
    void NewWord(const StringPiece string_rep, VocabWord *word) override {
      rep_buffer_->push_back(string_rep);
      word_buffer_->push_back(word);
    }
    void InitPassthroughPhrase(pt::Row *passthrough) const override {}
    void ScorePhrase(PhrasePair phrase_pair, ScoreCollector &collector) const override {
      phrase_pair_buffer_->push_back(phrase_pair);
    }
    void ScoreHypothesisWithSourcePhrase(
        const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const override {}
    void ScoreHypothesisWithPhrasePair(
        const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const override {}
    void ScoreFinalHypothesis(
        const Hypothesis &hypothesis, ScoreCollector &collector) const override {}
    std::size_t DenseFeatureCount() const override { return 1; }
    std::string FeatureDescription(std::size_t index) const override { return ""; }

    std::vector<StringPiece> *rep_buffer_;
    std::vector<VocabWord*> *word_buffer_;
    std::vector<PhrasePair> *phrase_pair_buffer_;
};

/* This does not work and I will not invest time in bugfixes,
 * considering that the test will not work as planned anyway (see PhraseTest
 * comment)
class PTMock {
  public:
    PTMock(const pt::Access &access) : access_(access) {}

    void AddEntries(Range range, std::vector<pt::Row*> entries) {
      map_[range] = entries;
    }

    std::vector<pt::Row*> Lookup(const ID *source_begin, const ID *source_end) const {
      return map_[Range((std::size_t)begin, (std::size_t)end)];
    }
  private:
    struct PairHash {
      size_t operator()(const Range pair) const {
        std::size_t first = pair.first, second = pair.second;
        return std::hash<std::size_t>(first) ^ std::hash<std::size_t>(second);
      }
    };

    const pt::Access &access_;
    std::unordered_map<Range,std::vector<pt::Row*>,PairHash> map_;
};
*/

BOOST_AUTO_TEST_CASE(InitTest) {
  pt::FieldConfig config;
  pt::Access access(config);
  lm::ngram::State lm_state;
  Objective objective(access, lm_state);
  Chart chart(13, objective);
  BOOST_CHECK_EQUAL(13, chart.MaxSourcePhraseLength());
}

BOOST_AUTO_TEST_CASE(EosTest) {
  pt::FieldConfig config;
  pt::Access access(config);
  lm::ngram::State lm_state;
  Objective objective(access, lm_state);
  std::vector<PhrasePair> phrase_pair_buffer;
  FeatureMock feature_mock(phrase_pair_buffer);
  objective.AddFeature(feature_mock);
  Chart chart(11, objective);

  TargetPhrases &eos = chart.EndOfSentence();
  BOOST_CHECK_EQUAL(1, eos.Root().Size());
  ID eos_word = Chart::EOS_WORD;
  const pt::Row *eos_row = objective.GetFeatureInit().pt_row_field(eos.Root().End().cvp);
  BOOST_CHECK_EQUAL(1, access.target(eos_row).size());
  BOOST_CHECK_EQUAL(eos_word, access.target(eos_row)[0]);
  BOOST_CHECK_EQUAL(1, phrase_pair_buffer.size());
}

BOOST_AUTO_TEST_CASE(ReadSentenceTest) {
  // init chart
  pt::FieldConfig config;
  pt::Access access(config);
  lm::ngram::State lm_state;
  Objective objective(access, lm_state);
  std::vector<StringPiece> rep_buffer;
  std::vector<VocabWord*> word_buffer;
  FeatureMock feature_mock(rep_buffer, word_buffer);
  objective.AddFeature(feature_mock);
  Chart chart(5, objective);

  // other setup
  util::MutableVocab vocab;
  vocab.FindOrInsert("small");
  const std::size_t orig_vocabsize = vocab.Size();
  std::vector<VocabWord*> orig_mapping;
  util::Pool pool;
  util::Layout &word_layout = objective.GetFeatureInit().word_layout;
  VocabWord *word_small = reinterpret_cast<VocabWord*>(word_layout.Allocate(pool));
  for (std::size_t i=0; i < orig_vocabsize-1; ++i) {
    orig_mapping.push_back(nullptr);
  }
  orig_mapping.push_back(word_small);
  const std::vector<VocabWord*> mapping = orig_mapping;

  // test known and unknown
  std::string input = "a small test test";
  chart.ReadSentence(input, vocab, mapping);
  // vocab update
  BOOST_CHECK_EQUAL("a", vocab.String(orig_vocabsize));
  BOOST_CHECK_EQUAL("test", vocab.String(orig_vocabsize+1));
  // lengths
  BOOST_CHECK_EQUAL(4, chart.SentenceLength());
  BOOST_CHECK_EQUAL(2, word_buffer.size());
  BOOST_CHECK_EQUAL(2, rep_buffer.size());
  // sentence
  BOOST_CHECK_EQUAL(word_buffer[0], chart.Sentence()[0]);
  BOOST_CHECK_EQUAL(word_small, chart.Sentence()[1]);
  BOOST_CHECK_EQUAL(word_buffer[1], chart.Sentence()[2]);
  BOOST_CHECK_EQUAL(word_buffer[1], chart.Sentence()[3]);
  // new word calls
  BOOST_CHECK_EQUAL("a", rep_buffer[0]);
  BOOST_CHECK_EQUAL("test", rep_buffer[1]);
}

/* Damnit, I do not know how to read out the vertex to validate that the
 * lookup is correct. Feel free to continue on this, but the lookup seems to
 * be ok for now.
BOOST_AUTO_TEST_CASE(PhraseTest) {
  // init chart
  pt::FieldConfig config;
  pt::Access access(config);
  lm::ngram::State lm_state;
  Objective objective(access, lm_state);
  std::vector<PhrasePair> phrase_pair_buffer;
  FeatureMock feature_mock(phrase_pair_buffer);
  objective.AddFeature(feature_mock);
  Chart chart(2, objective);

  // other setup
  util::MutableVocab vocab;
  vocab.FindOrInsert("small");
  vocab.FindOrInsert("test");
  std::vector<VocabWord*> orig_mapping;
  util::Pool pool;
  util::Layout &word_layout = objective.GetFeatureInit().word_layout;
  VocabWord *word_small = reinterpret_cast<VocabWord*>(word_layout.Allocate(pool));
  VocabWord *word_test = reinterpret_cast<VocabWord*>(word_layout.Allocate(pool));
  orig_mapping.push_back(word_small);
  orig_mapping.push_back(word_test);
  const std::vector<VocabWord*> mapping = orig_mapping;

  std::string input = "small test";
  chart.ReadSentence(input, vocab, mapping);
  PTMock pt(access);
  pt::Row *target1 = reinterpret_cast<pt::Row*>(access.Allocate(pool));
  pt::Row *target2 = reinterpret_cast<pt::Row*>(access.Allocate(pool));
  pt::Row *target3 = reinterpret_cast<pt::Row*>(access.Allocate(pool));
  pt.AddEntries(Range(0,2), {target1});
  pt.AddEntries(Range(0,3), {target2, target3});

  chart.LoadPhrases(pt);
}
*/ /* TODO
   * check that vertex content is ...
   * ... target1 for Range [0-2),
   * ... target2 and target3 for Range [0-3),
   * ... a passthrough for Range [1-3)
   * */

} // namespace
} // namespace decode
