#pragma once

#include "util/layout.hh"

namespace decode {

class FeatureInit {
  public:
    util::Layout &getHypothesisLayout();
    util::Layout &getTargetPhraseLayout();
    util::Layout &getWordLayout();

  private:
    util::Layout hypothesis_ = util::Layout();
    util::Layout target_phrase_ = util::Layout();
    util::Layout word_ = util::Layout();
};

} // namespace decode
