#ifndef DECODE_STACKS__
#define DECODE_STACKS__
#include "decode/hypothesis.hh"
#include "util/pool.hh"

#include <vector>

namespace search { class EdgeGenerator; }

namespace decode {

class Context;
class Chart;

typedef std::vector<Hypothesis*> Stack;

class Stacks {
  public:
    Stacks(Context &context, Chart &chart);

    // NULL if no hypothesis.
    const Hypothesis *End() const { return end_; }

  private:
    void PopulateLastStack(Context &context, Chart &chart);
    std::vector<Stack> stacks_;

    // this is needed to provide backing for an end-of-sentence phrase
    // TODO: consider refactoring
    util::Pool eos_phrase_pool_;

    util::Pool hypothesis_pool_;

    const Hypothesis *end_;
};

} // namespace decode

#endif // DECODE_STACKS__
