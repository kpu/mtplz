#include "decode/phrase_table.hh"

#include "decode/scorer.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include "util/string_piece_hash.hh"

#include <cmath>
#include <math.h>

using namespace util;

namespace decode {

namespace {
static const double_conversion::StringToDoubleConverter kConverter(
    double_conversion::StringToDoubleConverter::NO_FLAGS,
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::quiet_NaN(),
    "inf",
    "NaN");

} // namespace

void Weights::ReadFromFile(const StringPiece file_name) {
	FilePiece in(file_name.as_string().c_str(), &std::cerr);
	StringPiece line;
  
	try { while (true) {
			line = in.ReadLine();
			util::TokenIter<util::SingleCharacter, true> token(line, ' ');

			// Get weight name
			UTIL_THROW_IF(!token, Exception, "No tokens found in weight file line.");
			StringPiece name = *token;
			token++;

			std::vector<float> weights;
			// Get weight vector			
			for (; token; ++token) {
				int length;
				weights.push_back(kConverter.StringToFloat(token->data(), token->size(), &length));
        {
          using namespace std;
				  UTIL_THROW_IF(isnan(weights.back()), util::Exception, "Bad feature weight " << *token);
        }
			}
			UTIL_THROW_IF(weights.empty(), util::Exception, "No weights found for weight type: " << name);
			weights_map_[name.as_string()] = weights;
			
		} } catch (const util::EndOfFileException &e) {}

	PopulateWeights();
}

void Weights::PopulateWeights() {
	phrase_table_weights_ = GetWeights("phrase_table");
	lm_weight_ = GetSingleWeight("lm");
	distortion_weight_ = GetSingleWeight("distortion");
	target_word_insertion_weight_ = GetSingleWeight("target_word_insertion");
}

std::vector<float> Weights::GetWeights(const StringPiece name) const {
	WeightsMap::const_iterator it = FindStringPiece(weights_map_, name);
	UTIL_THROW_IF(it == weights_map_.end(), Exception, "Weights not found for name: " << name);
	UTIL_THROW_IF((it->second).size() == 0, Exception, "Found empty weight vector for name: " << name);
	return it->second;
}


float Weights::GetSingleWeight(const StringPiece name) const {
	WeightsMap::const_iterator it = FindStringPiece(weights_map_, name);
	UTIL_THROW_IF(it == weights_map_.end(), Exception, "Weight not found for name: " << name);
	UTIL_THROW_IF((it->second).size() != 1, Exception, "Expected single weight but found " << (it->second).size() << " for name: " << name);
	return it->second.at(0);
}
}
