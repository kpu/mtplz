#ifndef DECODE_OUTPUT__
#define DECODE_OUTPUT__

#include <string>

namespace util { class MutableVocab; }

namespace decode {

class Hypothesis;

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab, std::string &to);

} // namespace decode

#endif // DECODE_OUTPUT__
