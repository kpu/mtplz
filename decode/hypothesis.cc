#include "decode/hypothesis.hh"

namespace decode {

std::ostream &operator<<(std::ostream &stream, const Coverage &coverage) {
  for (std::size_t i = 0; i < coverage.FirstZero(); ++i) {
    stream << '1';
  }
  for (std::size_t i = 0; i < 64; ++i) {
    stream << ((coverage.bits_ >> i) & 1);
  }
  return stream;
}

} // namespace decode
