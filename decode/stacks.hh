#ifndef DECODE_STACKS__
#define DECODE_STACKS__

#include <vector>

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
    std::vector<Stack> stacks_;
};

} // namespace decode

#endif // DECODE_STACKS__
