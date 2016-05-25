#include "pt/access.hh"
#include "util/exception.hh"
#include "util/mmap.hh"

namespace pt {

namespace {
enum FieldLabel {
  Target = 0,
  DenseFeatures = 1,
  SparseFeatures = 2,
  LexicalReordering = 3,
  // Leave this last.
  LastLabel = 4,
};

void Append(FieldLabel label, std::size_t length, util::scoped_memory &mem) {
  if (length != FieldConfig::kNotPresent) {
    HugeRealloc(mem.size() + sizeof(uint32_t) + sizeof(uint64_t), false, mem);
    *reinterpret_cast<uint32_t*>(mem.end() - sizeof(uint32_t) - sizeof(uint64_t)) = label;
    *reinterpret_cast<uint64_t*>(mem.end() - sizeof(uint64_t)) = length;
  }
}

void Append(FieldLabel label, bool include, util::scoped_memory &mem) {
  if (include) {
    HugeRealloc(mem.size() + sizeof(uint32_t), false, mem);
    *reinterpret_cast<uint32_t*>(mem.end() - sizeof(uint32_t)) = label;
  }
}

bool ConsumeLabel(FieldLabel label, const char *&ptr, const char *end) {
  if (ptr == end) return false;
  uint32_t value = *reinterpret_cast<const uint32_t*>(ptr);
  UTIL_THROW_IF2(value >= LastLabel, "This code doesn't know what to do with field labeled " << value << ". Was the phrase table built with a newer version?");
  UTIL_THROW_IF2(value < label, "Field labels not in order with " << value);
  ptr += sizeof(uint32_t);
  return value == label;
}

void Consume(FieldLabel label, const char *&ptr, const char *end, std::size_t &out) {
  if (ConsumeLabel(label, ptr, end)) {
    out = *reinterpret_cast<const uint64_t*>(ptr);
    ptr += sizeof(uint64_t);
  } else {
    out = FieldConfig::kNotPresent;
  }
}

void Consume(FieldLabel label, const char *&ptr, const char *end, bool &out) {
  out = ConsumeLabel(label, ptr, end);
}

} // namespace

void FieldConfig::Save(util::scoped_memory &mem) {
  Append(Target, target, mem);
  Append(DenseFeatures, dense_features, mem);
  Append(SparseFeatures, sparse_features, mem);
  Append(LexicalReordering, lexical_reordering, mem);
}

void FieldConfig::Restore(const util::scoped_memory &mem) {
  const char *ptr = mem.begin();
  Consume(Target, ptr, mem.end(), target);
  Consume(DenseFeatures, ptr, mem.end(), dense_features);
  Consume(SparseFeatures, ptr, mem.end(), sparse_features);
  Consume(LexicalReordering, ptr, mem.end(), lexical_reordering);
}

} // namespace pt
