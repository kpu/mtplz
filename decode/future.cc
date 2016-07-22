#include "decode/future.hh"

#include "decode/chart.hh"

#include <cstddef>

#include <math.h>

namespace decode {

Future::Future(const Chart &chart)
  : sentence_length_plus_1_(chart.SentenceLength() + 1),
    entries_(sentence_length_plus_1_ * sentence_length_plus_1_, -INFINITY) {

  for (std::size_t begin = 0; begin <= chart.SentenceLength(); ++begin) {
    // Nothing is nothing (this is a useful concept when two phrases abut)
    Entry(begin, begin) = 0.0;
    // Insert phrases
    std::size_t max_end = std::min(begin + chart.MaxSourcePhraseLength(), chart.SentenceLength());
    for (std::size_t end = begin + 1; end <= max_end; ++end) {
      const TargetPhrases *phrases = chart.Range(begin, end);
      if (phrases) {
        Entry(begin, end) = phrases->Bound();
      }
    }
  }

  // All the phrases are in, now do minimum dynamic programming.  Lengths 0 and 1 were already handled above.
  for (std::size_t length = 2; length <= chart.SentenceLength(); ++length) {
    for (std::size_t begin = 0; begin <= chart.SentenceLength() - length; ++begin) {
      float &entry = Entry(begin, begin + length);
      for (std::size_t division = begin + 1; division < begin + length; ++division) {
        entry = std::max(entry, Entry(begin, division) + Entry(division, begin + length));
      }
    }
  }
}

} // namespace decode
