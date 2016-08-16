#pragma once

#include <stdint.h>

// TODO replace with decode-local type
namespace pt { struct Row; }

namespace decode {

enum class TargetPhraseType : uint8_t {
  Table, Passthrough, Begin, EOS
};

} // namespace decode
