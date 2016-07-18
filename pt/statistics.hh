#pragma once

namespace pt {

struct Statistics {
  uint64_t max_source_phrase_length;
  // Total size of the vocabulary (source and target words share the same space of ids)
  uint64_t vocab_size;
};

} // namespace pt
