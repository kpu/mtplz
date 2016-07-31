#ifndef DECODE_OUTPUT__
#define DECODE_OUTPUT__

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

namespace util {
class FileStream;
}

namespace decode {

class Hypothesis;
class VocabMap;
struct FeatureInit;

struct ScoreHistory {
	std::vector<float> scores;
	float total;
};

typedef boost::unordered_map<std::string, ScoreHistory> ScoreHistoryMap;

void Output(const Hypothesis &hypo, const VocabMap &vocab,
    ScoreHistoryMap &map, util::FileStream &out,
    const FeatureInit &feature_init, bool verbose);

} // namespace decode

#endif // DECODE_OUTPUT__
