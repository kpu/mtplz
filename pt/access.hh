#pragma once

#include "pt/types.hh"
#include "util/layout.hh"

#include <boost/optional.hpp> // Or C++17

#include <limits>

namespace util { class scoped_memory; }

/* Accessors to interpret a target phrase pointer */
namespace pt {

class FieldConfig {
  public:
    void Save(util::scoped_memory &mem);
    void Restore(const util::scoped_memory &mem);

    static constexpr std::size_t kNotPresent = std::numeric_limits<std::size_t>::max();

    bool target = true;
    std::size_t dense_features = kNotPresent;
    bool sparse_features = false;
    std::size_t lexical_reordering = kNotPresent;

    static bool Present(bool value) { return value; }
    static bool Present(std::size_t value) { return value != kNotPresent; }
};

template <class Field> class OptionalField {
  public:
    OptionalField(util::Layout &layout, bool present) {
      if (present) {
        field_ = Field(layout);
      }
    }

    OptionalField(util::Layout &layout, std::size_t size) {
      if (size != FieldConfig::kNotPresent) {
        field_ = Field(layout, size);
      }
    }

    auto operator()(const Row *phr) const -> typename std::result_of<Field(const void*)>::type {
      return (*field_)(phr);
    }
    template <class Alloc> auto operator()(Row *&phr, Alloc &alloc) const -> typename std::result_of<Field(void*&, Alloc &)>::type {
      return (*field_)(*reinterpret_cast<void **>(&phr), alloc);
    }
    auto operator()(Row *phr) const -> typename std::result_of<Field(void*)>::type {
      return (*field_)(phr);
    }
    operator bool() const { return bool(field_); }

  private: 
    boost::optional<Field> field_;
};

// Usage:
// To check if lexicalized reordering is in the phrase table:
//   if (access.lexicalized_reordering)
// Once you're sure it's there, access:
//   access.lexicalized_reordering(phrase);
class Access {
  private:
    util::Layout layout_;

  public:
    explicit Access(const FieldConfig &config) :
      target(layout_, config.target),
      dense_features(layout_, config.dense_features),
      sparse_features(layout_, config.sparse_features),
      lexical_reordering(layout_, config.lexical_reordering) {}

    OptionalField<util::VectorField<WordIndex, VectorSize> > target;
    OptionalField<util::ArrayField<float> > dense_features;
    OptionalField<util::VectorField<SparseFeature, VectorSize> > sparse_features;
    OptionalField<util::ArrayField<float> > lexical_reordering;
    // TODO word alignment, properties?

    // Get the pointer to the next phrase.
    const Row *End(const Row *phrase) const {
      // TODO optimize.
      // TODO this assumes there's at least one VectorField.
      const uint8_t *base = reinterpret_cast<const uint8_t*>(phrase);
      const uint8_t *end = base + layout_.OffsetsEnd() + 
        *(reinterpret_cast<const VectorSize*>(base + layout_.OffsetsEnd()) - 1);
      return reinterpret_cast<const Row*>(end);
    }

    template <class Allocator> Row *Allocate(Allocator &pool) {
      return static_cast<Row*>(layout_.Allocate(pool));
    }
};

} // namespace pt
