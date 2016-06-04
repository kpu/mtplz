#pragma once

#include "decode/objective.hh"
#include "decode/context.hh"
#include "decode/weights.hh"

namespace decode {

class System {
  public:
    System(const Config config, const StringPiece &weights_file) : objective_(), config_(config) {
      weights_.ReadFromFile(weights_file);
    }

    void LoadWeights() {
      objective_.LoadWeights(weights_);
    }

    const Config &GetConfig() const { return config_; }
    Objective &GetObjective() { return objective_; }
  private:
    Objective objective_;
    const Config config_;
    Weights weights_;
};
  
} // namespace decode
