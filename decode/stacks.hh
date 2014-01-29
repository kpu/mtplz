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
    void Populate(const Context &context, const Chart &chart, const Hypothesis &anterior, std::size_t phrase_length, search::EdgeGenerator &out);
    std::vector<Stack> stacks_;
};

} // namespace decode

#endif // DECODE_STACKS__
