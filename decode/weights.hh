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
    void SetWeight(const StringPiece name, float weight);
    float GetWeight(const StringPiece name) const;

  private:
    typedef boost::unordered_map<std::string, float> WeightsMap;
    WeightsMap weights_map_;
  };

} // namespace decode

#endif // DECODE_WEIGHTS__
