#ifndef DECODE_OUTPUT__
#define DECODE_OUTPUT__

namespace util {
class MutableVocab;
class FakeOFStream;
}

namespace decode {

class Hypothesis;

void Output(const Hypothesis &hypo, const util::MutableVocab &vocab, util::FakeOFStream &out);

} // namespace decode

#endif // DECODE_OUTPUT__
