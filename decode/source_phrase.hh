#pragma once

#include "decode/id.hh"

#include <utility> // for std::pair

namespace decode {

typedef std::pair<std::size_t,std::size_t> SourceSpan;

class SourcePhrase {
  public:
    SourcePhrase(const std::vector<ID> &base, std::size_t begin, std::size_t end)
      : base_(base), span_(begin, end) {}

    std::vector<ID>::const_iterator Begin() const {
      return base_.cbegin() + span_.first;
    }

    std::vector<ID>::const_iterator End() const {
      return base_.cbegin() + span_.second;
    }

    const SourceSpan Span() const {
      return span_;
    }

    const std::size_t Length() const {
      return span_.second - span_.first;
    }

  private:
    const std::vector<ID> &base_;
    const SourceSpan span_;
};

} // namespace decode
