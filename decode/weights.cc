#include "decode/phrase_table.hh"

#include "decode/scorer.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/exception.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include "util/string_piece_hash.hh"

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
	TokenIter<SingleCharacter> tok(line, ' ');
	
	UTIL_THROW_IF(!tok, Exception, "No tokens found in weight file line.");
	StringPiece name = *tok;
	tok++;
	  
	UTIL_THROW_IF(!tok, Exception, "Only one token found in weight file line: " << line);
	int length;
	float value = kConverter.StringToFloat(tok->data(), tok->size(), &length);
	UTIL_THROW_IF(isnan(value), util::Exception, "Bad score " << *tok);
	tok++;
	  
	UTIL_THROW_IF(tok, Exception, "More than two tokens found in weight file line: " << line);
	weights_map_[name.as_string()] = value;	  

      } } catch (const util::EndOfFileException &e) {}
  }
      
  void Weights::SetWeight(const StringPiece name, float weight) {
    weights_map_[name.as_string()] = weight;
  }

  float Weights::GetWeight(const StringPiece name) const {
    WeightsMap::const_iterator it = FindStringPiece(weights_map_, name);
    UTIL_THROW_IF(it == weights_map_.end(), Exception, "Weight not found for name: " << name);
    return it->second;
  }
}
