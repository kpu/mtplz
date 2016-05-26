#pragma once

#include "util/layout.hh"

namespace decode {

class FeatureInit {
  public:
    util::Layout &GetHypothesisLayout();
    util::Layout &GetTargetPhraseLayout();
    util::Layout &GetWordLayout();

  private:
    util::Layout hypothesis_ = util::Layout();
    util::Layout target_phrase_ = util::Layout();
    util::Layout word_ = util::Layout();
};

} // namespace decode
