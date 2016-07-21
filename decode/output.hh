#ifndef DECODE_OUTPUT__
#define DECODE_OUTPUT__

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

namespace util {
class MutableVocab;
class FileStream;
}

namespace decode {

class Hypothesis;
struct FeatureInit;

struct ScoreHistory {
	std::vector<float> scores;
	float total;
};

typedef boost::unordered_map<std::string, ScoreHistory> ScoreHistoryMap;

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab,
    util::FileStream &out, const FeatureInit &feature_init);
void OutputVerbose(const Hypothesis &hypo, const util::MutableVocab &vocab,
    ScoreHistoryMap &map, util::FileStream &out, const FeatureInit &feature_init);

} // namespace decode

#endif // DECODE_OUTPUT__
