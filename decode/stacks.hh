#ifndef DECODE_STACKS__
#define DECODE_STACKS__

#include <vector>
#include "util/pool.hh"


namespace search { class EdgeGenerator; }

namespace decode {

class Hypothesis;
class Context;
class Chart;

typedef std::vector<Hypothesis> Stack;

class Stacks {
  public:
    Stacks(Context &context, Chart &chart);

  private:
    void PopulateLastStack(Context &context, Chart &chart);
    std::vector<Stack> stacks_;

    // this is needed to provide backing for an end-of-sentence phrase
    // TODO: consider refactoring
    util::Pool eos_phrase_pool_;  
};

} // namespace decode

#endif // DECODE_STACKS__
