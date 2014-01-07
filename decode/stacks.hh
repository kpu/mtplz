#ifndef DECODE_STACKS__
#define DECODE_STACKS__

#include "util/pool.hh"

#include <boost/pool/object_pool.hpp>

#include <vector>

namespace search { class EdgeGenerator; }

namespace decode {

class Hypothesis;
class Context;
class Chart;

typedef std::vector<const Hypothesis *> Stack;

class Stacks {
  public:
    Stacks(const Context &context, const Chart &chart);

  private:
    void Populate(const Context &context, const Chart &chart, const Hypothesis &anterior, std::size_t phrase_length, search::EdgeGenerator &out);
    boost::object_pool<Hypothesis> pool_;
    std::vector<Stack> stacks_;
};

} // namespace decode

#endif // DECODE_STACKS__
