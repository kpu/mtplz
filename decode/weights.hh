#ifndef DECODE_WEIGHTS__
#define DECODE_WEIGHTS__

#include "lm/model.hh"
#include "decode/id.hh"
#include "util/string_piece.hh"
#include "util/string_piece_hash.hh"

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace decode {
  
class Weights {
public:
	 Weights() {
	 }
	 
	 void ReadFromFile(const StringPiece file_name);
	 
	 const std::vector<float>& PhraseTableWeights() const { return phrase_table_weights_; }
	 float LMWeight() const { return lm_weight_; }
	 float DistortionWeight() const { return distortion_weight_; }
	 float TargetWordInsertionWeight() const { return target_word_insertion_weight_; }
	 
private:

	 // MRK: TODO: this class is a bit confused right now.
	 // Perhaps we can eliminate the map structure altogether, and 
	 // allow everything to be hard coded.
	 // Another possibility would be to use hardcoded names for weights while allowing a "sentinel" value
	 // e.g. NULL.
	 void PopulateWeights();
	 std::vector<float> GetWeights(const StringPiece name) const;
	 float GetSingleWeight(const StringPiece name) const;

	 typedef boost::unordered_map<std::string, std::vector<float> > WeightsMap;
	 WeightsMap weights_map_;

	 std::vector<float> phrase_table_weights_;
	 float lm_weight_;
	 float distortion_weight_;
	 float target_word_insertion_weight_;
};

} // namespace decode

#endif // DECODE_WEIGHTS__
