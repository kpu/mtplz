#pragma once

namespace pt {

class FieldConfig;

// Takes ownership of from and to files.
void CreateTable(int from, int to, FieldConfig &config);

} // namespace pt
