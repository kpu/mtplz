#pragma once

#include "decode/objective.hh"
#include "decode/context.hh"

namespace decode {

class System {
  public:
    System(Config config) : feature_init_(), config_(config) {
      objective_ = Objective(feature_init_);
    }

    const Config &Config() { return config_; }
    Objective &Objective() { return objective_; }
  private:
    Objective objective_;
    FeatureInit feature_init_;
    Config config_;
};
  
} // namespace decode
