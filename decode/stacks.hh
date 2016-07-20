#pragma once

#include "decode/system.hh"
#include "decode/hypothesis_builder.hh"

#include <vector>

namespace search { class EdgeGenerator; }

namespace decode {

class Chart;

typedef std::vector<Hypothesis*> Stack;

class Stacks {
  public:
    Stacks(System &system, Chart &chart);

    // NULL if no hypothesis.
    const Hypothesis *End() const { return end_; }

  private:
    void PopulateLastStack(System &system, Chart &chart);
    std::vector<Stack> stacks_;

    util::Pool hypothesis_pool_;

    HypothesisBuilder hypothesis_builder_;

    const Hypothesis *end_;
};

} // namespace decode
