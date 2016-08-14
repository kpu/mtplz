#pragma once

#include <stdint.h>

namespace decode {

enum class TargetPhraseType : uint8_t {
  Table, Passthrough, EOS
};

} // namespace decode
