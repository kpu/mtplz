#pragma once

#include "decode/objective.hh"
#include "decode/context.hh"

namespace decode {

class System {
  public:
    System(Config config) : objective_(), config_(config) {}

    const Config &GetConfig() { return config_; }
    Objective &GetObjective() { return objective_; }
  private:
    Objective objective_;
    Config config_;
};
  
} // namespace decode
