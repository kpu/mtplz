#ifndef SEARCH_TYPES__
#define SEARCH_TYPES__

#include <stdint.h>

namespace lm { namespace ngram { class ChartState; } }

namespace search {

typedef float Score;

typedef uint32_t Arity;

// std::pair has a constructor, so it can't be in a union.
struct IntPair {
  uint32_t first, second;
  bool operator==(const IntPair &other) const {
    return (first == other.first) && (second == other.second);
  }
};

union Note {
  void *vp;
  void *cvp;
  IntPair ints;
};

struct NBestComplete {
  NBestComplete(Note in_history, const lm::ngram::ChartState &in_state, Score in_score) 
    : history(in_history), state(&in_state), score(in_score) {}

  Note history;
  const lm::ngram::ChartState *state;
  Score score;
};

} // namespace search

#endif // SEARCH_TYPES__
