#pragma once

#include <vector>
#include <utility> // for std::pair
#include <assert.h>

namespace decode {

struct VocabWord;

typedef std::pair<std::size_t,std::size_t> SourceSpan;

class SourcePhrase {
  public:
    SourcePhrase(const std::vector<VocabWord*> &base, std::size_t begin, std::size_t end)
      : base_(base), span_(begin, end) {
      assert(begin <= end);
      assert(end <= base.size());
    }

    std::vector<VocabWord*>::const_iterator begin() const {
      return base_.cbegin() + span_.first;
    }

    std::vector<VocabWord*>::const_iterator end() const {
      return base_.cbegin() + span_.second;
    }

    const SourceSpan Span() const {
      return span_;
    }

    const std::size_t Length() const {
      return span_.second - span_.first;
    }

  private:
    const std::vector<VocabWord*> &base_;
    const SourceSpan span_;
};

} // namespace decode
