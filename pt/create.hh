#pragma once

#include <cstddef>

namespace pt {

class FieldConfig;

// Column indices for various fields in text format.
struct TextColumns {
  std::size_t source = 0, target = 1;
  std::size_t dense_features = 2;
  std::size_t sparse_features = 3;
  std::size_t lexical_reordering = 4;
};

// Takes ownership of from and to files.
void CreateTable(int from, int to, const TextColumns columns, FieldConfig &config);

} // namespace pt
